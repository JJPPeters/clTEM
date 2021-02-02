//
// Created by Jon on 31/01/2020.
//

#include "simulationstem.h"
#include "utilities/vectorutils.h"

template <class T>
void SimulationStem<T>::initialiseBuffers() {

    auto sm = job->simManager;
    unsigned int rs = sm->resolution();

    if (rs*rs / 256 != clReductionBuffer.GetSize()) {
        clReductionBuffer = clMemory<T, Manual>(ctx, rs*rs / 256); // STEM only
    }
}

template <>
void SimulationStem<float>::initialiseKernels() {

    if (do_initialise_stem) {
        SumReduction = Kernels::sum_reduction_f.BuildToKernel(ctx);
        BandPassAbs = Kernels::band_pass_f.BuildToKernel(ctx);
    }

    do_initialise_stem = false;
}

template <>
void SimulationStem<double>::initialiseKernels() {

    if (do_initialise_stem) {
        SumReduction = Kernels::sum_reduction_d.BuildToKernel(ctx);
        BandPassAbs = Kernels::band_pass_d.BuildToKernel(ctx);
    }

    do_initialise_stem = false;
}

template <class T>
double SimulationStem<T>::doSumReduction(clMemory<T, Manual> data, clWorkGroup globalSizeSum,
                                           clWorkGroup localSizeSum, unsigned int nGroups, int totalSize)
{
    CLOG(DEBUG, "sim") << "Starting sum reduction";
    CLOG(DEBUG, "sim") << "Create local buffer";
//    clReductionBuffer = clMemory<float, Manual>(ctx, nGroups);

    CLOG(DEBUG, "sim") << "Doing sum reduction";
    SumReduction.SetArg(0, data, ArgumentType::Input);

    // Only really need to do these 3 once... (but we make a local 'outArray' so can't do that)
    SumReduction.SetArg(1, clReductionBuffer);
    SumReduction.SetArg(2, totalSize);
    SumReduction.SetLocalMemoryArg<T>(3, 256);

    SumReduction.run(globalSizeSum, localSizeSum);

    ctx->WaitForQueueFinish();

    // Now copy back
    CLOG(DEBUG, "sim") << "Copy from buffer";
    std::vector<T> sums = clReductionBuffer.CreateLocalCopy();

    CLOG(DEBUG, "sim") << "Doing final sum on CPU (" << nGroups << " parts)";
    // Find out which numbers to read back
    double sum = 0;
    for (int i = 0; i < nGroups; i++)
        sum += sums[i];
    return sum;
}

template <class T>
double SimulationStem<T>::getStemPixel(double inner, double outer, double xc, double yc, int parallel_ind, double d_kx, double d_ky)
{
    CLOG(DEBUG, "sim") << "Getting STEM pixel";
    unsigned int resolution = job->simManager->resolution();
    double angle_scale = job->simManager->inverseScaleAngle();

    clWorkGroup WorkSize(resolution, resolution, 1);

    CLOG(DEBUG, "sim") << "FFT shifting diffraction pattern";
    FftShift.SetArg(0, clWaveFunctionRecip[parallel_ind], ArgumentType::Input);
    FftShift.run(WorkSize);

    int output_type = 4; // square abs

    CLOG(DEBUG, "sim") << "Getting abs of diffraction pattern";

    if (d_kx != 0.0 || d_ky != 0.0) {
        ComplexToReal.SetArg(0, clWaveFunctionTemp_1, ArgumentType::Input);
        ComplexToReal.SetArg(1, clWaveFunctionTemp_2, ArgumentType::Output);
        ComplexToReal.SetArg(2, output_type); // should be 4
        ComplexToReal.run(WorkSize);

        translateDiffImage(d_kx, d_ky);

    } else {
        ComplexToReal.SetArg(0, clWaveFunctionTemp_1, ArgumentType::Input);
        ComplexToReal.SetArg(1, clWaveFunctionTemp_3, ArgumentType::Output);
        ComplexToReal.SetArg(2, output_type); // should be 4
        ComplexToReal.run(WorkSize);
    }

    double innerPx = inner / angle_scale;
    double outerPx = outer / angle_scale;

    double xcPx = xc / angle_scale;
    double ycPx = yc / angle_scale;

    CLOG(DEBUG, "sim") << "Masking diffraction pattern";
    BandPassAbs.SetArg(0, clWaveFunctionTemp_3, ArgumentType::Input);
    BandPassAbs.SetArg(1, clWaveFunctionTemp_2, ArgumentType::Output);
    BandPassAbs.SetArg(2, resolution);
    BandPassAbs.SetArg(3, resolution);
    BandPassAbs.SetArg(4, static_cast<T>(innerPx));
    BandPassAbs.SetArg(5, static_cast<T>(outerPx));
    BandPassAbs.SetArg(6, static_cast<T>(xcPx));
    BandPassAbs.SetArg(7, static_cast<T>(ycPx));

    BandPassAbs.run(WorkSize);

    ctx->WaitForQueueFinish();

    unsigned int totalSize = resolution * resolution;
    unsigned int nGroups = totalSize / 256;

    clWorkGroup globalSizeSum(totalSize, 1, 1);
    clWorkGroup localSizeSum(256, 1, 1);

    return doSumReduction(clWaveFunctionTemp_2, globalSizeSum, localSizeSum, nGroups, totalSize);
}

template<class GPU_Type>
bool SimulationStem<GPU_Type>::initialiseSimulation() {
    initialiseBuffers();
    initialiseKernels();

    return SimulationCbed<GPU_Type>::initialiseSimulation();
}

template<class GPU_Type>
void SimulationStem<GPU_Type>::simulate() {
    if(!initialiseSimulation())
        return;

    CLOG(DEBUG, "sim") << "Parallel pixels: " << job->pixels.size();

    typedef std::map<std::string, Image<double>> return_map;
    return_map Images;

    // now need to work out where our probes need to be made
    auto stemPixels = job->simManager->stemArea();
    unsigned int numberOfSlices = job->simManager->simulationCell()->sliceCount();

    double start_x = stemPixels->getRawLimitsX()[0];
    double start_y = stemPixels->getRawLimitsY()[0];

    int num_x = stemPixels->getPixelsX();

    double step_x = stemPixels->getScaleX();
    double step_y = stemPixels->getScaleY();

    unsigned int px_x = job->simManager->stemArea()->getPixelsX();
    unsigned int px_y = job->simManager->stemArea()->getPixelsY();

    unsigned int slice_step = job->simManager->intermediateSliceStep();
    unsigned int output_count = 1;
    if (slice_step > 0)
        output_count = std::ceil((float) numberOfSlices / slice_step);

    // initialise our images
    for (const auto &det : job->simManager->stemDetectors()) {
        Images[det.name] = Image<double>(px_x, px_y, output_count);
    }

    CLOG(DEBUG, "sim") << "Initialising probe(s)";
    for (int i = 0; i < job->pixels.size(); ++i) {
        int p = job->pixels[i];
        // index to x and y index
        int x_i = p % num_x;
        int y_i = (int) std::floor(p / num_x);

        double x_pos = start_x + x_i * step_x;
        double y_pos = start_y + y_i * step_y;

        initialiseProbeWave(x_pos, y_pos, i);
    }

    //
    // plasmon setup
    //
    std::shared_ptr<PlasmonScattering> plasmon = job->simManager->incoherenceEffects()->plasmons();
    bool do_plasmons = plasmon->enabled();
    double slice_dz = job->simManager->simulationCell()->sliceThickness();
    int padding_slices = (int) job->simManager->simulationCell()->preSliceCount();
    unsigned int scattering_count = 0;
    double next_scattering_depth = plasmon->getGeneratedDepth(job->id, scattering_count);

    // this gives us our current wavevector, but also our axis for azimuth rotation
    auto mp = job->simManager->microscopeParams();
    double k_v = mp->Wavenumber();
    auto orig_k = mp->Wavevector();
    Eigen::Vector3d k_vec(0.0, 0.0, k_v);
    Eigen::Vector3d y_axis(0.0, 1.0, 0.0);

    // apply any current rotation to the y_axis
    double current_tilt = mp->BeamTilt;
    double current_azimuth = mp->BeamAzimuth;
    Utils::rotateVectorSpherical(k_vec, y_axis, current_tilt/1000.0, current_azimuth);

    CLOG(DEBUG, "sim") << "Starting multislice loop";
    // loop through slices
    unsigned int output_counter = 0;
    for (int i = 0; i < numberOfSlices; ++i) {
        doMultiSliceStep(i);

        // remove padding slices and add one (as we are at the 'end' of the current slice
        double current_depth = (i + 1 - padding_slices) * slice_dz;
        if (do_plasmons && current_depth >= next_scattering_depth) {
            // modify propagator/transmission function
            double p_tilt = plasmon->getScatteringPolar();
            double p_azimuth = plasmon->getScatteringAzimuth();

            Utils::rotateVectorSpherical(k_vec, y_axis, p_tilt/1000.0, p_azimuth);

            modifyBeamTilt(k_vec(0), k_vec(1), k_vec(2));

            // update parameters for next scattering event!
            scattering_count++;
            next_scattering_depth = job->simManager->incoherenceEffects()->plasmons()->getGeneratedDepth(job->id, scattering_count);
        }

        if (pool.isStopped())
            return;

        if (slice_step > 0 && (i+1) % slice_step == 0) {
            for (const auto &det : job->simManager->stemDetectors()) {
                std::vector<double> im(stemPixels->getNumPixels(), 0.0);
                std::vector<double> im_w(stemPixels->getNumPixels(), 0.0);

                for (int j = 0; j < job->pixels.size(); ++j) {
                    im[job->pixels[j]] = getStemPixel(det.inner, det.outer, det.xcentre, det.ycentre, j, k_vec(0) - orig_k[0], k_vec(1) - orig_k[1]);
                    im_w[job->pixels[j]] = 1.0;
                }

                Images[det.name].getSliceRef(output_counter) = im;
                Images[det.name].getWeightingRef() = im_w;
            }
            ++output_counter;
        }

        if (pool.isStopped())
            return;

        job->simManager->reportSliceProgress(static_cast<double>(i+1) / numberOfSlices);
    }

    CLOG(DEBUG, "sim") << "Getting return images";


    if (output_counter < output_count) {
        for (const auto &det : job->simManager->stemDetectors()) {
            std::vector<double> im(stemPixels->getNumPixels(), 0.0);
            std::vector<double> im_w(stemPixels->getNumPixels(), 0.0);

            for (int i = 0; i < job->pixels.size(); ++i) {
                im[job->pixels[i]] = getStemPixel(det.inner, det.outer, det.xcentre, det.ycentre, i, k_vec(0) - orig_k[0], k_vec(1) - orig_k[1]);
                im_w[job->pixels[i]] = 1.0;
            }

            Images[det.name].getSliceRef(output_counter) = im;
            Images[det.name].getWeightingRef() = im_w;
        }
    }

    job->simManager->updateImages(Images, 1, job->simManager->liveStemEnabled());
}

template class SimulationStem<float>;
template class SimulationStem<double>;
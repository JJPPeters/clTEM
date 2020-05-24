//
// Created by Jon on 31/01/2020.
//

#include "simulationcbed.h"
#include "utilities/vectorutils.h"

template <class T>
void SimulationCbed<T>::initialiseBuffers() {
    SimulationCtem<T>::initialiseBuffers();
}

template <>
void SimulationCbed<float>::initialiseKernels() {

    if (do_initialise_cbed) {
        InitProbeWavefunction = Kernels::init_probe_wave_f.BuildToKernel(ctx);
    }

    do_initialise_cbed = false;

    SimulationGeneral<float>::initialiseKernels();
}

template <>
void SimulationCbed<double>::initialiseKernels() {

    if (do_initialise_cbed) {
        InitProbeWavefunction = Kernels::init_probe_wave_d.BuildToKernel(ctx);
    }

    do_initialise_cbed = false;

    SimulationGeneral<double>::initialiseKernels();
}

// n_parallel is the index (from 0) of the current parallel pixel
template <class T>
void SimulationCbed<T>::initialiseProbeWave(double posx, double posy, int n_parallel) {
    CLOG(DEBUG, "sim") << "Initialising probe wavefunction";
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create local variables for convenience
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    unsigned int resolution = job->simManager->resolution();
    double pixelscale = job->simManager->realScale();
    auto mParams = job->simManager->microscopeParams();
    double wavelength = mParams->Wavelength();

    clWorkGroup WorkSize(resolution, resolution, 1);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// intialise and create probe in fourier space
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: is this needed? must test on a sample we know (i.e. a single atom) or just save the probe image
    // convert from angstroms to pixel position (this bit seemed to make results 'more' sensible)
    auto current_pixel = job->getPixel();

    double start_x = job->simManager->paddedSimLimitsX(current_pixel)[0];
    double start_y = job->simManager->paddedSimLimitsY(current_pixel)[0];

    // account for the simulation area start point and convert to pixels
    posx = (posx - start_x) / pixelscale;
    posy = (posy - start_y) / pixelscale;

    // Fix inverted images
    posx = resolution - posx;
    posy = resolution - posy;

    InitProbeWavefunction.SetArg(0, clWaveFunctionRecip[n_parallel]);
    InitProbeWavefunction.SetArg(1, resolution);
    InitProbeWavefunction.SetArg(2, resolution);
    InitProbeWavefunction.SetArg(3, clXFrequencies);
    InitProbeWavefunction.SetArg(4, clYFrequencies);
    InitProbeWavefunction.SetArg(5, static_cast<T>(posx));
    InitProbeWavefunction.SetArg(6, static_cast<T>(posy));
    InitProbeWavefunction.SetArg(7, static_cast<T>(pixelscale));
    InitProbeWavefunction.SetArg(8, static_cast<T>(wavelength));
    InitProbeWavefunction.SetArg(9, static_cast<T>(mParams->C10));
    InitProbeWavefunction.SetArg(10, static_cast<std::complex<T>>(mParams->C12.getComplex()));
    InitProbeWavefunction.SetArg(11, static_cast<std::complex<T>>(mParams->C21.getComplex()));
    InitProbeWavefunction.SetArg(12, static_cast<std::complex<T>>(mParams->C23.getComplex()));
    InitProbeWavefunction.SetArg(13, static_cast<T>(mParams->C30));
    InitProbeWavefunction.SetArg(14, static_cast<std::complex<T>>(mParams->C32.getComplex()));
    InitProbeWavefunction.SetArg(15, static_cast<std::complex<T>>(mParams->C34.getComplex()));
    InitProbeWavefunction.SetArg(16, static_cast<std::complex<T>>(mParams->C41.getComplex()));
    InitProbeWavefunction.SetArg(17, static_cast<std::complex<T>>(mParams->C43.getComplex()));
    InitProbeWavefunction.SetArg(18, static_cast<std::complex<T>>(mParams->C45.getComplex()));
    InitProbeWavefunction.SetArg(19, static_cast<T>(mParams->C50));
    InitProbeWavefunction.SetArg(20, static_cast<std::complex<T>>(mParams->C52.getComplex()));
    InitProbeWavefunction.SetArg(21, static_cast<std::complex<T>>(mParams->C54.getComplex()));
    InitProbeWavefunction.SetArg(22, static_cast<std::complex<T>>(mParams->C56.getComplex()));
    InitProbeWavefunction.SetArg(23, static_cast<T>(mParams->Aperture));
    InitProbeWavefunction.SetArg(24, static_cast<T>(mParams->ApertureSmoothing));

    CLOG(DEBUG, "sim") << "Run probe wavefunction generation kernel";
    InitProbeWavefunction.run(WorkSize);

    ctx.WaitForQueueFinish();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IFFT probe to real space
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // IFFT
    CLOG(DEBUG, "sim") << "IFFT probe wavefunction";
    FourierTrans.run(clWaveFunctionRecip[n_parallel], clWaveFunctionReal[n_parallel], Direction::Inverse);
    ctx.WaitForQueueFinish();
}

template<class GPU_Type>
void SimulationCbed<GPU_Type>::initialiseSimulation() {
    initialiseBuffers();
    initialiseKernels();

    SimulationGeneral<GPU_Type>::initialiseSimulation();
}

template<class GPU_Type>
void SimulationCbed<GPU_Type>::simulate() {
    initialiseSimulation();

    typedef std::map<std::string, Image<double>> return_map;
    return_map Images;

    unsigned int numberOfSlices = job->simManager->simulationCell()->sliceCount();
    auto pos = job->simManager->cbedPosition();
    unsigned int resolution = job->simManager->resolution();

    initialiseProbeWave(pos->getXPos(), pos->getYPos());

    // TODO: set this properly
    // This will be a pre-calculated variable to set how often we pull our our slice data
    unsigned int slice_step = job->simManager->intermediateSliceStep();
    unsigned int output_count = 1;
    if (slice_step > 0)
        output_count = std::ceil((float) numberOfSlices / slice_step);

    auto diff = Image<double>(resolution, resolution, output_count);

    //
    // plasmon setup
    //
    std::shared_ptr<PlasmonScattering> plasmon = job->simManager->inelasticScattering()->plasmons();
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
            next_scattering_depth = job->simManager->inelasticScattering()->plasmons()->getGeneratedDepth(job->id, scattering_count);
        }

        if (pool.isStopped())
            return;

        if (slice_step > 0 && (i+1) % slice_step == 0) {
            diff.getSliceRef(output_counter) = getDiffractionImage(0, k_vec(0) - orig_k[0], k_vec(1) - orig_k[1]);
            output_counter++;
        }

        if (pool.isStopped())
            return;

        job->simManager->reportSliceProgress(static_cast<double>(i+1) / numberOfSlices);
    }

    if (output_counter < output_count) {
        diff.getSliceRef(output_counter) = getDiffractionImage(0, k_vec(0) - orig_k[0], k_vec(1) - orig_k[1]);
    }

    Images.insert(return_map::value_type("Diff", diff));

    job->simManager->updateImages(Images, 1); // Update this if we ever do more than one TDS in a job
}

template class SimulationCbed<float>;
template class SimulationCbed<double>;
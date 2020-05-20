//
// Created by Jon on 31/01/2020.
//

#include "simulationctem.h"
#include "utilities/vectorutils.h"

template <class T>
void SimulationCtem<T>::initialiseBuffers() {

    SimulationGeneral<T>::initialiseBuffers();

    auto sm = job->simManager;
    unsigned int rs = sm->getResolution();

    // when resolution changes (or if enabled)
    auto sim_mode = sm->getMode();
    if (sim_mode == SimulationMode::CTEM && (sim_mode != last_mode || rs*rs != clImageWaveFunction.GetSize())) {
        clImageWaveFunction = clMemory<std::complex<T>, Manual>(ctx, rs * rs);

        // TODO: I can further split these up, but they aren't a huge issue
        clTempBuffer = clMemory<std::complex<T>, Manual>(ctx, rs * rs);
        clCcdBuffer = clMemory<T, Manual>(ctx, 725);
    }
}

template <>
void SimulationCtem<float>::initialiseKernels() {
    SimulationGeneral<float>::initialiseKernels();

    if (do_initialise_ctem) {
        InitPlaneWavefunction = Kernels::init_plane_wave_f.BuildToKernel(ctx);
        ImagingKernel = Kernels::ctem_image_f.BuildToKernel(ctx);
        ABS2 = Kernels::sqabs_f.BuildToKernel(ctx);
        NtfKernel = Kernels::ccd_ntf_f.BuildToKernel(ctx);
        DqeKernel = Kernels::ccd_dqe_f.BuildToKernel(ctx);
    }

    do_initialise_ctem = false;
}

template <>
void SimulationCtem<double>::initialiseKernels() {
    SimulationGeneral<double>::initialiseKernels();

    if (do_initialise_ctem) {
        InitPlaneWavefunction = Kernels::init_plane_wave_d.BuildToKernel(ctx);
        ImagingKernel = Kernels::ctem_image_d.BuildToKernel(ctx);
        ABS2 = Kernels::sqabs_d.BuildToKernel(ctx);
        NtfKernel = Kernels::ccd_ntf_d.BuildToKernel(ctx);
        DqeKernel = Kernels::ccd_dqe_d.BuildToKernel(ctx);
    }

    do_initialise_ctem = false;
}

template <class T>
void SimulationCtem<T>::initialiseSimulation()
{
    initialiseBuffers();
    initialiseKernels();

    SimulationGeneral<T>::initialiseSimulation();

    CLOG(DEBUG, "sim") << "Starting CTEM initialisation";
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create local variables for convenience
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    unsigned int resolution = job->simManager->getResolution();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create plane wave function
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up plane wave kernel";

    clWorkGroup WorkSize(resolution, resolution, 1);
    double InitialValue = 1.0;
    InitPlaneWavefunction.SetArg(1, resolution);
    InitPlaneWavefunction.SetArg(2, resolution);
    InitPlaneWavefunction.SetArg(3, static_cast<T>(InitialValue));
    InitPlaneWavefunction.SetArg(0, clWaveFunctionReal[0], ArgumentType::Output);
    InitPlaneWavefunction.run(WorkSize);

    ctx.WaitForQueueFinish();
}

template <class T>
void SimulationCtem<T>::simulateCtemImage() {
    // Check if have a CCD set, then do that method instead
    std::string ccd = job->simManager->getCcdName();
    if (CCDParams::nameExists(ccd)) {
        std::vector<double> dqe_d = CCDParams::getDQE(ccd);
        std::vector<double> ntf_d = CCDParams::getNTF(ccd);
        // convert these to our GPU type
        std::vector<T> dqe(dqe_d.begin(), dqe_d.end());
        std::vector<T> ntf(ntf_d.begin(), ntf_d.end());
        int binning = job->simManager->getCcdBinning();
        // get dose
        double dose = job->simManager->getCcdDose(); // electrons per area
        // get electrons per pixel
        double scale = job->simManager->getRealScale();
        scale *= scale; // square it to get area of pixel
        double dose_per_pix = dose * scale;

        simulateImageDose(dqe, ntf, binning, dose_per_pix);
    } else {
        simulateImagePerfect();
    }
}

template <class T>
void SimulationCtem<T>::simulateImagePerfect()
{
    CLOG(DEBUG, "sim") << "Start CTEM image simulation (no dose calculation)";
    unsigned int resolution = job->simManager->getResolution();
    double wavelength = job->simManager->getWavelength();
    auto mParams = job->simManager->getMicroscopeParams();

    CLOG(DEBUG, "sim") << "Calculating CTEM image from wavefunction";
    // Set arguments for imaging kernel
    ImagingKernel.SetArg(0, clWaveFunctionRecip[0], ArgumentType::Input);
    ImagingKernel.SetArg(1, clImageWaveFunction, ArgumentType::Output);
    ImagingKernel.SetArg(2, resolution);
    ImagingKernel.SetArg(3, resolution);
    ImagingKernel.SetArg(4, clXFrequencies, ArgumentType::Input);
    ImagingKernel.SetArg(5, clYFrequencies, ArgumentType::Input);
    ImagingKernel.SetArg(6, static_cast<T>(wavelength));
    ImagingKernel.SetArg(7, static_cast<T>(mParams->C10));
    ImagingKernel.SetArg(8, static_cast<std::complex<T>>(mParams->C12.getComplex()));
    ImagingKernel.SetArg(9, static_cast<std::complex<T>>(mParams->C21.getComplex()));
    ImagingKernel.SetArg(10, static_cast<std::complex<T>>(mParams->C23.getComplex()));
    ImagingKernel.SetArg(11, static_cast<T>(mParams->C30));
    ImagingKernel.SetArg(12, static_cast<std::complex<T>>(mParams->C32.getComplex()));
    ImagingKernel.SetArg(13, static_cast<std::complex<T>>(mParams->C34.getComplex()));
    ImagingKernel.SetArg(14, static_cast<std::complex<T>>(mParams->C41.getComplex()));
    ImagingKernel.SetArg(15, static_cast<std::complex<T>>(mParams->C43.getComplex()));
    ImagingKernel.SetArg(16, static_cast<std::complex<T>>(mParams->C45.getComplex()));
    ImagingKernel.SetArg(17, static_cast<T>(mParams->C50));
    ImagingKernel.SetArg(18, static_cast<std::complex<T>>(mParams->C52.getComplex()));
    ImagingKernel.SetArg(19, static_cast<std::complex<T>>(mParams->C54.getComplex()));
    ImagingKernel.SetArg(20, static_cast<std::complex<T>>(mParams->C56.getComplex()));
    ImagingKernel.SetArg(21, static_cast<T>(mParams->Aperture));
    ImagingKernel.SetArg(22, static_cast<T>(mParams->Alpha)); //TODO check this is right...
    ImagingKernel.SetArg(23, static_cast<T>(mParams->Delta));

    clWorkGroup Work(resolution, resolution, 1);

    ImagingKernel.run(Work);
    ctx.WaitForQueueFinish();

    // Now get and display absolute value
    CLOG(DEBUG, "sim") << "IFFT to real space";
    FourierTrans.run(clImageWaveFunction, clWaveFunctionTemp_1, Direction::Inverse);
    ctx.WaitForQueueFinish();

    CLOG(DEBUG, "sim") << "Calculate absolute squared";
    ABS2.SetArg(0, clWaveFunctionTemp_1, ArgumentType::Input);
    ABS2.SetArg(1, clImageWaveFunction, ArgumentType::Output);
    ABS2.SetArg(2, resolution);
    ABS2.SetArg(3, resolution);
    ABS2.run(Work);
    ctx.WaitForQueueFinish();
}

// TODO: what should be done with the conversion factor?
// I think it might be like an amplification thing - as in if the detector gets n electrons, it will 'detect' n*conversion factor?
template <class T>
void SimulationCtem<T>::simulateImageDose(std::vector<T> dqe_data, std::vector<T> ntf_data, int binning,
                                                double doseperpix, double conversionfactor)
{
    // all the NTF, DQE stuff can be found here: 10.1016/j.jsb.2013.05.008
    CLOG(DEBUG, "sim") << "Start CTEM image simulation (with calculation)";

    unsigned int resolution = job->simManager->getResolution();
    auto mParams = job->simManager->getMicroscopeParams();

    clWorkGroup Work(resolution, resolution, 1);

    //
    // Do the 'normal' image calculation
    //

    simulateCtemImage();

    //
    // Dose stuff starts here!
    //

    // FFT
    CLOG(DEBUG, "sim") << "FFT back to reciprocal space";
    FourierTrans.run(clImageWaveFunction, clTempBuffer, Direction::Forwards);
    ctx.WaitForQueueFinish();

    // write DQE to opencl
    CLOG(DEBUG, "sim") << "Upload DQE buffer";
    clCcdBuffer.Write(dqe_data);
    ctx.WaitForQueueFinish();

    CLOG(DEBUG, "sim") << "Apply DQE";
    // apply DQE
    DqeKernel.SetArg(0, clTempBuffer, ArgumentType::InputOutput);
    DqeKernel.SetArg(1, clCcdBuffer, ArgumentType::Input);
    DqeKernel.SetArg(2, resolution);
    DqeKernel.SetArg(3, resolution);
    DqeKernel.SetArg(4, binning);

    DqeKernel.run(Work);
    ctx.WaitForQueueFinish();

    // IFFT back
    CLOG(DEBUG, "sim") << "IFFT to real space";
    FourierTrans.run(clTempBuffer, clImageWaveFunction, Direction::Inverse);
    ctx.WaitForQueueFinish();

    CLOG(DEBUG, "sim") << "Read from buffer";
    double N_tot = doseperpix * binning * binning; // Get this passed in, its dose per binned pixel i think.
    std::vector<std::complex<T>> compdata = clImageWaveFunction.CreateLocalCopy();

    CLOG(DEBUG, "sim") << "Add noise";

//    std::random_device rd;
    std::mt19937_64 rng(std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count()));

    for (int i = 0; i < resolution * resolution; i++) {
        // previously was using a Box-Muller transform to get a normal dist and assuming it would approximate a poisson distribution
        // see: https://stackoverflow.com/questions/19944111/creating-a-gaussian-random-generator-with-a-mean-and-standard-deviation

        // use the built in stuff for ease
        std::poisson_distribution<int> dist(N_tot * compdata[i].real()); // TODO: here we are assuming this function is only real?
        double poiss = dist(rng);

        compdata[i].real(conversionfactor * poiss);
        compdata[i].imag(0);
    }

    CLOG(DEBUG, "sim") << "Write back to buffer";
    clImageWaveFunction.Write(compdata);
    ctx.WaitForQueueFinish();

    CLOG(DEBUG, "sim") << "FFT to reciprocal space";
    FourierTrans.run(clImageWaveFunction, clTempBuffer, Direction::Forwards);
    ctx.WaitForQueueFinish();

    CLOG(DEBUG, "sim") << "Upload NTF buffer";
    clCcdBuffer.Write(ntf_data);
    ctx.WaitForQueueFinish();

    CLOG(DEBUG, "sim") << "Apply NTF";
    NtfKernel.SetArg(0, clTempBuffer, ArgumentType::InputOutput);
    NtfKernel.SetArg(1, clCcdBuffer, ArgumentType::Input);
    NtfKernel.SetArg(2, resolution);
    NtfKernel.SetArg(3, resolution);
    NtfKernel.SetArg(4, binning);

    NtfKernel.run(Work);
    ctx.WaitForQueueFinish();

    CLOG(DEBUG, "sim") << "FFT to real space";
    FourierTrans.run(clTempBuffer, clImageWaveFunction, Direction::Inverse);
    ctx.WaitForQueueFinish();
}

template <class T>
std::vector<double> SimulationCtem<T>::getCtemImage()
{
    CLOG(DEBUG, "sim") << "Getting CTEM image image";
    unsigned int resolution = job->simManager->getResolution();
    std::vector<double> data_out(resolution*resolution);

    // Original data is complex so copy complex version down first
    CLOG(DEBUG, "sim") << "Copy from buffer";
    std::vector<std::complex<T>> compdata = clImageWaveFunction.CreateLocalCopy();

    CLOG(DEBUG, "sim") << "Getting only real part";
    for (int i = 0; i < resolution * resolution; i++)
        data_out[i] = compdata[i].real(); // already abs in simulateCTEM function (but is still 'complex' type?)

    return data_out;
}

template<class GPU_Type>
void SimulationCtem<GPU_Type>::simulate() {
    initialiseSimulation();

    CLOG(DEBUG, "sim") << "Starting multislice loop";

    // convenient typedef and create the map to return our images
    typedef std::map<std::string, Image<double>> return_map;
    return_map Images;
    // Get all the variables we will be needing in one go
    unsigned int numberOfSlices = job->simManager->getNumberofSlices();
    unsigned int resolution = job->simManager->getResolution();
    std::valarray<unsigned int> im_crop = job->simManager->getImageCrop();
    bool sim_im = job->simManager->getSimulateCtemImage();

    // TODO: set this properly
    // This will be a pre-calculated variable to set how often we pull our our slice data

    unsigned int slice_step = job->simManager->getUsedIntermediateSlices();
    unsigned int output_count = 1;
    if (slice_step > 0)
        output_count = std::ceil((float) numberOfSlices / slice_step);

    // Create our images here (as we will need to be updating them throughout the slice process)
    auto ew = Image<double>(resolution, resolution, output_count, im_crop[0], im_crop[1], im_crop[2], im_crop[3]);
    auto diff = Image<double>(resolution, resolution, output_count);
    Image<double> ctem_im;
    if (sim_im)
        ctem_im = Image<double>(resolution, resolution, output_count, im_crop[0], im_crop[1], im_crop[2], im_crop[3]);

    //
    // plasmon setup
    //
    std::shared_ptr<PlasmonScattering> plasmon = job->simManager->getInelasticScattering()->getPlasmons();
    bool do_plasmons = plasmon->getPlasmonEnabled();
    double slice_dz = job->simManager->getSliceThickness();
    int padding_slices = (int) job->simManager->getPaddedPreSlices();
    unsigned int scattering_count = 0;
    double next_scattering_depth = plasmon->getGeneratedDepth(job->id, scattering_count);

    // this gives us our current wavevector, but also our axis for azimuth rotation
    auto mp = job->simManager->getMicroscopeParams();
    double k_v = mp->Wavenumber();
    auto orig_k = mp->Wavevector();
    Eigen::Vector3d k_vec(0.0, 0.0, k_v);
    Eigen::Vector3d y_axis(0.0, 1.0, 0.0);

    // apply any current rotation to the y_axis
    double current_tilt = mp->BeamTilt;
    double current_azimuth = mp->BeamAzimuth;
    Utils::rotateVectorSpherical(k_vec, y_axis, current_tilt/1000.0, current_azimuth);


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
            next_scattering_depth = job->simManager->getInelasticScattering()->getPlasmons()->getGeneratedDepth(job->id, scattering_count);
        }

        // this is mostly here because large images can take an age to copy across (so skip that if we are cancelling)
        if (pool.isStopped())
            return;

        // get data when we have the right number of slices (unless it is the end, that is always done after the loop)
        if (slice_step > 0 && (i + 1) % slice_step == 0) {
            ew.getSliceRef(output_counter) = getExitWaveImage();
            diff.getSliceRef(output_counter) = getDiffractionImage(0, k_vec(0) - orig_k[0], k_vec(1) - orig_k[1]);

            if (sim_im) {
                simulateCtemImage();
                ctem_im.getSliceRef(output_counter) = getCtemImage();
            }

            ++output_counter;
        }

        if (pool.isStopped())
            return;

        job->simManager->reportSliceProgress(static_cast<double>(i + 1) / numberOfSlices);
    }

    // get the final slice output
    if (output_counter < output_count) {
        ew.getSliceRef(output_counter) = getExitWaveImage();
        diff.getSliceRef(output_counter) = getDiffractionImage(0, k_vec(0) - orig_k[0], k_vec(1) - orig_k[1]);
        if (sim_im) {
            simulateCtemImage();
            ctem_im.getSliceRef(output_counter) = getCtemImage();
        }
    }
    CLOG(DEBUG, "sim") << "Getting return images";

    // get the images we need
    Images.insert(return_map::value_type("EW", ew));
    Images.insert(return_map::value_type("Diff", diff));
    if (sim_im)
        Images.insert(return_map::value_type("Image", ctem_im));

    job->simManager->updateImages(Images, 1);
}

template class SimulationCtem<float>;
template class SimulationCtem<double>;
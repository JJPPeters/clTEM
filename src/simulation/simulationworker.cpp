//
// Created by jon on 23/11/17.
//

#include "simulationworker.h"
#include "structure/structureparameters.h"
#include <utilities/stringutils.h>
#include <utilities/fileio.h>
#include "kernels.h"
#include "ccdparams.h"

template <class T>
void SimulationWorker<T>::Run(const std::shared_ptr<SimulationJob> &_job) {
    // here is where the simulation gubbins happens
    // Or, in the words of Adam Dyson, this is where the magic happens :)
    int p_num = ctx.GetContextDevice().GetPlatformNumber();
    int d_num = ctx.GetContextDevice().GetDeviceNumber();

    el::Helpers::setThreadName("p" + std::to_string(p_num) + ":d" + std::to_string(d_num));

    CLOG(DEBUG, "sim") << "Running simulation worker";

    job = _job;

    if (!job->simManager) {
        CLOG(DEBUG, "sim") << "Cannot access simulation parameters";
        pool.stop = true;
    }

    if (pool.stop) {
        CLOG(DEBUG, "sim") << "Threadpool stopping";
        job->promise.set_value();
        return;
    }

    CLOG(DEBUG, "sim") << "Starting simulation";

    initialiseSimulation();

    CLOG(DEBUG, "sim") << "Sorting atoms";

    sortAtoms(job->simManager->getTdsRuns() > 1);

    // now what we do depends on the simulation type (I think...)
    auto mode = job->simManager->getMode();

    try {
        if (mode == SimulationMode::CTEM) {
            CLOG(DEBUG, "sim") << "Doing CTEM simulation";
            doCtem();
        } else if (mode == SimulationMode::CBED) {
            CLOG(DEBUG, "sim") << "Doing CBED simulation";
            doCbed();
        } else if (mode == SimulationMode::STEM) {
            CLOG(DEBUG, "sim") << "Doing STEM simulation";
            doStem();
        }
    } catch (const std::runtime_error &e) {
        CLOG(ERROR, "sim") << "Error performing simulation: " << e.what();
        pool.stop = true;
        job->simManager->failedSimulation();
    }

    CLOG(DEBUG, "sim") << "Completed simulation";
    // finally, end this thread ?
    job->promise.set_value();
}

template <class T>
void SimulationWorker<T>::sortAtoms(bool doTds) {
    CLOG(DEBUG, "sim") << "Sorting Atoms";
    auto atoms = job->simManager->getStructure()->getAtoms();
    auto atom_count = (unsigned int) atoms.size(); // Needs to be cast to int as opencl kernel expects that size

    std::vector<int> AtomANum(atom_count);
    std::vector<T> AtomXPos(atom_count);
    std::vector<T> AtomYPos(atom_count);
    std::vector<T> AtomZPos(atom_count);

    CLOG(DEBUG, "sim") << "Getting atom positions";
    if (doTds)
        CLOG(DEBUG, "sim") << "Using TDS";

    for(int i = 0; i < atom_count; i++) {
        T dx = 0.0f, dy = 0.0f, dz = 0.0f;
        if (doTds) {
            // TODO: need a log guard here or in the structure file?
            dx = job->simManager->generateTdsFactor(atoms[i], 0);
            dy = job->simManager->generateTdsFactor(atoms[i], 1);
            dz = job->simManager->generateTdsFactor(atoms[i], 2);
        }

        AtomANum[i] = atoms[i].A;
        AtomXPos[i] = atoms[i].x + dx;
        AtomYPos[i] = atoms[i].y + dy;
        AtomZPos[i] = atoms[i].z + dz;
    }

    CLOG(DEBUG, "sim") << "Writing to buffers";

    ClAtomX.Write(AtomXPos);
    ClAtomY.Write(AtomYPos);
    ClAtomZ.Write(AtomZPos);
    ClAtomA.Write(AtomANum);

    CLOG(DEBUG, "sim") << "Creating sort kernel";

    // NOTE: DONT CHANGE UNLESS CHANGE ELSEWHERE ASWELL!
    // Or fix it so they are all referencing same variable.
    unsigned int BlocksX = job->simManager->getBlocksX();
    unsigned int BlocksY = job->simManager->getBlocksY();
    std::valarray<float> x_lims = job->simManager->getPaddedSimLimitsX(); // is this the right padding?
    std::valarray<float> y_lims = job->simManager->getPaddedSimLimitsY();
    std::valarray<float> z_lims = job->simManager->getPaddedStructLimitsZ();

    float dz = job->simManager->getSliceThickness();
    unsigned int numberOfSlices = job->simManager->getNumberofSlices();

    AtomSort.SetArg(0, ClAtomX, ArgumentType::Input);
    AtomSort.SetArg(1, ClAtomY, ArgumentType::Input);
    AtomSort.SetArg(2, ClAtomZ, ArgumentType::Input);
    AtomSort.SetArg(3, atom_count);
    AtomSort.SetArg(4, x_lims[0]);
    AtomSort.SetArg(5, x_lims[1]);
    AtomSort.SetArg(6, y_lims[0]);
    AtomSort.SetArg(7, y_lims[1]);
    AtomSort.SetArg(8, z_lims[0]);
    AtomSort.SetArg(9, z_lims[1]);
    AtomSort.SetArg(10, BlocksX);
    AtomSort.SetArg(11, BlocksY);
    AtomSort.SetArg(12, ClBlockIds, ArgumentType::Output);
    AtomSort.SetArg(13, ClZIds, ArgumentType::Output);
    AtomSort.SetArg(14, dz);
    AtomSort.SetArg(15, numberOfSlices);

    clWorkGroup SortSize(atom_count,1,1);
    CLOG(DEBUG, "sim") << "Running sort kernel";
    AtomSort.run(SortSize);

    ctx.WaitForQueueFinish(); // test

    CLOG(DEBUG, "sim") << "Reading sort kernel output";

    std::vector<int> HostBlockIDs = ClBlockIds.CreateLocalCopy();
    std::vector<int> HostZIDs = ClZIds.CreateLocalCopy();

    CLOG(DEBUG, "sim") << "Binning atoms";

    // this silly initialising is to make the first two levels of our vectors, we then dynamically
    // fill the next level in the following loop :)
    std::vector<std::vector<std::vector<float>>> Binnedx( BlocksX*BlocksY, std::vector<std::vector<float>>(numberOfSlices) );
    std::vector<std::vector<std::vector<float>>> Binnedy( BlocksX*BlocksY, std::vector<std::vector<float>>(numberOfSlices) );
    std::vector<std::vector<std::vector<float>>> Binnedz( BlocksX*BlocksY, std::vector<std::vector<float>>(numberOfSlices) );
    std::vector<std::vector<std::vector<int>>>   BinnedA( BlocksX*BlocksY, std::vector<std::vector<int>>  (numberOfSlices) );

    int count_in_range = 0;
    for(int i = 0; i < atom_count; ++i) {
        if (HostZIDs[i] >= 0 && HostBlockIDs[i] >= 0) {
            Binnedx[HostBlockIDs[i]][HostZIDs[i]].push_back(AtomXPos[i]);
            Binnedy[HostBlockIDs[i]][HostZIDs[i]].push_back(AtomYPos[i]);
            Binnedz[HostBlockIDs[i]][HostZIDs[i]].push_back(AtomZPos[i]);
            BinnedA[HostBlockIDs[i]][HostZIDs[i]].push_back(AtomANum[i]);
            ++count_in_range;
        }
    }

    unsigned long long max_bin_xy = 0;
    unsigned long long max_bin_z = 0;

    //
    // This only works because x and y have the same number of blocks (so I only need to look at one of x or y)
    //
    for (auto &bx_1 : Binnedx) {
        if (bx_1.size() > max_bin_xy)
            max_bin_xy = bx_1.size();

        for (auto &bx_2 : bx_1)
            if (bx_2.size() > max_bin_z)
                max_bin_z = bx_2.size();
    }

    int atomIterator = 0;

    std::vector<int> blockStartPositions(numberOfSlices*BlocksX*BlocksY+1);

    // Put all bins into a linear block of memory ordered by z then y then x and record start positions for every block.
    CLOG(DEBUG, "sim") << "Putting binned atoms into continuous array";

    for(int slicei = 0; slicei < numberOfSlices; slicei++) {
        for(int j = 0; j < BlocksY; j++) {
            for(int k = 0; k < BlocksX; k++) {
                blockStartPositions[slicei*BlocksX*BlocksY+ j*BlocksX + k] = atomIterator;

                if(!Binnedx[j * BlocksX + k][slicei].empty()) {
                    for(int l = 0; l < Binnedx[j*BlocksX+k][slicei].size(); l++) {
                        AtomXPos[atomIterator] = Binnedx[j*BlocksX+k][slicei][l];
                        AtomYPos[atomIterator] = Binnedy[j*BlocksX+k][slicei][l];
                        AtomZPos[atomIterator] = Binnedz[j*BlocksX+k][slicei][l];
                        AtomANum[atomIterator] = BinnedA[j*BlocksX+k][slicei][l];
                        atomIterator++;
                    }
                }
            }
        }
    }

    // Last element indicates end of last block as total number of atoms.
    blockStartPositions[numberOfSlices*BlocksX*BlocksY] = count_in_range;

    ClBlockStartPositions = clMemory<int, Manual>(ctx, numberOfSlices * BlocksX * BlocksY + 1);

    CLOG(DEBUG, "sim") << "Writing binned atom posisitons to bufffers";

    // Now upload the sorted atoms onto the device..
    ClAtomX.Write(AtomXPos);
    ClAtomY.Write(AtomYPos);
    ClAtomZ.Write(AtomZPos);
    ClAtomA.Write(AtomANum);

    ClBlockStartPositions.Write(blockStartPositions);

    // wait for the IO queue here so that we are sure the data is uploaded before we start usign it
    ctx.WaitForQueueFinish();
}

template <class T>
void SimulationWorker<T>::doCtem(bool simImage)
{
    CLOG(DEBUG, "sim") << "Starting multislice loop";
    // loop through slices
    unsigned int numberOfSlices = job->simManager->getNumberofSlices();
    for (int i = 0; i < numberOfSlices; ++i) {
        doMultiSliceStep(i);
        if (pool.stop)
            return;
        job->simManager->reportSliceProgress(((float)i+1) / (float)numberOfSlices);
    }

    if (simImage)
        simulateCtemImage();

    CLOG(DEBUG, "sim") << "Getting return images";

    unsigned int resolution = job->simManager->getResolution();
    typedef std::map<std::string, Image<T>> return_map;
    return_map Images;

    float real_scale = job->simManager->getRealScale();

    auto x_im_range = job->simManager->getRawSimLimitsX()[1] - job->simManager->getRawSimLimitsX()[0];
    auto x_sim_range = job->simManager->getPaddedSimLimitsX()[1] - job->simManager->getPaddedSimLimitsX()[0];
    auto crop_lr_total = (std::floor(x_sim_range - x_im_range)  / real_scale);

    auto y_im_range = job->simManager->getRawSimLimitsY()[1] - job->simManager->getRawSimLimitsY()[0];
    auto y_sim_range = job->simManager->getPaddedSimLimitsY()[1] - job->simManager->getPaddedSimLimitsY()[0];
    auto crop_tb_total = (std::floor(y_sim_range - y_im_range)  / real_scale);

    auto crop_l = (unsigned int) std::floor(crop_lr_total / 2.0);
    auto crop_b = (unsigned int) std::floor(crop_tb_total / 2.0);

    auto crop_r = (unsigned int) crop_lr_total - crop_l;
    auto crop_t = (unsigned int) crop_tb_total - crop_b;

    auto ew = Image<T>(resolution, resolution, getExitWaveImage(), crop_t, crop_l, crop_b, crop_r);
    auto diff = Image<T>(resolution, resolution, getDiffractionImage());

    // get the images we need
    Images.insert(return_map::value_type("EW", ew));
    Images.insert(return_map::value_type("Diff", diff));

    if (job->simManager->getSimulateCtemImage()) {

        std::string ccd = job->simManager->getCcdName();
        if (CCDParams::nameExists(ccd)) {
            auto dqe = CCDParams::getDQE(ccd);
            auto ntf = CCDParams::getNTF(ccd);
            int binning = job->simManager->getCcdBinning();
            // get dose
            float dose = job->simManager->getCcdDose(); // electrons per area
            // get electrons per pixel
            float scale = job->simManager->getRealScale();
            scale *= scale; // square it to get area of pixel
            float dose_per_pix = dose * scale;

            // Error here is because of dqe and ntf vectors
            simulateCtemImage(dqe, ntf, binning, dose_per_pix);
        }
        else {
            simulateCtemImage();
        }

        auto ctem_im = Image<T>(resolution, resolution, getCtemImage(), crop_t, crop_l, crop_b, crop_r);
        Images.insert(return_map::value_type("Image", ctem_im));
    }

    job->simManager->updateImages(Images, 1);
}

template <class T>
void SimulationWorker<T>::doCbed()
{
    auto pos = job->simManager->getCBedPosition();

    initialiseProbeWave(pos->getXPos(), pos->getYPos());

    CLOG(DEBUG, "sim") << "Starting multislice loop";
    // loop through slices
    unsigned int numberOfSlices = job->simManager->getNumberofSlices();
    for (int i = 0; i < numberOfSlices; ++i) {
        doMultiSliceStep(i);
        if (pool.stop) {
            CLOG(DEBUG, "sim") << "Thread pool stopping";
            return;
        }
        job->simManager->reportSliceProgress(((float)i+1) / (float)numberOfSlices);
    }

    CLOG(DEBUG, "sim") << "Getting return images";
    // get images and return them...
    unsigned int resolution = job->simManager->getResolution();
    typedef std::map<std::string, Image<T>> return_map;
    return_map Images;

    auto diff = Image<T>(resolution, resolution, getDiffractionImage());

    Images.insert(return_map::value_type("Diff", diff));

    job->simManager->updateImages(Images, 1); // Update this if we ever do more than one TDS in a job
}

template <class T>
void SimulationWorker<T>::doStem()
{
    CLOG(DEBUG, "sim") << "Parallel pixels: " << job->pixels.size();

    // now need to work out where our probes need to be made
    auto stemPixels = job->simManager->getStemArea();

    // get start position and the pixel step
    // Raw values here as this is used to calculate the beam positions
    float start_x = stemPixels->getRawLimitsX()[0];
    float start_y = stemPixels->getRawLimitsY()[0];

    int num_x = stemPixels->getPixelsX();

    float step_x = stemPixels->getScaleX();
    float step_y = stemPixels->getScaleY();

    CLOG(DEBUG, "sim") << "Initialising probe(s)";
    for (int i = 0; i < job->pixels.size(); ++i) {
        int p = job->pixels[i];
        // index to x and y index
        int x_i = p % num_x;
        int y_i = (int) std::floor(p / num_x);

        float x_pos = start_x + x_i * step_x;
        float y_pos = start_y + y_i * step_y;

        initialiseProbeWave(x_pos, y_pos, i);
    }

    CLOG(DEBUG, "sim") << "Starting multislice loop";
    // loop through slices
    unsigned int numberOfSlices = job->simManager->getNumberofSlices();
    for (int i = 0; i < numberOfSlices; ++i) {
        doMultiSliceStep(i);
        if (pool.stop) {
            CLOG(DEBUG, "sim") << "Thread pool stopping";
            return;
        }
        job->simManager->reportSliceProgress(((float)i+1) / (float)numberOfSlices);
    }

    CLOG(DEBUG, "sim") << "Getting return images";
    typedef std::map<std::string, Image<float>> return_map;
    return_map Images;
    unsigned int px_x = job->simManager->getStemArea()->getPixelsX();
    unsigned int px_y = job->simManager->getStemArea()->getPixelsY();

    for (const auto &det : job->simManager->getDetectors()) {
        std::vector<float> im(stemPixels->getNumPixels(), 0.0f);

        for (int i = 0; i < job->pixels.size(); ++i) {
            im[job->pixels[i]] = getStemPixel(det.inner, det.outer, det.xcentre, det.ycentre, i);
        }

        Images[det.name] = Image<float>(px_x, px_y, im);
    }

    job->simManager->updateImages(Images, 1);
}

template <class T>
void SimulationWorker<T>::initialiseSimulation() {
    CLOG(DEBUG, "sim") << "Initialising all buffers";
    initialiseBuffers();

    CLOG(DEBUG, "sim") << "Getting parameters";
    std::vector<float> params = job->simManager->getStructureParameterData();
    CLOG(DEBUG, "sim") << "Uploading parameters";
    ClParameterisation.Write(params);

    CLOG(DEBUG, "sim") << "Setting up all kernels";
    initialiseKernels();

    CLOG(DEBUG, "sim") << "Starting general initialisation";
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Get local copies of variables (for convenience)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool isFull3D = job->simManager->isFull3d();
    unsigned int resolution = job->simManager->getResolution();
    float wavelength = job->simManager->getWavelength();
    float pixelscale = job->simManager->getRealScale();
    auto mParams = job->simManager->getMicroscopeParams();
    float startx = job->simManager->getPaddedSimLimitsX()[0];
    float starty = job->simManager->getPaddedSimLimitsY()[0];
    int full3dints = job->simManager->getFull3dInts();
    std::string param_name = job->simManager->getStructureParametersName();

    // Work out area that is to be simulated (in real space)
    float SimSizeX = pixelscale * resolution;
    float SimSizeY = SimSizeX;

    float sigma = mParams->Sigma();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up our frequency calibrations
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Creating reciprocal space calibration";
    // This basically is all to create OpenCL buffers (1D) that let us know the frequency value of the pixels in the FFT
    // Not that this already accounts for the un-shifted nature of the FFT (i.e. 0 frequency is at 0, 0)
    // We also calculate our limit for low pass filtering the wavefunctions
    std::vector<T> k0x(resolution);
    std::vector<T> k0y(resolution);

    auto imidx = (unsigned int) std::floor((float) resolution / 2.0 + 0.5);
    auto imidy = (unsigned int) std::floor((float) resolution / 2.0 + 0.5);

    float temp;

    for (int i = 0; i < resolution; i++) {
        if (i >= imidx)
            temp = signed(i - resolution) / SimSizeX;
        else
            temp = i / SimSizeX;
        k0x[i] = temp;
    }

    for (int i = 0; i < resolution; i++) {
        if (i >= imidy)
            temp = signed(i - resolution) / SimSizeY;
        else
            temp = i / SimSizeY;
        k0y[i] = temp;
    }

    // Find maximum frequency for bandwidth limiting rule


    // TODO: not sure I want the -1 here
    float kmaxx = std::abs(k0x[imidx]);
    float kmaxy = std::abs(k0y[imidy]);

    float bandwidthkmax = std::min(kmaxy, kmaxx) * job->simManager->getInverseLimitFactor();

    CLOG(DEBUG, "sim") << "Writing to buffers";
    // write our frequencies to OpenCL buffers
    clXFrequencies.Write(k0x);
    clYFrequencies.Write(k0y);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create a few buffers we will need later
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    clWorkGroup WorkSize(resolution, resolution, 1);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up FFT shift kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up FFT shift kernel";

    // these will never change, so set them here
    fftShift.SetArg(0, clWaveFunction2[0], ArgumentType::Input);
    fftShift.SetArg(1, clWaveFunction3, ArgumentType::Output);
    fftShift.SetArg(2, resolution);
    fftShift.SetArg(3, resolution);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up low pass filter kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up low pass filter kernel";

    // These never change, so set them here
    BandLimit.SetArg(0, clWaveFunction3, ArgumentType::InputOutput);
    BandLimit.SetArg(1, resolution);
    BandLimit.SetArg(2, resolution);
    BandLimit.SetArg(3, bandwidthkmax);
    BandLimit.SetArg(4, clXFrequencies, ArgumentType::Input);
    BandLimit.SetArg(5, clYFrequencies, ArgumentType::Input);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up the kernels to calculate the atomic potentials
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up potential kernel";

    // Work out which blocks to load by ensuring we have the entire area around workgroup up to 5 angstroms away...
    // TODO: check this is doing what the above comment says it is doing...
    // TODO: I think the 8.0 and 3.0 should be the padding as set in the manager...
    int load_blocks_x = (int) std::ceil(8.0f / job->simManager->getBlockScaleX());
    int load_blocks_y = (int) std::ceil(8.0f / job->simManager->getBlockScaleY());
    float dz = job->simManager->getSliceThickness();
    int load_blocks_z = (int) std::ceil(3.0f / dz);

    // Set some of the arguments which dont change each iteration
    BinnedAtomicPotential.SetArg(0, clPotential, ArgumentType::Output);
    BinnedAtomicPotential.SetArg(5, ClParameterisation, ArgumentType::Input);
    if (param_name == "kirkland")
        BinnedAtomicPotential.SetArg(6, 0);
    else if (param_name == "peng")
        BinnedAtomicPotential.SetArg(6, 1);
    else if (param_name == "lobato")
        BinnedAtomicPotential.SetArg(6, 2);
    else
        throw std::runtime_error("Trying to use parameterisation I do not understand");
    BinnedAtomicPotential.SetArg(8, resolution);
    BinnedAtomicPotential.SetArg(9, resolution);
    BinnedAtomicPotential.SetArg(13, dz);
    BinnedAtomicPotential.SetArg(14, pixelscale);
    BinnedAtomicPotential.SetArg(15, job->simManager->getBlocksX());
    BinnedAtomicPotential.SetArg(16, job->simManager->getBlocksY());
    BinnedAtomicPotential.SetArg(17, job->simManager->getPaddedSimLimitsX()[1]);
    BinnedAtomicPotential.SetArg(18, job->simManager->getPaddedSimLimitsX()[0]);
    BinnedAtomicPotential.SetArg(19, job->simManager->getPaddedSimLimitsY()[1]);
    BinnedAtomicPotential.SetArg(20, job->simManager->getPaddedSimLimitsY()[0]);
    BinnedAtomicPotential.SetArg(21, load_blocks_x);
    BinnedAtomicPotential.SetArg(22, load_blocks_y);
    BinnedAtomicPotential.SetArg(23, load_blocks_z);
    BinnedAtomicPotential.SetArg(24, sigma); // Not sure why I am using this sigma and not commented sigma...
    BinnedAtomicPotential.SetArg(25, startx);
    BinnedAtomicPotential.SetArg(26, starty);
    if (isFull3D)
        BinnedAtomicPotential.SetArg(27, full3dints);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up the propogator
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up propagator kernel";

    GeneratePropagator.SetArg(0, clPropagator, ArgumentType::Output);
    GeneratePropagator.SetArg(1, clXFrequencies, ArgumentType::Input);
    GeneratePropagator.SetArg(2, clYFrequencies, ArgumentType::Input);
    GeneratePropagator.SetArg(3, resolution);
    GeneratePropagator.SetArg(4, resolution);
    GeneratePropagator.SetArg(5, dz); // Is this the right dz? (Propagator needs slice thickness not spacing between atom bins)
    GeneratePropagator.SetArg(6, wavelength);
    GeneratePropagator.SetArg(7, bandwidthkmax);

    // actually run this kernel now
    GeneratePropagator.run(WorkSize);
    ctx.WaitForQueueFinish();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up complex multiply kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up complex multiply kernel";

    ComplexMultiply.SetArg(3, resolution);
    ComplexMultiply.SetArg(4, resolution);

    if (job->simManager->getMode() == SimulationMode::CTEM)
        initialiseCtem();
}

template <class T>
void SimulationWorker<T>::initialiseCtem()
{

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
    float InitialValue = 1.0f;
    InitPlaneWavefunction.SetArg(1, resolution);
    InitPlaneWavefunction.SetArg(2, resolution);
    InitPlaneWavefunction.SetArg(3, InitialValue);
    InitPlaneWavefunction.SetArg(0, clWaveFunction1[0], ArgumentType::Output);
    InitPlaneWavefunction.run(WorkSize);

    ctx.WaitForQueueFinish();
}

// n_parallel is the index (from 0) of the current parallel pixel
template <class T>
void SimulationWorker<T>::initialiseProbeWave(T posx, T posy, int n_parallel) {
    CLOG(DEBUG, "sim") << "Initialising probe wavefunction";
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create local variables for convenience
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    unsigned int resolution = job->simManager->getResolution();
    float wavelength = job->simManager->getWavelength();
    float pixelscale = job->simManager->getRealScale();
    auto mParams = job->simManager->getMicroscopeParams();

    clWorkGroup WorkSize(resolution, resolution, 1);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// intialise and create probe in fourier space
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: is this needed? must test on a sample we know (i.e. a single atom) or just save the probe image
    // convert from angstroms to pixel position (this bit seemed to make results 'more' sensible)

    float start_x = job->simManager->getPaddedSimLimitsX()[0];
    float start_y = job->simManager->getPaddedSimLimitsY()[0];
    // account for the simulation area start point and convert to pixels
    posx = (posx - start_x) / pixelscale;
    posy = (posy - start_y) / pixelscale;

    // Fix inverted images
    posx = resolution - posx;
    posy = resolution - posy;

    InitProbeWavefunction.SetArg(0, clWaveFunction2[n_parallel]);
    InitProbeWavefunction.SetArg(1, resolution);
    InitProbeWavefunction.SetArg(2, resolution);
    InitProbeWavefunction.SetArg(3, clXFrequencies);
    InitProbeWavefunction.SetArg(4, clYFrequencies);
    InitProbeWavefunction.SetArg(5, posx);
    InitProbeWavefunction.SetArg(6, posy);
    InitProbeWavefunction.SetArg(7, pixelscale);
    InitProbeWavefunction.SetArg(8, wavelength);
    InitProbeWavefunction.SetArg(9, mParams->C10);
    InitProbeWavefunction.SetArg(10, mParams->C12.getComplex());
    InitProbeWavefunction.SetArg(11, mParams->C21.getComplex());
    InitProbeWavefunction.SetArg(12, mParams->C23.getComplex());
    InitProbeWavefunction.SetArg(13, mParams->C30);
    InitProbeWavefunction.SetArg(14, mParams->C32.getComplex());
    InitProbeWavefunction.SetArg(15, mParams->C34.getComplex());
    InitProbeWavefunction.SetArg(16, mParams->C41.getComplex());
    InitProbeWavefunction.SetArg(17, mParams->C43.getComplex());
    InitProbeWavefunction.SetArg(18, mParams->C45.getComplex());
    InitProbeWavefunction.SetArg(19, mParams->C50);
    InitProbeWavefunction.SetArg(20, mParams->C52.getComplex());
    InitProbeWavefunction.SetArg(21, mParams->C54.getComplex());
    InitProbeWavefunction.SetArg(22, mParams->C56.getComplex());
    InitProbeWavefunction.SetArg(23, mParams->Aperture);

    CLOG(DEBUG, "sim") << "Run probe wavefunction generation kernel";
    InitProbeWavefunction.run(WorkSize);

    ctx.WaitForQueueFinish();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IFFT probe to real space
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // IFFT
    CLOG(DEBUG, "sim") << "IFFT probe wavefunction";
    FourierTrans.Do(clWaveFunction2[n_parallel], clWaveFunction1[n_parallel], Direction::Inverse);
    ctx.WaitForQueueFinish();
}

template <class T>
void SimulationWorker<T>::doMultiSliceStep(int slice)
{
    CLOG(DEBUG, "sim") << "Start multislice step " << slice;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create local variables for convenience
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float dz = job->simManager->getSliceThickness();
    unsigned int resolution = job->simManager->getResolution();
    // in the current format, Tds is handled by job splitting so this is always 1??
    int n_parallel = job->simManager->getParallelPixels(); // total number of parallel pixels
    auto z_lim = job->simManager->getPaddedStructLimitsZ();

    // Didn't have MinimumZ so it wasnt correctly rescaled z-axis from 0 to SizeZ...
    float currentz = z_lim[1] - slice * dz;

    clWorkGroup Work(resolution, resolution, 1);
    clWorkGroup LocalWork(16, 16, 1);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Get our potentials for the current sim
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    unsigned int numberOfSlices = job->simManager->getNumberofSlices();

    BinnedAtomicPotential.SetArg(1, ClAtomX, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(2, ClAtomY, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(3, ClAtomZ, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(4, ClAtomA, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(7, ClBlockStartPositions, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(10, slice);
    BinnedAtomicPotential.SetArg(11, numberOfSlices);
    BinnedAtomicPotential.SetArg(12, currentz);

    CLOG(DEBUG, "sim") << "Calculating potentials";

    BinnedAtomicPotential.run(Work, LocalWork);

    ctx.WaitForQueueFinish();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Apply low pass filter to potentials
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "FFT potentials";
    FourierTrans.Do(clPotential, clWaveFunction3, Direction::Forwards);
    ctx.WaitForQueueFinish();
    CLOG(DEBUG, "sim") << "Band limit potentials";
    BandLimit.run(Work);
    ctx.WaitForQueueFinish();
    CLOG(DEBUG, "sim") << "IFFT band limited potentials";
    FourierTrans.Do(clWaveFunction3, clPotential, Direction::Inverse);
    ctx.WaitForQueueFinish();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Propogate slice
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for (int i = 1; i <= n_parallel; i++)
    {
        CLOG(DEBUG, "sim") << "Propogating (" << i << " of " << n_parallel << " parallel)";
        // Apply low pass filter to wavefunction
        CLOG(DEBUG, "sim") << "FFT incoming wavefunction";
        FourierTrans.Do(clWaveFunction1[i - 1], clWaveFunction3, Direction::Forwards);
        ctx.WaitForQueueFinish();
        CLOG(DEBUG, "sim") << "Band limit incoming wavefunction";
        BandLimit.run(Work);
        ctx.WaitForQueueFinish();
        CLOG(DEBUG, "sim") << "IFFT band limited incoming wavefunction";
        FourierTrans.Do(clWaveFunction3, clWaveFunction1[i - 1], Direction::Inverse);
        ctx.WaitForQueueFinish();

        //Multiply potential with wavefunction
        ComplexMultiply.SetArg(0, clPotential, ArgumentType::Input);
        ComplexMultiply.SetArg(1, clWaveFunction1[i - 1], ArgumentType::Input);
        ComplexMultiply.SetArg(2, clWaveFunction2[i - 1], ArgumentType::Output);
        CLOG(DEBUG, "sim") << "Multiply wavefunction and potentials";
        ComplexMultiply.run(Work);
        ctx.WaitForQueueFinish();

        // go to reciprocal space
        CLOG(DEBUG, "sim") << "FFT to reciprocal space";
        FourierTrans.Do(clWaveFunction2[i - 1], clWaveFunction3, Direction::Forwards);
        ctx.WaitForQueueFinish();

        // convolve with propagator
        ComplexMultiply.SetArg(0, clWaveFunction3, ArgumentType::Input);
        ComplexMultiply.SetArg(1, clPropagator, ArgumentType::Input);
        ComplexMultiply.SetArg(2, clWaveFunction2[i - 1], ArgumentType::Output);
        CLOG(DEBUG, "sim") << "Convolve with propogator";
        ComplexMultiply.run(Work);
        ctx.WaitForQueueFinish();

        // IFFT back to real space
        CLOG(DEBUG, "sim") << "IFFT to real space";
        FourierTrans.Do(clWaveFunction2[i - 1], clWaveFunction1[i - 1], Direction::Inverse);
        ctx.WaitForQueueFinish();
    }
}

template <class T>
void SimulationWorker<T>::simulateCtemImage()
{
    CLOG(DEBUG, "sim") << "Start CTEM image simulation (no dose calculation)";
    unsigned int resolution = job->simManager->getResolution();
    float wavelength = job->simManager->getWavelength();
    auto mParams = job->simManager->getMicroscopeParams();

    CLOG(DEBUG, "sim") << "Calculating CTEM image from wavefunction";
    // Set arguments for imaging kernel
    ImagingKernel.SetArg(0, clWaveFunction2[0], ArgumentType::Input);
    ImagingKernel.SetArg(1, clImageWaveFunction, ArgumentType::Output);
    ImagingKernel.SetArg(2, resolution);
    ImagingKernel.SetArg(3, resolution);
    ImagingKernel.SetArg(4, clXFrequencies, ArgumentType::Input);
    ImagingKernel.SetArg(5, clYFrequencies, ArgumentType::Input);
    ImagingKernel.SetArg(6, wavelength);
    ImagingKernel.SetArg(7, mParams->C10);
    ImagingKernel.SetArg(8, mParams->C12.getComplex());
    ImagingKernel.SetArg(9, mParams->C21.getComplex());
    ImagingKernel.SetArg(10, mParams->C23.getComplex());
    ImagingKernel.SetArg(11, mParams->C30);
    ImagingKernel.SetArg(12, mParams->C32.getComplex());
    ImagingKernel.SetArg(13, mParams->C34.getComplex());
    ImagingKernel.SetArg(14, mParams->C41.getComplex());
    ImagingKernel.SetArg(15, mParams->C43.getComplex());
    ImagingKernel.SetArg(16, mParams->C45.getComplex());
    ImagingKernel.SetArg(17, mParams->C50);
    ImagingKernel.SetArg(18, mParams->C52.getComplex());
    ImagingKernel.SetArg(19, mParams->C54.getComplex());
    ImagingKernel.SetArg(20, mParams->C56.getComplex());
    ImagingKernel.SetArg(21, mParams->Aperture);
    ImagingKernel.SetArg(22, mParams->Alpha); //TODO check this is right...
    ImagingKernel.SetArg(23, mParams->Delta);

    clWorkGroup Work(resolution, resolution, 1);

    ImagingKernel.run(Work);
    ctx.WaitForQueueFinish();

    // Now get and display absolute value
    CLOG(DEBUG, "sim") << "IFFT to real space";
    FourierTrans.Do(clImageWaveFunction, clWaveFunction1[0], Direction::Inverse);
    ctx.WaitForQueueFinish();

    CLOG(DEBUG, "sim") << "Calculate absolute squared";
    ABS2.SetArg(0, clWaveFunction1[0], ArgumentType::Input);
    ABS2.SetArg(1, clImageWaveFunction, ArgumentType::Output);
    ABS2.SetArg(2, resolution);
    ABS2.SetArg(3, resolution);
    ABS2.run(Work);
    ctx.WaitForQueueFinish();
};

// TODO: what should be done with the conversion factor?
// I think it might be like an amplification thing - as in if the detector gets n electrons, it will 'detect' n*conversion factor?
template <class T>
void SimulationWorker<T>::simulateCtemImage(std::vector<T> dqe_data, std::vector<T> ntf_data, int binning,
                                         T doseperpix, T conversionfactor)
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
    FourierTrans.Do(clImageWaveFunction, clTempBuffer, Direction::Forwards);
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
    FourierTrans.Do(clTempBuffer, clImageWaveFunction, Direction::Inverse);
    ctx.WaitForQueueFinish();

    CLOG(DEBUG, "sim") << "Read from buffer";
    float N_tot = doseperpix * binning * binning; // Get this passed in, its dose per binned pixel i think.
    std::vector<std::complex<T>> compdata = clImageWaveFunction.CreateLocalCopy();

    CLOG(DEBUG, "sim") << "Add noise";

    std::random_device rd;
    std::mt19937 rng(rd());

    for (int i = 0; i < resolution * resolution; i++) {
        // previously was using a Box-Muller transform to get a normal dist and assuming it would approximate a poisson distribution
        // see: https://stackoverflow.com/questions/19944111/creating-a-gaussian-random-generator-with-a-mean-and-standard-deviation

        // use the built in stuff for ease
        std::poisson_distribution<int> dist(N_tot * compdata[i].real()); // TODO: here we are assuming this function is only real?
        auto poiss = (float) dist(rng);

        compdata[i].real(conversionfactor * poiss);
        compdata[i].imag(0);
    }

    CLOG(DEBUG, "sim") << "Write back to buffer";
    clImageWaveFunction.Write(compdata);
    ctx.WaitForQueueFinish();

    CLOG(DEBUG, "sim") << "FFT to reciprocal space";
    FourierTrans.Do(clImageWaveFunction, clTempBuffer, Direction::Forwards);
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
    FourierTrans.Do(clTempBuffer, clImageWaveFunction, Direction::Inverse);
    ctx.WaitForQueueFinish();
}

template <class T>
std::vector<T> SimulationWorker<T>::getDiffractionImage(int parallel_ind)
{
    CLOG(DEBUG, "sim") << "Getting diffraction image";
    unsigned int resolution = job->simManager->getResolution();
    std::vector<T> data_out(resolution*resolution);

    // Original data is complex so copy complex version down first
    clWorkGroup Work(resolution, resolution, 1);

    CLOG(DEBUG, "sim") << "FFT shifting diffraction pattern";
    fftShift.SetArg(0, clWaveFunction2[parallel_ind], ArgumentType::Input);
    fftShift.run(Work);

    CLOG(DEBUG, "sim") << "Copy from buffer";
    std::vector<std::complex<T>> compdata = clWaveFunction3.CreateLocalCopy();

    // TODO: this could be done on GPU?
    CLOG(DEBUG, "sim") << "Calculating absolute squared value";
    for (int i = 0; i < resolution * resolution; i++)
        // Get absolute value for display...
        data_out[i] += std::norm(compdata[i]);// (compdata[i].x * compdata[i].x + compdata[i].y * compdata[i].y);

    return data_out;
}

template <class T>
std::vector<T> SimulationWorker<T>::getExitWaveImage(unsigned int t, unsigned int l, unsigned int b, unsigned int r) {
    CLOG(DEBUG, "sim") << "Getting exit wave image";
    unsigned int resolution = job->simManager->getResolution();
    std::vector<T> data_out(2*((resolution - t - b) * (resolution - l - r)));

    CLOG(DEBUG, "sim") << "Copy from buffer";
    std::vector<std::complex<T>> compdata = clWaveFunction1[0].CreateLocalCopy();

    CLOG(DEBUG, "sim") << "Process complex data";
    int cnt = 0;
    for (int j = 0; j < resolution; ++j)
        if (j >= b && j < (resolution - t))
            for (int i = 0; i < resolution; ++i)
                if (i >= l && i < (resolution - r)) {
                    int k = i + j * resolution;
                    data_out[cnt] = compdata[k].real();
                    data_out[cnt + 1] = compdata[k].imag();
                    cnt += 2;
                }

    return data_out;
}

template <class T>
std::vector<T> SimulationWorker<T>::getCtemImage()
{
    CLOG(DEBUG, "sim") << "Getting CTEM image image";
    unsigned int resolution = job->simManager->getResolution();
    std::vector<T> data_out(resolution*resolution);

    // Original data is complex so copy complex version down first
    CLOG(DEBUG, "sim") << "Copy from buffer";
    std::vector<std::complex<T>> compdata = clImageWaveFunction.CreateLocalCopy();

    CLOG(DEBUG, "sim") << "Getting only real part";
    for (int i = 0; i < resolution * resolution; i++)
        data_out[i] = compdata[i].real(); // already abs in simulateCTEM function (but is still 'complex' type?)

    return data_out;
}

template <class T>
T SimulationWorker<T>::doSumReduction(clMemory<T, Manual> data, clWorkGroup globalSizeSum,
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

    ctx.WaitForQueueFinish();

    // Now copy back
    CLOG(DEBUG, "sim") << "Copy from buffer";
    std::vector<T> sums = clReductionBuffer.CreateLocalCopy();

    CLOG(DEBUG, "sim") << "Doing final sum on CPU (" << nGroups << " parts)";
    // Find out which numbers to read back
    T sum = 0;
    for (int i = 0; i < nGroups; i++)
        sum += sums[i];
    return sum;
}

template <class T>
T SimulationWorker<T>::getStemPixel(T inner, T outer, T xc, T yc, int parallel_ind)
{
    CLOG(DEBUG, "sim") << "Getting STEM pixel";
    unsigned int resolution = job->simManager->getResolution();
    float angle_scale = job->simManager->getInverseScaleAngle();

    clWorkGroup WorkSize(resolution, resolution, 1);

    CLOG(DEBUG, "sim") << "FFT shifting diffraction pattern";
    fftShift.SetArg(0, clWaveFunction2[parallel_ind], ArgumentType::Input);
    fftShift.run(WorkSize);

    float innerPx = inner / angle_scale;
    float outerPx = outer / angle_scale;

    float xcPx = xc / angle_scale;
    float ycPx = yc / angle_scale;

    CLOG(DEBUG, "sim") << "Masking diffraction pattern";
    TDSMaskingAbsKernel.SetArg(0, clWaveFunction3, ArgumentType::Input);
    TDSMaskingAbsKernel.SetArg(1, clTDSMaskDiff, ArgumentType::Output);
    TDSMaskingAbsKernel.SetArg(2, resolution);
    TDSMaskingAbsKernel.SetArg(3, resolution);
    TDSMaskingAbsKernel.SetArg(4, innerPx);
    TDSMaskingAbsKernel.SetArg(5, outerPx);
    TDSMaskingAbsKernel.SetArg(6, xcPx);
    TDSMaskingAbsKernel.SetArg(7, ycPx);

    TDSMaskingAbsKernel.run(WorkSize);

    ctx.WaitForQueueFinish();

    unsigned int totalSize = resolution*resolution;
    unsigned int nGroups = totalSize / 256;

    clWorkGroup globalSizeSum(totalSize, 1, 1);
    clWorkGroup localSizeSum(256, 1, 1);

    return doSumReduction(clTDSMaskDiff, globalSizeSum, localSizeSum, nGroups, totalSize);
}

template <class T>
void SimulationWorker<T>::initialiseBuffers() {

    auto sm = job->simManager;

    // this needs to change if the parameter sizes have changed
    if (size_t ps = sm->getStructureParameterData().size(); ps != ClParameterisation.GetSize())
        ClParameterisation = clMemory<float, Manual>(ctx, ps);

    // these need to change if the atom_count changes
    if (size_t as = sm->getStructure()->getAtoms().size(); as != ClAtomA.GetSize()) {
        ClAtomA = clMemory<int, Manual>(ctx, as);
        ClAtomX = clMemory<T, Manual>(ctx, as);
        ClAtomY = clMemory<T, Manual>(ctx, as);
        ClAtomZ = clMemory<T, Manual>(ctx, as);

        ClBlockIds = clMemory<int, Manual>(ctx, as);
        ClZIds = clMemory<int, Manual>(ctx, as);
    }

//    ClBlockStartPositions is not here as it is sorted every time the atoms are sorted (depends on block size etc..

    // change when the resolution does
    unsigned int rs = sm->getResolution();
    if (rs != clXFrequencies.GetSize()) {
        clXFrequencies = clMemory<T, Manual>(ctx, rs);
        clYFrequencies = clMemory<T, Manual>(ctx, rs);
        clPropagator = clMemory<std::complex<T>, Manual>(ctx, rs * rs);
        clPotential = clMemory<std::complex<T>, Manual>(ctx, rs * rs);
        clWaveFunction3 = clMemory<std::complex<T>, Manual>(ctx, rs * rs);

        clWaveFunction1.clear();
        clWaveFunction2.clear();
        clWaveFunction4.clear();

        for (int i = 0; i < sm->getParallelPixels(); ++i) {
            clWaveFunction1.emplace_back(ctx, rs * rs);
            clWaveFunction2.emplace_back(ctx, rs * rs);
            clWaveFunction4.emplace_back(ctx, rs * rs);
        }
    }

    if (sm->getParallelPixels() < clWaveFunction1.size()) {
        clWaveFunction1.resize(sm->getParallelPixels());
        clWaveFunction2.resize(sm->getParallelPixels());
        clWaveFunction4.resize(sm->getParallelPixels());
    } else if (sm->getParallelPixels() > clWaveFunction1.size()) {
        for (int i = 0; i < sm->getParallelPixels() - clWaveFunction1.size(); ++i) {
            clWaveFunction1.emplace_back(ctx, rs * rs);
            clWaveFunction2.emplace_back(ctx, rs * rs);
            clWaveFunction4.emplace_back(ctx, rs * rs);
        }
    }

    // when resolution changes (or if enabled)
    auto sim_mode = sm->getMode();
    if (sim_mode == SimulationMode::CTEM && (sim_mode != last_mode || rs*rs != clImageWaveFunction.GetSize())) {
        clImageWaveFunction = clMemory<std::complex<T>, Manual>(ctx, rs * rs);

        // TODO: I can further split these up, but they aren't a huge issue
        clTempBuffer = clMemory<std::complex<T>, Manual>(ctx, rs * rs);
        clCcdBuffer = clMemory<T, Manual>(ctx, 725);
    }

    if (sim_mode == SimulationMode::STEM && (sim_mode != last_mode || rs*rs != clTDSMaskDiff.GetSize())) {
        clTDSMaskDiff = clMemory<T, Manual>(ctx, rs * rs);
        clReductionBuffer = clMemory<T, Manual>(ctx, rs*rs/256); // STEM only
    }

    // TODO: could clear unneeded buffers when sim type switches, but there aren't many of them... (the main ones are the wavefunction vectors)
}

template <class T>
void SimulationWorker<T>::initialiseKernels() {
    auto sm = job->simManager;

    unsigned int rs = sm->getResolution();
    if (rs != FourierTrans.GetWidth() || rs != FourierTrans.GetHeight())
        FourierTrans = clFourier(ctx, rs, rs);
    
    bool isFull3D = sm->isFull3d();
    if (do_initialise || isFull3D != last_do_3d) {
        if (isFull3D)
            BinnedAtomicPotential = Kernels::opt2source.BuildToKernel(ctx);
        else
            BinnedAtomicPotential = Kernels::conv2source.BuildToKernel(ctx);
    }
    last_do_3d = isFull3D;
    
    if (do_initialise) {
        AtomSort = Kernels::atom_sort.BuildToKernel(ctx);
        fftShift = Kernels::fftShiftSource.BuildToKernel(ctx);
        BandLimit = Kernels::BandLimitSource.BuildToKernel(ctx);
        GeneratePropagator = Kernels::propsource.BuildToKernel(ctx);
        ComplexMultiply = Kernels::multisource.BuildToKernel(ctx);
        InitPlaneWavefunction = Kernels::InitialiseWavefunctionSource.BuildToKernel(ctx);
        ImagingKernel = Kernels::imagingKernelSource.BuildToKernel(ctx);
        ABS2 = Kernels::SqAbsSource.BuildToKernel(ctx);
        InitProbeWavefunction = Kernels::InitialiseSTEMWavefunctionSourceTest.BuildToKernel(ctx);
        SumReduction = Kernels::floatSumReductionsource2.BuildToKernel(ctx);
        TDSMaskingAbsKernel = Kernels::floatabsbandPassSource.BuildToKernel(ctx);
        NtfKernel = Kernels::NtfSource.BuildToKernel(ctx);
        DqeKernel = Kernels::DqeSource.BuildToKernel(ctx);
    }

    do_initialise = false;
}

template class SimulationWorker<float>;
template class SimulationWorker<double>;
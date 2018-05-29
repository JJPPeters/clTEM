//
// Created by jon on 23/11/17.
//

#include "simulationworker.h"
#include "structure/structureparameters.h"
#include <utilities/stringutils.h>
#include <utilities/fileio.h>
#include "kernels.h"
#include "ccdparams.h"

void SimulationWorker::Run(std::shared_ptr<SimulationJob> _job)
{
    if (pool.stop)
        return;

    // here is where the simulation gubbins happens
    // Or, in the words of Adam Dyson, this is where the magic happens :)

    job = _job;

    if (!job->simManager)
        throw std::runtime_error("Cannot access simulation parameters");

    uploadParameters(job->simManager->getStructureParameters());

    // now what we do depends on the simulation type (I think...)
    auto mode = job->simManager->getMode();

    if (mode == SimulationMode::CTEM)
        doCtem();
    else if (mode == SimulationMode::CBED)
        doCbed();
    else if (mode == SimulationMode::STEM)
        doStem();

    // TODO: need to look at the old simulation to see what needs to be done now.
    // TODO: look into how/why I used my inheritance tree thing before...

    // finally, end this thread ?
    job->promise.set_value();
}

void SimulationWorker::uploadParameters(std::vector<float> param)
{
    ClParameterisation = ctx.CreateBuffer<float, Manual>(param.size());
    ClParameterisation->Write(param);
}

void SimulationWorker::sortAtoms(bool doTds)
{
    auto atoms = job->simManager->getStructure()->getAtoms();
    unsigned int atom_count = (unsigned int) atoms.size(); // Needs to be cast to int as opencl kernel expects that size

    std::vector<int> AtomANum(atom_count);
    std::vector<float> AtomXPos(atom_count);
    std::vector<float> AtomYPos(atom_count);
    std::vector<float> AtomZPos(atom_count);

    for(int i = 0; i < atom_count; i++)
    {
        float dx = 0, dy = 0, dz = 0;
        if (doTds)
        {
            // TODO: need a log guard here or in the structure file...
            dx = job->simManager->getStructure()->generateTdsFactor(atoms[i].ux);
            dy = job->simManager->getStructure()->generateTdsFactor(atoms[i].uy);
            dz = job->simManager->getStructure()->generateTdsFactor(atoms[i].uz);
        }

        AtomANum[i] = atoms[i].A;
        AtomXPos[i] = atoms[i].x + dx;
        AtomYPos[i] = atoms[i].y + dy;
        AtomZPos[i] = atoms[i].z + dz;
    }

    ClAtomA = ctx.CreateBuffer<int, Manual>(atom_count);
    ClAtomX = ctx.CreateBuffer<float, Manual>(atom_count);
    ClAtomY = ctx.CreateBuffer<float, Manual>(atom_count);
    ClAtomZ = ctx.CreateBuffer<float, Manual>(atom_count);

    ClBlockIds = ctx.CreateBuffer<int,Manual>(atom_count);
    ClZIds = ctx.CreateBuffer<int,Manual>(atom_count);

    ClAtomX->Write(AtomXPos);
    ClAtomY->Write(AtomYPos);
    ClAtomZ->Write(AtomZPos);
    ClAtomA->Write(AtomANum);

    // Make Kernel and set parameters
    clKernel clAtomSort = clKernel(ctx, Kernels::atom_sort.c_str(), 16, "clAtomSort");

    // NOTE: DONT CHANGE UNLESS CHANGE ELSEWHERE ASWELL!
    // Or fix it so they are all referencing same variable.
    // TODO: check if the blocks ever get changed, they always seem to just be 80???
    // TODO: get these from the structure class to keep them centralised
    int BlocksX = job->simManager->getBlocksX();
    int BlocksY = job->simManager->getBlocksY();
    std::valarray<float> x_lims = job->simManager->getPaddedSimLimitsX(); // is this the right padding?
    std::valarray<float> y_lims = job->simManager->getPaddedSimLimitsY();
    std::valarray<float> z_lims = job->simManager->getPaddedStructLimitsZ();

    float dz = job->simManager->getSliceThickness();
    numberOfSlices = job->simManager->getNumberofSlices();

    clAtomSort.SetArg(0, ClAtomX, ArgumentType::Input);
    clAtomSort.SetArg(1, ClAtomY, ArgumentType::Input);
    clAtomSort.SetArg(2, ClAtomZ, ArgumentType::Input);
    clAtomSort.SetArg(3, atom_count);
    clAtomSort.SetArg(4, x_lims[0]);
    clAtomSort.SetArg(5, x_lims[1]);
    clAtomSort.SetArg(6, y_lims[0]);
    clAtomSort.SetArg(7, y_lims[1]);
    clAtomSort.SetArg(8, z_lims[0]);
    clAtomSort.SetArg(9, z_lims[1]);
    clAtomSort.SetArg(10, BlocksX);
    clAtomSort.SetArg(11, BlocksY);
    clAtomSort.SetArg(12, ClBlockIds, ArgumentType::Output);
    clAtomSort.SetArg(13, ClZIds, ArgumentType::Output);
    clAtomSort.SetArg(14, dz);
    clAtomSort.SetArg(15, numberOfSlices);

    clWorkGroup SortSize(atom_count,1,1);
    clAtomSort(SortSize);

    ctx.WaitForQueueFinish(); // test

    std::vector<int> HostBlockIDs = ClBlockIds->CreateLocalCopy();
    std::vector<int> HostZIDs = ClZIds->CreateLocalCopy();

    std::vector<std::vector<std::vector<float>>> Binnedx((unsigned long) BlocksX*BlocksY);
    std::vector<std::vector<std::vector<float>>> Binnedy((unsigned long) BlocksX*BlocksY);
    std::vector<std::vector<std::vector<float>>> Binnedz((unsigned long) BlocksX*BlocksY);
    std::vector<std::vector<std::vector<int>>> BinnedA((unsigned long) BlocksX*BlocksY);

    for(int i = 0 ; i < BlocksX*BlocksY ; i++)
    {
        Binnedx[i].resize(numberOfSlices);
        Binnedy[i].resize(numberOfSlices);
        Binnedz[i].resize(numberOfSlices);
        BinnedA[i].resize(numberOfSlices);
    }

    int count_in_range = 0;
    for(int i = 0; i < atom_count; i++)
    {
        if (HostZIDs[i] > 0 && HostBlockIDs[i] > 0) {
            Binnedx[HostBlockIDs[i]][HostZIDs[i]].push_back(AtomXPos[i]);
            Binnedy[HostBlockIDs[i]][HostZIDs[i]].push_back(AtomYPos[i]);
            Binnedz[HostBlockIDs[i]][HostZIDs[i]].push_back(AtomZPos[i]);
            BinnedA[HostBlockIDs[i]][HostZIDs[i]].push_back(AtomANum[i]);
            ++count_in_range;
        }
    }

    unsigned long long max_bin_xy = 0;
    unsigned long long max_bin_z = 0;

    for (int i = 0; i < Binnedx.size(); ++i)
    {
        if (Binnedx[i].size() > max_bin_xy)
            max_bin_xy = Binnedx[i].size();

        for (int j = 0; j < Binnedx[i].size(); ++j)
            if (Binnedx[i][j].size() > max_bin_z)
                max_bin_z = Binnedx[i][j].size();
    }

    int atomIterator = 0;

    std::vector<int> blockStartPositions(numberOfSlices*BlocksX*BlocksY+1);

    // Put all bins into a linear block of memory ordered by z then y then x and record start positions for every block.

    for(int slicei = 0; slicei < numberOfSlices; slicei++)
    {
        for(int j = 0; j < BlocksY; j++)
        {
            for(int k = 0; k < BlocksX; k++)
            {
                blockStartPositions[slicei*BlocksX*BlocksY+ j*BlocksX + k] = atomIterator;

                if(Binnedx[j*BlocksX+k][slicei].size() > 0)
                {
                    for(int l = 0; l < Binnedx[j*BlocksX+k][slicei].size(); l++)
                    {
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

    ClBlockStartPositions = ctx.CreateBuffer<int, Manual>(numberOfSlices * BlocksX * BlocksY + 1);

    // Now upload the sorted atoms onto the device..
    ClAtomX->Write(AtomXPos);
    ClAtomY->Write(AtomYPos);
    ClAtomZ->Write(AtomZPos);
    ClAtomA->Write(AtomANum);

    ClBlockStartPositions->Write(blockStartPositions);

    // wait for the IO queue here so that we are sure the data is uploaded before we start usign it
    ctx.WaitForIOQueueFinish();
}

void SimulationWorker::doCtem(bool simImage)
{
    // increase the paddingso the structure starts at an integer number of pixels
    job->simManager->round_padding();

    // sort structure, TDS is always false so leave blank
    sortAtoms();

    initialiseCtem();

    // loop through slices
    for (int i = 0; i < numberOfSlices; ++i)
    {
        doMultiSliceStep(i);
        if (pool.stop)
            return;
        job->simManager->reportSliceProgress(((float)i+1) / (float)numberOfSlices);
    }

    if (simImage)
        simulateCtemImage();

    int resolution = job->simManager->getResolution();
    typedef std::map<std::string, Image<float>> return_map;
    return_map Images;

    float real_scale = job->simManager->getRealScale();

    // this is easy as we have set it
    int crop_l = (int) (std::abs(job->simManager->getPaddingX()[0]) / real_scale);
    int crop_b = (int) (std::abs(job->simManager->getPaddingY()[0]) / real_scale);

    // the other sides are a bit harder as they may be padded to match the other dimension
    //find larger dimension
    int crop_t = 0;
    int crop_r = 0;

    // this is slightly useless as we have limited the simulation to be square, but this will be useful if that changes
    auto ranges = job->simManager->getSimRanges();
    if (std::get<0>(ranges) == std::get<1>(ranges))
    {
        crop_t = crop_b;
        crop_r = crop_l;
    }
    else if (std::get<0>(ranges) > std::get<1>(ranges))
    {
        crop_r = crop_l;
        crop_t = (int) std::floor((std::get<0>(ranges) - std::get<1>(ranges)) / real_scale) + crop_b;
    }
    else
    {
        crop_t = crop_b;
        crop_r = (int) std::floor((std::get<1>(ranges) - std::get<0>(ranges)) / real_scale) + crop_l;
    }

    auto ew_a = Image<float>(resolution, resolution, getExitWaveAmplitudeImage(), crop_t, crop_l, crop_b, crop_r);
    auto ew_p = Image<float>(resolution, resolution, getExitWavePhaseImage(), crop_t, crop_l, crop_b, crop_r);
    auto diff = Image<float>(resolution, resolution, getDiffractionImage());

    // get the images we need
    Images.insert(return_map::value_type("EW_A", ew_a));
    Images.insert(return_map::value_type("EW_T", ew_p));
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

            simulateCtemImage(dqe, ntf, binning, dose_per_pix);
        }
        else {
            simulateCtemImage();
        }

        auto ctem_im = Image<float>(resolution, resolution, getCtemImage(), crop_t, crop_l, crop_b, crop_r);
        Images.insert(return_map::value_type("Image", ctem_im));
    }

    job->simManager->updateImages(Images, 1);
}

void SimulationWorker::doCbed()
{
    // reset the padding in case it has been changed before
    job->simManager->round_padding();

    sortAtoms(job->simManager->getTdsRuns() > 1);

    initialiseCbed();

    auto pos = job->simManager->getCBedPosition();

    initialiseProbeWave(pos->getXPos(), pos->getYPos());

    // loop through slices
    for (int i = 0; i < numberOfSlices; ++i)
    {
        doMultiSliceStep(i);
        if (pool.stop)
            return;
        job->simManager->reportSliceProgress(((float)i+1) / (float)numberOfSlices);
    }

    // get images and return them...
    int resolution = job->simManager->getResolution();
    typedef std::map<std::string, Image<float>> return_map;
    return_map Images;

    auto diff = Image<float>(resolution, resolution, getDiffractionImage());

    Images.insert(return_map::value_type("Diff", diff));

    job->simManager->updateImages(Images, 1); // TODO: update this if we ever do more than one TDS in a job
}

void SimulationWorker::doStem()
{
    // reset the padding in case it has been changed before
    job->simManager->round_padding();

    sortAtoms(job->simManager->getTdsRuns() > 1);

    initialiseStem();

    // now need to work out where our probes need to be made
    auto stemPixels = job->simManager->getStemArea();

    // get start position and the pixel step
    float start_x = stemPixels->getLimitsX()[0];
    float start_y = stemPixels->getLimitsY()[0];

    int num_x = stemPixels->getPixelsX();

    float step_x = stemPixels->getScaleX();
    float step_y = stemPixels->getScaleY();

    for (int i = 0; i < job->pixels.size(); ++i)
    {
        int p = job->pixels[i];
        // index to x and y index
        int x_i = p % num_x;
        int y_i = (int) std::floor(p / num_x);

        float x_pos = start_x + x_i * step_x;
        float y_pos = start_y + y_i * step_y;

        initialiseProbeWave(x_pos, y_pos, i);
    }

    // loop through slices
    for (int i = 0; i < numberOfSlices; ++i)
    {
        doMultiSliceStep(i);
        if (pool.stop)
            return;
        job->simManager->reportSliceProgress(((float)i+1) / (float)numberOfSlices);
    }

    typedef std::map<std::string, Image<float>> return_map;
    return_map Images;
    int px_x = job->simManager->getStemArea()->getPixelsX();
    int px_y = job->simManager->getStemArea()->getPixelsY();

    for (auto det : job->simManager->getDetectors())
    {
        std::vector<float> im(stemPixels->getNumPixels(), 0.0f);

        for (int i = 0; i < job->pixels.size(); ++i)
        {
            im[job->pixels[i]] = getStemPixel(det.inner, det.outer, det.xcentre, det.ycentre, i);
        }

        Images[det.name] = Image<float>(px_x, px_y, im);
    }

    job->simManager->updateImages(Images, 1); // TODO: check we only do one TDS config per job
}

void SimulationWorker::initialiseSimulation()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Get local copies of variables (for convenience)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool isFD = job->simManager->isFiniteDifference();
    bool isFull3D = job->simManager->isFull3d();
    int resolution = job->simManager->getResolution();
    float wavelength = job->simManager->getWavelength();
    float pixelscale = job->simManager->getRealScale();
    auto mParams = job->simManager->getMicroscopeParams();
    float startx = job->simManager->getPaddedSimLimitsX()[0];
    float starty = job->simManager->getPaddedSimLimitsY()[0];
    int full3dints = job->simManager->getFull3dInts();

    // Work out area that is to be simulated (in real space)
    float SimSizeX = pixelscale * resolution;
    float SimSizeY = SimSizeX;

    float sigma = mParams->Sigma();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up Finite Difference specific stuff
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // some things later will depend on member variables that are set in this method
    if(isFD)
        initialiseFiniteDifferenceSimulation();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up our frequency calibrations
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // This basically is all to create OpenCL buffers (1D) that let us know the frequency value of the pixels in the FFT
    // Not that this already accounts for the un-shifted nature of the FFT (i.e. 0 frequency is at 0, 0)
    // We also calculate our limit for low pass filtering the wavefunctions
    std::vector<float> k0x(resolution);
    std::vector<float> k0y(resolution);

    int imidx = (int) std::floor(resolution / 2 + 0.5);
    int imidy = (int) std::floor(resolution / 2 + 0.5);

    float temp = 0.0f;

    for (int i = 0; i < resolution; i++)
    {
        if (i >= imidx)
            temp = (i - resolution) / SimSizeX;
        else
            temp = i / SimSizeX;
        k0x[i] = temp;
    }

    for (int i = 0; i < resolution; i++)
    {
        if (i >= imidy)
            temp = (i - resolution) / SimSizeY;
        else
            temp = i / SimSizeY;
        k0y[i] = temp;
    }

    // Find maximum frequency for bandwidth limiting rule
    float bandwidthkmax = 0.0f;

    // TODO: not sure I want the -1 here
    float kmaxx = (float) std::pow((k0x[imidx - 1]), 2);
    float kmaxy = (float) std::pow((k0y[imidy - 1]), 2);

    // we are only dealing with squares so far, so this accounts for that
    // I don't think it is really necessary, as the resolution is always the same and the pixelscale already
    // accounts for the image padding (?)
    if (kmaxy < kmaxx)
        bandwidthkmax = kmaxy;
    else
        bandwidthkmax = kmaxx;

    // k not k^2.
    // previously had limited to 1/2, but Kirkland pg 159 says 2/3
    // Kirkland's code seems to use 1/2 however...
    bandwidthkmax = std::sqrt(bandwidthkmax) * job->simManager->getInverseLimitFactor();

    // write our frequencies to OpenCL buffers
    clXFrequencies = ctx.CreateBuffer<float, Manual>(resolution);
    clYFrequencies = ctx.CreateBuffer<float, Manual>(resolution);
    clXFrequencies->Write(k0x);
    clYFrequencies->Write(k0y);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create a few buffers we will need later
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    clPropagator = ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution);
    clPotential = ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution);

    clWorkGroup WorkSize(resolution, resolution, 1);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Setup Fourier Transforms, this is sort of obvious
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FourierTrans = clFourier(ctx, resolution, resolution);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up FFT shift kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    fftShift = clKernel(ctx, Kernels::fftShiftSource.c_str(), 4, "clfftShift");

    // these will never change, so set them here
    fftShift.SetArg(0, clWaveFunction2[0], ArgumentType::Input);
    fftShift.SetArg(1, clWaveFunction3, ArgumentType::Output);
    fftShift.SetArg(2, resolution);
    fftShift.SetArg(3, resolution);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up low pass filter kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BandLimit = clKernel(ctx, Kernels::BandLimitSource.c_str(), 6, "clBandLimit");

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
    // have various options depending on the user preferences
    if (isFull3D)
        BinnedAtomicPotential = clKernel(ctx, Kernels::opt2source.c_str(), 27, "clBinnedAtomicPotentialOpt");
    else if (isFD)
        BinnedAtomicPotential = clKernel(ctx, Kernels::fd2source.c_str(), 26, "clBinnedAtomicPotentialOptFD");
    else
        BinnedAtomicPotential = clKernel(ctx, Kernels::conv2source.c_str(), 26, "clBinnedAtomicPotentialConventional");

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
    BinnedAtomicPotential.SetArg(7, resolution);
    BinnedAtomicPotential.SetArg(8, resolution);
    BinnedAtomicPotential.SetArg(12, dz);
    BinnedAtomicPotential.SetArg(13, pixelscale);
    BinnedAtomicPotential.SetArg(14, job->simManager->getBlocksX());
    BinnedAtomicPotential.SetArg(15, job->simManager->getBlocksY());
    BinnedAtomicPotential.SetArg(16, job->simManager->getPaddedSimLimitsX()[1]);
    BinnedAtomicPotential.SetArg(17, job->simManager->getPaddedSimLimitsX()[0]);
    BinnedAtomicPotential.SetArg(18, job->simManager->getPaddedSimLimitsY()[1]);
    BinnedAtomicPotential.SetArg(19, job->simManager->getPaddedSimLimitsY()[0]);
    BinnedAtomicPotential.SetArg(20, load_blocks_x);
    BinnedAtomicPotential.SetArg(21, load_blocks_y);
    BinnedAtomicPotential.SetArg(22, load_blocks_z);
    BinnedAtomicPotential.SetArg(23, sigma); // Not sure why I am using this sigma and not commented sigma...
    BinnedAtomicPotential.SetArg(24, startx);
    BinnedAtomicPotential.SetArg(25, starty);
    if (isFull3D)
        BinnedAtomicPotential.SetArg(26, full3dints);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up the propogator
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    GeneratePropagator = clKernel(ctx, Kernels::propsource.c_str(), 8, "clGeneratePropagator");

    GeneratePropagator.SetArg(0, clPropagator, ArgumentType::Output);
    GeneratePropagator.SetArg(1, clXFrequencies, ArgumentType::Input);
    GeneratePropagator.SetArg(2, clYFrequencies, ArgumentType::Input);
    GeneratePropagator.SetArg(3, resolution);
    GeneratePropagator.SetArg(4, resolution);
//    if (isFD)
//        GeneratePropagator.SetArg(5, FDdz); // Is this the right dz? (Propagator needs slice thickness not spacing between atom bins)
//    else
    GeneratePropagator.SetArg(5, dz); // Is this the right dz? (Propagator needs slice thickness not spacing between atom bins)
    GeneratePropagator.SetArg(6, wavelength);
    GeneratePropagator.SetArg(7, bandwidthkmax);

    // actually run this kernel now
    GeneratePropagator(WorkSize);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up complex multiply kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ComplexMultiply = clKernel(ctx, Kernels::multisource.c_str(), 5, "clComplexMultiply");

    ComplexMultiply.SetArg(3, resolution);
    ComplexMultiply.SetArg(4, resolution);
}

void SimulationWorker::initialiseCtem()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create local variables for convenience
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int resolution = job->simManager->getResolution();
    bool isFD = job->simManager->isFiniteDifference();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create buffers for the main simulation
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Initialise Wavefunctions and create other OpenCL things
    // Note that for CTEM, the vectors are not really needed, but use them for compatibility
    clWorkGroup WorkSize(resolution, resolution, 1);

    clWaveFunction1.push_back(ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution));
    clWaveFunction2.push_back(ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution));
    clWaveFunction3 = ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution);
    clWaveFunction4.push_back(ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution));

    if (isFD)
    {
        clWaveFunction1Minus.push_back(ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution));
        clWaveFunction1Plus.push_back(ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create plane wave function
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    InitPlaneWavefunction = clKernel(ctx, Kernels::InitialiseWavefunctionSource.c_str(), 4, "clInitialiseWavefunction");

    float InitialValue = 1.0f;
    InitPlaneWavefunction.SetArg(1, resolution);
    InitPlaneWavefunction.SetArg(2, resolution);
    InitPlaneWavefunction.SetArg(3, InitialValue);
    if (isFD) // TODO: in Adam's code, this get's reset later anyway? (Does it?)
    {
        InitPlaneWavefunction.SetArg(0, clWaveFunction1Minus[0], ArgumentType::Output);
        InitPlaneWavefunction(WorkSize);
    }
    InitPlaneWavefunction.SetArg(0, clWaveFunction1[0], ArgumentType::Output);
    InitPlaneWavefunction(WorkSize);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Build imaging kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // this is only needed if the imaging part is used, but build it here (with it's buffer) to save time later
    clImageWaveFunction = ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution);
    ImagingKernel = clKernel(ctx, Kernels::imagingKernelSource.c_str(), 24, "clImagingKernel");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// initialise everyting else
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    initialiseSimulation();
}

void SimulationWorker::initialiseCbed()
{
    int resolution = job->simManager->getResolution();
    int n_parallel = job->simManager->getParallelPixels(); // this is the number of parallel pixels

    // Initialise Wavefunctions and Create other buffers...
    // Even though CBED only eer has 1 parallel simulation (per device), this set up is also used for STEM
    for (int i = 1; i <= n_parallel; i++)
    {
        clWaveFunction1.push_back(ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution));
        clWaveFunction2.push_back(ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution));
        clWaveFunction4.push_back(ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution));

        if (job->simManager->isFiniteDifference())
        {
            clWaveFunction1Minus.push_back(ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution));
            clWaveFunction1Plus.push_back(ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution));
        }
    }
    clWaveFunction3 = ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution);

    clTDSMaskDiff = ctx.CreateBuffer<cl_float, Manual>(resolution*resolution);

    InitProbeWavefunction = clKernel(ctx, Kernels::InitialiseSTEMWavefunctionSourceTest.c_str(), 24, "clInitialiseSTEMWavefunction");

    initialiseSimulation();
}

void SimulationWorker::initialiseStem()
{
    SumReduction = clKernel(ctx, Kernels::floatSumReductionsource2.c_str(), 4, "clSumReduction");
    TDSMaskingAbsKernel = clKernel(ctx, Kernels::floatabsbandPassSource.c_str(), 8, "clFloatBandPass");

    initialiseCbed();
}

// n_parallel is the index (from 0) of the current parallel pixel
void SimulationWorker::initialiseProbeWave(float posx, float posy, int n_parallel)
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create local variables for convenience
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int resolution = job->simManager->getResolution();
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

    InitProbeWavefunction(WorkSize);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IFFT probe to real space
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // IFFT
    FourierTrans.Do(clWaveFunction2[n_parallel], clWaveFunction1[n_parallel], Direction::Inverse);

    // copy to second wave function used for finite difference
    if (job->simManager->isFiniteDifference())
        clEnqueueCopyBuffer(ctx.GetIOQueue(), clWaveFunction1[n_parallel]->GetBuffer(), clWaveFunction1Minus[n_parallel]->GetBuffer(), 0, 0, resolution*resolution*sizeof(cl_float2), 0, 0, 0);
}

void SimulationWorker::doMultiSliceStep(int slice)
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// do finite difference method (if selected)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (job->simManager->isFiniteDifference())
    {
        doMultiSliceStepFiniteDiff(slice);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create local variables for convenience
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float dz = job->simManager->getSliceThickness();
    int resolution = job->simManager->getResolution();
    // in the current format, Tds is handled by job splitting so this is always 1??
    int n_parallel = job->simManager->getParallelPixels(); // total number of parallel pixels
    auto z_lim = job->simManager->getPaddedStructLimitsZ();

    // Didn't have MinimumZ so it wasnt correctly rescaled z-axis from 0 to SizeZ...
    float currentz = z_lim[1] - slice * dz;

    clWorkGroup Work((unsigned int) resolution, (unsigned int) resolution, 1);
    clWorkGroup LocalWork(16, 16, 1);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Get our potentials for the current sim
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ctx.WaitForQueueFinish();

    BinnedAtomicPotential.SetArg(1, ClAtomX, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(2, ClAtomY, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(3, ClAtomZ, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(4, ClAtomA, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(6, ClBlockStartPositions, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(9, slice);
    BinnedAtomicPotential.SetArg(10, numberOfSlices);
    BinnedAtomicPotential.SetArg(11, currentz);

    BinnedAtomicPotential(Work, LocalWork);

    ctx.WaitForQueueFinish();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Apply low pass filter to potentials
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FourierTrans.Do(clPotential, clWaveFunction3, Direction::Forwards);
    ctx.WaitForQueueFinish();
    BandLimit(Work);
    ctx.WaitForQueueFinish();
    FourierTrans.Do(clWaveFunction3, clPotential, Direction::Inverse);
    ctx.WaitForQueueFinish();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Propogate slice
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for (int i = 1; i <= n_parallel; i++)
    {
        // Apply low pass filter to wavefunction
        FourierTrans.Do(clWaveFunction1[i - 1], clWaveFunction3, Direction::Forwards);
        ctx.WaitForQueueFinish();
        BandLimit(Work);
        ctx.WaitForQueueFinish();
        FourierTrans.Do(clWaveFunction3, clWaveFunction1[i - 1], Direction::Inverse);
        ctx.WaitForQueueFinish();
        
        //Multiply potential with wavefunction
        ComplexMultiply.SetArg(0, clPotential, ArgumentType::Input);
        ComplexMultiply.SetArg(1, clWaveFunction1[i - 1], ArgumentType::Input);
        ComplexMultiply.SetArg(2, clWaveFunction2[i - 1], ArgumentType::Output);
        ComplexMultiply(Work);
        ctx.WaitForQueueFinish();

        // go to reciprocal space
        FourierTrans.Do(clWaveFunction2[i - 1], clWaveFunction3, Direction::Forwards);
        ctx.WaitForQueueFinish();

        // convolve with propagator
        ComplexMultiply.SetArg(0, clWaveFunction3, ArgumentType::Input);
        ComplexMultiply.SetArg(1, clPropagator, ArgumentType::Input);
        ComplexMultiply.SetArg(2, clWaveFunction2[i - 1], ArgumentType::Output);
        ComplexMultiply(Work);
        ctx.WaitForQueueFinish();

        // IFFT back to real space
        FourierTrans.Do(clWaveFunction2[i - 1], clWaveFunction1[i - 1], Direction::Inverse);
        ctx.WaitForQueueFinish();
    }
}

void SimulationWorker::doMultiSliceStepFiniteDiff(int slice)
{
    // TODO: get fdDz properly
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create local variables for convenience
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float dz = job->simManager->getSliceThickness();
    auto z_lim = job->simManager->getPaddedStructLimitsZ();
    int resolution = job->simManager->getResolution();
    int n_parallel = job->simManager->getParallelPixels(); // total number of parallel pixels
    float wavelength = job->simManager->getWavelength();
    float sigma = job->simManager->getMicroscopeParams()->Sigma();

    float currentz = z_lim[1] - slice * dz;

    clWorkGroup Work(resolution, resolution, 1);
    clWorkGroup LocalWork(16, 16, 1);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Get our potentials for the current sim
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    BinnedAtomicPotential.SetArg(1, ClAtomX, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(2, ClAtomY, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(3, ClAtomZ, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(4, ClAtomA, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(6, ClBlockStartPositions, ArgumentType::Input);
    BinnedAtomicPotential.SetArg(9, slice);
    BinnedAtomicPotential.SetArg(10, numberOfSlices);
    BinnedAtomicPotential.SetArg(11, currentz);

    BinnedAtomicPotential(Work, LocalWork);

    ctx.WaitForQueueFinish();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Apply low pass filter to potentials
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FourierTrans(clPotential, clWaveFunction3, Direction::Forwards);
    ctx.WaitForQueueFinish();
    BandLimit(Work);
    ctx.WaitForQueueFinish();
    FourierTrans(clWaveFunction3, clPotential, Direction::Inverse);

    ctx.WaitForQueueFinish();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Propogate slice
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < n_parallel; i++)
    {

        //FFT Psi into Grad2.
        FourierTrans(clWaveFunction1[i], clWaveFunction3, Direction::Forwards);

        ctx.WaitForQueueFinish();

        //Grad Kernel on Grad2.
        GradKernel.SetArg(0, clWaveFunction3, ArgumentType::Input);
        GradKernel.SetArg(1, clXFrequencies, ArgumentType::Input);
        GradKernel.SetArg(2, clYFrequencies, ArgumentType::Input);
        GradKernel.SetArg(3, resolution);
        GradKernel.SetArg(4, resolution);
        GradKernel(Work);

        ctx.WaitForQueueFinish();

        //IFT Grad2 into Grad.
        FourierTrans(clWaveFunction3, clWaveFunction4[i], Direction::Inverse);

        ctx.WaitForQueueFinish();

        //FD Kernel
        FiniteDifference.SetArg(0, clPotential, ArgumentType::Input);
        FiniteDifference.SetArg(1, clWaveFunction4[i], ArgumentType::Input);
        FiniteDifference.SetArg(2, clWaveFunction1Minus[i], ArgumentType::Input);
        FiniteDifference.SetArg(3, clWaveFunction1[i], ArgumentType::Input);
        FiniteDifference.SetArg(4, clWaveFunction1Plus[i], ArgumentType::Output);
        FiniteDifference.SetArg(5, dz);
        FiniteDifference.SetArg(6, wavelength);
        FiniteDifference.SetArg(7, sigma);
        FiniteDifference.SetArg(8, resolution);
        FiniteDifference.SetArg(9, resolution);
        FiniteDifference(Work);

        ctx.WaitForQueueFinish();

        //Bandlimit PsiPlus
        FourierTrans(clWaveFunction1Plus[i], clWaveFunction3, Direction::Forwards);
        ctx.WaitForQueueFinish();
        BandLimit(Work);
        ctx.WaitForQueueFinish();
        FourierTrans(clWaveFunction3, clWaveFunction1Plus[i], Direction::Inverse);

        ctx.WaitForQueueFinish();

        // Psi becomes PsiMinus
        clEnqueueCopyBuffer(ctx.GetIOQueue(), clWaveFunction1[i]->GetBuffer(), clWaveFunction1Minus[i]->GetBuffer(), 0, 0, resolution*resolution*sizeof(cl_float2), 0, nullptr, nullptr);

        ctx.WaitForQueueFinish();

        // PsiPlus becomes Psi.
        clEnqueueCopyBuffer(ctx.GetIOQueue(), clWaveFunction1Plus[i]->GetBuffer(), clWaveFunction1[i]->GetBuffer(), 0, 0, resolution*resolution*sizeof(cl_float2), 0, nullptr, nullptr);

        // To maintain status with other versions resulting end arrays should still be as follows.
        // Finished wavefunction in real space in clWaveFunction1.
        // Finished wavefunction in reciprocal space in clWaveFunction2.
        // 3 and 4 were previously temporary.
        ctx.WaitForQueueFinish();

        FourierTrans(clWaveFunction1[i], clWaveFunction2[i], Direction::Forwards);

        ctx.WaitForQueueFinish();
    }

    ctx.WaitForQueueFinish();
}

void SimulationWorker::simulateCtemImage()
{
    int resolution = job->simManager->getResolution();
    float wavelength = job->simManager->getWavelength();
    auto mParams = job->simManager->getMicroscopeParams();

    clKernel ABS2 = clKernel(ctx, Kernels::SqAbsSource.c_str(), 4, "clSqAbs");

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

    ImagingKernel(Work);

    // Now get and display absolute value
    FourierTrans.Do(clImageWaveFunction, clWaveFunction1[0], Direction::Inverse);

    ABS2.SetArg(0, clWaveFunction1[0], ArgumentType::Input);
    ABS2.SetArg(1, clImageWaveFunction, ArgumentType::Output);
    ABS2.SetArg(2, resolution);
    ABS2.SetArg(3, resolution);
    ABS2(Work);
};

// TODO: what should be done with the conversion factor?
// I think it might be like an amplification thing - as in if the detector gets n electrons, it will 'detect' n*conversion factor?
void SimulationWorker::simulateCtemImage(std::vector<float> dqe_data, std::vector<float> ntf_data, int binning,
                                         float doseperpix,
                                         float conversionfactor)
{
    // all the NTF, DQE stuff can be found here: 10.1016/j.jsb.2013.05.008

    int resolution = job->simManager->getResolution();
    float wavelength = job->simManager->getWavelength();
    auto mParams = job->simManager->getMicroscopeParams();

    // Set up some temporary memory objects for the image simulation
    auto Temp1 = ctx.CreateBuffer<cl_float2, Manual>(resolution*resolution);
    auto dqe_ntf_buffer = ctx.CreateBuffer<cl_float, Manual>(725);

    // build additional kernels required
    clKernel NTF = clKernel(ctx, Kernels::NtfSource.c_str(), 5, "clNTF");
    clKernel DQE = clKernel(ctx, Kernels::DqeSource.c_str(), 5, "clDQE");
    clKernel ABS = clKernel(ctx, Kernels::AbsSource.c_str(), 3, "clAbs");
    clKernel ABS2 = clKernel(ctx, Kernels::SqAbsSource.c_str(), 4, "clSqAbs");

    // simulate image
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
    ImagingKernel.SetArg(22, mParams->Alpha);
    ImagingKernel.SetArg(23, mParams->Delta);

    clWorkGroup Work(resolution, resolution, 1);

    ImagingKernel(Work);

    ctx.WaitForQueueFinish();

    // ifft to real space
    FourierTrans.Do(clImageWaveFunction, clWaveFunction1[0], Direction::Inverse);

    ctx.WaitForQueueFinish();

    //abs for detected image
    ABS2.SetArg(0, clWaveFunction1[0], ArgumentType::Input);
    ABS2.SetArg(1, Temp1, ArgumentType::Output);
    ABS2.SetArg(2, resolution);
    ABS2.SetArg(3, resolution);

    ABS2(Work);

    ctx.WaitForQueueFinish();

    //
    // Dose stuff starts here!
    //

    // FFT
    FourierTrans(Temp1, clImageWaveFunction, Direction::Forwards);

    ctx.WaitForQueueFinish();

    // write DQE to opencl
    dqe_ntf_buffer->Write(dqe_data);
    ctx.WaitForQueueFinish();

    // apply DQE
    DQE.SetArg(0, clImageWaveFunction, ArgumentType::InputOutput);
    DQE.SetArg(1, dqe_ntf_buffer, ArgumentType::Input);
    DQE.SetArg(2, resolution);
    DQE.SetArg(3, resolution);
    DQE.SetArg(4, binning);

    DQE(Work);

    ctx.WaitForQueueFinish();

    // IFFT back
    FourierTrans(clImageWaveFunction, Temp1, Direction::Inverse);

    ctx.WaitForQueueFinish();

    // TODO: what is this abs for? (or should it be abs squared?)
//    ABS2.SetArg(0, Temp1, ArgumentType::Input);
//    ABS2.SetArg(1, clWaveFunction1[0], ArgumentType::Output);
//    ABS2.SetArg(2, resolution);
//    ABS2.SetArg(3, resolution);
//
//    ABS2(Work);
//
//    ctx.WaitForQueueFinish();

    float N_tot = doseperpix * binning * binning; // Get this passed in, its dose per binned pixel i think.

    ctx.WaitForQueueFinish();
    std::vector<cl_float2> compdata = Temp1->CreateLocalCopy();
    ctx.WaitForQueueFinish();

    float fmin = std::numeric_limits<float>::min();

    std::random_device rd;
    std::mt19937 rng(rd());
//    std::normal_distribution<> dist(0, 1); // random dist with (mean, stddev)

    for (int i = 0; i < resolution * resolution; i++)
    {
        // previously was using a Box-Muller transform to get a normal dist and assuming it would approximate a poisson distribution
        // see: https://stackoverflow.com/questions/19944111/creating-a-gaussian-random-generator-with-a-mean-and-standard-deviation

        // use the built in stuff for ease
        std::poisson_distribution<int> dist(N_tot * compdata[i].x); // TODO: here we are assuming this funtion is only real?
        float poiss = (float) dist(rng);

        compdata[i].x = conversionfactor * poiss;
        compdata[i].y = 0;
    }

    clImageWaveFunction->Write(compdata);
    ctx.WaitForQueueFinish();

    FourierTrans(clImageWaveFunction, Temp1, Direction::Forwards);

    ctx.WaitForQueueFinish();

    dqe_ntf_buffer->Write(ntf_data);

    ctx.WaitForQueueFinish();

    NTF.SetArg(0, Temp1, ArgumentType::InputOutput);
    NTF.SetArg(1, dqe_ntf_buffer, ArgumentType::Input);
    NTF.SetArg(2, resolution);
    NTF.SetArg(3, resolution);
    NTF.SetArg(4, binning);

    NTF(Work);

    ctx.WaitForQueueFinish();

    FourierTrans(Temp1, clImageWaveFunction, Direction::Inverse);

    ctx.WaitForQueueFinish();

//    // might want to be sqrt (aka normal abs)
//    ABS2.SetArg(0, clWaveFunction1[0], ArgumentType::Input);
//    ABS2.SetArg(1, clImageWaveFunction, ArgumentType::Output);
//    ABS2.SetArg(2, resolution);
//    ABS2.SetArg(3, resolution);
//
//    ABS2(Work);
//
//    ctx.WaitForQueueFinish();
}

std::vector<float> SimulationWorker::getDiffractionImage(int parallel_ind)
{
    int resolution = job->simManager->getResolution();
    std::vector<float> data_out(resolution*resolution);

    // Original data is complex so copy complex version down first
    clWorkGroup Work(resolution, resolution, 1);

    fftShift.SetArg(0, clWaveFunction2[parallel_ind], ArgumentType::Input);
    fftShift(Work);

//    ctx.WaitForQueueFinish();

    std::vector<cl_float2> compdata = clWaveFunction3->CreateLocalCopy();

    for (int i = 0; i < resolution * resolution; i++)
        // Get absolute value for display...
        data_out[i] += (compdata[i].s[0] * compdata[i].s[0] + compdata[i].s[1] * compdata[i].s[1]);

    return data_out;
}

std::vector<float> SimulationWorker::getExitWaveAmplitudeImage(int t, int l, int b, int r)
{
    int resolution = job->simManager->getResolution();
    std::vector<float> data_out((resolution - t - b) * (resolution - l - r));

    std::vector<cl_float2> compdata = clWaveFunction1[0]->CreateLocalCopy();

    int cnt = 0;
    for (int j = 0; j < resolution; ++j)
        if (j >= b && j < (resolution - t))
            for (int i = 0; i < resolution; ++i)
                if (i >= l && i < (resolution - r))
                {
                    int k = i + j * resolution;
                    data_out[cnt] = std::sqrt(compdata[k].x * compdata[k].x + compdata[k].y * compdata[k].y);
                    ++cnt;
                }


    return data_out;
}

std::vector<float> SimulationWorker::getExitWavePhaseImage(int t, int l, int b, int r)
{
    int resolution = job->simManager->getResolution();
    std::vector<float> data_out((resolution - t - b) * (resolution - l - r));

    std::vector<cl_float2> compdata = clWaveFunction1[0]->CreateLocalCopy();

//    for (int i = 0; i < resolution * resolution; i++)
//        data_out[i] = std::atan2(compdata[i].s[1], compdata[i].s[0]);
    int cnt = 0;
    for (int j = 0; j < resolution; ++j)
        if (j >= b && j < (resolution - t))
            for (int i = 0; i < resolution; ++i)
                if (i >= l && i < (resolution - r))
                {
                    int k = i + j * resolution;
                    data_out[cnt] = std::atan2(compdata[k].y, compdata[k].x);
                    ++cnt;
                }

    return data_out;
}

std::vector<float> SimulationWorker::getCtemImage()
{
    int resolution = job->simManager->getResolution();
    std::vector<float> data_out(resolution*resolution);

    // Original data is complex so copy complex version down first
    std::vector<cl_float2> compdata = clImageWaveFunction->CreateLocalCopy();

    ctx.WaitForQueueFinish();

    for (int i = 0; i < resolution * resolution; i++)
        data_out[i] = compdata[i].x; // already abs in simulateCTEM function (but is still 'complex' type?)

    return data_out;
}

float SimulationWorker::doSumReduction(std::shared_ptr<clMemory<float, Manual>> data, clWorkGroup globalSizeSum, clWorkGroup localSizeSum, int nGroups, int totalSize)
{
    auto outArray = ctx.CreateBuffer<float, Manual>(nGroups);

    SumReduction.SetArg(0, data, ArgumentType::Input);

    // Only really need to do these 3 once... (but we make a local 'outArray' so can't do that)
    SumReduction.SetArg(1, outArray);
    SumReduction.SetArg(2, totalSize);
    SumReduction.SetLocalMemoryArg<float>(3, 256);

    SumReduction(globalSizeSum, localSizeSum);

    // Now copy back
    std::vector<float> sums = outArray->CreateLocalCopy();

    // Find out which numbers to read back
    float sum = 0;
    for (int i = 0; i < nGroups; i++)
    {
        sum += sums[i];
    }
    return sum;
}

float SimulationWorker::getStemPixel(float inner, float outer, float xc, float yc, int parallel_ind)
{
    int resolution = job->simManager->getResolution();
    float angle_scale = job->simManager->getInverseScaleAngle();

    clWorkGroup WorkSize(resolution, resolution, 1);

    fftShift.SetArg(0, clWaveFunction2[parallel_ind], ArgumentType::Input);
    fftShift(WorkSize);

    float innerPx = inner / angle_scale;
    float outerPx = outer / angle_scale;

    float xcPx = xc / angle_scale;
    float ycPx = yc / angle_scale;

    TDSMaskingAbsKernel.SetArg(0, clWaveFunction3, ArgumentType::Input);
    TDSMaskingAbsKernel.SetArg(1, clTDSMaskDiff, ArgumentType::Output);
    TDSMaskingAbsKernel.SetArg(2, resolution);
    TDSMaskingAbsKernel.SetArg(3, resolution);
    TDSMaskingAbsKernel.SetArg(4, innerPx);
    TDSMaskingAbsKernel.SetArg(5, outerPx);
    TDSMaskingAbsKernel.SetArg(6, xcPx);
    TDSMaskingAbsKernel.SetArg(7, ycPx);

    TDSMaskingAbsKernel(WorkSize);

    int totalSize = resolution*resolution;
    int nGroups = totalSize / 256;

    clWorkGroup globalSizeSum(totalSize, 1, 1);
    clWorkGroup localSizeSum(256, 1, 1);

    return doSumReduction(clTDSMaskDiff, globalSizeSum, localSizeSum, nGroups, totalSize);
}

void SimulationWorker::initialiseFiniteDifferenceSimulation()
{
    GradKernel = clKernel(ctx, Kernels::gradsource.c_str(), 5, "clGrad");
    FiniteDifference = clKernel(ctx, Kernels::fdsource.c_str(), 10, "clFiniteDifference");
}

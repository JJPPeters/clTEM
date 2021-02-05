//
// Created by Jon on 31/01/2020.
//

#include <utilities/simutils.h>
#include "simulationgeneral.h"

template <class T>
void SimulationGeneral<T>::initialiseBuffers() {

    auto sm = job->simManager;

    // this needs to change if the parameter sizes have changed
    size_t p_sz = job->simManager->structureParameters().parameters.size();
    if (size_t ps = p_sz; ps != ClParameterisation.GetSize())
        ClParameterisation = clMemory<T, Manual>(ctx, ps);

    // these need to change if the atom_count changes
    if (size_t as = sm->simulationCell()->crystalStructure()->atoms().size(); as != ClAtomA.GetSize()) {
        ClAtomA = clMemory<int, Manual>(ctx, as);
        ClAtomX = clMemory<T, Manual>(ctx, as);
        ClAtomY = clMemory<T, Manual>(ctx, as);
        ClAtomZ = clMemory<T, Manual>(ctx, as);

        unsigned int blocks_x = job->simManager->blocksX();
        unsigned int blocks_y = job->simManager->blocksY();
        unsigned int number_of_slices = job->simManager->simulationCell()->sliceCount();
        ClBlockStartPositions = clMemory<int, Manual>(ctx, number_of_slices * blocks_x * blocks_y + 1);

        ClBlockIds = clMemory<int, Manual>(ctx, as);
        ClZIds = clMemory<int, Manual>(ctx, as);
    }

    // change when the resolution does
    unsigned int rs = sm->resolution();
    if (rs != clXFrequencies.GetSize()) {
        clXFrequencies = clMemory<T, Manual>(ctx, rs);
        clYFrequencies = clMemory<T, Manual>(ctx, rs);
        clPropagator = clMemory<std::complex<T>, Manual>(ctx, rs * rs);

        bool precalc_transmisson = job->simManager->precalculateTransmission();
        if (precalc_transmisson) {
            int n_random = job->simManager->parallelPotentialsCount();
            rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());
            dist = std::uniform_int_distribution<>(0, n_random-1);

            int n_slice = job->simManager->simulationCell()->sliceCount();
            clTransmissionFunction.resize(n_random);
            for (int nr = 0; nr < n_random; ++nr) {
                clTransmissionFunction[nr].resize(n_slice);
                for (int ns = 0; ns < n_slice; ++ns)
                    clTransmissionFunction[nr][ns] = clMemory<std::complex<T>, Manual>(ctx, rs * rs);
            }
        } else {
            clTransmissionFunction.resize(1);
            clTransmissionFunction[0].resize(1);
            clTransmissionFunction[0][0] = clMemory<std::complex<T>, Manual>(ctx, rs * rs);
        }

        clWaveFunctionTemp_1 = clMemory<std::complex<T>, Manual>(ctx, rs * rs);
        clWaveFunctionTemp_2 = clMemory<T, Manual>(ctx, rs * rs);
        clWaveFunctionTemp_3 = clMemory<T, Manual>(ctx, rs * rs);

        clWaveFunctionReal.clear();
        clWaveFunctionRecip.clear();

        for (int i = 0; i < sm->parallelPixels(); ++i) {
            clWaveFunctionReal.emplace_back(ctx, rs * rs);
            clWaveFunctionRecip.emplace_back(ctx, rs * rs);
        }
    }

    if (sm->parallelPixels() < clWaveFunctionReal.size()) {
        clWaveFunctionReal.resize(sm->parallelPixels());
        clWaveFunctionRecip.resize(sm->parallelPixels());
    } else if (sm->parallelPixels() > clWaveFunctionReal.size()) {
        for (int i = 0; i < sm->parallelPixels() - clWaveFunctionReal.size(); ++i) {
            clWaveFunctionReal.emplace_back(ctx, rs * rs);
            clWaveFunctionRecip.emplace_back(ctx, rs * rs);
        }
    }

    // TODO: could clear unneeded buffers when sim type switches, but there aren't many of them... (the main ones are the wavefunction vectors)
}

template <>
void SimulationGeneral<float>::initialiseKernels() {
    auto sm = job->simManager;

    unsigned int rs = sm->resolution();
    if (rs != FourierTrans.GetWidth() || rs != FourierTrans.GetHeight())
        FourierTrans = clFourier<float>(ctx, rs, rs);

    bool isFull3D = sm->full3dEnabled();
    if (do_initialise_general || isFull3D != last_do_3d) {
        if (isFull3D)
            CalculateTransmissionFunction = Kernels::transmission_potentials_full_3d_f.BuildToKernel(ctx);
        else
            CalculateTransmissionFunction = Kernels::transmission_potentials_projected_f.BuildToKernel(ctx);
    }
    last_do_3d = isFull3D;

    if (do_initialise_general) {
        AtomSort = Kernels::atom_sort_f.BuildToKernel(ctx);
        FftShift = Kernels::fft_shift_f.BuildToKernel(ctx);
        BandLimit = Kernels::band_limit_f.BuildToKernel(ctx);
        GeneratePropagator = Kernels::propagator_f.BuildToKernel(ctx);
        ComplexMultiply = Kernels::complex_multiply_f.BuildToKernel(ctx);
        BilinearTranslate = Kernels::bilinear_translate_f.BuildToKernel(ctx);
        ComplexToReal = Kernels::complex_to_real_f.BuildToKernel(ctx);
    }

    do_initialise_general = false;
}

template <>
void SimulationGeneral<double>::initialiseKernels() {
    auto sm = job->simManager;

    unsigned int rs = sm->resolution();
    if (rs != FourierTrans.GetWidth() || rs != FourierTrans.GetHeight())
        FourierTrans = clFourier<double>(ctx, rs, rs);

    bool isFull3D = sm->full3dEnabled();
    if (do_initialise_general || isFull3D != last_do_3d) {
        if (isFull3D)
            CalculateTransmissionFunction = Kernels::transmission_potentials_full_3d_d.BuildToKernel(ctx);
        else
            CalculateTransmissionFunction = Kernels::transmission_potentials_projected_d.BuildToKernel(ctx);
    }
    last_do_3d = isFull3D;

    if (do_initialise_general) {
        AtomSort = Kernels::atom_sort_d.BuildToKernel(ctx);
        FftShift = Kernels::fft_shift_d.BuildToKernel(ctx);
        BandLimit = Kernels::band_limit_d.BuildToKernel(ctx);
        GeneratePropagator = Kernels::propagator_d.BuildToKernel(ctx);
        ComplexMultiply = Kernels::complex_multiply_d.BuildToKernel(ctx);
        BilinearTranslate = Kernels::bilinear_translate_d.BuildToKernel(ctx);
        ComplexToReal = Kernels::complex_to_real_d.BuildToKernel(ctx);
    }

    do_initialise_general = false;
}

template <class T>
void SimulationGeneral<T>::sortAtoms() {
    CLOG(DEBUG, "sim") << "Sorting Atoms";

    bool do_phonon = job->simManager->incoherenceEffects()->phonons()->getFrozenPhononEnabled();

    std::vector<AtomSite> atoms = job->simManager->simulationCell()->crystalStructure()->atoms();
    auto atom_count = static_cast<unsigned int>(atoms.size()); // Needs to be cast to int as opencl kernel expects that size

    std::vector<int> AtomANum;
    std::vector<T> AtomXPos;
    std::vector<T> AtomYPos;
    std::vector<T> AtomZPos;

    AtomANum.reserve(atom_count);
    AtomXPos.reserve(atom_count);
    AtomYPos.reserve(atom_count);
    AtomZPos.reserve(atom_count);

    CLOG(DEBUG, "sim") << "Getting atom positions";
    if (do_phonon)
        CLOG(DEBUG, "sim") << "Using TDS";

    // For sorting the atoms, we want the total area that the simulation covers
    // Basically, this only applies to STEM, so this atom sorting covered all the pixels,
    // even if we aren't going to be using all these atoms for each pixel
    // also used to limit the atoms we have to sort
    std::valarray<double> x_lims = job->simManager->paddedFullLimitsX();
    std::valarray<double> y_lims = job->simManager->paddedFullLimitsY();
    std::valarray<double> z_lims = job->simManager->paddedSimLimitsZ();

    Eigen::Vector3d u1v = {1.0, 0.0, 0.0};
    Eigen::Vector3d u2v = {0.0, 1.0, 0.0};
    Eigen::Vector3d u3v = {0.0, 0.0, 1.0};

    // If NOT forcing xyz, then get actual values
    if (!job->simManager->incoherenceEffects()->phonons()->forceXyzDisps()) {
        u1v = job->simManager->simulationCell()->crystalStructure()->getU1Vector();
        u2v = job->simManager->simulationCell()->crystalStructure()->getU2Vector();
        u3v = job->simManager->simulationCell()->crystalStructure()->getU3Vector();
    }

    for(int i = 0; i < atom_count; i++) {
        double disp_1 = 0.0, disp_2 = 0.0, disp_3 = 0.0;
        if (do_phonon) {
            // TODO: need a log guard here or in the structure file?
            disp_1 = job->simManager->incoherenceEffects()->phonons()->generateTdsFactor(atoms[i], 0);
            disp_2 = job->simManager->incoherenceEffects()->phonons()->generateTdsFactor(atoms[i], 1);
            disp_3 = job->simManager->incoherenceEffects()->phonons()->generateTdsFactor(atoms[i], 2);
        }

        auto d1 = disp_1 * u1v;
        auto d2 = disp_2 * u2v;
        auto d3 = disp_3 * u3v;

        // TODO: could move this check before the TDS if I can get a good estimate of the maximum displacement

        double new_x = atoms[i].x + d1[0] + d2[0] + d3[0];
        double new_y = atoms[i].y + d1[1] + d2[1] + d3[1];
        double new_z = atoms[i].z + d1[2] + d2[2] + d3[2];
        bool in_x = new_x > x_lims[0] && new_x < x_lims[1];
        bool in_y = new_y > y_lims[0] && new_y < y_lims[1];
        bool in_z = new_z > z_lims[0] && new_z < z_lims[1];

        if (in_x && in_y && in_z) {
            // puch back is OK because I have reserved the vector
            AtomANum.push_back(atoms[i].A);
            AtomXPos.push_back(new_x);
            AtomYPos.push_back(new_y);
            AtomZPos.push_back(new_z);
        }
    }

    // update our atom count to be the atoms we have in range
    atom_count = AtomANum.size();

    CLOG(DEBUG, "sim") << "Writing to buffers";

    ClAtomX.Write(AtomXPos);
    ClAtomY.Write(AtomYPos);
    ClAtomZ.Write(AtomZPos);
    ClAtomA.Write(AtomANum);

    CLOG(DEBUG, "sim") << "Creating sort kernel";

    // NOTE: DONT CHANGE UNLESS CHANGE ELSEWHERE ASWELL!
    // Or fix it so they are all referencing same variable.
    unsigned int BlocksX = job->simManager->blocksX();
    unsigned int BlocksY = job->simManager->blocksY();

    double dz = job->simManager->simulationCell()->sliceThickness();
    unsigned int numberOfSlices = job->simManager->simulationCell()->sliceCount();

    AtomSort.SetArg(0, ClAtomX, ArgumentType::Input);
    AtomSort.SetArg(1, ClAtomY, ArgumentType::Input);
    AtomSort.SetArg(2, ClAtomZ, ArgumentType::Input);
    AtomSort.SetArg(3, atom_count);
    AtomSort.SetArg(4, static_cast<T>(x_lims[0]));
    AtomSort.SetArg(5, static_cast<T>(x_lims[1]));
    AtomSort.SetArg(6, static_cast<T>(y_lims[0]));
    AtomSort.SetArg(7, static_cast<T>(y_lims[1]));
    AtomSort.SetArg(8, static_cast<T>(z_lims[0]));
    AtomSort.SetArg(9, static_cast<T>(z_lims[1]));
    AtomSort.SetArg(10, BlocksX);
    AtomSort.SetArg(11, BlocksY);
    AtomSort.SetArg(12, ClBlockIds, ArgumentType::Output);
    AtomSort.SetArg(13, ClZIds, ArgumentType::Output);
    AtomSort.SetArg(14, static_cast<T>(dz));
    AtomSort.SetArg(15, numberOfSlices);

    clWorkGroup SortSize(atom_count, 1, 1);
    CLOG(DEBUG, "sim") << "Running sort kernel";
    AtomSort.run(SortSize);

    ctx->WaitForQueueFinish(); // test

    CLOG(DEBUG, "sim") << "Reading sort kernel output";

    std::vector<int> HostBlockIDs = ClBlockIds.GetLocal();
    std::vector<int> HostZIDs = ClZIds.GetLocal();

    CLOG(DEBUG, "sim") << "Binning atoms";

    // this silly initialising is to make the first two levels of our vectors, we then dynamically
    // fill the next level in the following loop :)
    std::vector<std::vector<std::vector<T>>> Binnedx( BlocksX*BlocksY, std::vector<std::vector<T>>(numberOfSlices) );
    std::vector<std::vector<std::vector<T>>> Binnedy( BlocksX*BlocksY, std::vector<std::vector<T>>(numberOfSlices) );
    std::vector<std::vector<std::vector<T>>> Binnedz( BlocksX*BlocksY, std::vector<std::vector<T>>(numberOfSlices) );
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

    CLOG(DEBUG, "sim") << "Writing binned atom posisitons to bufffers";

    // Now upload the sorted atoms onto the device..
    ClAtomX.Write(AtomXPos);
    ClAtomY.Write(AtomYPos);
    ClAtomZ.Write(AtomZPos);
    ClAtomA.Write(AtomANum);

    ClBlockStartPositions.Write(blockStartPositions);

    // wait for the IO queue here so that we are sure the data is uploaded before we start using it
    ctx->WaitForQueueFinish();
}

template <class T>
bool SimulationGeneral<T>::initialiseSimulation() {

    bool same_simulation = job->simManager == current_manager;

    bool do_phonon = job->simManager->incoherenceEffects()->phonons()->getFrozenPhononEnabled();
    bool do_plasmon = job->simManager->incoherenceEffects()->plasmons()->enabled();
    bool moving_stem_frame = !job->simManager->parallelStem();
    bool do_multi_potential_tds = job->simManager->useParallelPotentials();

    // phonon is needed as the atoms need to be resorted (and transmission functions regenerated)
    // phonons are important as the transmission function will need to be modified
    // the moving stem frame is important as we will need to regenerate the transmission functions

    if (same_simulation && (!do_phonon || do_multi_potential_tds) && !do_plasmon && !moving_stem_frame) {
        CLOG(DEBUG, "sim") << "Manager already initialised, reusing that data";
        return true;
    }
    current_manager = job->simManager;

    CLOG(DEBUG, "sim") << "Initialising all buffers";
    initialiseBuffers();

    CLOG(DEBUG, "sim") << "Setting up all kernels";
    initialiseKernels();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Upload our parameters
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    CLOG(DEBUG, "sim") << "Getting parameters";
    std::vector<double> params_d = job->simManager->structureParameters().parameters;
    std::vector<T> params(params_d.begin(), params_d.end()); // TODO: avoid the copy if we are using a double type?
    CLOG(DEBUG, "sim") << "Uploading parameters";
    ClParameterisation.Write(params);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Sort our atoms!
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Sorting atoms";

    bool force_tds_resort = job->simManager->forcePhononAtomResort();

    // sort atoms if this is a new simulation (i.e. this hasn't been called before)
    // also sort if we are doing phonons and not using the multi-potential approximation
    // or if we are doing phonons and are forcing the resort
    if (!same_simulation || (do_phonon && (!do_multi_potential_tds || force_tds_resort)))
        sortAtoms();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Get local copies of variables (for convenience)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Starting general initialisation";
    auto current_pixel = job->getPixel();

    bool isFull3D = job->simManager->full3dEnabled();
    unsigned int resolution = job->simManager->resolution();
    auto mParams = job->simManager->microscopeParams();
    double wavenumber = mParams->Wavenumber();
    std::valarray<double> wavevector = mParams->Wavevector();
    double pixelscale = job->simManager->realScale();
    double startx = job->simManager->paddedSimLimitsX(current_pixel)[0];
    double starty = job->simManager->paddedSimLimitsY(current_pixel)[0];
    int full3dints = job->simManager->full3dIntegrals();

    // for running opencl kernels
    clWorkGroup WorkSize(resolution, resolution, 1);

    std::string param_name = job->simManager->structureParameters().name;

    // Work out area that is to be simulated (in real space)
    double SimSizeX = pixelscale * resolution;
    double SimSizeY = SimSizeX;

    double sigma = mParams->Sigma() * wavenumber / wavevector[2];

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up our frequency calibrations
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Creating reciprocal space calibration";
    // This basically is all to create OpenCL buffers (1D) that let us know the frequency value of the pixels in the FFT
    // Not that this already accounts for the un-shifted nature of the FFT (i.e. 0 frequency is at 0, 0)
    // We also calculate our limit for low pass filtering the wavefunctions
    std::vector<T> k0x(resolution);
    std::vector<T> k0y(resolution);

    auto imidx = (unsigned int) std::floor(static_cast<double>(resolution) / 2.0 + 0.5);
    auto imidy = (unsigned int) std::floor(static_cast<double>(resolution) / 2.0 + 0.5);

    double temp;

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
    T kmaxx = std::abs(k0x[imidx]);
    T kmaxy = std::abs(k0y[imidy]);

    double bandwidthkmax = std::min(kmaxy, kmaxx);

    CLOG(DEBUG, "sim") << "Writing to buffers";
    // write our frequencies to OpenCL buffers
    clXFrequencies.Write(k0x);
    clYFrequencies.Write(k0y);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up FFT shift kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up FFT shift kernel";

    // these will never change, so set them here
    FftShift.SetArg(0, clWaveFunctionRecip[0], ArgumentType::Input);
    FftShift.SetArg(1, clWaveFunctionTemp_1, ArgumentType::Output);
    FftShift.SetArg(2, resolution);
    FftShift.SetArg(3, resolution);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up complex to real kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up complex to real kernel";

    // these will never change, so set them here
    ComplexToReal.SetArg(1, clWaveFunctionTemp_3, ArgumentType::Output);
    ComplexToReal.SetArg(3, resolution);
    ComplexToReal.SetArg(4, resolution);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up bilinear translation kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up bilinear translation kernel";

    // these will never change, so set them here
    BilinearTranslate.SetArg(0, clWaveFunctionTemp_2, ArgumentType::Input);
    BilinearTranslate.SetArg(1, clWaveFunctionTemp_3, ArgumentType::Output);
    BilinearTranslate.SetArg(6, resolution);
    BilinearTranslate.SetArg(7, resolution);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up low pass filter kernel
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up low pass filter kernel";

    // These never change, so set them here
    BandLimit.SetArg(0, clWaveFunctionTemp_1, ArgumentType::InputOutput);
    BandLimit.SetArg(1, resolution);
    BandLimit.SetArg(2, resolution);
    BandLimit.SetArg(3, static_cast<T>(bandwidthkmax));
    BandLimit.SetArg(4, static_cast<T>(job->simManager->inverseLimitFactor()));
    BandLimit.SetArg(5, clXFrequencies, ArgumentType::Input);
    BandLimit.SetArg(6, clYFrequencies, ArgumentType::Input);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up the kernels to calculate the atomic potentials
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up potential kernel";

    // Work out which blocks to load by ensuring we have the entire area around workgroup up to 5 angstroms away...
    // TODO: check this is doing what the above comment says it is doing...
    // TODO: I think the 8.0 and 3.0 should be the padding as set in the manager...

    int load_blocks_x = (int) std::ceil(8.0 / job->simManager->blockScaleX());
    int load_blocks_y = (int) std::ceil(8.0 / job->simManager->blockScaleY());

    double dz = job->simManager->simulationCell()->sliceThickness();
    int load_blocks_z = (int) std::ceil(3.0 / dz);

    auto full_lims_x = job->simManager->paddedFullLimitsX();
    auto full_lims_y = job->simManager->paddedFullLimitsY();

    auto min_z = job->simManager->paddedSimLimitsZ()[0];

    auto blocks_x = job->simManager->blocksX();
    auto blocks_y = job->simManager->blocksY();

    int number_of_slices = job->simManager->simulationCell()->sliceCount();

    // Set some of the arguments which dont change each iteration
//    CalculateTransmissionFunction.SetArg(0, clTransmissionFunction, ArgumentType::Output);
    CalculateTransmissionFunction.SetArg(1, ClAtomX, ArgumentType::Input);
    CalculateTransmissionFunction.SetArg(2, ClAtomY, ArgumentType::Input);
    CalculateTransmissionFunction.SetArg(3, ClAtomZ, ArgumentType::Input);
    CalculateTransmissionFunction.SetArg(4, ClAtomA, ArgumentType::Input);
    CalculateTransmissionFunction.SetArg(5, ClParameterisation, ArgumentType::Input);
    CalculateTransmissionFunction.SetArg(6, static_cast<int>(job->simManager->structureParameters().form));
    CalculateTransmissionFunction.SetArg(7, job->simManager->structureParameters().i_per_atom);
    CalculateTransmissionFunction.SetArg(8, ClBlockStartPositions, ArgumentType::Input);
    CalculateTransmissionFunction.SetArg(9, resolution);
    CalculateTransmissionFunction.SetArg(10, resolution);
//    CalculateTransmissionFunction.SetArg(11, slice);
    CalculateTransmissionFunction.SetArg(12, number_of_slices);
    CalculateTransmissionFunction.SetArg(13, static_cast<T>(dz));
    CalculateTransmissionFunction.SetArg(14, static_cast<T>(pixelscale));
    CalculateTransmissionFunction.SetArg(15, blocks_x);
    CalculateTransmissionFunction.SetArg(16, blocks_y);
    CalculateTransmissionFunction.SetArg(17, static_cast<T>(full_lims_x[1]));
    CalculateTransmissionFunction.SetArg(18, static_cast<T>(full_lims_x[0]));
    CalculateTransmissionFunction.SetArg(19, static_cast<T>(full_lims_y[1]));
    CalculateTransmissionFunction.SetArg(20, static_cast<T>(full_lims_y[0]));
    CalculateTransmissionFunction.SetArg(21, load_blocks_x);
    CalculateTransmissionFunction.SetArg(22, load_blocks_y);
    CalculateTransmissionFunction.SetArg(23, load_blocks_z);
    CalculateTransmissionFunction.SetArg(24, static_cast<T>(sigma)); // Not sure why I am using this sigma and not commented sigma...
    CalculateTransmissionFunction.SetArg(25, static_cast<T>(startx + reference_perturb_x));
    CalculateTransmissionFunction.SetArg(26, static_cast<T>(starty + reference_perturb_y));
    if (isFull3D) {
        double int_shift_x = (wavevector[0] / wavevector[2]) * dz / full3dints;
        double int_shift_y = (wavevector[1] / wavevector[2]) * dz / full3dints;

//        CalculateTransmissionFunction.SetArg(27, static_cast<T>(slice_z);
        CalculateTransmissionFunction.SetArg(28, static_cast<T>(int_shift_x));
        CalculateTransmissionFunction.SetArg(29, static_cast<T>(int_shift_y));
        CalculateTransmissionFunction.SetArg(30, full3dints);
    } else {
        CalculateTransmissionFunction.SetArg(27, static_cast<T>(mParams->BeamTilt));
        CalculateTransmissionFunction.SetArg(28, static_cast<T>(mParams->BeamAzimuth));
    }

    bool precalc_transmisson = job->simManager->precalculateTransmission();

    if (precalc_transmisson) {
        clWorkGroup LocalWork(16, 16, 1);

        int n_random = job->simManager->parallelPotentialsCount();

        // loop over
        for (int j = 0; j < n_random; ++j) {

            for (int i = 0; i < number_of_slices; ++i) {
                CLOG(DEBUG, "sim") << "Calculating potentials";
                CalculateTransmissionFunction.SetArg(0, clTransmissionFunction[j][i], ArgumentType::Output);
                CalculateTransmissionFunction.SetArg(11, i);

                if (isFull3D) {
                    double slice_z = min_z + (number_of_slices - i) * dz;
                    CalculateTransmissionFunction.SetArg(27, static_cast<T>(slice_z));
                }

                CalculateTransmissionFunction.run(WorkSize, LocalWork);

                /// Apply low pass filter to transmission function
                CLOG(DEBUG, "sim") << "FFT transmission function";
                FourierTrans.run(clTransmissionFunction[j][i], clWaveFunctionTemp_1, Direction::Forwards);
                CLOG(DEBUG, "sim") << "Band limit transmission function";
                BandLimit.run(WorkSize);
                CLOG(DEBUG, "sim") << "IFFT band limited transmission function";
                FourierTrans.run(clWaveFunctionTemp_1, clTransmissionFunction[j][i], Direction::Inverse);

                ctx->WaitForQueueFinish();

                if (pool.isStopped())
                    return false;
            }

            // sort for our next iteration
            if (j < n_random - 1)
                sortAtoms();
        }
    } else {
        CalculateTransmissionFunction.SetArg(0, clTransmissionFunction[0][0], ArgumentType::Output);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set up the propagator
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CLOG(DEBUG, "sim") << "Set up propagator kernel";

    GeneratePropagator.SetArg(0, clPropagator, ArgumentType::Output);
    GeneratePropagator.SetArg(1, clXFrequencies, ArgumentType::Input);
    GeneratePropagator.SetArg(2, clYFrequencies, ArgumentType::Input);
    GeneratePropagator.SetArg(3, resolution);
    GeneratePropagator.SetArg(4, resolution);
    GeneratePropagator.SetArg(5, static_cast<T>(dz)); // Is this the right dz? (Propagator needs slice thickness not spacing between atom bins)
    GeneratePropagator.SetArg(6, static_cast<T>(wavenumber));
    GeneratePropagator.SetArg(7, static_cast<T>(wavevector[0]));
    GeneratePropagator.SetArg(8, static_cast<T>(wavevector[1]));
    GeneratePropagator.SetArg(9, static_cast<T>(wavevector[2]));
    GeneratePropagator.SetArg(10, static_cast<T>(bandwidthkmax * job->simManager->inverseLimitFactor()));

    // actually run this kernel now
    GeneratePropagator.run(WorkSize);
    ctx->WaitForQueueFinish();


    CLOG(DEBUG, "sim") << "Set up complex multiply kernel";

    ComplexMultiply.SetArg(3, resolution);
    ComplexMultiply.SetArg(4, resolution);

    return true;
}

template <class T>
void SimulationGeneral<T>::modifyBeamTilt(double kx, double ky, double kz){
    auto mParams = job->simManager->microscopeParams();
    bool isFull3D = job->simManager->full3dEnabled();
    int full3dints = job->simManager->full3dIntegrals();
    double dz = job->simManager->simulationCell()->sliceThickness();
    unsigned int resolution = job->simManager->resolution();

    // The transmission function does not need to be recalculated here (it is done on every slice)
    if (isFull3D) {
        double int_shift_x = (kx / kz) * dz / full3dints;
        double int_shift_y = (kx / kz) * dz / full3dints;

        CalculateTransmissionFunction.SetArg(28, static_cast<T>(int_shift_x));
        CalculateTransmissionFunction.SetArg(29, static_cast<T>(int_shift_y));
    } else {
        double new_azimuth = std::atan(ky / kx);
        double new_tilt = std::atan( std::sqrt(kx*kx + ky*ky) / kz );

        CalculateTransmissionFunction.SetArg(28, static_cast<T>(new_azimuth));
        CalculateTransmissionFunction.SetArg(29, static_cast<T>(new_tilt));
    }

    // The propagator does need to be recalculated now
    GeneratePropagator.SetArg(7, static_cast<T>(kx));
    GeneratePropagator.SetArg(8, static_cast<T>(ky));
    GeneratePropagator.SetArg(9, static_cast<T>(kz));

    clWorkGroup WorkSize(resolution, resolution, 1);
    GeneratePropagator.run(WorkSize);
    ctx->WaitForQueueFinish();
}

template <class T>
void SimulationGeneral<T>::doMultiSliceStep(int slice) {
    CLOG(DEBUG, "sim") << "Start multislice step " << slice;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create local variables for convenience
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    double dz = job->simManager->simulationCell()->sliceThickness();
    unsigned int resolution = job->simManager->resolution();
    // in the current format, Tds is handled by job splitting so this is always 1??
    int n_parallel = job->simManager->parallelPixels(); // total number of parallel pixels
    auto z_lim = job->simManager->paddedSimLimitsZ();

    clWorkGroup Work(resolution, resolution, 1);
    clWorkGroup LocalWork(16, 16, 1);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Generate transmission function if we need to!
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int trans_id = slice;

    bool precalc_transmisson = job->simManager->precalculateTransmission();
    if (!precalc_transmisson) {
        trans_id = 0;

        CLOG(DEBUG, "sim") << "Calculating potentials";
        CalculateTransmissionFunction.SetArg(0, clTransmissionFunction[0][0], ArgumentType::Output);
        CalculateTransmissionFunction.SetArg(11, slice);

        bool isFull3D = job->simManager->full3dEnabled();
        if (isFull3D) {
            auto min_z = job->simManager->paddedSimLimitsZ()[0];
            int number_of_slices = job->simManager->simulationCell()->sliceCount();

            double slice_z = min_z + (number_of_slices - slice) * dz;
            CalculateTransmissionFunction.SetArg(27, static_cast<T>(slice_z));
        }

        CalculateTransmissionFunction.run(Work, LocalWork);

        /// Apply low pass filter to transmission function
        CLOG(DEBUG, "sim") << "FFT transmission function";
        FourierTrans.run(clTransmissionFunction[0][0], clWaveFunctionTemp_1, Direction::Forwards);
        CLOG(DEBUG, "sim") << "Band limit transmission function";
        BandLimit.run(Work);
        CLOG(DEBUG, "sim") << "IFFT band limited transmission function";
        FourierTrans.run(clWaveFunctionTemp_1, clTransmissionFunction[0][0], Direction::Inverse);
    }

    bool do_multi_potential_tds = job->simManager->useParallelPotentials();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Propogate slice
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < n_parallel; i++) {
        CLOG(DEBUG, "sim") << "Propogating (" << i << " of " << n_parallel << " parallel)";

        int nv = 0;

        if (do_multi_potential_tds)
            nv = dist(rng);

        // Multiply transmission function with wavefunction
        ComplexMultiply.SetArg(0, clTransmissionFunction[nv][trans_id], ArgumentType::Input);
        ComplexMultiply.SetArg(1, clWaveFunctionReal[i], ArgumentType::Input);
        ComplexMultiply.SetArg(2, clWaveFunctionRecip[i], ArgumentType::Output);
        CLOG(DEBUG, "sim") << "Multiply wavefunction and potentials";
        ComplexMultiply.run(Work);

        // go to reciprocal space
        CLOG(DEBUG, "sim") << "FFT to reciprocal space";
        FourierTrans.run(clWaveFunctionRecip[i], clWaveFunctionTemp_1, Direction::Forwards);

        // convolve with propagator
        ComplexMultiply.SetArg(0, clWaveFunctionTemp_1, ArgumentType::Input);
        ComplexMultiply.SetArg(1, clPropagator, ArgumentType::Input);
        ComplexMultiply.SetArg(2, clWaveFunctionRecip[i], ArgumentType::Output);
        CLOG(DEBUG, "sim") << "Convolve with propagator";
        ComplexMultiply.run(Work);

        // IFFT back to real space
        CLOG(DEBUG, "sim") << "IFFT to real space";
        FourierTrans.run(clWaveFunctionRecip[i], clWaveFunctionReal[i], Direction::Inverse);

        // I think this is important as each parallel pixel shares (and particularly writes) to shared buffers
        ctx->WaitForQueueFinish();
    }
}

template <class T>
std::vector<double> SimulationGeneral<T>::getDiffractionImage(int parallel_ind, double d_kx, double d_ky) {
    CLOG(DEBUG, "sim") << "Getting diffraction image";
    unsigned int resolution = job->simManager->resolution();

    // Original data is complex so copy complex version down first
    clWorkGroup Work(resolution, resolution, 1);

    CLOG(DEBUG, "sim") << "FFT shifting diffraction pattern";
    FftShift.SetArg(0, clWaveFunctionRecip[parallel_ind], ArgumentType::Input);
    FftShift.run(Work);

    auto output_type = Utils::ComplexDisplay::AbsSquared; // should be 4

    CLOG(DEBUG, "sim") << "Getting abs of diffraction pattern";

    if (d_kx != 0.0 || d_ky != 0.0) {
        ComplexToReal.SetArg(0, clWaveFunctionTemp_1, ArgumentType::Input);
        ComplexToReal.SetArg(1, clWaveFunctionTemp_2, ArgumentType::Output);
        ComplexToReal.SetArg(2, static_cast<int>(output_type)); // should be 4
        ComplexToReal.run(Work);

        translateDiffImage(d_kx, d_ky);
    } else {
        ComplexToReal.SetArg(0, clWaveFunctionTemp_1, ArgumentType::Input);
        ComplexToReal.SetArg(1, clWaveFunctionTemp_3, ArgumentType::Output);
        ComplexToReal.SetArg(2, static_cast<int>(output_type)); // should be 4
        ComplexToReal.run(Work);
    }

    CLOG(DEBUG, "sim") << "Copy from buffer";
    std::vector<T> data_typed = clWaveFunctionTemp_3.GetLocal();

    return std::vector<double>(data_typed.begin(), data_typed.end());
}

template <class T>
std::vector<double> SimulationGeneral<T>::getExitWaveImage(unsigned int t, unsigned int l, unsigned int b, unsigned int r) {
    CLOG(DEBUG, "sim") << "Getting exit wave image";
    unsigned int resolution = job->simManager->resolution();
    std::vector<double> data_out(2*((resolution - t - b) * (resolution - l - r)));

    CLOG(DEBUG, "sim") << "Copy from buffer";
    std::vector<std::complex<T>> compdata = clWaveFunctionReal[0].GetLocal();

    CLOG(DEBUG, "sim") << "Process complex data";
    int cnt = 0;
    for (unsigned int j = 0; j < resolution; ++j)
        if (j >= b && j < (resolution - t))
            for (unsigned int i = 0; i < resolution; ++i)
                if (i >= l && i < (resolution - r)) {
                    unsigned int k = i + j * resolution;
                    data_out[cnt] = compdata[k].real();
                    data_out[cnt + 1] = compdata[k].imag();
                    cnt += 2;
                }

    return data_out;
}

template <typename T>
void SimulationGeneral<T>::translateDiffImage(double d_kx, double d_ky) {
    unsigned int resolution = job->simManager->resolution();
    clWorkGroup Work(resolution, resolution, 1);

    double scale = job->simManager->inverseScale();
    double shift_x = d_kx / scale;
    double shift_y = d_ky / scale;

    int int_shift_x = std::floor(shift_x);
    int int_shift_y = std::floor(shift_y);

    double sub_shift_x = shift_x - int_shift_x;
    double sub_shift_y = shift_y - int_shift_y;

    CLOG(DEBUG, "sim") << "Translating difraction pattern";

    BilinearTranslate.SetArg(2, int_shift_x);
    BilinearTranslate.SetArg(3, int_shift_y);
    BilinearTranslate.SetArg(4, static_cast<T>(sub_shift_x));
    BilinearTranslate.SetArg(5, static_cast<T>(sub_shift_y));

    BilinearTranslate.run(Work);
}

template class SimulationGeneral<float>;
template class SimulationGeneral<double>;
#include "simulationmanager.h"

#include <ios>
#include <fstream>
#include <memory>
#include <utilities/fileio.h>

SimulationManager::SimulationManager() : Resolution(256), completeJobs(0), default_xy_padding({-8.0, 8.0}), default_z_padding({-3.0, 3.0}), padding_x(default_xy_padding),
                                         padding_y(default_xy_padding), padding_z(default_z_padding), slice_dz(1.0),
                                         blocks_x(80), blocks_y(80), maxReciprocalFactor(2.0 / 3.0), numParallelPixels(1), simulateCtemImage(false),
                                         ccd_name(""), ccd_binning(1), ccd_dose(10000.0),
                                         slice_offset(0.0), structure_parameters_name("kirkland"), maintain_area(false),
                                         Mode(SimulationMode::CTEM), use_double_precision(false), intermediate_slices_enabled(false), intermediate_slices(0)
{
    // Here is where the default values are set!
    MicroParams = std::make_shared<MicroscopeParameters>();
    SimArea = std::make_shared<SimulationArea>();
    StemSimArea = std::make_shared<StemArea>();
    CbedPos = std::make_shared<CbedPosition>();
    inelastic_scattering = std::make_shared<InelasticScattering>();

    full3dInts = 20;
    isF3D = false;

    // I'm really assuming the rest of the aberrations are default 0
    MicroParams->Aperture = 20;
    MicroParams->Voltage = 200;
    MicroParams->Delta = 30;
    MicroParams->Alpha = 0.3;
    MicroParams->C30 = 10000;
}

void SimulationManager::setStructure(std::string filePath, CIF::SuperCellInfo info, bool fix_cif)
{
    // lock this in case we need multiple devices to load this structure
    std::unique_lock<std::mutex> lock(structure_mutex);

    Structure.reset(new CrystalStructure(filePath, info, fix_cif));

    if (!maintain_area) {
        auto x_lims = getStructLimitsX();
        auto y_lims = getStructLimitsY();

        SimArea->setRawLimitsX(x_lims[0], x_lims[1]);
        SimArea->setRawLimitsY(y_lims[0], y_lims[1]);

        getStemArea()->setRawLimitsX(x_lims[0], x_lims[1]);
        getStemArea()->setRawLimitsY(y_lims[0], y_lims[1]);

        getCBedPosition()->setXPos((x_lims[0] + x_lims[1]) / 2);
        getCBedPosition()->setYPos((y_lims[0] + y_lims[1]) / 2);
    }
}

void SimulationManager::setStructure(CIF::CIFReader cif, CIF::SuperCellInfo info)
{
    // lock this in case we need multiple devices to load this structure
    std::unique_lock<std::mutex> lock(structure_mutex);

    Structure.reset(new CrystalStructure(cif, info));

    if (!maintain_area) {
        auto x_lims = getStructLimitsX();
        auto y_lims = getStructLimitsY();

        SimArea->setRawLimitsX(x_lims[0], x_lims[1]);
        SimArea->setRawLimitsY(y_lims[0], y_lims[1]);

        getStemArea()->setRawLimitsX(x_lims[0], x_lims[1]);
        getStemArea()->setRawLimitsY(y_lims[0], y_lims[1]);

        getCBedPosition()->setXPos((x_lims[0] + x_lims[1]) / 2);
        getCBedPosition()->setYPos((y_lims[0] + y_lims[1]) / 2);
    }
}

std::tuple<double, double, double, int> SimulationManager::getSimRanges()
{
    double xRange = getPaddedSimLimitsX(0)[1] - getPaddedSimLimitsX(0)[0];
    double yRange = getPaddedSimLimitsY(0)[1] - getPaddedSimLimitsY(0)[0];
    double zRange = getPaddedStructLimitsZ()[1] - getPaddedStructLimitsZ()[0];
    int numAtoms = Structure->getAtomCountInRange(getPaddedSimLimitsX(0)[0], getPaddedSimLimitsX(0)[1],
                                                  getPaddedSimLimitsY(0)[0], getPaddedSimLimitsY(0)[1]);

    return std::make_tuple(xRange, yRange, zRange, numAtoms);
}

double SimulationManager::getRealScale()
{
    if(!Structure || !haveResolution())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    auto lim_x = getPaddedSimLimitsX(0);
    auto lim_y = getPaddedSimLimitsY(0);

    auto x_r = lim_x[1] - lim_x[0];
    auto y_r = lim_y[1] - lim_y[0];
    return std::max(x_r, y_r) / Resolution;
}

double SimulationManager::getInverseScale()
{
    if(!Structure || !haveResolution())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    return 1.0 / (getRealScale() * Resolution);
}

double SimulationManager::getInverseScaleAngle() {
    if(!Structure || !haveResolution() || !(MicroParams && MicroParams->Voltage > 0))
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    double inv_scale = getInverseScale();
    return 1000.0 * inv_scale * MicroParams->Wavelength();
}

double SimulationManager::getInverseMaxAngle()
{
    // need to do this in mrad, eventually should also pass inverse Angstrom for hover text?
    if(!Structure || !haveResolution() || !(MicroParams && MicroParams->Voltage > 0))
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    // this is the max reciprocal space scale for the entire image
    double angle_scale = getInverseScaleAngle(); // apply cut off here, because we can
    return 0.5 * angle_scale * Resolution * getInverseLimitFactor(); // half because we have a centered 0, 1000 to be in mrad
}

unsigned long SimulationManager::getTotalParts()
{
    if (Mode == SimulationMode::CTEM)
        return 1;
    else if (Mode == SimulationMode::CBED)
        return static_cast<unsigned long>(inelastic_scattering->getInelasticIterations());
    else if (Mode == SimulationMode::STEM) {
        // round up as still need to complete that 'fraction of a job'
        unsigned int inelastic_runs = inelastic_scattering->getInelasticIterations();
        return static_cast<unsigned long>(inelastic_runs * std::ceil(
                static_cast<double>(getStemArea()->getNumPixels()) / numParallelPixels));
    }

    return 0;
}

void SimulationManager::updateImages(std::map<std::string, Image<double>> &ims, int jobCount)
{
    CLOG(DEBUG, "sim") << "Updating images";
    std::lock_guard<std::mutex> lck(image_update_mtx);
    CLOG(DEBUG, "sim") << "Got a mutex lock";
    // this average factor is here to remove the effect of summing TDS configurations. i.e. the exposure is the same for TDS and non TDS
    auto average_factor = static_cast<double>(inelastic_scattering->getInelasticIterations());

    for (auto const& i : ims)
    {
        CLOG(DEBUG, "sim") << "Processing image " << i.first;
        if (Images.find(i.first) != Images.end()) {
            CLOG(DEBUG, "sim") << "Adding to existing image";
            auto current = Images[i.first];
            auto im = i.second;
            if (im.getSliceSize() != current.getSliceSize()) {
                CLOG(ERROR, "sim") << "Tried to merge simulation jobs with different output size";
                throw std::runtime_error("Tried to merge simulation jobs with different output size");
            }
            CLOG(DEBUG, "sim") << "Copying data";
            for (int j = 0; j < current.getDepth(); ++j)
                for (int k = 0; k < current.getSliceSize(); ++k)
                    current.getSliceRef(j)[k] += im.getSliceRef(j)[k] / average_factor;
            Images[i.first] = current;
        } else {
            CLOG(DEBUG, "sim") << "First time so creating image";
            auto new_averaged = i.second;
            for (int j = 0; j < new_averaged.getDepth(); ++j)
                for (double &d : new_averaged.getSliceRef(j))
                    d /= average_factor; // need to average this as the image is created (if TDS)
            Images[i.first] = new_averaged;
        }
    }

    // count how many jobs have been done...
    completeJobs += jobCount;

    auto v = getTotalParts();

    if (completeJobs > v) {
        CLOG(ERROR, "sim") << "Simulation received more parts than it expected";
        throw std::runtime_error("Simulation received more parts than it expected");
    }

    auto prgrss = static_cast<double>(completeJobs) / v;

    CLOG(DEBUG, "sim") << "Report progress: " << prgrss*100 << "%";

    reportTotalProgress(prgrss);

    // this means this simulation is finished
    if (completeJobs == v && imageReturn) {
        CLOG(DEBUG, "sim") << "All parts of this job finished";
        imageReturn(*this);
    }
}

void SimulationManager::failedSimulation() {
    if (imageReturn) {
        CLOG(DEBUG, "sim") << "Returning blank data";
        imageReturn(*this);
    }
}


void SimulationManager::reportTotalProgress(double prog)
{
    if (progressTotalReporter)
        progressTotalReporter(prog);
}

void SimulationManager::reportSliceProgress(double prog) {
    if (progressSliceReporter)
        progressSliceReporter(prog);
}

double SimulationManager::getBlockScaleX() {
//    auto r = getPaddedStructLimitsX();
    auto r = getPaddedFullLimitsX();
    return (r[1] - r[0]) / getBlocksX();
}

double SimulationManager::getBlockScaleY() {
//    auto r = getPaddedStructLimitsY();
    auto r = getPaddedFullLimitsY();
    return (r[1] - r[0]) / getBlocksY();
}

int SimulationManager::getBlocksX() {
    calculate_blocks();
    return blocks_x;
}

int SimulationManager::getBlocksY() {
    calculate_blocks();
    return blocks_y;
}

void SimulationManager::calculate_blocks() {
    // set number of blocks. Set the blocks to be 4 Angstroms apart (as this is half our buffer region)
    // so we are never loading more than two extra blocks (I suppose smaller is better, but also might make the
    // arrays a bit convoluted) TODO: test if this matters
    auto x_lims_2 = getPaddedFullLimitsX();
    auto y_lims_2 = getPaddedFullLimitsY();
    double xr = x_lims_2[1] - x_lims_2[0];
    double yr = y_lims_2[1] - y_lims_2[0];

    // always using x and y as same size (for now) so find the larger dimension
    // floor so blocks will be slightly larger than 4 Angstroms
    auto n_blocks = (int) std::ceil(std::max(xr, yr) / 8.0);

    blocks_x = n_blocks;
    blocks_y = n_blocks;
}

void SimulationManager::round_Z_padding()
{
    // Do the Z padding so that our slice thickness/shift works (and produces AT LEAST the desired padding)
    auto p_z = SimulationManager::default_z_padding;

    auto pad_slices_pre = (int) std::ceil((std::abs(p_z[1]) - slice_offset) / slice_dz);
    double pre_pad = slice_offset + pad_slices_pre * slice_dz;

    double zw = getStructLimitsZ()[1] - getStructLimitsZ()[0];
    auto struct_slices = (int) std::ceil((zw + slice_offset) / slice_dz); // slice offset needed here?
    double struct_slice_thick = struct_slices * slice_dz;

    double left_over = struct_slice_thick - slice_offset - zw;

    auto pad_slices_post = (int) std::ceil((std::abs(p_z[0]) - left_over) / slice_dz);
    double post_pad = pad_slices_post * slice_dz + left_over;

    // The simulation works from LARGEST z to SMALLEST. So the pre padding is actually on top of the z structure.
    padding_z = {-post_pad, pre_pad};
}

unsigned int SimulationManager::getNumberofSlices() {
    auto z_lims = getPaddedStructLimitsZ();
    double z_range = z_lims[1] - z_lims[0];

    // the 0.000001 is for errors in the float
    auto n_slices = (unsigned int) std::ceil((z_range / getSliceThickness()) - 0.000001);
    n_slices += (n_slices == 0);
    return n_slices;
}

unsigned int SimulationManager::getPaddedPreSlices() {
    return static_cast<unsigned int>(padding_z[1] / slice_dz);
}
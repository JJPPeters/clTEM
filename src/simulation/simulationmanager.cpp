#include "simulationmanager.h"

#include <ios>
#include <fstream>
#include <memory>
#include <utilities/fileio.h>

SimulationManager::SimulationManager() : Resolution(256), completeJobs(0),
                                         blocks_x(80), blocks_y(80), maxReciprocalFactor(2.0 / 3.0), numParallelPixels(1), simulateCtemImage(false),
                                         ccd_name(""), ccd_binning(1), ccd_dose(10000.0),
                                         structure_parameters_name("kirkland"), maintain_area(false),
                                         Mode(SimulationMode::CTEM), use_double_precision(false), intermediate_slices_enabled(false), intermediate_slices(0)
{
    // Here is where the default values are set!
    MicroParams = std::make_shared<MicroscopeParameters>();
    SimArea = std::make_shared<SimulationArea>();
    StemSimArea = std::make_shared<StemArea>();
    CbedPos = std::make_shared<CbedPosition>();
    inelastic_scattering = std::make_shared<InelasticScattering>();
    simulation_cell = std::make_shared<SimulationCell>();

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

    simulation_cell->setCrystalStructure(filePath, info, fix_cif);

    if (!maintain_area) {
        auto x_lims = simulation_cell->crystalStructure()->getLimitsX();
        auto y_lims = simulation_cell->crystalStructure()->getLimitsY();

        simulationArea()->setRawLimitsX(x_lims[0], x_lims[1]);
        simulationArea()->setRawLimitsY(y_lims[0], y_lims[1]);

        stemArea()->setRawLimitsX(x_lims[0], x_lims[1]);
        stemArea()->setRawLimitsY(y_lims[0], y_lims[1]);

        cbedPosition()->setXPos((x_lims[0] + x_lims[1]) / 2);
        cbedPosition()->setYPos((y_lims[0] + y_lims[1]) / 2);
    }
}

void SimulationManager::setStructure(CIF::CIFReader cif, CIF::SuperCellInfo info)
{
    // lock this in case we need multiple devices to load this structure
    std::unique_lock<std::mutex> lock(structure_mutex);

    simulation_cell->setCrystalStructure(cif, info);

    if (!maintain_area) {
        auto x_lims = simulation_cell->crystalStructure()->getLimitsX();
        auto y_lims = simulation_cell->crystalStructure()->getLimitsY();

        simulationArea()->setRawLimitsX(x_lims[0], x_lims[1]);
        simulationArea()->setRawLimitsY(y_lims[0], y_lims[1]);

        stemArea()->setRawLimitsX(x_lims[0], x_lims[1]);
        stemArea()->setRawLimitsY(y_lims[0], y_lims[1]);

        cbedPosition()->setXPos((x_lims[0] + x_lims[1]) / 2);
        cbedPosition()->setYPos((y_lims[0] + y_lims[1]) / 2);
    }
}

std::tuple<double, double, double, int> SimulationManager::simRanges()
{
    auto px = paddedSimLimitsX(0);
    auto py = paddedSimLimitsY(0);

    double xRange = px[1] - px[0];
    double yRange = py[1] - py[0];
    double zRange = simulation_cell->paddedStructLimitsZ()[1] - simulation_cell->paddedStructLimitsZ()[0];
    int numAtoms = simulation_cell->crystalStructure()->getAtomCountInRange(px[0], px[1], py[0], py[1]);

    return std::make_tuple(xRange, yRange, zRange, numAtoms);
}

double SimulationManager::realScale()
{
    if(!simulation_cell->crystalStructure() || !resolutionValid())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    auto lim_x = paddedSimLimitsX(0);
    auto lim_y = paddedSimLimitsY(0);

    auto x_r = lim_x[1] - lim_x[0];
    auto y_r = lim_y[1] - lim_y[0];
    return std::max(x_r, y_r) / Resolution;
}

double SimulationManager::inverseScale()
{
    if(!simulation_cell->crystalStructure() || !resolutionValid())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    return 1.0 / (realScale() * Resolution);
}

double SimulationManager::inverseMax()
{
    if(!simulation_cell->crystalStructure() || !resolutionValid())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    double inv_scale = inverseScale();
    return 0.5 * inv_scale * Resolution * inverseLimitFactor();
}

double SimulationManager::inverseScaleAngle() {
    if(!simulation_cell->crystalStructure() || !resolutionValid() || !(MicroParams && MicroParams->Voltage > 0))
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    double inv_scale = inverseScale();
    return 1000.0 * inv_scale * MicroParams->Wavelength();
}

double SimulationManager::inverseMaxAngle()
{
    // need to do this in mrad, eventually should also pass inverse Angstrom for hover text?
    if(!simulation_cell->crystalStructure() || !resolutionValid() || !(MicroParams && MicroParams->Voltage > 0))
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    // this is the max reciprocal space scale for the entire image
    double angle_scale = inverseScaleAngle(); // apply cut off here, because we can
    return 0.5 * angle_scale * Resolution * inverseLimitFactor(); // half because we have a centered 0, 1000 to be in mrad
}

unsigned long SimulationManager::totalParts()
{
    if (Mode == SimulationMode::CTEM || Mode == SimulationMode::CBED)
        return static_cast<unsigned long>(inelastic_scattering->iterations());
    else if (Mode == SimulationMode::STEM) {
        // round up as still need to complete that 'fraction of a job'
        unsigned int inelastic_runs = inelastic_scattering->iterations();
        return static_cast<unsigned long>(inelastic_runs * std::ceil(
                static_cast<double>(stemArea()->getNumPixels()) / numParallelPixels));
    }

    return 0;
}

void SimulationManager::updateImages(std::map<std::string, Image<double>> &ims, int jobCount)
{
    CLOG(DEBUG, "sim") << "Updating images";
    std::lock_guard<std::mutex> lck(image_update_mtx);
    CLOG(DEBUG, "sim") << "Got a mutex lock";
    // this average factor is here to remove the effect of summing TDS configurations. i.e. the exposure is the same for TDS and non TDS
    auto average_factor = static_cast<double>(inelasticScattering()->iterations());

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
                // we need to account for my complex number, that I have sort of bodged in, hence I calculate the k range as I have (and not slicesize)
                for (int k = 0; k < current.getSliceRef(j).size(); ++k)
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

    auto v = totalParts();

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

double SimulationManager::blockScaleX() {
//    auto r = getPaddedStructLimitsX();
    auto r = paddedFullLimitsX();
    return (r[1] - r[0]) / blocksX();
}

double SimulationManager::blockScaleY() {
//    auto r = getPaddedStructLimitsY();
    auto r = paddedFullLimitsY();
    return (r[1] - r[0]) / blocksY();
}

int SimulationManager::blocksX() {
    calculateBlocks();
    return blocks_x;
}

int SimulationManager::blocksY() {
    calculateBlocks();
    return blocks_y;
}

void SimulationManager::calculateBlocks() {
    // set number of blocks. Set the blocks to be 4 Angstroms apart (as this is half our buffer region)
    // so we are never loading more than two extra blocks (I suppose smaller is better, but also might make the
    // arrays a bit convoluted) TODO: test if this matters
    auto x_lims_2 = paddedFullLimitsX();
    auto y_lims_2 = paddedFullLimitsY();
    double xr = x_lims_2[1] - x_lims_2[0];
    double yr = y_lims_2[1] - y_lims_2[0];

    // always using x and y as same size (for now) so find the larger dimension
    // floor so blocks will be slightly larger than 4 Angstroms
    auto n_blocks = (int) std::ceil(std::max(xr, yr) / 8.0);

    blocks_x = n_blocks;
    blocks_y = n_blocks;
}
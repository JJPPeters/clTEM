#include "simulationmanager.h"

#include <ios>
#include <fstream>
#include <memory>
#include <utilities/fileio.h>

SimulationManager::SimulationManager() : sim_resolution(256), complete_jobs(0),
                                         blocks_x(80), blocks_y(80), max_inverse_factor(2.0 / 3.0), parallel_pixels(1), simulate_ctem_image(false),
                                         ccd_name(""), ccd_binning(1), ccd_dose(10000.0),
                                         structure_parameters_name("kirkland"), maintain_area(false),
                                         simulation_mode(SimulationMode::CTEM), use_double_precision(false), intermediate_slices_enabled(false), intermediate_slices(0)
{
    // Here is where the default values are set!
    micro_params = std::make_shared<MicroscopeParameters>();
    sim_area = std::make_shared<SimulationArea>();
    stem_sim_area = std::make_shared<StemArea>();
    cbed_pos = std::make_shared<CbedPosition>();
    incoherence_effects = std::make_shared<IncoherentEffects>();
    simulation_cell = std::make_shared<SimulationCell>();

    full_3d_integrals = 20;
    use_full_3d = false;

    //
    live_stem = false;

    // I'm really assuming the rest of the aberrations are default 0
    micro_params->CondenserAperture = 20;
    micro_params->ObjectiveAperture = 100;
    micro_params->Voltage = 200;
    micro_params->Delta = 30;
    micro_params->Alpha = 0.3;
    micro_params->C30 = 10000;
}

SimulationManager::SimulationManager(const SimulationManager &sm)
        : structure_mutex(), image_update_mutex(), sim_resolution(sm.sim_resolution),
          parallel_pixels(sm.parallel_pixels), use_full_3d(sm.use_full_3d), full_3d_integrals(sm.full_3d_integrals),
          complete_jobs(sm.complete_jobs), image_return_func(sm.image_return_func), report_progress_total_func(sm.report_progress_total_func), report_progress_slice_func(sm.report_progress_slice_func),
          image_container(sm.image_container), simulation_mode(sm.simulation_mode), stem_dets(sm.stem_dets),
          blocks_x(sm.blocks_x), blocks_y(sm.blocks_y), simulate_ctem_image(sm.simulate_ctem_image),
          max_inverse_factor(sm.max_inverse_factor), ccd_name(sm.ccd_name), ccd_binning(sm.ccd_binning), ccd_dose(sm.ccd_dose),
          structure_parameters_name(sm.structure_parameters_name), maintain_area(sm.maintain_area),
          use_double_precision(sm.use_double_precision), live_stem(sm.live_stem),
          intermediate_slices_enabled(sm.intermediate_slices_enabled), intermediate_slices(sm.intermediate_slices)
{
    micro_params = std::make_shared<MicroscopeParameters>(*(sm.micro_params));
    sim_area = std::make_shared<SimulationArea>(*(sm.sim_area));
    stem_sim_area = std::make_shared<StemArea>(*(sm.stem_sim_area));
    cbed_pos = std::make_shared<CbedPosition>(*(sm.cbed_pos));
    incoherence_effects = std::make_shared<IncoherentEffects>(*(sm.incoherence_effects));

    simulation_cell = std::make_shared<SimulationCell>(*(sm.simulation_cell));
}

SimulationManager &SimulationManager::operator=(const SimulationManager &sm) {
    intermediate_slices_enabled = sm.intermediate_slices_enabled;
    intermediate_slices = sm.intermediate_slices;
    use_double_precision = sm.use_double_precision;
    sim_resolution = sm.sim_resolution;
    parallel_pixels = sm.parallel_pixels;
    use_full_3d = sm.use_full_3d;
    full_3d_integrals = sm.full_3d_integrals;
    complete_jobs = sm.complete_jobs;
    image_return_func = sm.image_return_func;
    report_progress_total_func = sm.report_progress_total_func;
    report_progress_slice_func = sm.report_progress_slice_func;
    image_container = sm.image_container;
    simulation_mode = sm.simulation_mode;
    stem_dets = sm.stem_dets;
    blocks_x = sm.blocks_x;
    blocks_y = sm.blocks_y;
    simulate_ctem_image = sm.simulate_ctem_image;
    max_inverse_factor = sm.max_inverse_factor;
    ccd_name = sm.ccd_name;
    ccd_binning = sm.ccd_binning;
    ccd_dose = sm.ccd_dose;
    structure_parameters_name = sm.structure_parameters_name;
    maintain_area = sm.maintain_area;
    live_stem = sm.live_stem;

    simulation_cell = std::make_shared<SimulationCell>(*(sm.simulation_cell));
    micro_params = std::make_shared<MicroscopeParameters>(*(sm.micro_params));
    sim_area = std::make_shared<SimulationArea>(*(sm.sim_area));
    stem_sim_area = std::make_shared<StemArea>(*(sm.stem_sim_area));
    cbed_pos = std::make_shared<CbedPosition>(*(sm.cbed_pos));
    incoherence_effects = std::make_shared<IncoherentEffects>(*(sm.incoherence_effects));

    return *this;
}

void SimulationManager::setStructure(std::string filePath, CIF::SuperCellInfo info, bool fix_cif)
{
    // lock this in case we need multiple devices to load this structure
    std::unique_lock<std::mutex> lock(structure_mutex);

    simulation_cell->setCrystalStructure(filePath, info, fix_cif);

    if (!maintain_area) {
        auto x_lims = simulation_cell->crystalStructure()->limitsX();
        auto y_lims = simulation_cell->crystalStructure()->limitsY();

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
        auto x_lims = simulation_cell->crystalStructure()->limitsX();
        auto y_lims = simulation_cell->crystalStructure()->limitsY();

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
    int numAtoms = simulation_cell->crystalStructure()->atomCountInRange(px[0], px[1], py[0], py[1]);

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
    return std::max(x_r, y_r) / sim_resolution;
}

double SimulationManager::inverseScale()
{
    if(!simulation_cell->crystalStructure() || !resolutionValid())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    return 1.0 / (realScale() * sim_resolution);
}

double SimulationManager::inverseMax()
{
    if(!simulation_cell->crystalStructure() || !resolutionValid())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    double inv_scale = inverseScale();
    return 0.5 * inv_scale * sim_resolution * inverseLimitFactor();
}

double SimulationManager::inverseScaleAngle() {
    if(!simulation_cell->crystalStructure() || !resolutionValid() || !(micro_params && micro_params->Voltage > 0))
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    double inv_scale = inverseScale();
    return 1000.0 * inv_scale * micro_params->Wavelength();
}

double SimulationManager::inverseMaxAngle()
{
    // need to do this in mrad, eventually should also pass inverse Angstrom for hover text?
    if(!simulation_cell->crystalStructure() || !resolutionValid() || !(micro_params && micro_params->Voltage > 0))
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    // this is the max reciprocal space scale for the entire image
    double angle_scale = inverseScaleAngle(); // apply cut off here, because we can
    return 0.5 * angle_scale * sim_resolution * inverseLimitFactor(); // half because we have a centered 0, 1000 to be in mrad
}

unsigned long SimulationManager::totalParts()
{
    if (simulation_mode == SimulationMode::CTEM || simulation_mode == SimulationMode::CBED)
        return static_cast<unsigned long>(incoherence_effects->iterations(simulation_mode));
    else if (simulation_mode == SimulationMode::STEM) {
        // round up as still need to complete that 'fraction of a job'
        unsigned int inelastic_runs = incoherence_effects->iterations(simulation_mode);
        return static_cast<unsigned long>(inelastic_runs * std::ceil(
                static_cast<double>(stemArea()->getNumPixels()) / parallel_pixels));
    }

    return 0;
}

void SimulationManager::updateImages(std::map<std::string, Image<double>> &ims, int jobCount, bool update)
{
    CLOG(DEBUG, "sim") << "Updating images";
    std::lock_guard<std::mutex> lck(image_update_mutex);
    CLOG(DEBUG, "sim") << "Got a mutex lock";

    for (auto const& i : ims)
    {
        CLOG(DEBUG, "sim") << "Processing image " << i.first;
        if (image_container.find(i.first) != image_container.end()) {
            CLOG(DEBUG, "sim") << "Adding to existing image";
            auto current = image_container[i.first];
            auto im = i.second;

            if (im.getSliceSize() != current.getSliceSize()) {
                CLOG(ERROR, "sim") << "Tried to merge simulation jobs with different output size";
                throw std::runtime_error("Tried to merge simulation jobs with different output size");
            }
            if (im.getWeightingSize() != current.getWeightingSize()) {
                CLOG(ERROR, "sim") << "Tried to merge simulation jobs with different weighting sizes";
                throw std::runtime_error("Tried to merge simulation jobs with different weighting sizes");
            }

            CLOG(DEBUG, "sim") << "Copying data";
            for (int j = 0; j < current.getDepth(); ++j)
                // we need to account for my complex number, that I have sort of bodged in, hence I calculate the k range as I have (and not slicesize)
                for (int k = 0; k < current.getSliceRef(j).size(); ++k)
                    current.getSliceRef(j)[k] += im.getSliceRef(j)[k]; // average factor is calculated using weighting now...

            // there is no weighting per slice at the moment
            for (int k = 0; k < current.getWeightingSize(); ++k)
                current.getWeightingRef()[k] += im.getWeightingRef()[k];

            image_container[i.first] = current;
        } else {
            CLOG(DEBUG, "sim") << "First time so creating image";
            // weighting is not done inside the image class
            image_container[i.first] = i.second;
        }
    }

    // count how many jobs have been done...
    complete_jobs += jobCount;

    auto v = totalParts();

    if (complete_jobs > v) {
        CLOG(ERROR, "sim") << "Simulation received more parts than it expected";
        throw std::runtime_error("Simulation received more parts than it expected");
    }

    auto prgrss = static_cast<double>(complete_jobs) / v;

    CLOG(DEBUG, "sim") << "Report progress: " << prgrss*100 << "%";

    reportTotalProgress(prgrss);

    // this means this simulation is finished
    if (image_return_func && (complete_jobs == v || update)) {
        CLOG(DEBUG, "sim") << "All parts of this job finished";
        image_return_func(*this);
    }
}

void SimulationManager::failedSimulation() {
    if (image_return_func) {
        CLOG(DEBUG, "sim") << "Returning blank data";
        image_return_func(*this);
    }
}


void SimulationManager::reportTotalProgress(double prog)
{
    if (report_progress_total_func)
        report_progress_total_func(prog);
}

void SimulationManager::reportSliceProgress(double prog) {
    if (report_progress_slice_func)
        report_progress_slice_func(prog);
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

std::valarray<unsigned int> SimulationManager::imageCrop() {
    double real_scale = realScale();

    auto x_im_range = rawSimLimitsX(0)[1] - rawSimLimitsX(0)[0];
    auto x_sim_range = paddedSimLimitsX(0)[1] - paddedSimLimitsX(0)[0];
    auto crop_lr_total = (std::floor(x_sim_range - x_im_range)  / real_scale);

    auto y_im_range = rawSimLimitsY(0)[1] - rawSimLimitsY(0)[0];
    auto y_sim_range = paddedSimLimitsY(0)[1] - paddedSimLimitsY(0)[0];
    auto crop_tb_total = (std::floor(y_sim_range - y_im_range)  / real_scale);

    auto crop_l = static_cast<unsigned int>(std::floor(crop_lr_total / 2.0));
    auto crop_b = static_cast<unsigned int>(std::floor(crop_tb_total / 2.0));

    auto crop_r = static_cast<unsigned int>(crop_lr_total - crop_l);
    auto crop_t = static_cast<unsigned int>(crop_tb_total - crop_b);

    return {crop_t, crop_l, crop_b, crop_r};
}

bool SimulationManager::resolutionValid() {
    return sim_resolution == 256 || sim_resolution == 512 || sim_resolution == 768 || sim_resolution == 1024 || sim_resolution == 1536 || sim_resolution == 2048 || sim_resolution == 3072 || sim_resolution == 4096 || sim_resolution == 8192;
}

std::valarray<double> SimulationManager::paddedSimLimitsX(int pixel) {
    return currentAreaBase(pixel).getCorrectedLimitsX() + simulation_cell->paddingX();
}

std::valarray<double> SimulationManager::paddedSimLimitsY(int pixel) {
    return currentAreaBase(pixel).getCorrectedLimitsY() + simulation_cell->paddingY();
}

std::valarray<double> SimulationManager::paddedSimLimitsZ() {
    return simulation_cell->paddedStructLimitsZ();
}

std::valarray<double> SimulationManager::paddedFullLimitsX() {
    return fullAreaBase().getCorrectedLimitsX() + simulation_cell->paddingX();
}

std::valarray<double> SimulationManager::paddedFullLimitsY() {
    return fullAreaBase().getCorrectedLimitsY() + simulation_cell->paddingY();
}

std::valarray<double> SimulationManager::rawSimLimitsX(int pixel) {
    return currentAreaBase(pixel).getRawLimitsX();
}

std::valarray<double> SimulationManager::rawSimLimitsY(int pixel) {
    return currentAreaBase(pixel).getRawLimitsY();
}

std::valarray<double> SimulationManager::rawFullLimitsX() {
    return fullAreaBase().getRawLimitsX();
}

std::valarray<double> SimulationManager::rawFullLimitsY() {
    return fullAreaBase().getRawLimitsY();
}

SimulationArea SimulationManager::fullAreaBase() {
    // This function takes whatever simulation type is active, and returns a 'SimulationArea' class to describe it's
    // limits
    SimulationArea sa;

    if (simulation_mode == SimulationMode::STEM)
        sa = static_cast<SimulationArea>(*stem_sim_area);
    else if (simulation_mode == SimulationMode::CBED)
        sa = cbed_pos->getSimArea();
    else if (simulation_mode == SimulationMode::CTEM)
        sa = *sim_area;

    return sa;
}

SimulationArea SimulationManager::currentAreaBase(int pixel) {
    // This function takes whatever simulation type is active, and returns a 'SimulationArea' class to describe it's
    // limits

    if (simulation_mode == SimulationMode::STEM && parallel_pixels == 1)
        // if parallel pixels are used, we need the full sim area...
        return stem_sim_area->getPixelSimArea(pixel);
    else
        return fullAreaBase(); // These don't ever change!
}

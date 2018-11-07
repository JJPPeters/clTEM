#include "simulationmanager.h"

#include <ios>
#include <fstream>
#include <memory>
#include <utilities/fileio.h>

std::valarray<float> const SimulationManager::default_xy_padding = {-8.0f, 8.0f};
std::valarray<float> const SimulationManager::default_z_padding = {-3.0f, 3.0f};

SimulationManager::SimulationManager() : Resolution(0), completeJobs(0), padding_x(SimulationManager::default_xy_padding),
                                         padding_y(SimulationManager::default_xy_padding), padding_z(SimulationManager::default_z_padding), slice_dz(1.0f),
                                         blocks_x(80), blocks_y(80), maxReciprocalFactor(2.0f / 3.0f), numParallelPixels(1), simulateCtemImage(true),
                                         ccd_name(""), ccd_binning(1), ccd_dose(10000.0f), TdsRunsCbed(1), TdsRunsStem(1), TdsEnabledCbed(false), TdsEnabledStem(false),
                                         slice_offset(0.0f), structure_parameters_name(""), structure_parameters(), maintain_area(false)
{
    // Here is where the default values are set!
    MicroParams = std::make_shared<MicroscopeParameters>();
    SimArea = std::make_shared<SimulationArea>();
    StemSimArea = std::make_shared<StemArea>();
    CbedPos = std::make_shared<CbedPosition>();
    thermal_vibrations = std::make_shared<ThermalVibrations>();


    Mode = SimulationMode::CTEM;
    full3dInts = 20;
    isFD = false;
    isF3D = false;

    // I'm really assuming the rest of the aberrations are default 0
    MicroParams->Aperture = 20;
    MicroParams->Voltage = 200;
    MicroParams->Delta = 30;
    MicroParams->Alpha = 0.3;
    MicroParams->C30 = 10000;

    std::random_device rd;
    rng = std::mt19937(rd());
    dist = std::normal_distribution<>(0, 1);
}

void SimulationManager::setStructure(std::string filePath)
{
    // lock this in case we need multiple devices to load this structure
    std::unique_lock<std::mutex> lock(structure_mutex);

    Structure.reset(new CrystalStructure(filePath));

    if (!maintain_area) {
        auto x_lims = getStructLimitsX();
        auto y_lims = getStructLimitsY();

        SimArea->setRawLimitsX(x_lims[0], x_lims[1]);
        SimArea->setRawLimitsY(y_lims[0], y_lims[1]);

        getStemArea()->setRawLimitsX(x_lims[0], x_lims[1]);
        getStemArea()->setRawLimitsY(y_lims[0], y_lims[1]);
    }
}

std::tuple<float, float, float, int> SimulationManager::getSimRanges()
{
    float xRange = getPaddedSimLimitsX()[1] - getPaddedSimLimitsX()[0];
    float yRange = getPaddedSimLimitsY()[1] - getPaddedSimLimitsY()[0];
    float zRange = getPaddedStructLimitsZ()[1] - getPaddedStructLimitsZ()[0];
    int numAtoms = Structure->getAtomCountInRange(getPaddedSimLimitsX()[0], getPaddedSimLimitsX()[1],
                                                  getPaddedSimLimitsY()[0], getPaddedSimLimitsY()[1]);

    return std::make_tuple(xRange, yRange, zRange, numAtoms);
}

float SimulationManager::getRealScale()
{
    if(!Structure || !haveResolution())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    auto x_r = getPaddedSimLimitsX()[1] - getPaddedSimLimitsX()[0];
    auto y_r = getPaddedSimLimitsY()[1] - getPaddedSimLimitsY()[0];
    return std::max(x_r, y_r) / Resolution;
}

float SimulationManager::getInverseScale()
{
    if(!Structure || !haveResolution())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

        return 1.0f / (getRealScale() * Resolution);
}

float SimulationManager::getInverseMax()
{
    if(!Structure || !haveResolution())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    return 0.5f * getInverseScale() * Resolution * getInverseLimitFactor();
}

float SimulationManager::getInverseScaleAngle() {
    if(!Structure || !haveResolution() && MicroParams && MicroParams->Voltage > 0)
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    float inv_scale = getInverseScale();
    return 1000.0f * inv_scale * MicroParams->Wavelength();
}

float SimulationManager::getInverseMaxAngle()
{
    // need to do this in mrad, eventually should also pass inverse Angstrom for hover text?
    if(!Structure || !haveResolution() && MicroParams && MicroParams->Voltage > 0)
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    // this is the max reciprocal space scale for the entire image
    float angle_scale = getInverseScaleAngle(); // apply cut off here, because we can
    return 0.5f * angle_scale * Resolution * getInverseLimitFactor(); // half because we have a centered 0, 1000 to be in mrad
}

unsigned long SimulationManager::getTotalParts()
{
    if (Mode == SimulationMode::CTEM)
        return 1;
    else if (Mode == SimulationMode::CBED)
        return (unsigned long) getTdsRuns();
    else if (Mode == SimulationMode::STEM)
        // round up as still need to complete that 'fraction of a job'
        return (unsigned long) (getTdsRuns() * std::ceil((float) getStemArea()->getNumPixels() / numParallelPixels));
}

void SimulationManager::updateImages(std::map<std::string, Image<float>> ims, int jobCount)
{
    std::lock_guard<std::mutex> lck(image_update_mtx);

    // this average factor is here to remove the effect of summing TDS configurations. i.e. the exposure is the same for TDS and non TDS
    auto average_factor = (float) getTdsRuns();

    for (auto const& i : ims)
    {
        if (Images.find(i.first) != Images.end()) {
            auto current = Images[i.first];
            auto im = i.second;
            if (im.data.size() != current.data.size())
                throw std::runtime_error("Tried to merge simulation jobs with different output size");
            for (int j = 0; j < current.data.size(); ++j)
                current.data[j] += im.data[j] / average_factor;
            Images[i.first] = current;
        } else {
            auto new_averaged = i.second;
            for (float &d : new_averaged.data)
                d /= average_factor; // need to average this as the image is created (if TDS)
            Images.insert(std::map<std::string, Image<float>>::value_type(i.first, new_averaged));
        }
    }

    // count how many jobs have been done...
    completeJobs += jobCount;

    auto v = getTotalParts();

    if (completeJobs > v)
        throw std::runtime_error("Simulation received more parts than it expected");

    reportTotalProgress((float) completeJobs / (float) getTotalParts());

    // this means this simulation is finished
    if (completeJobs == getTotalParts() && imageReturn)
        imageReturn(Images, *this);
}

void SimulationManager::reportTotalProgress(float prog)
{
    if (progressTotalReporter)
        progressTotalReporter(prog);
}

void SimulationManager::reportSliceProgress(float prog)
{
    if (progressSliceReporter)
        progressSliceReporter(prog);
}

float SimulationManager::getBlockScaleX()
{
    auto r = getPaddedStructLimitsX();
    return (r[1] - r[0]) / getBlocksX();
}

float SimulationManager::getBlockScaleY() {
    auto r = getPaddedStructLimitsY();
    return (r[1] - r[0]) / getBlocksY();
}

int SimulationManager::getBlocksX()
{
    calculate_blocks();
    return blocks_x;
}

int SimulationManager::getBlocksY()
{
    calculate_blocks();
    return blocks_y;
}

void SimulationManager::calculate_blocks()
{
    // set number of blocks. Set the blocks to be 4 Angstroms apart (as this is half our buffer region)
    // so we are never loading more than two extra blocks (I suppose smaller is better, but also might make the
    // arrays a bit convoluted) TODO: test if this matters
    auto x_lims_2 = getPaddedSimLimitsX();
    auto y_lims_2 = getPaddedSimLimitsY();
    float xr = x_lims_2[1] - x_lims_2[0];
    float yr = y_lims_2[1] - y_lims_2[0];

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
    float pre_pad = slice_offset + pad_slices_pre * slice_dz;

    float zw = getStructLimitsZ()[1] - getStructLimitsZ()[0];
    auto struct_slices = (int) std::ceil((zw + slice_offset) / slice_dz); // slice offset needed here?
    float struct_slice_thick = struct_slices * slice_dz;

    float left_over = struct_slice_thick - slice_offset - zw;

    auto pad_slices_post = (int) std::ceil((std::abs(p_z[0]) - left_over) / slice_dz);
    float post_pad = pad_slices_post * slice_dz + left_over;

    // The simulation works from LARGEST z to SMALLEST. So the pre padding is actually on top of the z structure.
    padding_z = {-post_pad, pre_pad};
}

bool SimulationManager::calculateFiniteDiffSliceThickness(float &dz_out) {
    auto x_lims = getPaddedSimLimitsX();
    auto sim_size = x_lims[1] - x_lims[0];
    // can check these are the same as Y

    double lambda = getWavelength();
    double sigma = MicroParams->Sigma();
    double V = MicroParams->Voltage * 1000.0;

    double ke = getInverseMax();
    double ke2 = ke * ke;

    // local copy of pi for convenience
    // using a double version as this calulation needs to be accurate
    double Pi = 3.141592653589793238462643383279502884;

    // This is just rearranging Eq 6.122 in Kirkland's book to a get a quadratic in (dz)^2 which we then solve. Could also adjust the band limiting.

    // start from kirklands equation, but group constants together
    double aa = 4 * Pi * Pi;
    double bb = 4 * Pi * Pi / (lambda * lambda);
    double cc = sigma * V / (lambda * Pi);

    // now rearrange and group again
    double A = (ke2 - cc) * aa;
    A = A * A;
    double B = 2 * (ke2 - cc) * aa - bb;
    double C = 3.0;

    double B24AC = B * B - 4 * A * C;

    // Now use these to determine acceptable resolution or enforce extra band limiting beyond 2/3
    if (B24AC < 0) {
//        throw std::runtime_error("No stable finite difference solution exists");
        return false;
    }

    double b24ac = std::sqrt(B24AC);
    double maxStableDz = (-B + b24ac) / (2 * A);

    if (maxStableDz < 0)
    {
        return false;
    }

    maxStableDz = 0.99f * std::sqrt(maxStableDz); // because we need it to be less than and we have (dz)^2, not dz

    // this was set in Adam's code, no idea why this is an upper limit?
    if (maxStableDz > 0.06f)
        maxStableDz = 0.06f;

    dz_out = (float) maxStableDz;

    return true;
}

float SimulationManager::getSliceThickness() {
    if (isFD) {
        float dz;
        bool valid = calculateFiniteDiffSliceThickness(dz);
        if (!valid)
            throw std::runtime_error("No stable finite difference solution exists");
        return dz;
    }
    else
        return slice_dz;
}

unsigned int SimulationManager::getNumberofSlices() {
    auto z_lims = getPaddedStructLimitsZ();
    float z_range = z_lims[1] - z_lims[0];

    // the 0.000001 is for errors in the float
    auto n_slices = (unsigned int) std::ceil((z_range / getSliceThickness()) - 0.000001);
    n_slices += (n_slices == 0);
    return n_slices;
}

unsigned int SimulationManager::getTdsRuns() {
    if (Mode == SimulationMode::STEM && TdsEnabledStem)
        return getStoredTdsRuns();
    else if (Mode == SimulationMode::CBED && TdsEnabledCbed)
        return getStoredTdsRuns();
    else
        return 1;
}

unsigned int SimulationManager::getStoredTdsRuns() {
    if (Mode == SimulationMode::CTEM)
        return 1;
    else if (Mode == SimulationMode::STEM)
        return TdsRunsStem;
    else if (Mode == SimulationMode::CBED)
        return TdsRunsCbed;

    return 1;
}

float SimulationManager::generateTdsFactor(AtomSite& at, int direction) {
    if (direction < 0 || direction > 2)
        throw std::runtime_error("Error trying to apply thermal displacement to axis: " + std::to_string(direction));

    // need element (just pass atom?)

    // TODO: check this behaves as expected, may want to reset the random stuff
    // sqrt as we have the mean squared displacement (variance), but want the standard deviation

    float u = 0.0f;

    if ( thermal_vibrations->force_default )
        u = thermal_vibrations->getDefault();
    else if (thermal_vibrations->force_defined)
        u = thermal_vibrations->getVibrations((unsigned int) at.A);
    else if (Structure->isThermalFileDefined()) {
        if (direction == 0)
            u = at.ux;
        else if (direction == 1)
            u = at.uy;
        else if (direction == 2)
            u = at.uz;
    } else {
        // defaults are built into this
        u = thermal_vibrations->getVibrations((unsigned int) at.A);
    }


    float randNormal = std::sqrt(u) * (float) dist(rng);

    return randNormal;
}

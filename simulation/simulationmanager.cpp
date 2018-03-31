#include "simulationmanager.h"

#include <ios>
#include <fstream>

//#include <QtCore/QFile>
//#include <QtCore/QTextStream>

std::valarray<float> const SimulationManager::default_xy_padding = {-8.0f, 8.0f};
std::valarray<float> const SimulationManager::default_z_padding = {-3.0f, 3.0f};

SimulationManager::SimulationManager() : Resolution(0), completeJobs(0), padding_x(SimulationManager::default_xy_padding),
                                         padding_y(SimulationManager::default_xy_padding), padding_z(SimulationManager::default_z_padding), slice_dz(1.0f),
                                         blocks_x(80), blocks_y(80), maxReciprocalFactor(2.0f / 3.0f)
{
    // Here is where the default values are set!
    MicroParams = std::shared_ptr<MicroscopeParameters>(new MicroscopeParameters);
    SimArea = std::shared_ptr<SimulationArea>(new SimulationArea);
    StemSimArea = std::shared_ptr<StemArea>(new StemArea);
    CbedPos = std::shared_ptr<CbedPosition>(new CbedPosition);

    Mode = SimulationMode::CTEM;
    full3dInts = 20;
    isFD = false;
    isF3D = false;
    TdsRuns = 1;

    // I'm really assuming the rest of the aberrations are default 0
    MicroParams->Aperture = 20;
    MicroParams->Voltage = 200;
    MicroParams->Delta = 3;
    MicroParams->Alpha = 0.5;
    MicroParams->C30 = 10000;
}

void SimulationManager::setStructure(std::string filePath)
{
    // lock this in case we need multiple devices to load this structure
    std::unique_lock<std::mutex> lock(structure_mutex);

    Structure.reset(new CrystalStructure(filePath));

    auto x_lims = getStructLimitsX();
    auto y_lims = getStructLimitsY();

    SimArea->setRangeX(x_lims[0], x_lims[1]);
    SimArea->setRangeY(y_lims[0], y_lims[1]);

    calculate_blocks();
}

std::tuple<float, float, float, int> SimulationManager::getSimRanges()
{
    round_padding();
    float xRange = getPaddedSimLimitsX()[1] - getPaddedSimLimitsX()[0];
    float yRange = getPaddedSimLimitsY()[1] - getPaddedSimLimitsY()[0];
    float zRange = getPaddedStructLimitsZ()[1] - getPaddedStructLimitsZ()[0];
    int numAtoms = Structure->getAtomCountInRange(getPaddedSimLimitsX()[0], getPaddedSimLimitsX()[1],
                                                  getPaddedSimLimitsY()[0], getPaddedSimLimitsY()[1]);

    return std::make_tuple(xRange, yRange, zRange, numAtoms);
}

//float SimulationManager::getSimSideLength()
//{
//    if(!Structure || !haveResolution())
//        throw std::runtime_error("Can't calculate scales without resolution and structure");
//
//    return SimArea->getLargestSimXyRange();
//}

float SimulationManager::getRealScale()
{
    if(!Structure || !haveResolution())
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    round_padding(); // TODO: do I need a fallback for e.g. no atoms in range?

//    return SimArea->getLargestSimXyRange() / Resolution;
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
    // need to do this in mrad, eventually should also pass inverse Angstrom for hover text?
    if(!Structure || !haveResolution() && MicroParams && MicroParams->Voltage > 0)
        throw std::runtime_error("Can't calculate scales without resolution and structure");

    // this is the max reciprocal space scale for the entire image
    float maxFreq =  1.0f / (2.0f * getRealScale());
    return 1000.0f * maxFreq * MicroParams->Wavelength() / 2.0f;
}

unsigned long SimulationManager::getTotalParts()
{
    if (Mode == SimulationMode::CTEM)
        return 1;
    else if (Mode == SimulationMode::CBED)
        return (unsigned long) getTdsRuns();
    else if (Mode == SimulationMode::STEM)
        // round up as still need to complete that 'fraction of a job'
        return (unsigned long) (getTdsRuns() * std::ceil(getStemArea()->getNumPixels() / numParallelPixels));
}

void SimulationManager::updateImages(std::map<std::string, Image<float>> ims, int jobCount)
{
    std::lock_guard<std::mutex> lck(image_update_mtx);

    for (auto const& i : ims)
    {

        if (Images.find(i.first) != Images.end()) {
            auto current = Images[i.first];
            auto im = i.second;
            if (im.data.size() != current.data.size())
                throw std::runtime_error("Tried to merge simulation jobs with different output size");
            for (int j = 0; j < current.data.size(); ++j)
                current.data[j] += im.data[j];
            Images[i.first] = current;
        } else
            Images.insert(std::map<std::string, Image<float>>::value_type(i.first, i.second));
    }

    // count how many jobs have been done...
    completeJobs += jobCount;

    if (completeJobs > getTotalParts())
        throw std::runtime_error("Simulation received more parts than it expected");

    // this means this simulation is finished
    if (completeJobs == getTotalParts() && imageReturn)
        imageReturn(Images);
}

void SimulationManager::reportProgress(float prog)
{
    if (progressReporter)
        progressReporter(prog);
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
//    calculate_blocks();
    return blocks_x;
}

int SimulationManager::getBlocksY()
{
//    calculate_blocks();
    return blocks_y;
}

void SimulationManager::calculate_blocks()
{
    // set number of blocks. Set the blocks to be 4 Angstroms apart (as this is half our buffer region)
    // so we are never loading more than two extra blocks (I suppose smaller is better, but also might make the
    // arrays a bit convoluted) TODO: test if this matters
    auto x_lims_2 = getPaddedStructLimitsX();
    auto y_lims_2 = getPaddedStructLimitsY();
    float xr = x_lims_2[1] - x_lims_2[0];
    float yr = y_lims_2[1] - y_lims_2[0];

    // always using x and y as same size (for now) so find the larger dimension
    // floor so blocks will be slightly larger than 4 Angstroms
    auto n_blocks = (int) std::floor(std::max(xr, yr) / 8.0);
    blocks_x = n_blocks;
    blocks_y = n_blocks;
}

void SimulationManager::round_padding()
{
    if (!haveResolution())
        return;

    if (Mode != SimulationMode::CTEM)
    {
        padding_x = SimulationManager::default_xy_padding;
        padding_y = SimulationManager::default_xy_padding;
        return;
    }

    float xw = getSimLimitsX()[1] - getSimLimitsX()[0];
    float yw = getSimLimitsY()[1] - getSimLimitsY()[0];

    float dim = std::max(xw, yw);
    int res = getResolution();

    float padding = calculateRoundedPadding(dim, res);

    padding_x = {-padding, padding};
    padding_y = {-padding, padding};
}

std::valarray<float> SimulationManager::getSimLimitsX()
{
    if (Mode == SimulationMode::STEM)
        return StemSimArea->getLimitsX();
    else if (Mode == SimulationMode::CBED)
        return {CbedPos->getXPos(), CbedPos->getXPos()};
    else
        return SimArea->getLimitsX();
}

std::valarray<float> SimulationManager::getSimLimitsY()
{
    if (Mode == SimulationMode::STEM)
        return StemSimArea->getLimitsY();
    else if (Mode == SimulationMode::CBED)
        return {CbedPos->getYPos(), CbedPos->getYPos()};
    else
        return SimArea->getLimitsY();
}

float SimulationManager::calculatePaddedRealScale(float range, int resolution, bool round_padding) {

    float padding = 0.0f;
    if (round_padding)
        padding = 2 * calculateRoundedPadding(range, resolution);
    else
        padding = SimulationManager::default_xy_padding[1] - SimulationManager::default_xy_padding[0];

    return (range + 2 * padding) / (float) resolution;
}

float SimulationManager::calculateRoundedPadding(float range, int resolution)
{
    float pd = SimulationManager::default_xy_padding[1] - SimulationManager::default_xy_padding[0];

    float n_f = (pd * resolution) / (range + pd);
    float n = (int) std::ceil(n_f);

    // had some integer rouding errors so made everything a float...
    float padding = range / ( ((float) resolution / (float) n) - 1 ); // this is total padding
    return padding / 2;
}

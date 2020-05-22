#include <utility>

#include <utility>

#include <utility>

#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H

#include <string>
#include <mutex>
#include <map>
#include <valarray>
#include <inelastic/phonon.h>
#include <inelastic/plasmon.h>
#include <inelastic/inelastic.h>
#include <structure/simulationcell.h>

#include "structure/structureparameters.h"
#include "utilities/commonstructs.h"
#include "utilities/enums.h"
#include "utilities/stringutils.h"
#include "utilities/logging.h"

class SimulationManager
{
public:
    SimulationManager();

    SimulationManager(const SimulationManager& sm)
            : structure_mutex(), image_update_mtx(), Resolution(sm.Resolution),
              numParallelPixels(sm.numParallelPixels), isF3D(sm.isF3D), full3dInts(sm.full3dInts),
              completeJobs(sm.completeJobs), imageReturn(sm.imageReturn), progressTotalReporter(sm.progressTotalReporter), progressSliceReporter(sm.progressSliceReporter),
              Images(sm.Images), Mode(sm.Mode), StemDets(sm.StemDets),
              blocks_x(sm.blocks_x), blocks_y(sm.blocks_y), simulateCtemImage(sm.simulateCtemImage),
              maxReciprocalFactor(sm.maxReciprocalFactor), ccd_name(sm.ccd_name), ccd_binning(sm.ccd_binning), ccd_dose(sm.ccd_dose),
              structure_parameters_name(sm.structure_parameters_name), maintain_area(sm.maintain_area),
              use_double_precision(sm.use_double_precision),
              intermediate_slices_enabled(sm.intermediate_slices_enabled), intermediate_slices(sm.intermediate_slices)
    {
        MicroParams = std::make_shared<MicroscopeParameters>(*(sm.MicroParams));
        SimArea = std::make_shared<SimulationArea>(*(sm.SimArea));
        StemSimArea = std::make_shared<StemArea>(*(sm.StemSimArea));
        CbedPos = std::make_shared<CbedPosition>(*(sm.CbedPos));
        inelastic_scattering = std::make_shared<InelasticScattering>(*(sm.inelastic_scattering));

        if (sm.simulation_cell)// structure doesnt always exist
            simulation_cell = std::make_shared<SimulationCell>(*(sm.simulation_cell));
    }

    SimulationManager& operator=(const SimulationManager& sm)
    {
        intermediate_slices_enabled = sm.intermediate_slices_enabled;
        intermediate_slices = sm.intermediate_slices;
        use_double_precision = sm.use_double_precision;
        Resolution = sm.Resolution;
        numParallelPixels = sm.numParallelPixels;
        isF3D = sm.isF3D;
        full3dInts = sm.full3dInts;
        completeJobs = sm.completeJobs;
        imageReturn = sm.imageReturn;
        progressTotalReporter = sm.progressTotalReporter;
        progressSliceReporter = sm.progressSliceReporter;
        Images = sm.Images;
        Mode = sm.Mode;
        StemDets = sm.StemDets;
        blocks_x = sm.blocks_x;
        blocks_y = sm.blocks_y;
        simulateCtemImage = sm.simulateCtemImage;
        maxReciprocalFactor = sm.maxReciprocalFactor;
        ccd_name = sm.ccd_name;
        ccd_binning = sm.ccd_binning;
        ccd_dose = sm.ccd_dose;
        structure_parameters_name = sm.structure_parameters_name;
        maintain_area = sm.maintain_area;

        if (sm.simulation_cell) // structure doesnt always exist
            simulation_cell = std::make_shared<SimulationCell>(*(sm.simulation_cell));
        MicroParams = std::make_shared<MicroscopeParameters>(*(sm.MicroParams));
        SimArea = std::make_shared<SimulationArea>(*(sm.SimArea));
        StemSimArea = std::make_shared<StemArea>(*(sm.StemSimArea));
        CbedPos = std::make_shared<CbedPosition>(*(sm.CbedPos));
        inelastic_scattering = std::make_shared<InelasticScattering>(*(sm.inelastic_scattering));

        return *this;
    }

    void setStructure(std::string fPath, CIF::SuperCellInfo info = CIF::SuperCellInfo(), bool fix_cif=false);

    void setStructure(CIF::CIFReader cif, CIF::SuperCellInfo info);

    unsigned long totalParts();

    std::shared_ptr<SimulationCell> simulationCell() {return simulation_cell;}

    std::shared_ptr<MicroscopeParameters> microscopeParams() {return MicroParams;}

    std::shared_ptr<SimulationArea> simulationArea() {return SimArea;}

    std::vector<StemDetector>& stemDetectors() {return StemDets;}

    std::shared_ptr<StemArea> stemArea() {return StemSimArea;}

    std::shared_ptr<CbedPosition> cbedPosition() {return CbedPos;}

    SimulationArea ctemArea() {return *SimArea;}

    // should have corrected the shift issue so this more intuitive version works!!
    // this covers only the area that is needed right now (i.e. for just this pixel)
    std::valarray<double> paddedSimLimitsX(int pixel) {
        return currentAreaBase(pixel).getCorrectedLimitsX() + simulation_cell->paddingX();
    }
    std::valarray<double> paddedSimLimitsY(int pixel) {
        return currentAreaBase(pixel).getCorrectedLimitsY() + simulation_cell->paddingY();
    }
    std::valarray<double> paddedSimLimitsZ() {
        return simulation_cell->paddedStructLimitsZ();
    }

    // this one covers the total sim area (even if it is not all needed for each simulation, ie area covered by all pixels)
    std::valarray<double> paddedFullLimitsX() {
        return fullAreaBase().getCorrectedLimitsX() + simulation_cell->paddingX();
    }
    std::valarray<double> paddedFullLimitsY() {
        return fullAreaBase().getCorrectedLimitsY() + simulation_cell->paddingY();
    }

    std::valarray<double> rawSimLimitsX(int pixel) {
        return currentAreaBase(pixel).getRawLimitsX();
    }
    std::valarray<double> rawSimLimitsY(int pixel) {
        return currentAreaBase(pixel).getRawLimitsY();
    }

    std::valarray<double> rawFullLimitsX() {
        return fullAreaBase().getRawLimitsX();
    }
    std::valarray<double> rawFullLimitsY() {
        return fullAreaBase().getRawLimitsY();
    }

    std::valarray<unsigned int> imageCropEnabled() {
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

    int blocksX();
    int blocksY();

    double blockScaleX();
    double blockScaleY();

    std::tuple<double, double, double, int> simRanges();

    void setResolution(unsigned int res) {Resolution = res;}
    unsigned int resolution() {return Resolution;}
    void setMode(SimulationMode md){Mode = md;}
    SimulationMode mode(){return Mode;}
    std::string modeString() {return Utils::simModeToString(Mode);}

    bool resolutionValid() {return Resolution == 256 || Resolution == 512 || Resolution == 768 || Resolution == 1024 || Resolution == 1536 || Resolution == 2048 || Resolution == 3072 || Resolution == 4096 || Resolution == 8192;}

    /// Get the simulation scale in Angstroms per pixel
    double realScale();
    /// Get the simulation inverse scale in inverse Angstroms per pixel
    double inverseScale();
    /// GGet the max band limited simulation inverse in inverse Angstroms per pixel
    double inverseMax();
    /// Get the simulation inverse scale in mrad per pixel
    double inverseScaleAngle();
    /// Get the max band limited simulation inverse in mrad
    double inverseMaxAngle();
    double inverseLimitFactor() {return maxReciprocalFactor;}

    bool full3dEnabled(){return isF3D;}

    void setFull3dEnabled(bool use) {isF3D = use;}
    unsigned int full3dIntegrals(){return full3dInts;}
    unsigned int parallelPixels() {return (Mode != SimulationMode::STEM) ? 1 : numParallelPixels;}
    void setParallelPixels(unsigned int npp) {numParallelPixels = npp;}
    void setFull3dIntegrals(unsigned int n3d){full3dInts= n3d;}

    //
    // Inelastic scattering
    //

    std::shared_ptr<InelasticScattering> inelasticScattering() {return inelastic_scattering;}

    //
    // Return functions
    //

    void setImageReturnFunc(std::function<void(SimulationManager)> f) {imageReturn = std::move(f);}
    void setProgressTotalReporterFunc(std::function<void(double)> f) {progressTotalReporter = std::move(f);}
    void setProgressSliceReporterFunc(std::function<void(double)> f) {progressSliceReporter = std::move(f);}


    void updateImages(std::map<std::string, Image<double>> &ims, int jobCount);
    std::map<std::string, Image<double>> images() { return Images; }
    void failedSimulation();

    void reportTotalProgress(double prog);
    void reportSliceProgress(double prog);

    bool ctemImageEnabled() {return simulateCtemImage;}
    void setCtemImageEnabled(bool val) {simulateCtemImage = val;}

    std::string ccdName() {return ccd_name;}
    void setCcdName(std::string nm) {ccd_name = std::move(nm);}

    int ccdBinning() {return ccd_binning;}
    void setCcdBinning(int bin) {ccd_binning = bin;}

    double const ccdDose() {return ccd_dose;}
    void setCcdDose(double dose) {ccd_dose = dose;}

    void setStructureParameters(std::string name) { structure_parameters_name = std::move(name); }

    Parameterisation structureParameters() {return StructureParameters::getParameters(structure_parameters_name);}
    std::vector<double> structureParametersData() {return StructureParameters::getParametersData(structure_parameters_name);}
    std::string structureParametersName() {return structure_parameters_name;}

    void setMaintainAreas(bool maintain) {maintain_area = maintain;}
    bool maintainAreas() {return maintain_area;}

    bool doublePrecisionEnabled() {return use_double_precision;}
    void setDoublePrecisionEnabled(bool ddp) {use_double_precision = ddp;}

    unsigned int intermediateSliceStep() {return intermediate_slices_enabled ? intermediate_slices : 0;}
    unsigned int storedintermediateSliceStep() {return intermediate_slices;}
    bool intermediateSlicesEnabled() {return intermediate_slices_enabled;}

    void setIntermediateSlices(unsigned int is) {intermediate_slices = is;}
    void setIntermediateSlicesEnabled(bool ise) {intermediate_slices_enabled = ise;}

private:
    bool use_double_precision;

    SimulationArea fullAreaBase() {
        // This function takes whatever simulation type is active, and returns a 'SimulationArea' class to describe it's
        // limits
        SimulationArea sa;

        if (Mode == SimulationMode::STEM)
            sa = static_cast<SimulationArea>(*StemSimArea);
        else if (Mode == SimulationMode::CBED)
            sa = CbedPos->getSimArea();
        else if (Mode == SimulationMode::CTEM)
            sa = *SimArea;

        return sa;
    }

    SimulationArea currentAreaBase(int pixel) {
        // This function takes whatever simulation type is active, and returns a 'SimulationArea' class to describe it's
        // limits

        if (Mode == SimulationMode::STEM && numParallelPixels == 1)
            // if parallel pixels are used, we need the full sim area...
            return StemSimArea->getPixelSimArea(pixel);
        else
            return fullAreaBase(); // These don't ever change!
    }

    std::string structure_parameters_name;

    bool maintain_area;

    std::mutex structure_mutex;

    std::shared_ptr<SimulationCell> simulation_cell;

    unsigned int Resolution;
    bool simulateCtemImage;

    //
    // Inelastic scattering
    //

    std::shared_ptr<InelasticScattering> inelastic_scattering;

    //
    //
    //

    double maxReciprocalFactor;

    // STEM only
    unsigned int numParallelPixels;

    void calculateBlocks();
    int blocks_x, blocks_y;

    bool isF3D;

    unsigned int full3dInts;

    std::string ccd_name;
    int ccd_binning;
    double ccd_dose;

    // Exporting intermediate slices

    unsigned int intermediate_slices;
    bool intermediate_slices_enabled;

    // Data return

    unsigned int completeJobs;

    std::mutex image_update_mtx;

    std::function<void(SimulationManager)> imageReturn;
    std::function<void(double)> progressTotalReporter;
    std::function<void(double)> progressSliceReporter;

    std::map<std::string, Image<double>> Images;

    std::shared_ptr<MicroscopeParameters> MicroParams;

    std::shared_ptr<SimulationArea> SimArea;

    std::vector<StemDetector> StemDets;

    std::shared_ptr<StemArea> StemSimArea;

    std::shared_ptr<CbedPosition> CbedPos;

    SimulationMode Mode;
};

#endif // SIMULATIONMANAGER_H

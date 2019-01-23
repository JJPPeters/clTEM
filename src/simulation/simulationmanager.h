#include <utility>

#include <utility>

#include <utility>

#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H

#include <string>
#include <mutex>
#include <map>
#include <valarray>
#include <structure/thermalvibrations.h>

#include "structure/crystalstructure.h"
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
            : structure_mutex(), image_update_mtx(), Resolution(sm.Resolution), TdsRunsStem(sm.TdsRunsStem), TdsRunsCbed(sm.TdsRunsCbed),
              numParallelPixels(sm.numParallelPixels), isF3D(sm.isF3D), full3dInts(sm.full3dInts),
              completeJobs(sm.completeJobs), imageReturn(sm.imageReturn), progressTotalReporter(sm.progressTotalReporter), progressSliceReporter(sm.progressSliceReporter),
              Images(sm.Images), Mode(sm.Mode), StemDets(sm.StemDets), TdsEnabledStem(sm.TdsEnabledStem), TdsEnabledCbed(sm.TdsEnabledCbed),
              default_xy_padding(sm.default_xy_padding), default_z_padding(sm.default_z_padding),
              padding_x(sm.padding_x), padding_y(sm.padding_y), padding_z(sm.padding_z), slice_dz(sm.slice_dz),
              blocks_x(sm.blocks_x), blocks_y(sm.blocks_y), simulateCtemImage(sm.simulateCtemImage),
              maxReciprocalFactor(sm.maxReciprocalFactor), ccd_name(sm.ccd_name), ccd_binning(sm.ccd_binning), ccd_dose(sm.ccd_dose),
              slice_offset(sm.slice_offset), structure_parameters_name(sm.structure_parameters_name), maintain_area(sm.maintain_area),
              rng(std::mt19937(std::random_device()())), dist(std::normal_distribution<>(0, 1))
    {
        MicroParams = std::make_shared<MicroscopeParameters>(*(sm.MicroParams));
        SimArea = std::make_shared<SimulationArea>(*(sm.SimArea));
        StemSimArea = std::make_shared<StemArea>(*(sm.StemSimArea));
        CbedPos = std::make_shared<CbedPosition>(*(sm.CbedPos));
        thermal_vibrations = std::make_shared<ThermalVibrations>(*(sm.thermal_vibrations));

        if (sm.Structure)// structure doesnt always exist
            Structure = std::make_shared<CrystalStructure>(*(sm.Structure));
    }

    SimulationManager& operator=(const SimulationManager& sm)
    {
        Resolution = sm.Resolution;
        TdsRunsStem = sm.TdsRunsStem;
        TdsRunsCbed = sm.TdsRunsCbed;
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
        TdsEnabledStem = sm.TdsEnabledStem;
        TdsEnabledCbed = sm.TdsEnabledCbed;
        default_xy_padding = sm.default_xy_padding;
        default_z_padding = sm.default_z_padding;
        padding_x = sm.padding_x;
        padding_y = sm.padding_y;
        padding_z = sm.padding_z;
        slice_dz = sm.slice_dz;
        blocks_x = sm.blocks_x;
        blocks_y = sm.blocks_y;
        simulateCtemImage = sm.simulateCtemImage;
        maxReciprocalFactor = sm.maxReciprocalFactor;
        ccd_name = sm.ccd_name;
        ccd_binning = sm.ccd_binning;
        ccd_dose = sm.ccd_dose;
        slice_offset = sm.slice_offset;
        structure_parameters_name = sm.structure_parameters_name;
        maintain_area = sm.maintain_area;
        rng = std::mt19937(std::random_device()());
        dist = std::normal_distribution<>(0, 1);

        if (sm.Structure) // structure doesnt always exist
            Structure = std::make_shared<CrystalStructure>(*(sm.Structure));
        MicroParams = std::make_shared<MicroscopeParameters>(*(sm.MicroParams));
        SimArea = std::make_shared<SimulationArea>(*(sm.SimArea));
        StemSimArea = std::make_shared<StemArea>(*(sm.StemSimArea));
        CbedPos = std::make_shared<CbedPosition>(*(sm.CbedPos));
        thermal_vibrations = std::make_shared<ThermalVibrations>(*(sm.thermal_vibrations));

        return *this;
    }

    void setStructure(std::string filePath);

    unsigned long getTotalParts();

    std::shared_ptr<CrystalStructure> getStructure() {return Structure;}

    std::shared_ptr<MicroscopeParameters> getMicroscopeParams() {return MicroParams;}

    std::shared_ptr<SimulationArea> getSimulationArea() {return SimArea;}

    std::vector<StemDetector>& getDetectors() {return StemDets;}

    std::shared_ptr<StemArea> getStemArea() {return StemSimArea;}

    std::shared_ptr<CbedPosition> getCBedPosition() {return CbedPos;}

    bool haveStructure() {
        if (Structure)
            return true;
        return false;
    }

    SimulationArea getCtemArea() {return *SimArea;}

    void setDefaultPaddingXY(const std::valarray<float> &pd_xy) {
        if(pd_xy.size() != 2)
            throw std::runtime_error("XY padding value must be valarray with 2 values");
        default_xy_padding = pd_xy;
    }

    void setDefaultPaddingZ(const std::valarray<float> &pd_z) {
        if(pd_z.size() != 2)
            throw std::runtime_error("Z padding value must be valarray with 2 values");
        default_z_padding = pd_z;
    }

    // The rounding doesnt always do something, but is for a uniform style so stuff can be added in later
    std::valarray<float> getDefaultPaddingXY() {return default_xy_padding;}
    std::valarray<float> getDefaultPaddingZ() {return default_z_padding;}

    std::valarray<float> getPaddingX() {round_X_padding(); return padding_x;}
    std::valarray<float> getPaddingY() {round_Y_padding(); return padding_y;}
    std::valarray<float> getPaddingZ() {round_Z_padding(); return padding_z;}

    std::valarray<float> getStructLimitsX() {
        if (Structure)
            return Structure->getLimitsX();
        else
            return {0.f, 0.f};
    }

    std::valarray<float> getStructLimitsY() {
        if (Structure)
            return Structure->getLimitsY();
        else
            return {0.f, 0.f};
    }
    std::valarray<float> getStructLimitsZ() {
        if (Structure)
            return Structure->getLimitsZ();
        else
            return {0.f, 0.f};
    }

    std::valarray<float> getPaddedStructLimitsX() { return getStructLimitsX() + getPaddingX(); }
    std::valarray<float> getPaddedStructLimitsY() { return getStructLimitsY() + getPaddingY(); }
    std::valarray<float> getPaddedStructLimitsZ() { return getStructLimitsZ() + getPaddingZ(); }

    // should have corrected the shift issue so this more intuitive version works!!
    std::valarray<float> getPaddedSimLimitsX() {
        return getCurrentAreaBase().getCorrectedLimitsX() + getPaddingX();
    }
    std::valarray<float> getPaddedSimLimitsY() {
        return getCurrentAreaBase().getCorrectedLimitsY() + getPaddingY();
    }

    std::valarray<float> getRawSimLimitsX() {
        return getCurrentAreaBase().getRawLimitsX();
    }
    std::valarray<float> getRawSimLimitsY() {
        return getCurrentAreaBase().getRawLimitsY();
    }

    int getBlocksX();
    int getBlocksY();

    float getBlockScaleX();
    float getBlockScaleY();

    std::tuple<float, float, float, int> getSimRanges();

    void setResolution(unsigned int res) {Resolution = res;}
    unsigned int getResolution() {return Resolution;}
    void setMode(SimulationMode md){Mode = md;}
    SimulationMode getMode(){return Mode;}
    std::string getModeString() {return Utils::simModeToString(Mode);}

    bool haveResolution() {return Resolution == 256 || Resolution == 512 || Resolution == 768 || Resolution == 1024 || Resolution == 1536 || Resolution == 2048 || Resolution == 3072 || Resolution == 4096;}

    /// Get the simulation scale in Angstroms per pixel
    float getRealScale();
    /// Get the simulation inverse scale in inverse Angstroms per pixel
    float getInverseScale();
    /// Get the simulation inverse scale in mrad per pixel
    float getInverseScaleAngle();
    /// Get the max band limited simulation inverse in mrad
    float getInverseMaxAngle();
    float getInverseLimitFactor() {return maxReciprocalFactor;}

    float getWavelength() {return MicroParams->Wavelength();}
    bool isFull3d(){return isF3D;}

    void setFull3d(bool use) {isF3D = use;}

    unsigned int getFull3dInts(){return full3dInts;}
    unsigned int getStoredTdsRuns();
    unsigned int getTdsRuns();
    unsigned int getStoredTdsRunsCbed() { return TdsRunsCbed; }
    unsigned int getStoredTdsRunsStem() { return TdsRunsStem; }
    unsigned int getTdsRunsCbed() { return (!TdsEnabledCbed) ? 1 : TdsRunsCbed; }
    unsigned int getTdsRunsStem() { return (!TdsEnabledStem) ? 1 : TdsRunsStem; }
    unsigned int getParallelPixels() {return (Mode != SimulationMode::STEM) ? 1 : numParallelPixels;}
    bool getTdsEnabledStem() { return TdsEnabledStem; }
    bool getTdsEnabledCbed() { return TdsEnabledCbed; }

    void setTdsEnabledStem(bool use){TdsEnabledStem = use;}
    void setTdsEnabledCbed(bool use){TdsEnabledCbed = use;}
    void setTdsRunsStem(unsigned int runs){TdsRunsStem = runs;}
    void setTdsRunsCbed(unsigned int runs){TdsRunsCbed = runs;}
    void setParallelPixels(unsigned int npp) {numParallelPixels = npp;}
    void setFull3dInts(unsigned int n3d){full3dInts= n3d;}

    void setImageReturnFunc(std::function<void(std::map<std::string, Image<float>>, SimulationManager)> f) {imageReturn = std::move(f);}
    void setProgressTotalReporterFunc(std::function<void(float)> f) {progressTotalReporter = std::move(f);}
    void setProgressSliceReporterFunc(std::function<void(float)> f) {progressSliceReporter = std::move(f);}

    void updateImages(std::map<std::string, Image<float>> ims, int jobCount);
    void failedSimulation();


    void reportTotalProgress(float prog);
    void reportSliceProgress(float prog);

    bool getSimulateCtemImage() {return simulateCtemImage;}
    void setSimulateCtemImage(bool val) {simulateCtemImage = val;}

    std::string getCcdName() {return ccd_name;}
    void setCcdName(std::string nm) {ccd_name = std::move(nm);}

    int getCcdBinning() {return ccd_binning;}
    void setCcdBinning(int bin) {ccd_binning = bin;}

    float getCcdDose() {return ccd_dose;}
    void setCcdDose(float dose) {ccd_dose = dose;}

    float getSliceThickness() {return slice_dz;}
    float getSliceOffset() {return slice_offset;}

    unsigned int getNumberofSlices();

    void setSliceThickness(float thk) { slice_dz = thk; }
    void setSliceOffset(float off) { slice_offset = off; }

    void setStructureParameters(std::string name) { structure_parameters_name = std::move(name); }

    Parameterisation getStructureParameter() {return StructureParameters::getParameter(structure_parameters_name);}
    std::vector<float> getStructureParameterData() {return StructureParameters::getParameterData(structure_parameters_name);}
    std::string getStructureParametersName() {return structure_parameters_name;}

    std::shared_ptr<ThermalVibrations> getThermalVibrations() {return thermal_vibrations;}

    float generateTdsFactor(AtomSite& at, int direction);

    void setMaintainAreas(bool maintain) {maintain_area = maintain;}
    bool getMaintainAreas() {return maintain_area;}

private:
    std::valarray<float> default_xy_padding;
    std::valarray<float> default_z_padding;

    // Nothing happens with the xy padding now, but z needs to be rounded to get the correct slice offset
    void round_X_padding() {padding_x = default_xy_padding;}
    void round_Y_padding() {padding_y = default_xy_padding;}
    void round_Z_padding();

    SimulationArea getCurrentAreaBase() {
        // This function takes whatever simulation type is active, and returns a 'SimulationArea' class to describe it's
        // limits
        SimulationArea sa;

        if (Mode == SimulationMode::STEM)
            sa = static_cast<SimulationArea>(*StemSimArea);
        else if (Mode == SimulationMode::CBED)
            sa = CbedPos->getSimArea();
        else
            sa = *SimArea;

        return sa;
    }

    std::string structure_parameters_name;

    bool maintain_area;

    std::mt19937 rng;
    std::normal_distribution<> dist;
    std::shared_ptr<ThermalVibrations> thermal_vibrations;

    std::mutex structure_mutex;

    std::shared_ptr<CrystalStructure> Structure;

    std::valarray<float> padding_x, padding_y, padding_z;

    unsigned int Resolution;
    unsigned int TdsRunsStem;
    unsigned int TdsRunsCbed;
    bool TdsEnabledStem;
    bool TdsEnabledCbed;
    bool simulateCtemImage;

    float maxReciprocalFactor;

    // STEM only
    unsigned int numParallelPixels;

    float slice_dz;
    float slice_offset;

    void calculate_blocks();
    int blocks_x, blocks_y;

    bool isF3D;

    unsigned int full3dInts;

    std::string ccd_name;
    int ccd_binning;
    float ccd_dose;

    // Data return

    unsigned int completeJobs;

    std::mutex image_update_mtx;

    std::function<void(std::map<std::string, Image<float>>, SimulationManager)> imageReturn;
    std::function<void(float)> progressTotalReporter;
    std::function<void(float)> progressSliceReporter;

    std::map<std::string, Image<float>> Images;

    std::shared_ptr<MicroscopeParameters> MicroParams;

    std::shared_ptr<SimulationArea> SimArea;

    std::vector<StemDetector> StemDets;

    std::shared_ptr<StemArea> StemSimArea;

    std::shared_ptr<CbedPosition> CbedPos;

    SimulationMode Mode;
};

#endif // SIMULATIONMANAGER_H

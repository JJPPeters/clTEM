#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H

#include <string>
#include <mutex>
#include <map>
#include <valarray>

#include "structure/crystalstructure.h"
#include "utilities/commonstructs.h"
#include "utilities/enums.h"
#include "utilities/stringutils.h"

class SimulationManager
{
public:
    SimulationManager();

    SimulationManager(const SimulationManager& sm)
            : structure_mutex(), image_update_mtx(), Resolution(sm.Resolution), TdsRunsStem(sm.TdsRunsStem), TdsRunsCbed(sm.TdsRunsCbed),
              numParallelPixels(sm.numParallelPixels), isFD(sm.isFD), isF3D(sm.isF3D), full3dInts(sm.full3dInts),
              completeJobs(sm.completeJobs), imageReturn(sm.imageReturn), progressTotalReporter(sm.progressTotalReporter), progressSliceReporter(sm.progressSliceReporter),
              Images(sm.Images), Mode(sm.Mode), StemDets(sm.StemDets), TdsEnabled(sm.TdsEnabled),
              padding_x(sm.padding_x), padding_y(sm.padding_y), padding_z(sm.padding_z), slice_dz(sm.slice_dz),
              blocks_x(sm.blocks_x), blocks_y(sm.blocks_y), simulateCtemImage(sm.simulateCtemImage),
              maxReciprocalFactor(sm.maxReciprocalFactor), ccd_name(sm.ccd_name), ccd_binning(sm.ccd_binning), ccd_dose(sm.ccd_dose)
    {
        if (sm.Structure) // structure doesnt always exist
            Structure = std::make_shared<CrystalStructure>(*(sm.Structure));
        MicroParams = std::make_shared<MicroscopeParameters>(*(sm.MicroParams));
        SimArea = std::make_shared<SimulationArea>(*(sm.SimArea));
        StemSimArea = std::make_shared<StemArea>(*(sm.StemSimArea));
        CbedPos = std::make_shared<CbedPosition>(*(sm.CbedPos));
    }

    SimulationManager& operator=(const SimulationManager& sm)
    {
        Resolution = sm.Resolution;
        TdsRunsStem = sm.TdsRunsStem;
        TdsRunsCbed = sm.TdsRunsCbed;
        numParallelPixels = sm.numParallelPixels;
        isFD = sm.isFD;
        isF3D = sm.isF3D;
        full3dInts = sm.full3dInts;
        completeJobs = sm.completeJobs;
        imageReturn = sm.imageReturn;
        progressTotalReporter = sm.progressTotalReporter;
        progressSliceReporter = sm.progressSliceReporter;
        Images = sm.Images;
        Mode = sm.Mode;
        StemDets = sm.StemDets;
        TdsEnabled = sm.TdsEnabled;
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

        if (sm.Structure) // structure doesnt always exist
            Structure = std::make_shared<CrystalStructure>(*(sm.Structure));
        MicroParams = std::make_shared<MicroscopeParameters>(*(sm.MicroParams));
        SimArea = std::make_shared<SimulationArea>(*(sm.SimArea));
        StemSimArea = std::make_shared<StemArea>(*(sm.StemSimArea));
        CbedPos = std::make_shared<CbedPosition>(*(sm.CbedPos));

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

    bool haveStructure()
    {
        if (Structure)
            return true;
        return false;
    }

    SimulationArea getCtemArea() {return *SimArea;}

    std::valarray<float> getPaddingX() {return padding_x;}
    std::valarray<float> getPaddingY() {return padding_y;}
    std::valarray<float> getPaddingZ() {return padding_z;}

    std::valarray<float> getStructLimitsX() {return Structure->getLimitsX();}
    std::valarray<float> getStructLimitsY() {return Structure->getLimitsY();}
    std::valarray<float> getStructLimitsZ() {return Structure->getLimitsZ();}

    std::valarray<float> getSimLimitsX();
    std::valarray<float> getSimLimitsY();

    std::valarray<float> getPaddedStructLimitsX() {return getStructLimitsX() + padding_x;}
    std::valarray<float> getPaddedStructLimitsY() {return getStructLimitsY() + padding_y;}
    std::valarray<float> getPaddedStructLimitsZ() {return getStructLimitsZ() + padding_z;}

    // the odd padding here is because I want the SimArea to be in the structure coordinates (i.e. without padding)
    // so this shifts to the 'padded frame' then adds the padding
//    std::valarray<float> getPaddedSimLimitsX() { return getSimLimitsX() - padding_x[0] + padding_x; }
//    std::valarray<float> getPaddedSimLimitsY() { return getSimLimitsY() - padding_y[0] + padding_y; }

    // should have corrected the shift issue so this more intuitive versino works!!
    std::valarray<float> getPaddedSimLimitsX() {return getSimLimitsX() + padding_x;}
    std::valarray<float> getPaddedSimLimitsY() {return getSimLimitsY() + padding_y;}

    float calculatePaddedRealScale(float range, int resolution, bool round_padding = false);

    int getBlocksX();// {return blocks_x;}
    int getBlocksY();// {return blocks_y;}

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
    /// Get the max band limited simulation inverse in inverse Angstroms
    float getInverseMax();
    /// Get the max band limited simulation inverse in mrad
    float getInverseMaxAngle();
    float getInverseLimitFactor() {return maxReciprocalFactor;}

    float getKiloVoltage() {return MicroParams->Voltage;}
    float getVoltage() {return MicroParams->Voltage * 1000;}
    float getWavelength() {return MicroParams->Wavelength();}
    float getSigma() {return MicroParams->Sigma();}
    bool isFiniteDifference(){return isFD;}
    bool isFull3d(){return isF3D;}

    void setFull3d(bool use) {isF3D = use;}
    void setFiniteDifference(bool use) {isFD = use;}

    unsigned int getFull3dInts(){return full3dInts;}
    unsigned int getStoredTdsRuns();
    unsigned int getTdsRuns();
    unsigned int getStoredTdsRunsCbed() { return TdsRunsCbed; }
    unsigned int getStoredTdsRunsStem() { return TdsRunsStem; }
    unsigned int getTdsRunsCbed() { return (!TdsEnabled) ? 1 : TdsRunsCbed; }
    unsigned int getTdsRunsStem() { return (!TdsEnabled) ? 1 : TdsRunsStem; }
    unsigned int getStoredParallelPixels() {return numParallelPixels;}
    unsigned int getParallelPixels() {return (Mode != SimulationMode::STEM) ? 1 : numParallelPixels;}
    bool getTdsEnabled() { return TdsEnabled; }

    void setTdsEnabled(bool use){TdsEnabled = use;}
    void setTdsRunsStem(unsigned int runs){TdsRunsStem = runs;}
    void setTdsRunsCbed(unsigned int runs){TdsRunsCbed = runs;}
    void setParallelPixels(unsigned int npp) {numParallelPixels = npp;}
    void setFull3dInts(unsigned int n3d){full3dInts= n3d;}

    void setImageReturnFunc(std::function<void(std::map<std::string, Image<float>>, SimulationManager)> f) {imageReturn = f;}
    void setProgressTotalReporterFunc(std::function<void(float)> f) {progressTotalReporter = f;}
    void setProgressSliceReporterFunc(std::function<void(float)> f) {progressSliceReporter = f;}

    void updateImages(std::map<std::string, Image<float>> ims, int jobCount);
    void reportTotalProgress(float prog);
    void reportSliceProgress(float prog);

    void round_padding();
    float calculateRoundedPadding(float range, int resolution);

    bool getSimulateCtemImage() {return simulateCtemImage;}
    void setSimulateCtemImage(bool val) {simulateCtemImage = val;}

    std::string getCcdName() {return ccd_name;}
    void setCcdName(std::string nm) {ccd_name = nm;}

    int getCcdBinning() {return ccd_binning;}
    void setCcdBinning(int bin) {ccd_binning = bin;}

    float getCcdDose() {return ccd_dose;}
    void setCcdDose(float dose) {ccd_dose = dose;}

    float getSliceThickness();
    unsigned int getNumberofSlices();

    void setSliceThickness(float thk) { slice_dz = thk; }

private:
    static std::valarray<float> const default_xy_padding;
    static std::valarray<float> const default_z_padding;

    std::mutex structure_mutex;

//    bool GotStructure;

    std::shared_ptr<CrystalStructure> Structure;

//    bool HaveDevices;

    std::valarray<float> padding_x, padding_y, padding_z;

    unsigned int Resolution;
    unsigned int TdsRunsStem;
    unsigned int TdsRunsCbed;
    bool TdsEnabled;
    bool simulateCtemImage;

    float maxReciprocalFactor;

    // STEM only
    unsigned int numParallelPixels;

    float slice_dz;

    void calculate_blocks();
    int blocks_x, blocks_y;

    bool calculateFiniteDiffSliceThickness(float &dz_out);

    //TODO: variables for full3D etc??
    bool isFD;
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

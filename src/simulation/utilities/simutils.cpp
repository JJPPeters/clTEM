//
// Created by jonat on 27/01/2019.
//

#include "simutils.h"
namespace Utils {
    bool checkSimulationPrerequisites(std::shared_ptr<SimulationManager> Manager, std::vector<clDevice> &Devices) {
        std::vector <std::string> errorList;

        if (Devices.empty())
            errorList.emplace_back("No OpenCL devices selected.");

        if (!Manager->getStructure())
            errorList.emplace_back("No structure loaded.");
        else if (Manager->getStructureParameter().Max_Atomic_Number < Manager->getStructure()->getMaxAtomicNumber())
            errorList.emplace_back("Potentials do not include all structure atomic numbers. Max: " +
                                   std::to_string(Manager->getStructureParameter().Max_Atomic_Number));

        if (!Manager->haveResolution())
            errorList.emplace_back("No valid simulation resolution set.");

        auto mp = Manager->getMicroscopeParams();
        if (mp->Voltage <= 0)
            errorList.emplace_back("Voltage must be a non-zero positive number.");
        if (mp->Aperture <= 0)
            errorList.emplace_back("Aperture must be a non-zero positive number.");

        if (Manager->getStructureParameterData().empty())
            errorList.emplace_back("Potentials have not been loaded correctly.");

        // TODO: check beta (alpha) and delta?

        // TODO: check TDS entries

        // TODO: CBED position in simulation area

        // TODO: STEM area in simulation area

        // Check STEM detectors exist
        if (Manager->getMode() == SimulationMode::STEM)
            if (Manager->getDetectors().empty())
                errorList.emplace_back("STEM simulation requires at least 1 detector.");

        // TODO: dose sim for TEM checks
        if (Manager->getMode() == SimulationMode::CTEM)
            if (Manager->getCcdDose() <= 0.0)
                errorList.emplace_back("CCD dose cannot be 0.");

        // TODO: warnings option (stem detector radius checks...

        if (!errorList.empty()) {
            std::string final;
            for (const auto &err : errorList) {
                final += err + "\n";
            }
            throw std::runtime_error(final);
        }

        return true;
    }
}
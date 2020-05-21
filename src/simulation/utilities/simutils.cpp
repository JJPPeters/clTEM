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

        if (Manager->isFull3d() && Manager->getFull3dInts() < 1)
            errorList.emplace_back("Full 3d integrals must be non-zero positive number.");

        if (Manager->getMode() == SimulationMode::STEM && Manager->getParallelPixels() < 1)
            errorList.emplace_back("Parallel STEM pixels must be non-zero positive number.");

        // check TDS entries
        if (Manager->getInelasticScattering()->getInelasticEnabled() && Manager->getInelasticScattering()->getInelasticIterations() < 1)
            errorList.emplace_back("Inelastic scattering iterations must be larger than 0.");

        // plasmon settings
        if (Manager->getInelasticScattering()->getPlasmons()->getPlasmonEnabled()) {
            auto plasmon = Manager->getInelasticScattering()->getPlasmons();
            if (plasmon->getMeanFreePath() <= 0.0)
                errorList.emplace_back("Plasmon mean free path must be non-zero positive number.");

            if (plasmon->getCharacteristicAngle() <= 0.0)
                errorList.emplace_back("Plasmon characteristic angle must be non-zero positive number.");

        }

        // Check STEM detectors exist
        if (Manager->getMode() == SimulationMode::STEM)
            if (Manager->getDetectors().empty())
                errorList.emplace_back("STEM simulation requires at least 1 detector.");

        // dose sim for TEM checks
        if (Manager->getMode() == SimulationMode::CTEM)
            if (Manager->getCcdDose() <= 0.0)
                errorList.emplace_back("CCD dose cannot be 0.");

        // TODO: warnings option i.e. things that wont cause a crash, but will cause a silly output...)

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
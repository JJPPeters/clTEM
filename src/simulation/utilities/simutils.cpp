//
// Created by jonat on 27/01/2019.
//

#include "simutils.h"

namespace Utils {
    bool checkSimulationPrerequisites(std::shared_ptr<SimulationManager> Manager, std::vector<clDevice> &Devices) {
        std::vector <std::string> errorList;

        if (Devices.empty())
            errorList.emplace_back("No OpenCL devices selected.");

        if (!Manager->simulationCell()->crystalStructure())
            errorList.emplace_back("No structure loaded.");
        else if (Manager->structureParameters().max_atomic_number <
                Manager->simulationCell()->crystalStructure()->maxAtomicNumber())
            errorList.emplace_back("Potentials do not include all structure atomic numbers. Max: " +
                                   std::to_string(Manager->structureParameters().max_atomic_number));
        else if (Manager->structureParameters().max_atomic_number == 0)
            errorList.emplace_back("Potentials do not include any atomic numbers.");
        else if (Manager->simulationCell()->crystalStructure()->maxAtomicNumber() == 0 || Manager->simulationCell()->crystalStructure()->atoms().empty())
            errorList.emplace_back("No atoms in structure.");

        if (!Manager->resolutionValid())
            errorList.emplace_back("No valid simulation resolution set.");

        if(Manager->simulationCell()->sliceThickness() <= 0.0)
            errorList.emplace_back("Slice thickness must be a non-zero positive number.");

        if(Manager->intermediateSlicesEnabled() && Manager->intermediateSliceStep() <= 0)
            errorList.emplace_back("Intermediate slice output step must be a non-zero positive number.");

        auto mp = Manager->microscopeParams();
        if (mp->Voltage <= 0)
            errorList.emplace_back("Voltage must be a non-zero positive number.");
        if (Manager->mode() != SimulationMode::CTEM && mp->CondenserAperture <= 0)
            errorList.emplace_back("Aperture must be a non-zero positive number.");
        if (Manager->mode() == SimulationMode::CTEM && mp->ObjectiveAperture <= 0)
            errorList.emplace_back("Aperture must be a non-zero positive number.");

        if (Manager->structureParameters().parameters.empty())
            errorList.emplace_back("Potentials have not been loaded correctly.");

        if (Manager->full3dEnabled() && Manager->full3dIntegrals() < 1)
            errorList.emplace_back("Full 3d integrals must be non-zero positive number.");

        if (Manager->mode() == SimulationMode::STEM && Manager->parallelPixels() < 1)
            errorList.emplace_back("Parallel STEM pixels must be non-zero positive number.");

        if (Manager->mode() == SimulationMode::STEM && Manager->parallelPixels() < 1)
            errorList.emplace_back("Parallel STEM pixels must be non-zero positive number.");

        // check TDS entries
        if (Manager->parallelPotentialsCount() < 1)
            errorList.emplace_back("Mixed potential phonon approximation must be a non-zero positive number.");

        // plasmon settings
        if (Manager->incoherenceEffects()->plasmons()->enabled()) {
            auto plasmon = Manager->incoherenceEffects()->plasmons();
            if (plasmon->meanFreePath() <= 0.0)
                errorList.emplace_back("Plasmon mean free path must be non-zero positive number.");

            if (plasmon->characteristicAngle() <= 0.0)
                errorList.emplace_back("Plasmon characteristic angle must be non-zero positive number.");

        }

        // Check STEM detectors exist
        if (Manager->mode() == SimulationMode::STEM)
            if (Manager->stemDetectors().empty())
                errorList.emplace_back("STEM simulation requires at least 1 detector.");

        // dose sim for TEM checks
        if (Manager->mode() == SimulationMode::CTEM)
            if (Manager->ccdDose() <= 0.0)
                errorList.emplace_back("CCD dose cannot be 0.");

        // TODO: warnings option i.e. things that wont cause a crash, but will cause a silly output...

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
//
// Created by jon on 04/04/18.
//

#ifndef CLTEM_JSONUTILS_H
#define CLTEM_JSONUTILS_H


#include <simulationmanager.h>
#include <ccdparams.h>
#include "json.hpp"

#include "incoherence/inelastic/phonon.h"

namespace JSONUtils {

    using json = nlohmann::json;

    json BasicManagerToJson(SimulationManager& man, bool force_all = false);

    json FullManagerToJson(SimulationManager& man);

    json stemDetectorToJson(StemDetector d);

    json thermalVibrationsToJson(SimulationManager& man);

    SimulationManager JsonToManager(json& j);

    // This function is used for the command line to check if the area has been defined
    SimulationManager JsonToManager(json& j, bool& area_set);

    PhononScattering JsonToThermalVibrations(json& j);

    template <typename T>
    T readJsonEntry(json j, std::string current)
    {
        return j.at(current).get<T>();
    }

    template <typename T, typename... Args>
    T readJsonEntry(json j, std::string current, Args... args)
    {
        return readJsonEntry<T>(j.at(current), args...);
    }

};


#endif //CLTEM_JSONUTILS_H

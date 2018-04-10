//
// Created by jon on 04/04/18.
//

#ifndef CLTEM_JSONUTILS_H
#define CLTEM_JSONUTILS_H


#include <simulationmanager.h>
#include <ccdparams.h>
#include "json.hpp"

namespace JSONUtils {

    using json = nlohmann::json;

    json BasicManagerToJson(SimulationManager& man, bool force_all = false);

    json FullManagerToJson(SimulationManager& man);

    json stemDetectorToJson(StemDetector d);

};


#endif //CLTEM_JSONUTILS_H

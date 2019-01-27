//
// Created by jonat on 27/01/2019.
//

#ifndef CLTEM_SIMUTILS_H
#define CLTEM_SIMUTILS_H

#include <memory>
#include <string>
#include <simulationmanager.h>

namespace Utils {
    bool checkSimulationPrerequisites(std::shared_ptr<SimulationManager> Manager, std::vector<clDevice> &Devices);
}

#endif //CLTEM_SIMUTILS_H

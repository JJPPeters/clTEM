//
// Created by jon on 15/11/17.
//

#ifndef CLTEM_JOBSPLITTER_H
#define CLTEM_JOBSPLITTER_H

#include "simulationmanager.h"

class SimulationJob
{
public:
    SimulationJob() { }

    SimulationJob(std::shared_ptr<SimulationManager> _sMan) : simManager(_sMan) { }

    SimulationJob(std::shared_ptr<SimulationManager> _sMan, std::vector<int> _px) : simManager(_sMan), pixels(_px) { }

    // need a pointer to this so we can access variables we need without making a copy
    // TODO: consider multiple threads trying to access this...
    // Should be OK if only reading (can I enforce this?)
    std::shared_ptr<SimulationManager> simManager;

    // only used for STEM simulations, here are the (randomised) pixel indices to simulate.
    std::vector<int> pixels;
};

#endif //CLTEM_JOBSPLITTER_H

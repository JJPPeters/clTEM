//
// Created by jon on 15/11/17.
//

#ifndef CLTEM_JOBSPLITTER_H
#define CLTEM_JOBSPLITTER_H

#include "simulationmanager.h"
#include <future>

class SimulationJob
{
public:
    SimulationJob() { }

    SimulationJob(std::shared_ptr<SimulationManager> _sMan, unsigned int _id) : simManager(_sMan), id(_id) { }

    SimulationJob(std::shared_ptr<SimulationManager> _sMan, std::vector<int> _px, unsigned int _id) : simManager(_sMan), id(_id), pixels(_px) { }

    // need a pointer to this so we can access variables we need without making a copy
    // TODO: consider multiple threads trying to access this...
    // Should be OK if only reading (can I enforce this?)
    std::shared_ptr<SimulationManager> simManager;

    // id is mostly used for accessing the correct parts of manager, in the case of precalculated values
    // this is currently only for plasmons.
    unsigned int id;

    std::promise<void> promise;

    std::future<void> get_future() {return promise.get_future();}

    unsigned int getPixel() {
        if (!pixels.empty())
            return pixels[0];
        return 0;
    }

    // only used for STEM simulations, here are the (randomised) pixel indices to simulate.
    std::vector<int> pixels;
};

#endif //CLTEM_JOBSPLITTER_H

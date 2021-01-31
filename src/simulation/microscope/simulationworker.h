#include <utility>

//
// Created by jon on 23/11/17.
//

#ifndef CLTEM_SIMULATIONWORKER_H
#define CLTEM_SIMULATIONWORKER_H

#include "clwrapper.h"
#include "utilities/logging.h"

#include "simulationgeneral.h"
#include "simulationctem.h"
#include "simulationcbed.h"
#include "simulationstem.h"

template <class GPU_Type>
class SimulationWorker : public SimulationStem<GPU_Type>
{
    using SimulationGeneral<GPU_Type>::pool;
    using SimulationGeneral<GPU_Type>::job;
    using SimulationGeneral<GPU_Type>::ctx;

public:
    SimulationWorker(clDevice &_dev, ThreadPool &_s, unsigned int _id) : SimulationStem<GPU_Type>(_dev, _s, _id) {}

    ~SimulationWorker() = default;

    void Run(const std::shared_ptr<SimulationJob> &_job) override;
};


#endif //CLTEM_SIMULATIONWORKER_H

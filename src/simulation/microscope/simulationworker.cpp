//
// Created by jon on 23/11/17.
//

#include "simulationworker.h"

template <class GPU_Type>
void SimulationWorker<GPU_Type>::Run(const std::shared_ptr<SimulationJob> &_job) {
    // here is where the simulation gubbins happens
    // Or, in the words of Adam Dyson, this is where the magic happens :)
    int p_num = ctx.GetContextDevice().GetPlatformNumber();
    int d_num = ctx.GetContextDevice().GetDeviceNumber();

    el::Helpers::setThreadName("p" + std::to_string(p_num) + ":d" + std::to_string(d_num));

    CLOG(DEBUG, "sim") << "Running simulation worker";

    job = _job;

    if (!_job->simManager) {
        CLOG(DEBUG, "sim") << "Cannot access simulation parameters";
        pool.setStopped();
    }

    if (pool.isStopped()) {
        CLOG(DEBUG, "sim") << "Threadpool stopping";
        _job->promise.set_value();
        return;
    }

    // do teh actual simulation here
    auto mode = _job->simManager->mode();

    try {
        if (mode == SimulationMode::CTEM) {
            CLOG(DEBUG, "sim") << "Doing CTEM simulation";
            SimulationCtem<GPU_Type>::simulate();
        } else if (mode == SimulationMode::CBED) {
            CLOG(DEBUG, "sim") << "Doing CBED simulation";
            SimulationCbed<GPU_Type>::simulate();
        } else if (mode == SimulationMode::STEM) {
            CLOG(DEBUG, "sim") << "Doing STEM simulation";
            SimulationStem<GPU_Type>::simulate();
        }
    } catch (const std::runtime_error &e) {
        CLOG(ERROR, "sim") << "Error performing simulation: " << e.what();
        pool.setStopped();
        _job->simManager->failedSimulation();
    }


    CLOG(DEBUG, "sim") << "Completed simulation";
    // finally, end this thread ?
    _job->promise.set_value();
}

template class SimulationWorker<float>;
template class SimulationWorker<double>;
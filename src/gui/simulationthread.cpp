#include "simulationthread.h"

SimulationThread::SimulationThread(std::vector<std::shared_ptr<SimulationManager>> managers, std::vector<clDevice> devs)
{
    //TODO: does this part need mutexing or making thread safe?
    simRunner = std::make_shared<SimulationRunner>(managers, devs);
}

void SimulationThread::run()
{
    simRunner->runSimulations();
}

void SimulationThread::cancelSimulation()
{
    if (simRunner)
        simRunner->cancelSimulation();

    // wait for cancellation
    wait();
}

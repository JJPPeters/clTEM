#include "simulationthread.h"

SimulationThread::SimulationThread(std::vector<std::shared_ptr<SimulationManager>> managers, std::vector<clDevice> devs)
{
    //TODO: does this part need mutexing or making thread safe?
    simRunner = std::make_shared<SimulationRunner>(managers, devs);
}

void SimulationThread::run()
{
    el::Helpers::setThreadName("gui sim thread");
    CLOG(DEBUG, "gui") << "Starting simulation runners";

    simRunner->runSimulations();
}

void SimulationThread::cancelSimulation()
{
    CLOG(DEBUG, "gui") << "Cancelling simulation runners";
    if (simRunner)
        simRunner->cancelSimulation();

    // wait for cancellation
    CLOG(DEBUG, "gui") << "Cancelling this thread";
    quit();
    CLOG(DEBUG, "gui") << "Waiting for this thread";
    wait();
    CLOG(DEBUG, "gui") << "All cancelled";
}

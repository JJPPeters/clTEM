#ifndef SIMULATIONTHREAD_H
#define SIMULATIONTHREAD_H


#include <QThread>
#include <QMutex>
#include <simulationrunner.h>
#include "simulationmanager.h"

class SimulationThread : public QThread
{
    Q_OBJECT

public:
    SimulationThread(std::vector<std::shared_ptr<SimulationManager>> managers, std::vector<clDevice> devs);

    void cancelSimulation();

private:
    // I want run to be as minimal as possible, it should just call other functions...
    void run() Q_DECL_OVERRIDE;

    std::shared_ptr<SimulationRunner> simRunner;
};

#endif // SIMULATIONTHREAD_H
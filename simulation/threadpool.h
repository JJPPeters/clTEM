//
// Created by jon on 11/11/17.
//

#ifndef CLTEM_THREADPOOL_H
#define CLTEM_THREADPOOL_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <deque>
#include <clwrapper/clwrapper.h>
//#include "threadworker.h"
#include "simulationjob.h"

class ThreadWorker;

// A lot (if not all) is taken from http://progsch.net/wordpress/?p=81
// and the update https://github.com/progschj/ThreadPool
class ThreadPool
{
public:
    ThreadPool(std::vector<clDevice> devList, int num_jobs);

    ~ThreadPool();

    // add new work item to the pool
    void enqueue(std::shared_ptr<SimulationJob> job);

    void stopThreads();

private:
    friend class ThreadWorker;
    friend class SimulationWorker;

    std::vector<std::thread> workers;

    std::deque<std::shared_ptr<SimulationJob>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};


#endif //CLTEM_THREADPOOL_H


// Created by jon on 11/11/17.
//

#include "threadpool.h"
#include "simulationworker.h"

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(std::vector<clDevice> devList, int num_jobs) : stop(false)
{
    size_t n_threads = std::min(devList.size(), (size_t) num_jobs);
    for(size_t i = 0; i < n_threads; ++i) // TODO: depends what is lower, n jobs or n devices
    {
        workers.emplace_back(std::thread(std::move(SimulationWorker(*this, i, OpenCL::MakeContext(devList[i])))));
    }
}

// the destructor joins all threads
ThreadPool::~ThreadPool()
{
    stopThreads();
}

void ThreadPool::stopThreads()
{
    // stop all threads
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();

    // join them
    for(size_t i = 0; i < workers.size(); ++i)
        workers[i].join();

    workers.clear();
}


// Created by jon on 11/11/17.
//

#include "threadpool.h"
#include "microscope/simulationworker.h"

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(std::vector<clDevice> devList, int num_jobs, bool double_precision) : stop(false)
{
    size_t n_threads = std::min(devList.size(), (size_t) num_jobs);
    for(unsigned int i = 0; i < n_threads; ++i) { // TODO: depends what is lower, n jobs or n devices
        if (double_precision)
            workers.emplace_back(std::thread(std::move(SimulationWorker<double>(*this, i, OpenCL::MakeContext(devList[i])))));
        else
            workers.emplace_back(std::thread(std::move(SimulationWorker<float>(*this, i, OpenCL::MakeContext(devList[i])))));
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
    for (auto &worker : workers)
        worker.join();
    workers.clear();

    for (auto &task : tasks)
        task->promise.set_value();
    tasks.clear();
}

auto ThreadPool::enqueue(std::shared_ptr<SimulationJob> job) -> std::future<void> {
    std::future<void> res = job->get_future();
    { // acquire lock
        std::unique_lock<std::mutex> lock(queue_mutex);

        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        // add the task
        tasks.push_back(job);
    } // release lock

    // wake up one thread
    condition.notify_one();
    return res;
}

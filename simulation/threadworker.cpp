//
// Created by jon on 23/11/17.
//

#include "threadpool.h"
#include "threadworker.h"

void ThreadWorker::operator()()
{
    std::shared_ptr<SimulationJob> task;

    while(true)
    {
        {   // acquire lock
            std::unique_lock<std::mutex> lock(pool.queue_mutex);

            pool.condition.wait(lock, [this]{ return this->pool.stop || !this->pool.tasks.empty(); });

            if(pool.stop && pool.tasks.empty()) // exit if the pool is stopped
                return;

            // get the task from the queue
//            task = std::move(pool.tasks.front());
            task = std::move(pool.tasks.front());
            pool.tasks.pop_front();

        }   // release lock

        // execute the task
        Run(task);
    }
}
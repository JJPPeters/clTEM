//
// Created by jon on 23/11/17.
//

#include "threadpool.h"
#include "threadworker.h"

void ThreadWorker::operator()()
{
    std::shared_ptr<SimulationJob> task;

    while(!pool.tasks.empty())
    {
        {   // acquire lock
            std::unique_lock<std::mutex> lock(pool.queue_mutex);

            pool.condition.wait(lock, [this]{ return this->pool.stop || !this->pool.tasks.empty(); });

            if(pool.stop) { // exit if the pool is stopped
                for (auto &pt : pool.tasks)
                    pt->promise.set_value(); // make our threads have some output so other threads don't wait for them
                return;
            }

            // get the task from the queue
            task = std::move(pool.tasks.front());
            pool.tasks.pop_front();

        }   // release lock

        // execute the task
        Run(task);
    }
}
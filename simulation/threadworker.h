//
// Created by jon on 23/11/17.
//

#ifndef CLTEM_OPENCLWORKER_H
#define CLTEM_OPENCLWORKER_H

#include <thread>
#include <mutex>
#include <vector>
#include <clwrapper/clwrapper.h>
#include <iostream>
#include "threadpool.h"
#include <chrono>


// our worker thread objects
class ThreadWorker
{
public:
    ThreadWorker(ThreadPool &s, int _id) : pool(s), id(_id) { }

    void operator()();

    virtual void Run(std::shared_ptr<SimulationJob> job) {};

    int id;

protected:
    ThreadPool &pool;
};

#endif //CLTEM_OPENCLWORKER_H

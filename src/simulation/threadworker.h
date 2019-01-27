//
// Created by jon on 23/11/17.
//

#ifndef CLTEM_OPENCLWORKER_H
#define CLTEM_OPENCLWORKER_H

#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <chrono>
#include "threadpool.h"

// our worker thread objects
class ThreadWorker
{
public:
    ThreadWorker(ThreadPool &s, unsigned int _id) : pool(s), id(_id) { }

    void operator()();

    virtual void Run(const std::shared_ptr<SimulationJob> &job) {};

    unsigned int id;

protected:
    ThreadPool &pool;
};

#endif //CLTEM_OPENCLWORKER_H

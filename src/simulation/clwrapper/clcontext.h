//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_T_CLCONTEXT_H
#define CLWRAPPER_T_CLCONTEXT_H

#include <list>
#include <iostream>
#include <utility>
#include <vector>
#include <memory>
#include <algorithm>

#include "cldevice.h"
#include "CL/cl.hpp"
#include "notify.h"

template <class T, template <class> class AutoPolicy> class clMemory_impl;

enum MemoryFlags
{
    ReadWrite = CL_MEM_READ_WRITE,
    ReadOnly = CL_MEM_READ_ONLY,
    WriteOnly = CL_MEM_WRITE_ONLY
};

class clContext
{

private:
    cl::Context Context;
    cl::CommandQueue Queue;
    cl::CommandQueue IOQueue;
    clDevice ContextDevice;

    std::vector<std::weak_ptr<Notify>> MemList;

public:
    clContext() {}

    clContext(const cl::Context& _context, const cl::CommandQueue& _queue, clDevice _device)
            : Context(_context), Queue(_queue), IOQueue(_queue), ContextDevice(std::move(_device)){}

    clContext(const cl::Context& _context, const cl::CommandQueue& _queue, const cl::CommandQueue& _ioqueue, clDevice _device)
            : Context(_context), Queue(_queue), IOQueue(_ioqueue), ContextDevice(std::move(_device)){}

    ~clContext() = default;

    void WaitForQueueFinish() {
        int status = Queue.finish();
        clError::Throw(status);
    }
    void WaitForIOQueueFinish() {
        int status = IOQueue.finish();
        clError::Throw(status);
    }
    void QueueFlush() {
        int status = Queue.flush();
        clError::Throw(status);
    }
    void IOQueueFlush() {
        int status = IOQueue.flush();
        clError::Throw(status);
    }

    clDevice& GetContextDevice(){ return ContextDevice; }
    cl::Context& GetContext(){ return Context;}
    cl::CommandQueue& GetQueue(){ return Queue; }
    cl::CommandQueue& GetIOQueue(){ return IOQueue; }

    cl_context& GetContextHandle(){ return Context();}
    cl_command_queue& GetQueueHandle(){ return Queue();}
    cl_command_queue& GetIOQueueHandle(){ return IOQueue();}

    size_t GetOccupiedMemorySize()
    {
        size_t total = 0;
        for (auto &it : MemList)
            if (auto p = it.lock())
                total += p->GetSizeInBytes();
        return total;
    }

    template <class T, template <class> class AutoPolicy>
    void AddMemRecord(const std::shared_ptr<clMemory_impl<T, AutoPolicy>> &rec)
    {
        std::weak_ptr<Notify> base_ptr = std::static_pointer_cast<Notify>(rec);

        // std::find doesn't play nice with weak_ptrs
        // https://stackoverflow.com/questions/55226398/find-weak-ptr-in-vector
        auto it = std::find_if(MemList.begin(), MemList.end(),
                               [&base_ptr](const std::weak_ptr<Notify>& ptr1) {
                                   return ptr1.lock() == base_ptr.lock();
                               });

        if (it == MemList.end())
            MemList.emplace_back(base_ptr);
    }

    template <class T, template <class> class AutoPolicy>
    void RemoveMemRecord(const std::shared_ptr<clMemory_impl<T, AutoPolicy>> &rec)
    {
        std::weak_ptr<Notify> base_ptr = std::static_pointer_cast<Notify>(rec);

        auto it = std::find_if(MemList.begin(), MemList.end(),
                               [&base_ptr](const std::weak_ptr<Notify>& ptr1) {
                                   return ptr1.lock() == base_ptr.lock();
                               });

        if (it != MemList.end())
            MemList.erase(it);
    }
};

#endif //CLWRAPPER_T_CLCONTEXT_H

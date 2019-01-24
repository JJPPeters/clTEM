//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_T_CLCONTEXT_H
#define CLWRAPPER_T_CLCONTEXT_H

#include <list>
#include <iostream>
#include <vector>
#include "boost/shared_ptr.hpp"

#include "cldevice.h"
#include "CL/cl.h"

template <class T, template <class> class AutoPolicy> class clMemory;

enum MemoryFlags
{
    ReadWrite = CL_MEM_READ_WRITE,
    ReadOnly = CL_MEM_READ_ONLY,
    WriteOnly = CL_MEM_WRITE_ONLY
};

class MemoryRecord
{
public:
    explicit MemoryRecord(size_t _size): size(_size){};
    size_t size;
};

class clContext
{

private:
    cl_int Status;
    cl_context Context;
    cl_command_queue Queue;
    cl_command_queue IOQueue;
    clDevice ContextDevice;
    std::vector<std::shared_ptr<MemoryRecord>> MemList;

public:
    clContext(const clDevice &_ContextDevice, cl_context _Context, cl_command_queue _Queue, cl_int _Status)
            : Status(_Status), Context(_Context), Queue(_Queue), IOQueue(_Queue), ContextDevice(_ContextDevice){}
    clContext(const clDevice &_ContextDevice, cl_context _Context, cl_command_queue _Queue, cl_command_queue _IOQueue, cl_int _Status)
            : Status(_Status), Context(_Context), Queue(_Queue), IOQueue(_IOQueue), ContextDevice(_ContextDevice){}

    ~clContext() {
        if (Queue) {
            Status = clReleaseCommandQueue(Queue);
            clError::Throw(Status);
        }
        if (Queue != IOQueue && IOQueue) {
            Status = clReleaseCommandQueue(IOQueue);
            clError::Throw(Status);
        }
        if (Context) {
            Status = clReleaseContext(Context);
            clError::Throw(Status);
        }
    }

    void WaitForQueueFinish() {
        int status = clFinish(Queue);
        clError::Throw(status);
    }
    void WaitForIOQueueFinish() {
        int status = clFinish(IOQueue);
        clError::Throw(status);
    }
    void QueueFlush() {
        int status = clFlush(Queue);
        clError::Throw(status);
    }
    void IOQueueFlush() {
        int status = clFlush(IOQueue);
        clError::Throw(status);
    }

    clDevice GetContextDevice(){return ContextDevice;}
    cl_context& GetContext(){return Context;}
    cl_int GetStatus(){return Status;}
    cl_command_queue& GetQueue(){ return Queue; }
    cl_command_queue& GetIOQueue(){return IOQueue;}

    size_t GetOccupiedMemorySize()
    {
        size_t total = 0;
        for (auto &it : MemList)
            total += it->size;
        return total;
    }

    void RemoveMemRecord(const std::shared_ptr<MemoryRecord> &rec)
    {
        auto it = std::find(MemList.begin(), MemList.end(), rec);
        if (it != MemList.end())
            MemList.erase(it);
    }

    template<class T,template <class> class AutoPolicy> std::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t size)
    {
        std::shared_ptr<MemoryRecord> rec = std::make_shared<MemoryRecord>(size*sizeof(T));
        auto b = clCreateBuffer(Context, MemoryFlags::ReadWrite, size*sizeof(T), nullptr, &Status);
        clError::Throw(Status, "");
        std::shared_ptr<clMemory<T,AutoPolicy>> Mem = std::make_shared<clMemory<T,AutoPolicy>>(this, size, b, rec);
        MemList.emplace_back(rec);
        return Mem;
    }

    template<class T,template <class> class AutoPolicy > std::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t size, enum MemoryFlags flags)
    {
        std::shared_ptr<MemoryRecord> rec = std::make_shared<MemoryRecord>(size*sizeof(T));
        auto b = clCreateBuffer(Context, flags, size*sizeof(T), nullptr, &Status);
        std::shared_ptr<clMemory<T,AutoPolicy>> Mem = std::make_shared<clMemory<T,AutoPolicy>>(this, size, b, rec);
        clError::Throw(Status, "");
        MemList.emplace_back(rec);
        return Mem;
    }
};

#endif //CLWRAPPER_T_CLCONTEXT_H

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
    MemoryRecord(size_t _size): size(_size){};
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
    std::vector<MemoryRecord*> MemList;

public:
    clContext(clDevice _ContextDevice, cl_context _Context, cl_command_queue _Queue, cl_int _Status)
            : ContextDevice(_ContextDevice), Context(_Context), Queue(_Queue), IOQueue(_Queue), Status(_Status){};
    clContext(clDevice _ContextDevice, cl_context _Context, cl_command_queue _Queue, cl_command_queue _IOQueue, cl_int _Status)
            : ContextDevice(_ContextDevice), Context(_Context), Queue(_Queue), IOQueue(_IOQueue), Status(_Status){};

//    clContext(const clContext& cpy)
//            : ContextDevice(cpy.ContextDevice), Context(cpy.Context), Queue(cpy.Queue), IOQueue(cpy.Queue), Status(cpy.Status), MemList(cpy.MemList.begin(), cpy.MemList.end())
//    { }
//
//    clContext& operator=(const clContext& RHS)
//    {
//        ContextDevice = RHS.ContextDevice;
//        Context = RHS.Context;
//        Queue = RHS.Queue;
//        IOQueue = RHS.IOQueue;
//        Status = RHS.Status;
//        MemList = std::vector<MemoryRecord*>(RHS.MemList.begin(), RHS.MemList.end());
//    }

    void WaitForQueueFinish(){clFinish(Queue);};
    void WaitForIOQueueFinish(){clFinish(IOQueue);};
    void QueueFlush(){clFlush(Queue);};
    void IOQueueFlush(){clFlush(IOQueue);};

    clDevice GetContextDevice(){return ContextDevice;};
    cl_context& GetContext(){return Context;};
    cl_int GetStatus(){return Status;};
    cl_command_queue& GetQueue(){ return Queue; };
    cl_command_queue& GetIOQueue(){return IOQueue;};

    size_t GetOccupiedMemorySize()
    {
        std::vector<MemoryRecord*>::iterator it; size_t total = 0;
        for(it = MemList.begin(); it != MemList.end(); it++)
            total += (*it)->size;
        return total;
    }

    void RemoveMemRecord(MemoryRecord* rec)
    {
        std::vector<MemoryRecord*>::iterator it = std::find(MemList.begin(), MemList.end(), rec);
        if (it != MemList.end())
            MemList.erase(it);
    }

    template<class T,template <class> class AutoPolicy> std::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t size)
    {
        MemoryRecord* rec = new MemoryRecord(size*sizeof(T));
        std::shared_ptr<clMemory<T,AutoPolicy>> Mem( new clMemory<T,AutoPolicy>(this,size,clCreateBuffer(Context, MemoryFlags::ReadWrite, size*sizeof(T), 0, &Status),rec));
        MemList.push_back(rec);
        return Mem;
    };

    template<class T,template <class> class AutoPolicy > std::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(size_t size, enum MemoryFlags flags)
    {
        MemoryRecord* rec = new MemoryRecord(size*sizeof(T));
        std::shared_ptr<clMemory<T,AutoPolicy>> Mem( new clMemory<T,AutoPolicy>(this,size,clCreateBuffer(Context, flags, size*sizeof(T), 0, &Status),rec));
        MemList.push_back(rec);
        return Mem;
    };
};

#endif //CLWRAPPER_T_CLCONTEXT_H

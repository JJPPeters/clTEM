//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_CLMEMORY_H
#define CLWRAPPER_MAIN_CLMEMORY_H

#include "CL/cl.h"

#include "clcontext.h"
#include "clevent.h"
#include "auto.h"
#include "manual.h"
#include "notify.h"

#include <iostream>

class clContext;
class MemoryRecord;

template <class T, template <class> class AutoPolicy>
class clMemory : public AutoPolicy<T>
{
private:
    cl_mem Buffer;
    size_t Size;
    clContext* Context;
    std::shared_ptr<MemoryRecord> Rec;

public:
    // template <class T, template <class> class AutoPolicy> using Ptr = std::shared_ptr<clMemory<T,AutoPolicy>>;
//    typedef std::shared_ptr<clMemory<T, AutoPolicy>> Ptr;
    friend class clContext;
    typedef T MemType;
    cl_mem& GetBuffer(){ return Buffer; };
    size_t	GetSize(){ return Size*sizeof(MemType); };

    // Will wait for this event to complete before performing read.
    clEvent StartReadEvent;
    // This event signifies a read has been performed.
    clEvent FinishedReadEvent;
    // This event will be completed after we write to this memory.
    clEvent FinishedWriteEvent;
    // Write will not begin until this event is completed.
    clEvent StartWriteEvent;

    virtual clEvent GetFinishedWriteEvent(){return FinishedWriteEvent;};
    virtual clEvent GetFinishedReadEvent(){return FinishedReadEvent;};
    virtual clEvent GetStartWriteEvent(){return StartWriteEvent;};
    virtual clEvent GetStartReadEvent(){return StartReadEvent;};

    clEvent Read(std::vector<T> &data)
    {
        cl_int status;
        status = clEnqueueReadBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],0,NULL,&FinishedReadEvent.event);
        clError::Throw(status);
        FinishedReadEvent.Set();
        return FinishedReadEvent;
    };

    // Wait on single event before reading
    clEvent Read(std::vector<T> &data, clEvent Start)
    {
        cl_int status;
        StartReadEvent = Start;
        status = clEnqueueReadBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],1,&Start.event,&FinishedReadEvent.event);
        clError::Throw(status);
        FinishedReadEvent.Set();
        return FinishedReadEvent;
    };

    clEvent Write(std::vector<T> &data)
    {
        cl_int status;
        status = clEnqueueWriteBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],0,NULL,&FinishedWriteEvent.event);
        clError::Throw(status);
        FinishedWriteEvent.Set();
        return FinishedWriteEvent;
    };

    // Wait on single event before writing.
    clEvent Write(std::vector<T> &data, clEvent Start)
    {
        cl_int status;
        StartWriteEvent = Start;
        status = clEnqueueWriteBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],1,&Start.event,&FinishedWriteEvent.event);
        clError::Throw(status);
        FinishedWriteEvent.Set();
        return FinishedWriteEvent;
    };

    clMemory<T,AutoPolicy>(clContext* context, size_t size, cl_mem buffer, std::shared_ptr<MemoryRecord> rec) : Context(context), Buffer(buffer), Size(size),
                                                                                                AutoPolicy<T>(size), FinishedReadEvent(), FinishedWriteEvent(), StartReadEvent(), StartWriteEvent(), Rec(rec){};

    void SetFinishedEvent(clEvent KernelFinished)
    {
        StartReadEvent = KernelFinished;
    };

    ~clMemory<T,AutoPolicy>()
    {
        Context->RemoveMemRecord(Rec);
        Release();
    };

private:

    void Release()
    {
        if(Buffer) // Does this work?
        {
            cl_int status;
            status = clReleaseMemObject(Buffer);
            clError::Throw(status);
        }
    };
};

#endif //CLWRAPPER_MAIN_CLMEMORY_H

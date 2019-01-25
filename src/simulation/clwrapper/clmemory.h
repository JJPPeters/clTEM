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
    std::shared_ptr<clContext> Context;
    std::shared_ptr<MemoryRecord> Rec;

public:
    // template <class T, template <class> class AutoPolicy> using Ptr = std::shared_ptr<clMemory<T,AutoPolicy>>;
//    typedef std::shared_ptr<clMemory<T, AutoPolicy>> Ptr;
    friend class clContext;
    typedef T MemType;
    cl_mem& GetBuffer(){ return Buffer; };
    size_t	GetSize(){ return Size*sizeof(MemType); };

    // Will wait for this event to complete before performing read.
    std::shared_ptr<clEvent> StartReadEvent;
    // This event signifies a read has been performed.
    std::shared_ptr<clEvent> FinishedReadEvent;
    // This event will be completed after we write to this memory.
    std::shared_ptr<clEvent> FinishedWriteEvent;
    // Write will not begin until this event is completed.
    std::shared_ptr<clEvent> StartWriteEvent;

    virtual std::shared_ptr<clEvent> GetFinishedWriteEvent(){return FinishedWriteEvent;};
    virtual std::shared_ptr<clEvent> GetFinishedReadEvent(){return FinishedReadEvent;};
    virtual std::shared_ptr<clEvent> GetStartWriteEvent(){return StartWriteEvent;};
    virtual std::shared_ptr<clEvent> GetStartReadEvent(){return StartReadEvent;};

    std::shared_ptr<clEvent> Read(std::vector<T> &data)
    {
        cl_int status;
        status = clEnqueueReadBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],0,NULL,&FinishedReadEvent->event);
        clError::Throw(status);
        return FinishedReadEvent;
    };

    // Wait on single event before reading
    std::shared_ptr<clEvent> Read(std::vector<T> &data, std::shared_ptr<clEvent> Start)
    {
        cl_int status;
        StartReadEvent = Start;
        status = clEnqueueReadBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],1,&Start->event,&FinishedReadEvent->event);
        clError::Throw(status);
        return FinishedReadEvent;
    };

    std::shared_ptr<clEvent> Write(std::vector<T> &data)
    {
        cl_int status;
        status = clEnqueueWriteBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],0,NULL,&FinishedWriteEvent->event);
        clError::Throw(status);
        return FinishedWriteEvent;
    };

    // Wait on single event before writing.
    std::shared_ptr<clEvent> Write(std::vector<T> &data, std::shared_ptr<clEvent> Start)
    {
        cl_int status;
        StartWriteEvent = Start;
        status = clEnqueueWriteBuffer(Context->GetIOQueue(),Buffer,CL_FALSE,0,data.size()*sizeof(T),&data[0],1,&Start->event,&FinishedWriteEvent->event);
        clError::Throw(status);
        return FinishedWriteEvent;
    };

    clMemory<T,AutoPolicy>(std::shared_ptr<clContext> context, size_t size, cl_mem buffer, std::shared_ptr<MemoryRecord> rec) : Context(context), Buffer(buffer), Size(size),
                                                                                                AutoPolicy<T>(size), FinishedReadEvent(std::make_shared<clEvent>()), FinishedWriteEvent(std::make_shared<clEvent>()), StartReadEvent(std::make_shared<clEvent>()), StartWriteEvent(std::make_shared<clEvent>()), Rec(rec){};

    void SetFinishedEvent(std::shared_ptr<clEvent> KernelFinished)
    {
        StartReadEvent = KernelFinished;
    };

    ~clMemory<T,AutoPolicy>()
    {
        Context->RemoveMemRecord(Rec);

        StartReadEvent.reset();
        StartWriteEvent.reset();
        FinishedReadEvent.reset();
        FinishedWriteEvent.reset();

        if(Buffer) {
            cl_int status;
            status = clReleaseMemObject(Buffer);
            clError::Throw(status);
        }
    };
};

#endif //CLWRAPPER_MAIN_CLMEMORY_H

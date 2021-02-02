//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_CLMEMORY_H
#define CLWRAPPER_MAIN_CLMEMORY_H

#include "CL/cl.hpp"

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
    friend class clContext;
    typedef T MemType;

private:
    cl::Buffer Buffer;

    size_t Size;
    std::shared_ptr<clContext> Context;
    std::shared_ptr<MemoryRecord> Rec;

public:
    // Will wait for this event to complete before performing read.
    clEvent StartReadEvent;
    // This event signifies a read has been performed.
    clEvent FinishedReadEvent;
    // This event will be completed after we write to this memory.
    clEvent FinishedWriteEvent;
    // Write will not begin until this event is completed.
    clEvent StartWriteEvent;

    clMemory<T,AutoPolicy>() : AutoPolicy<T>(0), Size(0) {}

    clMemory<T,AutoPolicy>(const std::shared_ptr<clContext>& context, size_t size, enum MemoryFlags flags = MemoryFlags::ReadWrite)
            : AutoPolicy<T>(size), Context(context), Size(size), FinishedReadEvent(), FinishedWriteEvent(), StartReadEvent(), StartWriteEvent() {
        Buffer = cl::Buffer(context->GetContext(), flags, Size*sizeof(T));
    };

    clMemory<T, AutoPolicy>& operator=(const clMemory<T, AutoPolicy>& rhs) {
        AutoPolicy<T>::operator=(rhs);
        Buffer = rhs.Buffer;
        Size = rhs.Size;
        Context = rhs.Context;
        Rec = rhs.Rec;

        return *this;
    }


    cl::Buffer& GetBuffer(){ return Buffer; };
    cl_mem& GetBufferHandle(){ return Buffer(); };
    size_t	GetSize(){ return Size; }//*sizeof(MemType); }; // Why did I have this using bytes?

    virtual clEvent GetFinishedWriteEvent(){return FinishedWriteEvent;};
    virtual clEvent GetFinishedReadEvent(){return FinishedReadEvent;};
    virtual clEvent GetStartWriteEvent(){return StartWriteEvent;};
    virtual clEvent GetStartReadEvent(){return StartReadEvent;};

    clEvent Read(std::vector<T> &data) {
        cl_int status;
        status = Context->GetIOQueue().enqueueReadBuffer(Buffer, CL_FALSE, 0, Size*sizeof(T), &data[0], nullptr, &FinishedReadEvent.event);
        clError::Throw(status);
        return FinishedReadEvent;
    };

    // Wait on single event before reading
    clEvent Read(std::vector<T> &data, clEvent Start) {
        cl_int status;
        StartReadEvent = Start;
        std::vector<cl::Event> start_vector;
        if (StartReadEvent.event())
            start_vector.push_back(StartReadEvent.event);
        status = Context->GetIOQueue().enqueueReadBuffer(Buffer, CL_FALSE, 0, Size*sizeof(T), &data[0], &start_vector, &FinishedReadEvent.event);
        clError::Throw(status);
        return FinishedReadEvent;
    };

    clEvent Write(std::vector<T> &data) {
        cl_int status;
        status = Context->GetIOQueue().enqueueWriteBuffer(Buffer, CL_FALSE, 0, Size*sizeof(T), &data[0], nullptr, &FinishedWriteEvent.event);
        clError::Throw(status);
        return FinishedWriteEvent;
    };

    // Wait on single event before writing.
    clEvent Write(std::vector<T> &data, clEvent Start) {
        cl_int status;
        StartWriteEvent = Start;
        std::vector<cl::Event> start_vector;
        if (StartReadEvent.event())
            start_vector.push_back(StartReadEvent.event);
        status = Context->GetIOQueue().enqueueWriteBuffer(Buffer, CL_FALSE, 0, Size*sizeof(T), &data[0], &start_vector, &FinishedWriteEvent.event);
        clError::Throw(status);
        return FinishedWriteEvent;
    };


    void SetFinishedEvent(clEvent KernelFinished) {
        StartReadEvent = KernelFinished;
    };

    ~clMemory<T, AutoPolicy>() {
        if (Context && Rec)
            Context->RemoveMemRecord(Rec);
    };
};

#endif //CLWRAPPER_MAIN_CLMEMORY_H

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
class clKernel;
template <class T, template <class> class AutoPolicy> class clMemory;

template <class T, template <class> class AutoPolicy>
class clMemory_impl : public AutoPolicy<T>
{
    friend class clContext;
    friend class clMemory<T, AutoPolicy>;

private:
    cl::Buffer Buffer;

    size_t Size;
//    std::shared_ptr<MemoryRecord> Rec;

    // Will wait for this event to complete before performing read.
    clEvent StartReadEvent;
    // This event signifies a read has been performed.
    clEvent FinishedReadEvent;
    // This event will be completed after we write to this memory.
    clEvent FinishedWriteEvent;
    // Write will not begin until this event is completed.
    clEvent StartWriteEvent;

    enum MemoryFlags BufferFlags;

protected:
    std::shared_ptr<clContext> Context;

public:

    clMemory_impl<T,AutoPolicy>(const std::shared_ptr<clContext>& context, size_t size, enum MemoryFlags flags = MemoryFlags::ReadWrite)
            : AutoPolicy<T>(size), Context(context), Size(size), BufferFlags(flags),
              FinishedReadEvent(), FinishedWriteEvent(), StartReadEvent(), StartWriteEvent() {
        Buffer = cl::Buffer(context->GetContext(), flags, Size*sizeof(T));

        Fill(0);
    }

    cl::Buffer& GetBuffer(){ return Buffer; }
    cl_mem& GetBufferHandle(){ return Buffer(); }
    size_t	GetSize() { return Size; }

    clEvent GetFinishedWriteEvent() override{return FinishedWriteEvent;}
    clEvent GetFinishedReadEvent() override{return FinishedReadEvent;}
    clEvent GetStartWriteEvent(){return StartWriteEvent;}
    clEvent GetStartReadEvent(){return StartReadEvent;}

    void SetFinishedEvent(clEvent KernelFinished) {
        StartReadEvent = KernelFinished;
    };

    clEvent Read(std::vector<T> &data) {
        cl_int status;
        status = Context->GetIOQueue().enqueueReadBuffer(Buffer, CL_FALSE, 0, Size*sizeof(T), &data[0], nullptr, &FinishedReadEvent.event);
        clError::Throw(status);
        return FinishedReadEvent;
    }

    // Wait on single event before reading
    clEvent Read(std::vector<T> &data, clEvent Start) {
        cl_int status;
        StartReadEvent = Start;
        std::vector<cl::Event> start_vector;
        if (StartReadEvent.event())
            start_vector.push_back(StartReadEvent.event);
        status = Context->GetIOQueue().enqueueReadBuffer(Buffer, CL_FALSE, 0, Size * sizeof(T), &data[0],
                                                         &start_vector, &FinishedReadEvent.event);
        clError::Throw(status);
        return FinishedReadEvent;
    }

    clEvent Write(std::vector<T> &data) {
        cl_int status;
        status = Context->GetIOQueue().enqueueWriteBuffer(Buffer, CL_FALSE, 0, Size*sizeof(T), &data[0], nullptr, &FinishedWriteEvent.event);

        clError::Throw(status);

        return FinishedWriteEvent;
    }

    cl_int _Write(std::vector<T> &data) {
        cl_int status;
        status = Context->GetIOQueue().enqueueWriteBuffer(Buffer, CL_FALSE, 0, Size*sizeof(T), &data[0], nullptr, &FinishedWriteEvent.event);
        clError::Throw(status);

        return status;
    }

    // Wait on single event before writing.
    clEvent Write(std::vector<T> &data, clEvent Start) {
        cl_int status;
        StartWriteEvent = Start;
        std::vector<cl::Event> start_vector;
        if (StartWriteEvent.event())
            start_vector.push_back(StartWriteEvent.event);
        status = Context->GetIOQueue().enqueueWriteBuffer(Buffer, CL_FALSE, 0, Size*sizeof(T), &data[0], &start_vector, &FinishedWriteEvent.event);
        clError::Throw(status);

        return FinishedWriteEvent;
    }

    clEvent Fill(T value) {
        cl_int status;
        status = Context->GetIOQueue().enqueueFillBuffer(Buffer, &value, 0, Size, nullptr,
                                                         &FinishedWriteEvent.event);

        clError::Throw(status);

        return FinishedWriteEvent;
    }

    size_t GetSizeInBytes() {
        return Size * sizeof(T);
    }

};














template <class T, template <class> class AutoPolicy>
class clMemory
{
    friend class clKernel;

protected:
    std::shared_ptr<clMemory_impl<T,AutoPolicy>> mem_ptr;

public:
    clMemory<T,AutoPolicy>() {};

    clMemory<T,AutoPolicy>(const std::shared_ptr<clContext>& context, size_t size, enum MemoryFlags flags = MemoryFlags::ReadWrite) {
        mem_ptr = std::make_shared<clMemory_impl<T,AutoPolicy>>(context, size, flags);
        mem_ptr->Context->AddMemRecord(mem_ptr);
    };

    void SetNeededNow(bool value) {
        mem_ptr->SetNeededNow(value);
    }

    void EnsureOnDevice() {
        mem_ptr->EnsureOnDevice();
    }

    cl::Buffer& GetBuffer(){
        return mem_ptr->GetBuffer();
    }
    cl_mem& GetBufferHandle(){
        return mem_ptr->GetBufferHandle();
    }
    size_t	GetSize(){
        if (mem_ptr)
            return mem_ptr->GetSize();
        else
            return 0;
    }

    clEvent GetFinishedWriteEvent() {
        return mem_ptr->GetFinishedWriteEvent();
    }
    clEvent GetFinishedReadEvent() {
        return mem_ptr->GetFinishedReadEvent();
    }
    clEvent GetStartWriteEvent(){
        return mem_ptr->GetStartWriteEvent();
    }
    clEvent GetStartReadEvent(){
        return mem_ptr->GetStartReadEvent();
    }

    clEvent Read(std::vector<T> &data) {
        return mem_ptr->Read(data);
    }

    // Wait on single event before reading
    clEvent Read(std::vector<T> &data, clEvent& Start) {
        return mem_ptr->Read(data, Start);
    }

    clEvent Write(std::vector<T> &data) {
        auto ev = mem_ptr->Write(data);
        return ev;

    }

    // Wait on single event before writing.
    clEvent Write(std::vector<T> &data, clEvent& Start) {
        return mem_ptr->Write(data, Start);
    }


    void SetFinishedEvent(clEvent& KernelFinished) {
        mem_ptr->SetFinishedEvent(KernelFinished);
    }

    bool getAuto() {
        return mem_ptr->getAuto();
    }

    void UpdateEventOnly(clEvent& KernelFinished) {
        mem_ptr->UpdateEventOnly(KernelFinished);
    }

    void Update(clEvent& KernelFinished) {
        mem_ptr->Update(KernelFinished);
    }

    std::vector<T> GetLocal() {
        return mem_ptr->GetLocal();
    }

};

#endif //CLWRAPPER_MAIN_CLMEMORY_H

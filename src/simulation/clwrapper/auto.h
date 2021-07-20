//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_AUTO_H
#define CLWRAPPER_MAIN_AUTO_H

#include "clevent.h"
#include "notify.h"
#include <vector>

// This class can facilitate automatically retrieving changes to OpenCL memory buffers.
// from kernels with argument types specified.
template <class T> class Auto : public Notify
{
    size_t Size;
    bool isAuto;
    bool isUpToDate;
    std::vector<T> Local;

public:
    Auto<T>(): Size(0), isAuto(true), isUpToDate(true) {}
    explicit Auto<T>(size_t size): Size(size), isAuto(true), isUpToDate(true), Local() {}
    Auto<T>& operator=(const Auto<T> &rhs) {
        Size = rhs.Size;
        isAuto = rhs.isAuto;
        isUpToDate = rhs.isUpToDate;
        Local = rhs.Local;

        return *this;
    }

    bool getAuto() {return isAuto;}
    virtual size_t GetSizeInBytes()=0;

    virtual clEvent Read(std::vector<T>&data)=0;
    virtual clEvent Read(std::vector<T>&data,clEvent KernelFinished)=0;

    virtual clEvent GetStartWriteEvent()=0;
    virtual clEvent GetStartReadEvent()=0;

    clEvent GetFinishedWriteEvent() override =0;

    clEvent GetFinishedReadEvent() override =0;

    virtual void SetFinishedEvent(clEvent KernelFinished) = 0;

    // This call will block if the Memory is currently waiting on
    // an event before updating itself.
    std::vector<T>& GetLocal()
    {
        clEvent es = GetStartReadEvent();
        clEvent e = GetFinishedReadEvent();

        es.Wait();

        if(!isUpToDate)
        {
            Update(es);
            isUpToDate = true;

            if((es = GetFinishedReadEvent()).isSet())
                es.Wait();
        }
        else
            e.Wait();

        return Local;
    }

    // Called by clKernel for Output types to generate automatic
    // memory updates (non blocking)
    void Update(clEvent KernelFinished)
    {
        if(Local.empty() || Local.size() != Size)
            Local.resize(Size);
        Read(Local,KernelFinished);
        isUpToDate = true;
        SetFinishedEvent(KernelFinished);
    }

    void UpdateEventOnly(clEvent KernelFinished)
    {
        isUpToDate = false;
        SetFinishedEvent(KernelFinished);
    }
};

#endif //CLWRAPPER_MAIN_AUTO_H

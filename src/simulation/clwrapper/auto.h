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
public:
Auto<T>(size_t size): Size(size), isAuto(true), isUpToDate(true){
    Local.resize(0);
};

size_t Size;
bool isAuto;
bool isUpToDate;

// Reading directly from Local is unadvised as it will be updated whenever the kernel
// actually completes which is not in general, directly after it has been called as kernel
// enqueue is non blocking. Use GetLocal() to force waiting for current version.
std::vector<T> Local;

virtual std::shared_ptr<clEvent> Read(std::vector<T>&data)=0;
virtual std::shared_ptr<clEvent> Read(std::vector<T>&data,clEvent KernelFinished)=0;

virtual std::shared_ptr<clEvent> GetStartWriteEvent()=0;
virtual std::shared_ptr<clEvent> GetStartReadEvent()=0;
virtual std::shared_ptr<clEvent> GetFinishedWriteEvent()=0;
virtual std::shared_ptr<clEvent> GetFinishedReadEvent()=0;

virtual void SetFinishedEvent(std::shared_ptr<clEvent> KernelFinished) = 0;

// This call will block if the Memory is currently waiting on
// an event before updating itself.
std::vector<T>& GetLocal()
{
    std::shared_ptr<clEvent> es = GetStartReadEvent();
    std::shared_ptr<clEvent> e = GetFinishedReadEvent();

    if(es->isSet())
        es->Wait();

    if(!isUpToDate)
    {
        Update(es);
        isUpToDate = true;

        if((es = GetFinishedReadEvent()).isSet())
            es->Wait();
    }
    else if(e->isSet())
        e->Wait();

    return Local;
};

// Called by clKernel for Output types to generate automatic
// memory updates (non blocking)
void Update(std::shared_ptr<clEvent> KernelFinished)
{
    if(Local.empty() || Local.size() != Size)
        Local.resize(Size);
    Read(Local,KernelFinished);
    isUpToDate = true;
}

void UpdateEventOnly(std::shared_ptr<clEvent> KernelFinished)
{
    isUpToDate = false;
    SetFinishedEvent(KernelFinished);
};
};

#endif //CLWRAPPER_MAIN_AUTO_H

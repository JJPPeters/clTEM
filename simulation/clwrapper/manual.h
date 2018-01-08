//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_MANUAL_H
#define CLWRAPPER_MAIN_MANUAL_H

#include "CL/cl.h"

#include "clevent.h"
#include "clerror.h"

// This class is inherited by OpenCL memory buffers that have to manage there own memory lifetimes.
template <class T> class Manual : public Notify
{
    public:
    Manual<T>(size_t size): Size(size), isAuto(false) {};
    const bool isAuto;
    size_t Size;
    clEvent KernelFinished;
    void Update(clEvent _KernelFinished){KernelFinished = _KernelFinished;};

    virtual clEvent Read(std::vector<T>&data)=0;
    virtual clEvent Read(std::vector<T>&data,clEvent KernelFinished)=0;
    virtual clEvent GetStartWriteEvent()=0;
    virtual clEvent GetStartReadEvent()=0;
    virtual clEvent GetFinishedWriteEvent()=0;
    virtual clEvent GetFinishedReadEvent()=0;

    virtual void SetFinishedEvent(clEvent KernelFinished) =0;

    // This will create a vector filled with the current contents of the memory
    // Will block until the read has been completed
    std::vector<T> CreateLocalCopy()
    {
        cl_int status;

        std::vector<T> Local(Size);
        if(KernelFinished.event != NULL && KernelFinished.isSet())
        {
            clEvent e =	Read(Local,KernelFinished);
            status = clWaitForEvents(1,&e.event);
        }
        else
        {
            clEvent e = Read(Local);
            status = clWaitForEvents(1,&e.event);
        }
        clError::Throw(status);
        return Local;
    };

    void UpdateEventOnly(clEvent KernelFinished)
    {
        SetFinishedEvent(KernelFinished);
    };
};

#endif //CLWRAPPER_MAIN_MANUAL_H

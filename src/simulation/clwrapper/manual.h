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
    std::shared_ptr<clEvent> KernelFinished;
    void Update(std::shared_ptr<clEvent> _KernelFinished){KernelFinished = _KernelFinished;};

    virtual std::shared_ptr<clEvent> Read(std::vector<T>&data)=0;
    virtual std::shared_ptr<clEvent> Read(std::vector<T>&data, std::shared_ptr<clEvent> KernelFinished)=0;
    virtual std::shared_ptr<clEvent> GetStartWriteEvent()=0;
    virtual std::shared_ptr<clEvent> GetStartReadEvent()=0;
    virtual std::shared_ptr<clEvent> GetFinishedWriteEvent()=0;
    virtual std::shared_ptr<clEvent> GetFinishedReadEvent()=0;

    virtual void SetFinishedEvent(std::shared_ptr<clEvent> KernelFinished) =0;

    // This will create a vector filled with the current contents of the memory
    // Will block until the read has been completed
    std::vector<T> CreateLocalCopy()
    {
        cl_int status;

        std::vector<T> Local(Size);
        if(KernelFinished && KernelFinished->event && KernelFinished->isSet())
        {
            std::shared_ptr<clEvent> e = Read(Local,KernelFinished);
            status = clWaitForEvents(1, &e->event);
        }
        else
        {
            std::shared_ptr<clEvent> e = Read(Local);
            status = clWaitForEvents(1, &e->event);
        }
        clError::Throw(status);
        return Local;
    };

    void UpdateEventOnly(std::shared_ptr<clEvent> KernelFinished)
    {
        SetFinishedEvent(KernelFinished);
    };
};

#endif //CLWRAPPER_MAIN_MANUAL_H

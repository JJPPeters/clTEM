//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_MANUAL_H
#define CLWRAPPER_MAIN_MANUAL_H

#include "CL/cl.hpp"

#include "clevent.h"
#include "clerror.h"

// This class is inherited by OpenCL memory buffers that have to manage there own memory lifetimes.
template <class T> class Manual : public Notify
{
    const bool isAuto;
    size_t Size;
    clEvent KernelFinished;

public:
    Manual<T>(): Size(0), isAuto(false) {}
    explicit Manual<T>(size_t size): Size(size), isAuto(false) {}
    Manual<T>& operator=(const Manual<T> &rhs) {
        Size = rhs.Size;
        KernelFinished = rhs.KernelFinished;

        return *this;
    }

    void Update(clEvent _KernelFinished) override {KernelFinished = _KernelFinished;};

    virtual clEvent Read(std::vector<T>&data)=0;
    virtual clEvent Read(std::vector<T>&data, clEvent KernelFinished)=0;
    virtual clEvent GetStartWriteEvent()=0;
    virtual clEvent GetStartReadEvent()=0;
    clEvent GetFinishedWriteEvent() override = 0;
    clEvent GetFinishedReadEvent() override = 0;

    virtual void SetFinishedEvent(clEvent KernelFinished) =0;

    // This will create a vector filled with the current contents of the memory
    // Will block until the read has been completed
    std::vector<T> CreateLocalCopy() {
        cl_int status = CL_SUCCESS;

        std::vector<T> Local(Size);

        clEvent e = Read(Local, KernelFinished);
        e.Wait();

        clError::Throw(status);
        return Local;
    };

    void UpdateEventOnly(clEvent KernelFinished) {
        SetFinishedEvent(KernelFinished);
    };

    bool getAuto() {return isAuto;}
};

#endif //CLWRAPPER_MAIN_MANUAL_H

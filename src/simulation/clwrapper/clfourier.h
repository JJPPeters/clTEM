//
// Created by jon on 09/10/16.
//

#ifndef CLWRAPPER_MAIN_CLFOURIER_H
#define CLWRAPPER_MAIN_CLFOURIER_H

#include "clFFT.h"

#include <complex>
#include "CL/cl.hpp"

#include "clstatic.h"
#include "clcontext.h"
#include "clevent.h"
#include "clmemory.h"

class AutoTeardownFFT;
template <class T, template <class> class AutoPolicy> class clMemory;
template <class T> class Manual;

// Can't use scoped enums for legacy code
namespace Direction
{
    enum TransformDirection
    {
        Forwards,
        Inverse
    };
}

template <class T>
class clFourier
{
    std::shared_ptr<clContext> Context;
    clfftStatus fftStatus;
    clfftSetupData fftSetupData;
    clfftPlanHandle fftplan;

    //intermediate buffer
    clMemory<char, Manual> clMedBuffer;
//    cl_int medstatus;
    unsigned int width, height;
    size_t buffersize;

public:

    clFourier() : fftplan(0), width(0), height(0) {}

    clFourier(std::shared_ptr<clContext> Context, unsigned int _width, unsigned int _height);

    clFourier(const clFourier &RHS): Context(RHS.Context), fftplan(0), width(RHS.width), height(RHS.height), buffersize(0) {
        if (Context && Context->GetContextHandle())
            Setup(width,height);
    };

    ~clFourier();

    void releaseResources() {
        if (fftplan) {
            fftStatus = clfftDestroyPlan(&fftplan);
            clFftError::Throw(fftStatus, "clFourier");
        }

        fftStatus = clfftTeardown();
        clFftError::Throw(fftStatus, "FourierTearDownTest");
    }

    template <template <class> class AutoPolicy, template <class> class AutoPolicy2>
    clEvent run(clMemory<std::complex<T>, AutoPolicy2> &input, clMemory<std::complex<T>, AutoPolicy> &output,
                Direction::TransformDirection Direction)
    {
        clfftDirection Dir = (Direction == Direction::Forwards) ? CLFFT_FORWARD : CLFFT_BACKWARD;

        std::vector<cl_event> eventwaitlist;
        clEvent e = input.GetFinishedWriteEvent();
        clEvent e2 = input.GetFinishedReadEvent();
        if (e.event())
            eventwaitlist.push_back(e.event());
        if (e2.event())
            eventwaitlist.push_back(e2.event());

        clEvent finished;

        if(buffersize)
            fftStatus = clfftEnqueueTransform( fftplan, Dir, 1, &Context->GetQueueHandle(), (cl_uint)eventwaitlist.size(),
                                               !eventwaitlist.empty() ? &eventwaitlist[0] : nullptr, &finished.event(),
                                               &input.GetBufferHandle(), &output.GetBufferHandle(), clMedBuffer.GetBufferHandle() );
        else
            fftStatus = clfftEnqueueTransform( fftplan, Dir, 1, &Context->GetQueueHandle(), (cl_uint)eventwaitlist.size(),
                                               !eventwaitlist.empty() ? &eventwaitlist[0] : nullptr, &finished.event(),
                                               &input.GetBufferHandle(), &output.GetBufferHandle(), NULL );

        if(output.getAuto())
            output.Update(finished);

        return finished;
    };

    unsigned int GetWidth() { return width; }
    unsigned int GetHeight() { return height; }

private:
    void Setup(unsigned int _width, unsigned int _height);
};

//// Singleton to auto call clfftteardown on program termination
class AutoTeardownFFT
{
public:
    AutoTeardownFFT() = default;;
    AutoTeardownFFT &operator=(AutoTeardownFFT const &rhs) = delete;

    AutoTeardownFFT(AutoTeardownFFT const& copy) = delete;

    ~AutoTeardownFFT()
    {
        clfftStatus status = clfftTeardown();
        clFftError::Throw(status, "FourierTearDown");
    }
    inline static AutoTeardownFFT& GetInstance() { static AutoTeardownFFT instance; return instance; }
};

#endif //CLWRAPPER_MAIN_CLFOURIER_H

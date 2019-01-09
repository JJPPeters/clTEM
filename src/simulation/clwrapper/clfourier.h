//
// Created by jon on 09/10/16.
//

#ifndef CLWRAPPER_MAIN_CLFOURIER_H
#define CLWRAPPER_MAIN_CLFOURIER_H

#include "CL/cl.h"
#include "clFFT.h"

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
};

class clFourier
{
    clContext* Context;
    clfftStatus fftStatus;
    clfftSetupData fftSetupData;
    clfftPlanHandle fftplan;

    //intermediate buffer
    std::shared_ptr<clMemory<char,Manual>> clMedBuffer;
//    cl_int medstatus;
    size_t buffersize;
    void Setup(unsigned int _width, unsigned int _height);
    unsigned int width,height;

public:

    clFourier(clContext &Context, unsigned int _width, unsigned int _height);

    clFourier(const clFourier &RHS): Context(RHS.Context), width(RHS.width), height(RHS.height), buffersize(0)
    {
        Setup(width,height);
    };


    clFourier& operator=(const clFourier &RHS)
    {
        if(this != &RHS){
            clfftDestroyPlan(&fftplan);
            Context = RHS.Context;
            width = RHS.width;
            height = RHS.height;
            Setup(width,height);
        }
        return *this;
    };

    ~clFourier();

    template <class T, template <class> class AutoPolicy, template <class> class AutoPolicy2>
    clEvent Do(std::shared_ptr<clMemory<T,AutoPolicy2>>& input, std::shared_ptr<clMemory<T,AutoPolicy>>& output, Direction::TransformDirection Direction)
    {
        clfftDirection Dir = (Direction == Direction::Forwards) ? CLFFT_FORWARD : CLFFT_BACKWARD;

        std::vector<cl_event> eventwaitlist;
        clEvent e = input->GetFinishedWriteEvent();
        clEvent e2 = input->GetFinishedReadEvent();
        if (e.isSet())
        {
            eventwaitlist.push_back(e.event);
        }
        if (e2.isSet())
        {
            eventwaitlist.push_back(e2.event);
        }

        clEvent finished;

        if(buffersize)
            fftStatus = clfftEnqueueTransform( fftplan, Dir, 1, &Context->GetQueue(), (cl_uint)eventwaitlist.size(),
                                               !eventwaitlist.empty() ? &eventwaitlist[0] : nullptr, &finished.event,
                                               &input->GetBuffer(), &output->GetBuffer(), clMedBuffer->GetBuffer() );
        else
            fftStatus = clfftEnqueueTransform( fftplan, Dir, 1, &Context->GetQueue(), (cl_uint)eventwaitlist.size(),
                                               !eventwaitlist.empty() ? &eventwaitlist[0] : nullptr, &finished.event,
                                               &input->GetBuffer(), &output->GetBuffer(), NULL );

        finished.Set();

        if(output->isAuto)
            output->Update(finished);

        return finished;
    };

    // This worked fine, but would confuse my editor?
    template <class T, template <class> class AutoPolicy, template <class> class AutoPolicy2>
    clEvent operator()(std::shared_ptr<clMemory<T,AutoPolicy2>>& input, std::shared_ptr<clMemory<T,AutoPolicy>>& output, Direction::TransformDirection Direction)
    {
        return Do(input, output, Direction);
    }
};

//// Singleton to auto call clfftteardown on program termination
class AutoTeardownFFT
{
private:
    AutoTeardownFFT() = default;;
    AutoTeardownFFT &operator=(AutoTeardownFFT const&rhs) = delete;
public:
    AutoTeardownFFT(AutoTeardownFFT const& copy) = delete;

    ~AutoTeardownFFT()
    {
        clfftStatus status = clfftTeardown();
        clFftError::Throw(status, "FourierTearDown");
    }
    inline static AutoTeardownFFT& GetInstance() { static AutoTeardownFFT instance; return instance; }
};

#endif //CLWRAPPER_MAIN_CLFOURIER_H

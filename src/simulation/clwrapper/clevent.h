//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_CLEVENT_H
#define CLWRAPPER_MAIN_CLEVENT_H

#include "CL/cl.h"

#include "clerror.h"

// Used to synchronise OpenCL functions that depend on other results.
class clEvent
{
    bool hasBeenSet;

public:
    clEvent(): hasBeenSet(false){};

    cl_event event;

    bool isSet(){ return hasBeenSet;};
    void Set(){ hasBeenSet = true; };
    void Wait()
    {
        cl_int status;
        status = clWaitForEvents(1, &event);
        clError::Throw(status);
    };
    // If profiling is enable can use these functions
    cl_ulong GetStartTime()
    {
        cl_ulong param;
        cl_int status;
        status = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &param, NULL);
        clError::Throw(status);
        return param;
    };
    cl_ulong GetEndTime()
    {
        cl_ulong param;
        cl_int status;
        status = clWaitForEvents(1,&event);
        clError::Throw(status);
        status = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &param, NULL);
        clError::Throw(status);
        return param;
    };
    cl_ulong GetElapsedTime(){return GetEndTime() - GetStartTime();};

};

#endif //CLWRAPPER_MAIN_CLEVENT_H

//
// Created by jon on 08/10/16.
//

#include "clkernel.h"

clEvent clKernel::operator()(clWorkGroup Global)
{
    std::vector<cl_event> eventwaitlist;

    // Check callbacks for any input types... need to wait on there write events..
    for( int arg = 0 ; arg < NumberOfArgs ; arg++)
    {
        // Data is being written to an input type, wait for it to finish
        if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::InputOutputNoUpdate )
        {
            clEvent e = Callbacks[arg]->GetFinishedWriteEvent();
            if (e.isSet())
            {
                eventwaitlist.push_back(e.event);
            }
        }
        // Current data is presently being retrieved (don't overwrite yet)
        if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::OutputNoUpdate || ArgType[arg] == ArgumentType::InputOutputNoUpdate)
        {
            clEvent e = Callbacks[arg]->GetFinishedReadEvent();
            if (e.isSet())
            {
                eventwaitlist.push_back(e.event);
            }
        }
    }

    clEvent KernelFinished;
    status = clEnqueueNDRangeKernel(Context->GetQueue(),Kernel,2,NULL,Global.worksize,NULL,(cl_uint)eventwaitlist.size(),eventwaitlist.size() ? &eventwaitlist[0] : NULL, &KernelFinished.event);
    clError::Throw(status, Name);

    RunCallbacks(KernelFinished);

    return KernelFinished;
}

clEvent clKernel::operator()(clWorkGroup Global, clEvent StartEvent)
{
    std::vector<cl_event> eventwaitlist;

    // Check callbacks for any input types... need to wait on there write events..
    for( int arg = 0 ; arg < NumberOfArgs ; arg++)
    {
        // Data is being written to an input type, wait for it to finish
        if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::InputOutputNoUpdate )
        {
            clEvent e = Callbacks[arg]->GetFinishedWriteEvent();
            if (e.isSet())
            {
                eventwaitlist.push_back(e.event);
            }
        }
        // Current data is presently being retrieved (don't overwrite yet)
        if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::OutputNoUpdate || ArgType[arg] == ArgumentType::InputOutputNoUpdate)
        {
            clEvent e = Callbacks[arg]->GetFinishedReadEvent();
            if (e.isSet())
            {
                eventwaitlist.push_back(e.event);
            }
        }
    }

    eventwaitlist.push_back(StartEvent.event);

    clEvent KernelFinished;
    status = clEnqueueNDRangeKernel(Context->GetQueue(),Kernel,2,NULL,Global.worksize,NULL,eventwaitlist.size(),&eventwaitlist[0],&KernelFinished.event);
    clError::Throw(status, Name);

    RunCallbacks(KernelFinished);

    return KernelFinished;
}

clEvent clKernel::operator()(clWorkGroup Global, clWorkGroup Local)
{
    std::vector<cl_event> eventwaitlist;

    // Check callbacks for any input types... need to wait on there write events..
    for( int arg = 0 ; arg < NumberOfArgs ; arg++)
    {
        // Data is being written to an input type, wait for it to finish
        if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::InputOutputNoUpdate )
        {
            clEvent e = Callbacks[arg]->GetFinishedWriteEvent();
            if (e.isSet())
            {
                eventwaitlist.push_back(e.event);
            }
        }
        // Current data is presently being retrieved (don't overwrite yet)
        if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::OutputNoUpdate || ArgType[arg] == ArgumentType::InputOutputNoUpdate)
        {
            clEvent e = Callbacks[arg]->GetFinishedReadEvent();
            if (e.isSet())
            {
                eventwaitlist.push_back(e.event);
            }
        }
    }

    clEvent KernelFinished;
    status = clEnqueueNDRangeKernel(Context->GetQueue(),Kernel,2,NULL,Global.worksize,Local.worksize,eventwaitlist.size(),eventwaitlist.size() ? &eventwaitlist[0] : NULL,&KernelFinished.event);
    clError::Throw(status, Name);

    RunCallbacks(KernelFinished);

    return KernelFinished;
}

clEvent clKernel::operator()(clWorkGroup Global, clWorkGroup Local, clEvent StartEvent)
{
    std::vector<cl_event> eventwaitlist;

    // Check callbacks for any input types... need to wait on there write events..
    for( int arg = 0 ; arg < NumberOfArgs ; arg++)
    {
        // Data is being written to an input type, wait for it to finish
        if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::InputOutputNoUpdate )
        {
            clEvent e = Callbacks[arg]->GetFinishedWriteEvent();
            if (e.isSet())
            {
                eventwaitlist.push_back(e.event);
            }
        }
        // Current data is presently being retrieved (don't overwrite yet)
        if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::OutputNoUpdate || ArgType[arg] == ArgumentType::InputOutputNoUpdate)
        {
            clEvent e = Callbacks[arg]->GetFinishedReadEvent();
            if (e.isSet())
            {
                eventwaitlist.push_back(e.event);
            }
        }
    }

    eventwaitlist.push_back(StartEvent.event);

    clEvent KernelFinished;
    status = clEnqueueNDRangeKernel(Context->GetQueue(),Kernel,2, nullptr,Global.worksize,Local.worksize, eventwaitlist.size(), &eventwaitlist[0],&KernelFinished.event);
    clError::Throw(status);

    RunCallbacks(KernelFinished);

    return KernelFinished;
}

void clKernel::RunCallbacks(clEvent KernelFinished)
{
    for( int arg = 0 ; arg < NumberOfArgs ; arg++) {
        if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput) {
            Callbacks[arg]->Update(KernelFinished);
        } else if(ArgType[arg] == ArgumentType::OutputNoUpdate || ArgType[arg] == ArgumentType::InputOutputNoUpdate ) {
            Callbacks[arg]->UpdateEventOnly(KernelFinished);
        }
    }
}

void clKernel::BuildKernelFromString(const char* codestring, std::string kernelname)
{
    // denorms now flushed to zero, and no checks for NaNs or infs, should be faster...
    const char options[] = "-cl-finite-math-only -cl-unsafe-math-optimizations -cl-no-signed-zeros -Werror"; //-cl-finite-math-only -cl-mad-enable -Werror";
    size_t log_size;
    Name = kernelname;

    Program = clCreateProgramWithSource(Context->GetContext(), 1, &codestring, nullptr, &status);
    clError::Throw(status, kernelname);

    status = clBuildProgram(Program,1,&Context->GetContextDevice().GetDeviceID(),options, nullptr, nullptr);
    if (status != 0) {
        status = clGetProgramBuildInfo(Program, Context->GetContextDevice().GetDeviceID(), CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);

        std::vector<char> buildlog_char(log_size);
        status = clGetProgramBuildInfo(Program, Context->GetContextDevice().GetDeviceID(), CL_PROGRAM_BUILD_LOG, log_size, &buildlog_char[0], nullptr);

        std::string buildlog_str(buildlog_char.begin(), buildlog_char.end());
        clError::Throw(status, kernelname + "\nBuild log:\n" + buildlog_str);
    }
    Kernel = clCreateKernel(Program,kernelname.c_str(),&status);
    clError::Throw(status, kernelname);
}
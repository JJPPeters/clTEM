//
// Created by jon on 08/10/16.
//

#include "clkernel.h"

std::shared_ptr<clEvent> clKernel::operator()(clWorkGroup Global) { return run(Global); }

std::shared_ptr<clEvent> clKernel::operator()(clWorkGroup Global, std::shared_ptr<clEvent> StartEvent) { return run(Global, StartEvent); }

std::shared_ptr<clEvent> clKernel::operator()(clWorkGroup Global, clWorkGroup Local) { return run(Global, Local); }

std::shared_ptr<clEvent> clKernel::operator()(clWorkGroup Global, clWorkGroup Local, std::shared_ptr<clEvent> StartEvent) { return run(Global, Local, StartEvent); }

void clKernel::RunCallbacks(std::shared_ptr<clEvent> KernelFinished)
{
    for( int arg = 0 ; arg < NumberOfArgs ; arg++) {
        if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput) {
            Callbacks[arg].lock()->Update(KernelFinished);
        } else if(ArgType[arg] == ArgumentType::OutputNoUpdate || ArgType[arg] == ArgumentType::InputOutputNoUpdate ) {
            Callbacks[arg].lock()->UpdateEventOnly(KernelFinished);
        }
    }
}

void clKernel::BuildKernelFromString(std::string codestring, std::string kernelname)
{
    // denorms now flushed to zero, and no checks for NaNs or infs, should be faster...
    std::string options = "-cl-finite-math-only -cl-unsafe-math-optimizations -cl-no-signed-zeros -Werror"; //-cl-finite-math-only -cl-mad-enable -Werror";
    size_t log_size;
    Name = kernelname;

    auto code_char = codestring.c_str();
    Program = clCreateProgramWithSource(Context->GetContext(), 1, &code_char, nullptr, &status);
    clError::Throw(status, kernelname);

    status = clBuildProgram(Program,1,&Context->GetContextDevice().GetDeviceID(), options.c_str(), nullptr, nullptr);
    if (status != CL_SUCCESS) {
        status = clGetProgramBuildInfo(Program, Context->GetContextDevice().GetDeviceID(), CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);

        std::vector<char> buildlog_char(log_size);
        status = clGetProgramBuildInfo(Program, Context->GetContextDevice().GetDeviceID(), CL_PROGRAM_BUILD_LOG, log_size, &buildlog_char[0], nullptr);

        std::string buildlog_str(buildlog_char.begin(), buildlog_char.end());
        clError::Throw(status, kernelname + "\nBuild log:\n" + buildlog_str);
    }
    Kernel = clCreateKernel(Program,kernelname.c_str(),&status);
    clError::Throw(status, kernelname);
}

std::shared_ptr<clEvent> clKernel::run(clWorkGroup Global) {
    std::vector<cl_event> eventwaitlist;

    // Check callbacks for any input types... need to wait on there write events..
    for( int arg = 0 ; arg < NumberOfArgs ; arg++)
    {
        // Data is being written to an input type, wait for it to finish
        if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::InputOutputNoUpdate )
        {
            std::shared_ptr<clEvent> e = Callbacks[arg].lock()->GetFinishedWriteEvent();
            if (e->isSet())
            {
                eventwaitlist.push_back(e->event);
            }
        }
        // Current data is presently being retrieved (don't overwrite yet)
        if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::OutputNoUpdate || ArgType[arg] == ArgumentType::InputOutputNoUpdate)
        {
            std::shared_ptr<clEvent> e = Callbacks[arg].lock()->GetFinishedReadEvent();
            if (e->isSet())
            {
                eventwaitlist.push_back(e->event);
            }
        }
    }

    std::shared_ptr<clEvent> KernelFinished = std::make_shared<clEvent>();
    status = clEnqueueNDRangeKernel(Context->GetQueue(),Kernel,2, nullptr,Global.worksize, nullptr,(cl_uint)eventwaitlist.size(), !eventwaitlist.empty() ? &eventwaitlist[0] : nullptr, &KernelFinished->event);
    clError::Throw(status, Name);

    RunCallbacks(KernelFinished);

    return KernelFinished;
}

std::shared_ptr<clEvent> clKernel::run(clWorkGroup Global, std::shared_ptr<clEvent> StartEvent) {
    std::vector<cl_event> eventwaitlist;

    // Check callbacks for any input types... need to wait on there write events..
    for( int arg = 0 ; arg < NumberOfArgs ; arg++)
    {
        // Data is being written to an input type, wait for it to finish
        if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::InputOutputNoUpdate )
        {
            std::shared_ptr<clEvent> e = Callbacks[arg].lock()->GetFinishedWriteEvent();
            if (e->isSet())
            {
                eventwaitlist.push_back(e->event);
            }
        }
        // Current data is presently being retrieved (don't overwrite yet)
        if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::OutputNoUpdate || ArgType[arg] == ArgumentType::InputOutputNoUpdate)
        {
            std::shared_ptr<clEvent> e = Callbacks[arg].lock()->GetFinishedReadEvent();
            if (e->isSet())
            {
                eventwaitlist.push_back(e->event);
            }
        }
    }

    eventwaitlist.push_back(StartEvent->event);

    std::shared_ptr<clEvent> KernelFinished = std::make_shared<clEvent>();
    status = clEnqueueNDRangeKernel(Context->GetQueue(), Kernel, 2, nullptr, Global.worksize, nullptr, static_cast<cl_uint>(eventwaitlist.size()), &eventwaitlist[0], &KernelFinished->event);
    clError::Throw(status, Name);

    RunCallbacks(KernelFinished);

    return KernelFinished;
}

std::shared_ptr<clEvent> clKernel::run(clWorkGroup Global, clWorkGroup Local) {
    std::vector<cl_event> eventwaitlist;

    // Check callbacks for any input types... need to wait on there write events..
    for( int arg = 0 ; arg < NumberOfArgs ; arg++)
    {
        // Data is being written to an input type, wait for it to finish
        if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::InputOutputNoUpdate )
        {
            std::shared_ptr<clEvent> e = Callbacks[arg].lock()->GetFinishedWriteEvent();
            if (e->isSet())
            {
                eventwaitlist.push_back(e->event);
            }
        }
        // Current data is presently being retrieved (don't overwrite yet)
        if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::OutputNoUpdate || ArgType[arg] == ArgumentType::InputOutputNoUpdate)
        {
            std::shared_ptr<clEvent> e = Callbacks[arg].lock()->GetFinishedReadEvent();
            if (e->isSet())
            {
                eventwaitlist.push_back(e->event);
            }
        }
    }

    std::shared_ptr<clEvent> KernelFinished = std::make_shared<clEvent>();
    status = clEnqueueNDRangeKernel(Context->GetQueue(), Kernel, 2, nullptr, Global.worksize, Local.worksize, static_cast<cl_uint>(eventwaitlist.size()), !eventwaitlist.empty() ? &eventwaitlist[0] : nullptr, &KernelFinished->event);
    clError::Throw(status, Name);

    RunCallbacks(KernelFinished);

    return KernelFinished;
}

std::shared_ptr<clEvent> clKernel::run(clWorkGroup Global, clWorkGroup Local, std::shared_ptr<clEvent> StartEvent) {
    std::vector<cl_event> eventwaitlist;

    // Check callbacks for any input types... need to wait on there write events..
    for( int arg = 0 ; arg < NumberOfArgs ; arg++)
    {
        // Data is being written to an input type, wait for it to finish
        if(ArgType[arg] == ArgumentType::Input || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::InputOutputNoUpdate )
        {
            std::shared_ptr<clEvent> e = Callbacks[arg].lock()->GetFinishedWriteEvent();
            if (e->isSet())
            {
                eventwaitlist.push_back(e->event);
            }
        }
        // Current data is presently being retrieved (don't overwrite yet)
        if(ArgType[arg] == ArgumentType::Output || ArgType[arg] == ArgumentType::InputOutput || ArgType[arg] == ArgumentType::OutputNoUpdate || ArgType[arg] == ArgumentType::InputOutputNoUpdate)
        {
            std::shared_ptr<clEvent> e = Callbacks[arg].lock()->GetFinishedReadEvent();
            if (e->isSet())
            {
                eventwaitlist.push_back(e->event);
            }
        }
    }

    eventwaitlist.push_back(StartEvent->event);

    std::shared_ptr<clEvent> KernelFinished = std::make_shared<clEvent>();
    status = clEnqueueNDRangeKernel(Context->GetQueue(), Kernel, 2, nullptr, Global.worksize, Local.worksize, static_cast<cl_uint>(eventwaitlist.size()), &eventwaitlist[0], &KernelFinished->event);
    clError::Throw(status);

    RunCallbacks(KernelFinished);

    return KernelFinished;
}

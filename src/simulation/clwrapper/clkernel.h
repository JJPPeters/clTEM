//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_CLKERNEL_H
#define CLWRAPPER_MAIN_CLKERNEL_H

#include <algorithm>
#include <utility>

#include "CL/cl.hpp"

#include "clmemory.h"
#include "clevent.h"
#include "clworkgroup.h"
#include "clerror.h"


// Optionally passed to argument setting.
// Output types will be updated automatically when data is modified

namespace ArgumentType
{
    // Unspecified first so containers will default to this value.
    enum ArgTypes
    {
        Unspecified,
        Input,
        Output,
        InputOutput,
        OutputNoUpdate,
        InputOutputNoUpdate
    };
};

class clKernel
{
private:
    std::shared_ptr<clContext> Context;
    cl::Program Program;
    cl::Kernel Kernel;
    std::string Name;

    unsigned int NumberOfArgs;
    std::vector<ArgumentType::ArgTypes> ArgType;
    std::vector<std::shared_ptr<Notify>> Callbacks;

public:
    clKernel() {}

    clKernel(std::shared_ptr<clContext> _context, const std::string &codestring, std::string _name, unsigned int _numArgs, std::string opts="")
            : Context(std::move(_context)), Name(std::move(_name)), NumberOfArgs(_numArgs)
    {
        ArgType.resize(NumberOfArgs);
        Callbacks.resize(NumberOfArgs);

        cl_int status;
        Program = cl::Program(Context->GetContext(), codestring, false, &status);
        status = Program.build(opts.c_str()); // could just put true above - need to remember to pass it the string

        std::string buildlog_str = Program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(Context->GetContextDevice().getDevice(), &status);
        clError::Throw(status, Name + "\nBuild log:\n" + buildlog_str);

        Kernel = cl::Kernel(Program, Name.c_str(), &status);
        clError::Throw(status, Name + "\nBuild log:\n" + buildlog_str);
    }

    template <class T, template <class> class AutoPolicy>
    void SetArg(cl_uint index, clMemory<T, AutoPolicy>& arg, ArgumentType::ArgTypes ArgumentType = ArgumentType::Unspecified) {
        ArgType[index] = ArgumentType;
        Callbacks[index] = arg.mem_ptr;

        cl_int status = Kernel.setArg(index, arg.GetBuffer());
        clError::Throw(status,  Name + " arg " + std::to_string(index));
    }

    // Overload for OpenCL Memory Buffers
    template <class T>
    void SetArg(cl_uint index, const T arg, ArgumentType::ArgTypes ArgumentType = ArgumentType::Unspecified) {
        ArgType[index] = ArgumentType;

        cl_int status = Kernel.setArg(index, arg);
        clError::Throw(status,  Name + " arg " + std::to_string(index));
    }

    template <class T>
    void SetLocalMemoryArg(cl_uint index, unsigned int size) {
        cl_int status = Kernel.setArg(index, size*sizeof(T), nullptr);
        clError::Throw(status,  Name + " arg " + std::to_string(index));
    }

    clEvent run(clWorkGroup Global);
//    clEvent run(clWorkGroup Global, clEvent StartEvent);
    clEvent run(clWorkGroup Global, clWorkGroup Local);
//    clEvent run(clWorkGroup Global, clWorkGroup Local, clEvent StartEvent);
//
//    cl_int GetStatus(){ return status; };
//
private:
//
//    void swap(clKernel& first, clKernel& second) {
//        std::swap(first.Program,second.Program);
//        std::swap(first.Kernel,second.Kernel);
//        std::swap(first.Context,second.Context);
//        std::swap(first.ArgType,second.ArgType);
//        std::swap(first.Callbacks,second.Callbacks);
//        std::swap(first.CodeString,second.CodeString);
//        std::swap(first.Name,second.Name);
//    }
//
    void RunCallbacks(clEvent KernelFinished);
//    void BuildKernelFromString();
};


#endif //CLWRAPPER_MAIN_CLKERNEL_H

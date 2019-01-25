//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_CLKERNEL_H
#define CLWRAPPER_MAIN_CLKERNEL_H

#include <algorithm>
#include <utility>

#include "CL/cl.h"

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
public:
    class BuildException: public std::runtime_error
    {
    public:
        BuildException(std::string message, cl_int status): runtime_error(message), Status(status){};
        cl_int Status;
    };

    clKernel() : NumberOfArgs(0), status(CL_SUCCESS), Program(nullptr), Kernel(nullptr) { }
    ~clKernel()
    {
        if(Program) {
            status = clReleaseProgram(Program);
            clError::Throw(status, Name);
        }
        if (Kernel) {
            status = clReleaseKernel(Kernel);
            clError::Throw(status, Name);
        }
    }

    clKernel(std::shared_ptr<clContext> _context, std::string codestring, unsigned int _NumberOfArgs, std::string _name)
            : Context(_context), NumberOfArgs(_NumberOfArgs), Name(_name), Program(nullptr), Kernel(nullptr), status(CL_SUCCESS)
    {
        ArgType.resize(_NumberOfArgs);
        Callbacks.resize(_NumberOfArgs);
        BuildKernelFromString(codestring, _name);
    }

    clKernel& operator=(clKernel Copy){ //TODO: i had to change the copy from a reference (&) to not, why?
        if(this!=&Copy)
        {
            Context = Copy.Context;
            NumberOfArgs = Copy.NumberOfArgs;
            Name = Copy.Name;
//            CodeString = Copy.CodeString;
            ArgType.clear();
            ArgType.resize(NumberOfArgs);
            Callbacks.clear();
            Callbacks.resize(NumberOfArgs);
//            BuildKernelFromString(CodeString,Name);
        }
        return *this;
    }

    clKernel(const clKernel& Copy): Context(Copy.Context), NumberOfArgs(Copy.NumberOfArgs), Name(Copy.Name)
    {
        ArgType.clear();
        Callbacks.clear();

        ArgType.resize(NumberOfArgs);
        Callbacks.resize(NumberOfArgs);
//        BuildKernelFromString(CodeString,Name);
    }

    // Overload for OpenCL Memory Buffers
    template <class T, template <class> class AutoPolicy> void SetArg(unsigned int position, std::shared_ptr<clMemory<T,AutoPolicy>>& arg, ArgumentType::ArgTypes ArgumentType = ArgumentType::Unspecified)
    {
        ArgType[position] = ArgumentType;
        Callbacks[position] = arg;

        status |= clSetKernelArg(Kernel, position, sizeof(cl_mem), nullptr);
        clError::Throw(status,  Name + " arg " + std::to_string(position) + ": Possibly trying to set a buffer argument with a non-buffer.");

        status |= clSetKernelArg(Kernel, position, sizeof(cl_mem), &arg->GetBuffer());
        clError::Throw(status,  Name + " arg " + std::to_string(position));
    }

    // Can enter arguments as literals now...
    template <class T> void SetArg(unsigned int position, const T arg, ArgumentType::ArgTypes ArgumentType = ArgumentType::Unspecified)
    {
        ArgType[position] = ArgumentType;

        status = clSetKernelArg(Kernel, position, sizeof(cl_mem), nullptr);
        if (status == CL_SUCCESS)
            clError::Throw(CL_INVALID_ARG_VALUE,  Name + " arg " + std::to_string(position) + ": Possibly trying to set a non-buffer argument with a buffer.");
        status = CL_SUCCESS;

        status |= clSetKernelArg(Kernel, (cl_uint)position, sizeof(T), &arg);
        clError::Throw(status, Name + " arg " + std::to_string(position));
    }

    template <class T> void SetLocalMemoryArg(unsigned int position, int size) {
        status |= clSetKernelArg(Kernel, position, size*sizeof(T), nullptr);
        clError::Throw(status,  Name + " arg " + std::to_string(position));
    }

    std::shared_ptr<clEvent> operator()(clWorkGroup Global);
    std::shared_ptr<clEvent> operator()(clWorkGroup Global, std::shared_ptr<clEvent> StartEvent);
    std::shared_ptr<clEvent> operator()(clWorkGroup Global, clWorkGroup Local);
    std::shared_ptr<clEvent> operator()(clWorkGroup Global, clWorkGroup Local, std::shared_ptr<clEvent> StartEvent);

    std::shared_ptr<clEvent> run(clWorkGroup Global);
    std::shared_ptr<clEvent> run(clWorkGroup Global, std::shared_ptr<clEvent> StartEvent);
    std::shared_ptr<clEvent> run(clWorkGroup Global, clWorkGroup Local);
    std::shared_ptr<clEvent> run(clWorkGroup Global, clWorkGroup Local, std::shared_ptr<clEvent> StartEvent);

    cl_int GetStatus(){ return status; };
    unsigned int NumberOfArgs;

private:
    cl_int status;
    std::vector<ArgumentType::ArgTypes> ArgType;
    std::vector<std::weak_ptr<Notify>> Callbacks;
    std::shared_ptr<clContext> Context;
    cl_program Program;
    cl_kernel Kernel;
    std::string Name;
//    const char* CodeString;

    void swap(clKernel& first, clKernel& second)
    {
        std::swap(first.Program,second.Program);
        std::swap(first.Kernel,second.Kernel);
        std::swap(first.Context,second.Context);
        std::swap(first.ArgType,second.ArgType);
        std::swap(first.Callbacks,second.Callbacks);
        std::swap(first.Name,second.Name);
    }

    void RunCallbacks(std::shared_ptr<clEvent> KernelFinished);
    void BuildKernelFromString(std::string, std::string kernelname);
};


#endif //CLWRAPPER_MAIN_CLKERNEL_H

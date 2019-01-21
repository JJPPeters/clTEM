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

    clKernel(){ NotDefault = false; }
    ~clKernel()
    {
        if(NotDefault)
        {
            status = clReleaseProgram(Program);
            clError::Throw(status, Name);
            status = clReleaseKernel(Kernel);
            clError::Throw(status, Name);
        }
    }

    clKernel(clContext &_context, const char* codestring, unsigned int _NumberOfArgs, std::string _name)
            : Context(&_context), NumberOfArgs(_NumberOfArgs), Name(_name)
    {
        NotDefault = true;
        ArgType.resize(_NumberOfArgs);
        Callbacks.resize(_NumberOfArgs);
        BuildKernelFromString(codestring,_name);
        CodeString = codestring;
    }

    clKernel& operator=(clKernel Copy){ //TODO: i had to change the copy form a reference (&) to not, why?
        if(this!=&Copy)
        {
            Context = Copy.Context;
            NumberOfArgs = Copy.NumberOfArgs;
            Name = Copy.Name;
            CodeString = Copy.CodeString;
            NotDefault = Copy.NotDefault;
            ArgType.clear();
            ArgType.resize(NumberOfArgs);
            Callbacks.clear();
            Callbacks.resize(NumberOfArgs);
            BuildKernelFromString(CodeString,Name);
        }
        return *this;
    }

    clKernel(const clKernel& Copy): Context(Copy.Context), NumberOfArgs(Copy.NumberOfArgs), Name(Copy.Name), CodeString(Copy.CodeString)
    {
        NotDefault = Copy.NotDefault;
        ArgType.clear();
        Callbacks.clear();

        if (!NotDefault)
            return;

        ArgType.resize(NumberOfArgs);
        Callbacks.resize(NumberOfArgs);
        BuildKernelFromString(CodeString,Name);
    }

    // Overload for OpenCL Memory Buffers
    template <class T, template <class> class AutoPolicy> void SetArg(unsigned int position, std::shared_ptr<clMemory<T,AutoPolicy>>& arg, ArgumentType::ArgTypes ArgumentType = ArgumentType::Unspecified)
    {
        ArgType[position] = ArgumentType;
        Callbacks[position] = arg.get();

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

    clEvent operator()(clWorkGroup Global);
    clEvent operator()(clWorkGroup Global, clEvent StartEvent);
    clEvent operator()(clWorkGroup Global, clWorkGroup Local);
    clEvent operator()(clWorkGroup Global, clWorkGroup Local, clEvent StartEvent);

    cl_int GetStatus(){ return status; };
    unsigned int NumberOfArgs;

private:
    bool NotDefault;
    cl_int status;
    std::vector<ArgumentType::ArgTypes> ArgType;
    std::vector<Notify*> Callbacks;
    clContext* Context;
    cl_program Program;
    cl_kernel Kernel;
    std::string Name;
    const char* CodeString;

    void swap(clKernel& first, clKernel& second)
    {
        std::swap(first.NotDefault,second.NotDefault);
        std::swap(first.Program,second.Program);
        std::swap(first.Kernel,second.Kernel);
        std::swap(first.Context,second.Context);
        std::swap(first.ArgType,second.ArgType);
        std::swap(first.Callbacks,second.Callbacks);
        std::swap(first.Name,second.Name);
    }

    void RunCallbacks(clEvent KernelFinished);
    void BuildKernelFromString(const char* codestring, std::string kernelname);
};


#endif //CLWRAPPER_MAIN_CLKERNEL_H

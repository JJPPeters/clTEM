//
// Created by jon on 08/10/16.
//

#include "clerror.h"

#include <stdexcept>

std::string clError::GetError(cl_int error_code)
{
    switch (error_code) {
        case CL_SUCCESS:                            return "Success!";
        case CL_DEVICE_NOT_FOUND:                   return "Device not found";
        case CL_DEVICE_NOT_AVAILABLE:               return "Device not available";
        case CL_COMPILER_NOT_AVAILABLE:             return "Compiler not available";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory object allocation failure";
        case CL_OUT_OF_RESOURCES:                   return "Out of resources";
        case CL_OUT_OF_HOST_MEMORY:                 return "Out of host memory";
        case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling information not available";
        case CL_MEM_COPY_OVERLAP:                   return "Memory copy overlap";
        case CL_IMAGE_FORMAT_MISMATCH:              return "Image format mismatch";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image format not supported";
        case CL_BUILD_PROGRAM_FAILURE:              return "Program build failure";
        case CL_MAP_FAILURE:                        return "Map failure";
        case CL_INVALID_VALUE:                      return "Invalid value";
        case CL_INVALID_DEVICE_TYPE:                return "Invalid device type";
        case CL_INVALID_PLATFORM:                   return "Invalid platform";
        case CL_INVALID_DEVICE:                     return "Invalid device";
        case CL_INVALID_CONTEXT:                    return "Invalid context";
        case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid queue properties";
        case CL_INVALID_COMMAND_QUEUE:              return "Invalid command queue";
        case CL_INVALID_HOST_PTR:                   return "Invalid host pointer";
        case CL_INVALID_MEM_OBJECT:                 return "Invalid memory object";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid image format descriptor";
        case CL_INVALID_IMAGE_SIZE:                 return "Invalid image size";
        case CL_INVALID_SAMPLER:                    return "Invalid sampler";
        case CL_INVALID_BINARY:                     return "Invalid binary";
        case CL_INVALID_BUILD_OPTIONS:              return "Invalid build options";
        case CL_INVALID_PROGRAM:                    return "Invalid program";
        case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid program executable";
        case CL_INVALID_KERNEL_NAME:                return "Invalid kernel name";
        case CL_INVALID_KERNEL_DEFINITION:          return "Invalid kernel definition";
        case CL_INVALID_KERNEL:                     return "Invalid kernel";
        case CL_INVALID_ARG_INDEX:                  return "Invalid argument index";
        case CL_INVALID_ARG_VALUE:                  return "Invalid argument value";
        case CL_INVALID_ARG_SIZE:                   return "Invalid argument size";
        case CL_INVALID_KERNEL_ARGS:                return "Invalid kernel arguments";
        case CL_INVALID_WORK_DIMENSION:             return "Invalid work dimension";
        case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid work group size";
        case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid work item size";
        case CL_INVALID_GLOBAL_OFFSET:              return "Invalid global offset";
        case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid event wait list";
        case CL_INVALID_EVENT:                      return "Invalid event";
        case CL_INVALID_OPERATION:                  return "Invalid operation";
        case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL object";
        case CL_INVALID_BUFFER_SIZE:                return "Invalid buffer size";
        case CL_INVALID_MIP_LEVEL:                  return "Invalid mip-map level";
        default: return "Unknown " + std::to_string(error_code);
    }
}

std::string clFftError::GetError(clfftStatus error_code)
{
    switch (error_code) {
        case CLFFT_BUGCHECK :                       return "Bugcheck";
        case CLFFT_NOTIMPLEMENTED:                  return "Functionality is not implemented yet";
        case CLFFT_TRANSPOSED_NOTIMPLEMENTED:       return "Transposed functionality is not implemented for this transformation";
        case CLFFT_FILE_NOT_FOUND:                  return "Tried to open an existing file on the host system, but failed";
        case CLFFT_FILE_CREATE_FAILURE:             return "Tried to create a file on the host system, but failed";
        case CLFFT_VERSION_MISMATCH:                return "Version conflict between client and library";
        case CLFFT_INVALID_PLAN:                    return "Requested plan could not be found";
        case CLFFT_DEVICE_NO_DOUBLE:                return "Double precision not supported on this device";
        case CLFFT_DEVICE_MISMATCH:                 return "Attempt to run on a device using a plan baked for a different device";
        default: return clError::GetError( (cl_int) error_code );
    }
}

void clError::Throw(cl_int error_code, std::string info)
{
    if (error_code != CL_SUCCESS)
    {
        std::string message = clError::GetError(error_code);
        if (info.size() > 0)
            message += ": " + info;
        throw std::runtime_error(message.c_str());
    }
}

void clFftError::Throw(clfftStatus error_code, std::string info)
{
    if ((cl_int) error_code != CL_SUCCESS)
    {
        std::string message = clError::GetError(error_code);
        if (info.size() > 0)
            message += ": " + info;
        throw std::runtime_error(message.c_str());
    }
}
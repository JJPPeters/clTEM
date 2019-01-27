//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_CLDEVICE_H
#define CLWRAPPER_CLDEVICE_H

#include <string>
#include <utility>

#include "CL/cl.hpp"
#include "clerror.h"
#include "utils.h"

namespace Device
{
    enum DeviceType: cl_device_type
    {
        GPU = CL_DEVICE_TYPE_GPU,
        CPU = CL_DEVICE_TYPE_CPU,
        All = CL_DEVICE_TYPE_ALL,
        DEFAULT = CL_DEVICE_TYPE_DEFAULT,
        ACCELERATOR = CL_DEVICE_TYPE_ACCELERATOR
    };
};

class clDevice {
private:
    cl::Device device;

    std::string platform_name;
    unsigned int  platform_number;

    std::string device_name;
    unsigned int device_number;


public:
    clDevice() : device(nullptr) {};
    clDevice(cl::Device _device, std::string _platform_name, unsigned int _platform_number, unsigned int _device_number) : device(_device), platform_name(_platform_name), platform_number(_platform_number), device_number(_device_number)  {
        cl_int status;
        device_name = device.getInfo<CL_DEVICE_NAME>(&status);
        clError::Throw(status, "clDevice");
//        device_name = Utils::Trim(device.getInfo<CL_DEVICE_NAME>(&status));
    };

    cl::Device& getDevice(){ return device; };
    std::string GetDeviceName(){ return device_name; };
    std::string GetPlatformName(){ return platform_name; };
    unsigned int GetDeviceNumber(){ return device_number; };
    unsigned int GetPlatformNumber(){ return (int) platform_number; };
    Device::DeviceType getDeviceType();

};


#endif //CLWRAPPER_CLDEVICE_H

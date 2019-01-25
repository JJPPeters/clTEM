//
// Created by jon on 08/10/16.
//

#include <iostream>
#include <vector>

#include "clstatic.h"
#include "utils.h"
#include "clerror.h"


std::vector<clDevice> OpenCL::GetDeviceList(Device::DeviceType dev_type)
{
    std::vector<clDevice> DeviceList;

    size_t valueSize;
    std::vector<char> value;
    std::vector<char> Pvalue;

    //Setup OpenCL
    cl_int status;
    cl_uint numPlatforms = 0;

    // get all platforms
    status = clGetPlatformIDs(0, nullptr, &numPlatforms);
    clError::Throw(status);

    std::vector<cl_platform_id> platforms(numPlatforms);
    status = clGetPlatformIDs(numPlatforms, &platforms[0], nullptr);
    clError::Throw(status);

    std::vector<cl_uint> DevPerPlatform;
    std::vector<std::vector<cl_device_id>> devices;

    for (int i = 0; i < numPlatforms; i++)
    {
        // fill vector with dummy data that will be written over immediately
        DevPerPlatform.push_back(0);
        //devices.push_back(NULL);

        // get all devices
        status = clGetDeviceIDs(platforms[i], dev_type, 0, nullptr, &DevPerPlatform[i]);
        if (DevPerPlatform[i] == 0)
            break; // no devices on this platform, but no need to throw (could be bacuse we are only looking for certain device types
        clError::Throw(status);
        devices.emplace_back(DevPerPlatform[i]);
        status = clGetDeviceIDs(platforms[i], dev_type, DevPerPlatform[i], &devices[i][0], nullptr);
        clError::Throw(status);

        status = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, nullptr, &valueSize);
        clError::Throw(status);
        Pvalue = std::vector<char>(valueSize);
        status = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, valueSize, &Pvalue[0], nullptr);
        clError::Throw(status);
        std::string pName(Pvalue.begin(), Pvalue.end());
        pName.erase(std::remove(pName.begin(), pName.end(), '\0'), pName.end());

        // for each device get and store name, platform, and device number
        for (int j = 0; j < DevPerPlatform[i]; j++)
        {
            // get device name
            status = clGetDeviceInfo(devices[i][j], CL_DEVICE_NAME, 0, nullptr, &valueSize);
            clError::Throw(status);
            value = std::vector<char>(valueSize);
            status = clGetDeviceInfo(devices[i][j], CL_DEVICE_NAME, valueSize, &value[0], nullptr);
            clError::Throw(status);
            std::string dName(value.begin(), value.end());
            dName.erase(std::remove(dName.begin(), dName.end(), '\0'), dName.end());

            clDevice newDev(devices[i][j], i, j, Utils::Trim(pName), Utils::Trim(dName));
            DeviceList.push_back(newDev);
        }
    }

    return DeviceList;
}

std::shared_ptr<clContext> OpenCL::MakeTwoQueueContext(clDevice& dev, Queue::QueueType Qtype, Queue::QueueType IOQtype)
{
    cl_int status;
    cl_context ctx = clCreateContext(nullptr,1,&dev.GetDeviceID(), nullptr, nullptr,&status);
    clError::Throw(status);
    cl_command_queue q = clCreateCommandQueue(ctx,dev.GetDeviceID(),Qtype,&status);
    clError::Throw(status);
    cl_command_queue ioq = clCreateCommandQueue(ctx,dev.GetDeviceID(),IOQtype,&status);
    clError::Throw(status);

    return std::make_shared<clContext>(dev, ctx, q, ioq, status);
}

std::shared_ptr<clContext> OpenCL::MakeTwoQueueContext(std::vector<clDevice> &devices, Queue::QueueType Qtype, Queue::QueueType IOQtype, Device::DeviceType devType)
{
    auto it =  devices.begin();
    clDevice dev;

    bool found = false;

    for(int i = 1; i <= devices.size() && !found; i++)
    {
        if((*it).GetDeviceType() == devType)
        {
            dev = *it;
            found = true;
        }
        ++it;
    }

    if(!found)
    {
        throw "No suitable device";
    }

    cl_int status;
    cl_context ctx = clCreateContext(nullptr,1,&dev.GetDeviceID(), nullptr, nullptr,&status);
    clError::Throw(status);
    cl_command_queue q = clCreateCommandQueue(ctx,dev.GetDeviceID(),Qtype,&status);
    clError::Throw(status);
    cl_command_queue ioq = clCreateCommandQueue(ctx,dev.GetDeviceID(),IOQtype,&status);
    clError::Throw(status);

    return std::make_shared<clContext>(dev,ctx,q,ioq,status);
}


std::shared_ptr<clContext> OpenCL::MakeContext(clDevice& dev, Queue::QueueType Qtype)
{
    cl_int status;
    cl_context ctx = clCreateContext(nullptr, 1, &dev.GetDeviceID(), nullptr, nullptr,&status);
    clError::Throw(status);
    cl_command_queue q = clCreateCommandQueue(ctx, dev.GetDeviceID(),Qtype,&status);
    clError::Throw(status);

    return std::make_shared<clContext>(dev,ctx,q,status);
}

std::shared_ptr<clContext> OpenCL::MakeContext(std::vector<clDevice> &devices, Queue::QueueType Qtype, Device::DeviceType devType)
{
    auto it =  devices.begin();
    clDevice dev;

    bool found = false;

    for(int i = 1; i <= devices.size() && !found; i++)
    {
        if((*it).GetDeviceType() == devType)
        {
            dev = *it;
            found = true;
        }
        ++it;
    }

    if(!found)
    {
        throw "No suitable device";
    }

    cl_int status;
    cl_context ctx = clCreateContext(NULL,1,&dev.GetDeviceID(),NULL,NULL,&status);
    clError::Throw(status);
    cl_command_queue q = clCreateCommandQueue(ctx,dev.GetDeviceID(),Qtype,&status);
    clError::Throw(status);

    return std::make_shared<clContext>(dev,ctx,q,status);
}
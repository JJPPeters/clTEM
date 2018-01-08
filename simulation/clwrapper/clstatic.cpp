//
// Created by jon on 08/10/16.
//

#include <iostream>
#include <vector>

#include "clstatic.h"
#include "utils.h"
#include "clerror.h"


std::list<clDevice> OpenCL::GetDeviceList(Device::DeviceType dev_type)
{
    std::list<clDevice> DeviceList;

    size_t valueSize;
    std::vector<char> value;
    std::vector<char> Pvalue;

    //Setup OpenCL
    cl_int status;
    cl_uint numPlatforms = 0;

    // get all platforms
    status = clGetPlatformIDs(NULL, NULL, &numPlatforms);
    clError::Throw(status);

    std::vector<cl_platform_id> platforms(numPlatforms);
    status = clGetPlatformIDs(numPlatforms, &platforms[0], NULL);
    clError::Throw(status);

    std::vector<cl_uint> DevPerPlatform;
    std::vector<std::vector<cl_device_id>> devices;

    for (int i = 0; i < numPlatforms; i++)
    {
        // fill vector with dummy data that will be written over immediately
        DevPerPlatform.push_back(0);
        //devices.push_back(NULL);

        // get all devices
        status = clGetDeviceIDs(platforms[i], dev_type, 0, NULL, &DevPerPlatform[i]);
        clError::Throw(status);
        devices.push_back(std::vector<cl_device_id>(DevPerPlatform[i]));
        status = clGetDeviceIDs(platforms[i], dev_type, DevPerPlatform[i], &devices[i][0], NULL);
        clError::Throw(status);

        status = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL, &valueSize);
        clError::Throw(status);
        Pvalue = std::vector<char>(valueSize);
        status = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, valueSize, &Pvalue[0], NULL);
        clError::Throw(status);
        std::string pName(Pvalue.begin(), Pvalue.end());

        // for each device get and store name, platform, and device number
        for (int j = 0; j < DevPerPlatform[i]; j++)
        {
            // get device name
            status = clGetDeviceInfo(devices[i][j], CL_DEVICE_NAME, 0, NULL, &valueSize);
            clError::Throw(status);
            value = std::vector<char>(valueSize);
            status = clGetDeviceInfo(devices[i][j], CL_DEVICE_NAME, valueSize, &value[0], NULL);
            clError::Throw(status);
            std::string dName(value.begin(), value.end());

            clDevice newDev(devices[i][j], i, j, pName, Utils::Trim(dName));
            DeviceList.push_back(newDev);
        }
    }

    return DeviceList;
}

clContext OpenCL::MakeTwoQueueContext(clDevice& dev, Queue::QueueType Qtype, Queue::QueueType IOQtype)
{
    cl_int status;
    cl_context ctx = clCreateContext(NULL,1,&dev.GetDeviceID(),NULL,NULL,&status);
    clError::Throw(status);
    cl_command_queue q = clCreateCommandQueue(ctx,dev.GetDeviceID(),Qtype,&status);
    clError::Throw(status);
    cl_command_queue ioq = clCreateCommandQueue(ctx,dev.GetDeviceID(),IOQtype,&status);
    clError::Throw(status);

    clContext Context(dev, ctx, q, ioq, status);
    return Context;
}

clContext OpenCL::MakeTwoQueueContext(std::list<clDevice> &devices, Queue::QueueType Qtype, Queue::QueueType IOQtype, Device::DeviceType devType)
{
    std::list<clDevice>::iterator it =  devices.begin();
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
    cl_command_queue ioq = clCreateCommandQueue(ctx,dev.GetDeviceID(),IOQtype,&status);
    clError::Throw(status);

    clContext Context(dev,ctx,q,ioq,status);
    return Context;
}


clContext OpenCL::MakeContext(clDevice& dev, Queue::QueueType Qtype)
{
    cl_int status;
    cl_context ctx = clCreateContext(NULL,1,&dev.GetDeviceID(),NULL,NULL,&status);
    clError::Throw(status);
    cl_command_queue q = clCreateCommandQueue(ctx,dev.GetDeviceID(),Qtype,&status);
    clError::Throw(status);

    clContext Context(dev,ctx,q,status);
    return Context;
}

clContext OpenCL::MakeContext(std::list<clDevice> &devices, Queue::QueueType Qtype, Device::DeviceType devType)
{
    std::list<clDevice>::iterator it =  devices.begin();
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

    clContext Context(dev,ctx,q,status);
    return Context;
}

clDevice OpenCL::GetDeviceByIndex(std::list<clDevice> DeviceList, int index)
{
    std::list<clDevice>::iterator it = DeviceList.begin();
    std::advance(it, index);
    return (*it);
}
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

    //Setup OpenCL
    cl_int status;

    // get all platforms
    std::vector<cl::Platform> platforms;
    status = cl::Platform::get(&platforms);
    clError::Throw(status);

    unsigned int p_counter = 0;

    for (auto & p : platforms) {
        std::string p_name = p.getInfo<CL_PLATFORM_NAME>();
//        std::string p_name = Utils::Trim(p.getInfo<CL_PLATFORM_NAME>());

        std::vector<cl::Device> devices;
        status = p.getDevices(dev_type, &devices);
        clError::Throw(status);

        unsigned int d_counter = 0;
        for (auto & d : devices) {
            DeviceList.emplace_back(clDevice(d, p_name, p_counter, d_counter));
            d_counter++;
        }
        p_counter++;
    }

    return DeviceList;
}

clContext OpenCL::MakeTwoQueueContext(clDevice& dev, Queue::QueueType Qtype, Queue::QueueType IOQtype)
{
    cl_int status = CL_SUCCESS;
    std::vector<cl::Device> device_list = {dev.getDevice()};
    cl::Context ctx(device_list);
    cl::CommandQueue q(ctx, dev.getDevice(), Qtype, &status);
    clError::Throw(status);
    cl::CommandQueue ioq(ctx, dev.getDevice(), IOQtype, &status);
    clError::Throw(status);

    return clContext(ctx, q, ioq, dev);
}

clContext OpenCL::MakeContext(clDevice &dev, Queue::QueueType Qtype)
{
    cl_int status = CL_SUCCESS;
    std::vector<cl::Device> device_list = {dev.getDevice()};
    cl::Context ctx(device_list);
    cl::CommandQueue q(ctx, dev.getDevice(), Qtype, &status);
    clError::Throw(status);

    return clContext(ctx, q, dev);
}
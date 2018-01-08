//
// Created by jon on 08/10/16.
//

#include "cldevice.h"

Device::DeviceType clDevice::GetDeviceType()
{
    cl_int status;
    Device::DeviceType deviceType;
    status =  clGetDeviceInfo(deviceID, CL_DEVICE_TYPE, sizeof(Device::DeviceType), &deviceType, NULL);
    clError::Throw(status);

    return deviceType;
};
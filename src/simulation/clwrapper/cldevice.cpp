//
// Created by jon on 08/10/16.
//

#include "cldevice.h"

Device::DeviceType clDevice::getDeviceType() {
    cl_int status;
    auto deviceType = static_cast<Device::DeviceType>(device.getInfo<CL_DEVICE_TYPE>(&status));
    clError::Throw(status, "clDevice");
    return deviceType;
};
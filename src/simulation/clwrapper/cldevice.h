//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_CLDEVICE_H
#define CLWRAPPER_CLDEVICE_H

#include <string>

#include "CL/cl.h"

#include "clerror.h"

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
public:
    clDevice(){};
    clDevice(cl_device_id devID, int platNum, int devNum, std::string platName, std::string devName )
            : deviceID(devID), deviceNum(devNum), platformNum(platNum), platformname(platName), devicename(devName){};

    ~clDevice()
    {
        //cl_int status = clReleaseDevice(deviceID);
        //clError::Throw(status);
    }

    cl_device_id& GetDeviceID(){ return deviceID; };
    std::string GetDeviceName(){ return devicename; };
    std::string GetPlatformName(){ return platformname; };
    int GetPlatformNumber(){ return platformNum; };
    int GetDeviceNumber(){ return deviceNum; };
    Device::DeviceType GetDeviceType();

private:
    int platformNum;
    int deviceNum;
    std::string platformname;
    std::string devicename;
    cl_device_id deviceID;

};


#endif //CLWRAPPER_CLDEVICE_H

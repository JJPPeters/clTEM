//
// Created by jon on 24/11/17.
//

#ifndef CLTEM_CLMANAGER_H
#define CLTEM_CLMANAGER_H

#include "clwrapper/clwrapper.h"
#include <vector>

struct ClManager
{
    static std::vector<clDevice> currentDevices;

    /// Purely for convenience
    static std::list<clDevice> getDeviceList()
    {
        return OpenCL::GetDeviceList(Device::DeviceType::All);
    }
};

#endif //CLTEM_CLMANAGER_H

//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_CLSTATUC_H
#define CLWRAPPER_MAIN_CLSTATUC_H

#include <list>
#include "cldevice.h"
#include "clcontext.h"

namespace Queue
{
    enum QueueType
    {
        InOrder = 0,
        InOrderWithProfiling = 0 | CL_QUEUE_PROFILING_ENABLE,
        OutOfOrder = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
        OutOfOrderWithProfiling = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE
    };
};

class OpenCL
{
public:
    static std::vector<clDevice> GetDeviceList(Device::DeviceType dev_type = Device::DeviceType::All);
    static clDevice GetDeviceByIndex(std::vector<clDevice> DeviceList, int index);
    static std::shared_ptr<clContext> MakeContext(clDevice& dev, Queue::QueueType Qtype = Queue::QueueType::InOrder);
    static std::shared_ptr<clContext> MakeContext(std::vector<clDevice>& devices, Queue::QueueType Qtype = Queue::QueueType::InOrder, Device::DeviceType devType = Device::DeviceType::GPU);
    static std::shared_ptr<clContext> MakeTwoQueueContext(clDevice& dev, Queue::QueueType Qtype = Queue::QueueType::InOrder, Queue::QueueType IOQtype = Queue::QueueType::InOrder);
    static std::shared_ptr<clContext> MakeTwoQueueContext(std::vector<clDevice>& devices, Queue::QueueType Qtype = Queue::QueueType::InOrder, Queue::QueueType IOQtype = Queue::QueueType::InOrder, Device::DeviceType devType = Device::DeviceType::GPU);
};


#endif //CLWRAPPER_MAIN_CLSTATUC_H

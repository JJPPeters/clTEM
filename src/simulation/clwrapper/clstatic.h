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
    enum QueueType : cl_command_queue_properties
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

    static clContext MakeContext(clDevice &dev, Queue::QueueType Qtype = Queue::QueueType::InOrder);
    static clContext MakeTwoQueueContext(clDevice& dev, Queue::QueueType Qtype = Queue::QueueType::InOrder, Queue::QueueType IOQtype = Queue::QueueType::InOrder);

//    template<class T,template <class> class AutoPolicy>
//    static std::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(std::shared_ptr<clContext> ctx, size_t size)
//    {
//        cl_int status;
//        std::shared_ptr<MemoryRecord> rec = std::make_shared<MemoryRecord>(size*sizeof(T));
//        auto b = clCreateBuffer(ctx->GetContext(), MemoryFlags::ReadWrite, size*sizeof(T), nullptr, &status);
//        clError::Throw(status, "");
//        std::shared_ptr<clMemory<T,AutoPolicy>> Mem = std::make_shared<clMemory<T,AutoPolicy>>(ctx, size, b, rec);
//        ctx->AddMemRecord(rec);
//        return Mem;
//    }
//
//    template<class T,template <class> class AutoPolicy >
//    static std::shared_ptr<clMemory<T,AutoPolicy>> CreateBuffer(std::shared_ptr<clContext> ctx, size_t size, enum MemoryFlags flags)
//    {
//        cl_int status;
//        std::shared_ptr<MemoryRecord> rec = std::make_shared<MemoryRecord>(size*sizeof(T));
//        auto b = clCreateBuffer(ctx->GetContext(), flags, size*sizeof(T), nullptr, &status);
//        std::shared_ptr<clMemory<T,AutoPolicy>> Mem = std::make_shared<clMemory<T,AutoPolicy>>(ctx, size, b, rec);
//        clError::Throw(status, "");
//        ctx->AddMemRecord(rec);
//        return Mem;
//    }
};


#endif //CLWRAPPER_MAIN_CLSTATUC_H

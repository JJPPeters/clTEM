//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_NOTIFY_H
#define CLWRAPPER_MAIN_NOTIFY_H

#include "clevent.h"

class Notify
{
        public:
        virtual void Update(std::shared_ptr<clEvent> KernelFinished)=0;
        virtual void UpdateEventOnly(std::shared_ptr<clEvent> KernelFinished) = 0;
        virtual std::shared_ptr<clEvent> GetFinishedWriteEvent()=0;
        virtual std::shared_ptr<clEvent> GetFinishedReadEvent()=0;
};
#endif //CLWRAPPER_MAIN_NOTIFY_H

//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_NOTIFY_H
#define CLWRAPPER_MAIN_NOTIFY_H

#include "clevent.h"

class Notify
{
public:
    virtual void Update(clEvent KernelFinished)=0;
    virtual void UpdateEventOnly(clEvent KernelFinished) = 0;
    virtual clEvent GetFinishedWriteEvent()=0;
    virtual clEvent GetFinishedReadEvent()=0;
};
#endif //CLWRAPPER_MAIN_NOTIFY_H

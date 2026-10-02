#ifndef DEVICESTATEHELPER_H
#define DEVICESTATEHELPER_H

#include "schedpolicy.h"

namespace devicestate_helper {
    int getCurrentPowerModeWeight(sched_policy::PowerMode powerMode);
}

#endif
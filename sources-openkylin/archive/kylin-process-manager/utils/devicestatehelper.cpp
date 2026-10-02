#include "devicestatehelper.h"

int devicestate_helper::getCurrentPowerModeWeight(sched_policy::PowerMode powerMode)
{
    int weight;
    switch (powerMode)
    {
    case sched_policy::PowerMode::Save:
        weight = 1;
        break;
    case sched_policy::PowerMode::Balance:
        weight = 2;
        break;
    case sched_policy::PowerMode::Performance:
        weight = 3;
        break;
    default:
        weight = 0;
        break;
    }
    return weight;
}
#include "powermanagercenter.h"
#include "config.h"
#include "brightness.h"
#include "idle.h"
#include "lowbattery.h"
#include "powermanagercontrol.h"
#include "powerpolicy.h"
#include "error.h"

PowerManagerCenter::PowerManagerCenter() :
        m_config(new Config)
{

    initPowerManagerCenter();
}

PowerManagerCenter::~PowerManagerCenter() {}

int PowerManagerCenter::initPowerManagerCenter()
{
    return 1;
}

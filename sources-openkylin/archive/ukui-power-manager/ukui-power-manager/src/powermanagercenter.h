#ifndef POWERMANAGERCENTER_H
#define POWERMANAGERCENTER_H

#include "common.h"

class Config;
class Brightness;
class Lid;
class Idle;
class PowerPolicy;
class PowerManagerControl;
class LowBattery;


class PowerManagerCenter : public QObject
{
    Q_OBJECT
public:
    PowerManagerCenter();
    virtual ~PowerManagerCenter();

private:
    int initPowerManagerCenter();
    QPointer<Config> m_config;
    QPointer<Brightness> m_brightness;
    QPointer<Lid> m_lid;
    QPointer<Idle> m_idle;
    QPointer<PowerPolicy> m_powerPolicy;
    QPointer<PowerManagerControl> m_control;
    QPointer<LowBattery> m_lowBattery;
private slots:
};
#endif // POWERMANAGERCENTER_H

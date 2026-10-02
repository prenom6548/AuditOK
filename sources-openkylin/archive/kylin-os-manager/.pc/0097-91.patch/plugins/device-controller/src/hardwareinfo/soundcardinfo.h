#ifndef SOUNDCARDINFO_H
#define SOUNDCARDINFO_H

#include "hardwareinfo.h"
#include "deviceinfo.h"
#include "../driverinfo/drivermodeinfo.h"

class SoundCardInfo : public HardWareInfo, public DeviceInfo {
public:
    SoundCardInfo();

    QString bus{QString()};
    QString clock{QString()};   // 时钟频率
    QString width{QString()};   // 数据位宽
    DriverModeInfo modeinfo;

    QString getValue(QString value);

};

#endif // SOUNDCARDINFO_H

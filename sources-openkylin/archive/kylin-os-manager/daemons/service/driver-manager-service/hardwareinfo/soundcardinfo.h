#ifndef SOUNDCARDINFO_H
#define SOUNDCARDINFO_H

#include "hardwareinfo.h"
#include "../driverinfo/drivermodeinfo.h"

class SoundCardInfo : public HardWareInfo
{
public:
    SoundCardInfo();

    QString bus{QString()};
    QString clock{QString()};   // 时钟频率
    QString width{QString()};   // 数据位宽
    DriverModeInfo modeinfo;
};

#endif // SOUNDCARDINFO_H

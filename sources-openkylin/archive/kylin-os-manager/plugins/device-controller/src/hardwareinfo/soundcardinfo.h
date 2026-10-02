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

    void setValue(QString key, QString value);

    void disableSoundCard();

    void enableSoundCard();

    bool checkStatus();
};

#endif // SOUNDCARDINFO_H

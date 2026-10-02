#ifndef CAMERAINFO_H
#define CAMERAINFO_H

#include <QString>
#include "deviceinfo.h"

class CameraInfo : public DeviceInfo{
public:
    CameraInfo();

    QString vendor{QString()}; // 供应商
    QString model{QString()}; // 型号
    QString name{QString()};
    QString interface{QString{}};
    QString driver{QString()};
    QString version{QString{}};
    QString businfo{QString()};
    QString speed{QString{}};

    QString getValue(QString value);

    void setValue(QString key, QString value);
};

#endif //CAMERAINFO_H

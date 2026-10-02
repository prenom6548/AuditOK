#ifndef MONITORINFO_H
#define MONITORINFO_H

#include <QString>
#include "deviceinfo.h"

class MonitorInfo : public DeviceInfo {
public:
    MonitorInfo();

    ~MonitorInfo();

    QString vendor{QString()}; // 供应商
    QString name{QString()};
    QString size{QString()};
    QString ratio{QString{}};
    QString resolution{QString()};
    QString max_resolution{QString{}};
    QString is_main{QString()};
    QString gamma{QString{}};
    QString interface{QString{}};
    QString model{QString{}};
    QString visible_area{QString{}};
    QString year{QString{}};
    QString week{QString{}};

    QString getValue(QString value);

    void setValue(QString key, QString value);

    QString getResolution();

    bool isMainMonitor();
};


#endif //MONITORINFO_H

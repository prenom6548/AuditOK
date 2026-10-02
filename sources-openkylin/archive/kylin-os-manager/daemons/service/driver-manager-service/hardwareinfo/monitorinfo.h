#ifndef MONITORINFO_H
#define MONITORINFO_H

#include <QString>

class MonitorInfo {
public:
    MonitorInfo();

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
};


#endif //MONITORINFO_H

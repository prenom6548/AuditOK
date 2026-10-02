#ifndef DISKINFO_H
#define DISKINFO_H

#include <QString>

class DiskInfo {
public:
    DiskInfo();

    QString model{QString{}};
    QString vendor{QString{}};
    QString name{QString{}};
    QString size{QString{}};
    QString type{QString{"0"}};    //固态？
    QString serial{QString{}};
    QString frimwareversion{QString{}};
    int is_main;
    QString interface{QString{}};
    QString ufs{QString()};
    QString ufsrevision{QString()};
};


#endif //DISKINFO_H

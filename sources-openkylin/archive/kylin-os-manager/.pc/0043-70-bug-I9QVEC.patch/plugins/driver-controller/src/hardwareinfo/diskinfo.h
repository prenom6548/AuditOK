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
    QString type{QString{}};    //固态？
    QString serial{QString{}};
    QString frimwareversion{QString{}};
};


#endif //DISKINFO_H

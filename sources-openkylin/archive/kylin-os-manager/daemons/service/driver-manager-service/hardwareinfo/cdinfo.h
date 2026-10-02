#ifndef CDINFO_H
#define CDINFO_H

#include <QString>

class CDInfo {
public:
    CDInfo();

    QString vendor{QString()}; // 供应商
    QString model{QString()}; // 型号
    QString businfo{QString()};
    QString name{QString()};
    QString version{QString()};
    QString driver{QString()};
    QString devicenum{QString()};
    QString speed{QString()};
    QString serialnum{QString()};
};


#endif //CDINFO_H

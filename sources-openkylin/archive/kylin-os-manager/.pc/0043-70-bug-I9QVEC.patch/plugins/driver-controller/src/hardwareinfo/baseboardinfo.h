#ifndef BASEBOARDINFO_H
#define BASEBOARDINFO_H

#include <QString>

class BaseboardInfo {
public:
    BaseboardInfo();

    QString model{QString{}};
    QString vendor{QString{}};
    QString version{QString{}};
    QString chassishandle{QString{}};  // 主板所属机箱
    QString biosvendor{QString{}};
    QString biosversion{QString{}};

};


#endif //BASEBOARDINFO_H

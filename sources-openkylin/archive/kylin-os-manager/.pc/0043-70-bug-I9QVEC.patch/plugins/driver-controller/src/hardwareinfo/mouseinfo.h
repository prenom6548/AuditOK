#ifndef MOUSEINFO_H
#define MOUSEINFO_H

#include <QString>

class MouseInfo{
public:
    MouseInfo();

    QString name{QString{}};
    QString devtype{QString{}};
    QString vendor{QString{}};
    QString interface{QString{}};
    QString model{QString{}};
    QString driver{QString{}};
};


#endif //MOUSEINFO_H

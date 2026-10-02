#ifndef KEYBOARDINFO_H
#define KEYBOARDINFO_H

#include <QString>
#include "deviceinfo.h"

class KeyboardInfo : public DeviceInfo{
public:
    KeyboardInfo();

    QString name{QString{}};
    QString devtype{QString{}};
    QString vendor{QString{}};
    QString interface{QString{}};
    QString model{QString{}};
    QString driver{QString{}};

    QString getValue(QString value);

    void setValue(QString key, QString value);
};

#endif //KEYBOARDINFO_H

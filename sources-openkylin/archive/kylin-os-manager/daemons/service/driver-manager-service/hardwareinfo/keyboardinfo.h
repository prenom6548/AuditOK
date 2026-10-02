#ifndef KEYBOARDINFO_H
#define KEYBOARDINFO_H

#include <QString>

class KeyboardInfo {
public:
    KeyboardInfo();

    QString name{QString{}};
    QString devtype{QString{}};
    QString vendor{QString{}};
    QString interface{QString{}};
    QString model{QString{}};
    QString driver{QString{}};
};


#endif //KEYBOARDINFO_H

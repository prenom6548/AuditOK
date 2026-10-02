#ifndef FANINFO_H
#define FANINFO_H

#include <QString>

class FanInfo {
public:
    FanInfo();

    QString name{QString()};
    QString speed{QString()};
};


#endif //FANINFO_H

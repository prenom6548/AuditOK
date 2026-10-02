#ifndef APPUTIL_H
#define APPUTIL_H

#include <QString>
#include <QHostAddress>

class AppUtil
{

public:
    static QString convertToIPv4Addr(const QHostAddress &addr);
};

#endif // APPUTIL_H

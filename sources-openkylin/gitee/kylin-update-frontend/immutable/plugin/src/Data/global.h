#ifndef GLOBAL_H
#define GLOBAL_H

#include <qstring.h>


class Global
{
public:
    Global();
    ~Global();

public:
    static QString UpdateMode;
    static QString Version;
    static int ServerNum;
};

#endif // GLOBAL_H

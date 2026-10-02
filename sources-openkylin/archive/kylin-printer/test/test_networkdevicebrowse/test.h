#ifndef TEST_H
#define TEST_H

#include <QObject>
#include <QThread>

#include "device_information.h"

class Test : public QObject
{
    Q_OBJECT
private:
    /* data */
public:
    Test(/* args */);
    ~Test();
    void show (DeviceInformation &);
};



#endif // TEST_H
#ifndef DEVICEREAD_H
#define DEVICEREAD_H


#include <QThread>
#include <QTimer>
//#include "setting.h"
//#include "camerapage.h"

class DeviceRead : public QThread
{
    Q_OBJECT
public:
    explicit DeviceRead();
public Q_SLOTS:
    void run();

Q_SIGNALS:
    void finish();
};

#endif // DEVICEREAD_H

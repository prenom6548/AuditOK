#ifndef CAMERACODESTATE_H
#define CAMERACODESTATE_H


#include <QThread>
#include "KylinCamera.h"
#include <QDebug>
#include <sys/stat.h>


class CameraCodeState : public QThread
{
    Q_OBJECT
public:
    explicit CameraCodeState(KylinCamera *camera);
    void setFlag(bool res);

private:
    void run();    
    KylinCamera *cameraCurrent = nullptr;
    bool flag = false;

Q_SIGNALS:
    void camerafinish();
};

#endif // CAMERACODESTATE_H

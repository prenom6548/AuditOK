#include "camerakeepalive.h"

CameraKeepAlive::CameraKeepAlive(KylinCamera &camera)
{
    currentCamera = &camera;
    heartBeatsNum = 0;
}

void CameraKeepAlive::run()
{
    while(heartBeatsNum < 5)
    {
        QThread::sleep(1);
        bool isAlive = true;
        if(!isAlive)
        {
            heartBeatsNum++;
            qDebug() << "camera is Dead!" << heartBeatsNum;
        }
        else
        {
            heartBeatsNum = 0;
        }
        qDebug() << "camera Alive!" << heartBeatsNum;
    }
    heartBeatsNum = 0;
    Q_EMIT cameraDead();
}

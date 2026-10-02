#ifndef CAMERAKEEPALIVE_H
#define CAMERAKEEPALIVE_H


#include <QThread>
#include <QString>
#include <QDebug>
#include <QTimer>
#include "KylinCamera.h"

class CameraKeepAlive : public QThread
{
    Q_OBJECT
public:
    explicit CameraKeepAlive(KylinCamera &camera);
private:
    void run();
    QTimer *perSecTimer = nullptr;
    QTimer *aliveSecTimer = nullptr;
    KylinCamera *currentCamera = nullptr;

    int heartBeatsNum;

Q_SIGNALS:
    void cameraDead();
};

#endif // CAMERAKEEPALIVE_H

#ifndef CAMERATOPPAGE_H
#define CAMERATOPPAGE_H

#include <QObject>
#include <QWidget>
#include <QStackedWidget>
#include <QCameraInfo>
#include "nodevicepage.h"
#include "hasdevicepage.h"
#include "devicepull.h"

class CameraTopPage: public QStackedWidget
{
    Q_OBJECT
public:
    DevicePull *devicePull = nullptr;
    CameraTopPage(QWidget *parent = 0);
    HasDevicePage *hasDevicePage = nullptr;
    ~CameraTopPage();
private:
    NoDevicePage *noDevicePage = nullptr;

Q_SIGNALS:
    void noDevice();
private Q_SLOTS:
    void displayCameraError(int);
    void displayCamera();
};

#endif // CAMERATOPPAGE_H

#ifndef DEVICEPULL_H
#define DEVICEPULL_H

#include <QWidget>
#include "currentdeviceinfo.h"
#include <QCameraInfo>
#include <QFileSystemWatcher>
#include "deviceread.h"
#include "util/login1_dbus.h"

class DevicePull: public QWidget
{
    Q_OBJECT
public:
    DevicePull();
    ~DevicePull();
public Q_SLOTS:
    void systemLock(bool res);
private Q_SLOTS:
    void handle();
    void setNoReactionFlag();
private:
    bool noReactionFlag = false;
    login1dbusManager *m_dbus;
    int countHistory;
    int countCurrent;
    bool flag = false;
    QFileSystemWatcher *watcher = nullptr;
    DeviceRead *devRead = nullptr;
    //1.拔摄像头情况的处理
    void devicePullOut();
    //2.插摄像头情况的处理
    void devicePullIn();

    //不同情況的處理
    //1.1.拔出当前正在使用的设备
    void devicePullOutInUse();
    //1.2.拔出未在使用的设备
    void devicePullOutNoInUse();

    //2.1.插入设备前，历史列表中无其他设备
    void devicePullInWhenNoDevice();
    //2.2.插入设备前，历史列表中有其他设备
    void devicePullInWhenHaveDevice();

    //1.1.1.拔出当前使用设备，历史列表中有其他设备
    void devicePullOutInUseHasOtherDevice();
    //1.1.2.拔出当前使用设备，历史列表中该设备是唯一设备
    void devicePullOutInUseHasNotOtherDevice();
    //侧边栏提示信息
    void showNotify(QString message);
Q_SIGNALS:
    void creatCameraPageSig(const char *);
    void displayCameraErrorSig();
    void change_deviceSig(const char *);
    void device_pull_out();
    void settingPageSig();

};

#endif // DEVICEPULL_H

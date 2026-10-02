#ifndef DRIVERMANAGERSERVICEDBUS_H
#define DRIVERMANAGERSERVICEDBUS_H

#include <QObject>
#include <QString>
#include "getDriver.h"
#include "hotplug/usblistenerthread.h"
#include "hotplug/usbhotplugserver.h"
#include "driverfinder/hotplugdevdriverfinder.h"
#include "drivermanagerclient.h"
#include "../dbus_args.h"
#include "../kom/kom-service.h"

class DriverManagerServiceTool : public QObject, protected QDBusContext {
Q_OBJECT
public:
    DriverManagerServiceTool(QObject *parent = nullptr);

    getDriver *getdriver = nullptr;

private:
    USBListenerThread *m_usbListenerThread;
    UsbHotplugServer *m_usbHotplugServer;
    HotplugDevDriverFinder *m_hotplugDriverFinder;

    DrivermanagerClient *m_driverClient;

    QString pcInfo = "";
    QString cpuInfo = "";
    QString memoryInfo = "";
    QString baseboardInfo = "";
    QString graphicCardInfo = "";
    QString scannerDriverInfo = "";
    QString printerDriverInfo = "";
    QString soundCardInfo = "";
    QString inputDevInfo = "";
    QString videoDevInfo = "";
    QString otherDevInfo = "";
    QString bluetoothInfo = "";
    bool initFinish = false;

    void initConnect();

    bool isAllowedCaller();

public slots:

    QString getPcInfo();

    QString getCpuInfo();

    QString getMemoryInfo();

    QString getBaseboardInfo();

    QString getDiskInfo();

    QString getMonitorInfo();

    QString getKeyboardInfo();

    QString getMouseInfo();

    QString getCameraInfo();

    QString getBatteryInfo();

    QString getCDInfo();

    QString getFanInfo();

    QString getGraphicCardDriver();

    QString getScannerDriver();

    QString getPrinterDriver();

    QString getBluetoothInfo();

    QString getSoundcardInfo();

    QString getNetWorkcardInfo();

    QString getInputDeviceDriver();

    QString getVideoDeviceDriver();

    QString getOtherDeviceDriver();

    // 客户端安装驱动后重新检测
    void redetectGraphicCard();

    void redetectScanner();

    void redetectPrinter();

    void redetectInputDev();

    void redetectVideoDev();

    void redetectOtherDev();

    void redetectBluetoothDev();

    void enableWirelessNetworkCard();

    void disableWirelessNetworkCard();

    void enableWiredNetworkCard();

    void disableWiredNetworkCard();

    QString getWirelessNetworkCardStatus();

    QString getWiredNetworkCardStatus();

public:
    // 判断具体需要重新检测的类型
    void checkRedetectType(QString action, QString vid, QString pid, QString vendor, QString model, QString devtype);

    void updateHWinfo(DeviceType type, QString hwInfo);

private slots:

    void updateInfo();  // 安装成功后的刷新信息
};


#endif // DRIVERMANAGERSERVICEDBUS_H

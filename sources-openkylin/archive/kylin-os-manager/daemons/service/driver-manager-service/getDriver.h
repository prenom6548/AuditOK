#ifndef GETDRIVER_H
#define GETDRIVER_H

#include <QObject>
#include <QString>

#include "driverfinder/graphicdriverfinder.h"
#include "driverfinder/scannerdriverfinder.h"
#include "driverfinder/printerdriverfinder.h"
#include "driverfinder/videoDriverFinder.h"
#include "driverfinder/inputdevicedriverfinder.h"
#include "driverfinder/otherdevicedriverfinder.h"

#include "hardwarefinder/bluetoothfinder.h"
#include "hardwarefinder/soundcardfinder.h"
#include "hardwarefinder/networkcardfinder.h"
#include "hardwarefinder/scannerfinder.h"
#include "hardwarefinder/inputdevicefinder.h"
#include "hardwarefinder/videodevicefinder.h"
#include "hardwarefinder/printerfinder.h"
#include "hardwarefinder/pcinfogetter.h"
#include "hardwarefinder/cpuinfofinder.h"
#include "hardwarefinder/memoryinfofinder.h"
#include "hardwarefinder/baseboardinfofinder.h"
#include "hardwarefinder/diskinfofinder.h"
#include "hardwarefinder/monitorinfofinder.h"
#include "hardwarefinder/keyboardinfofinder.h"
#include "hardwarefinder/mouseinfofinder.h"
#include "hardwarefinder/camerainfofinder.h"
#include "hardwarefinder/batteryinfofinder.h"
#include "hardwarefinder/cdinfofinder.h"
#include "hardwarefinder/faninfofinder.h"

#include "hardwareinfo/graphiccardinfo.h"
#include "hardwareinfo/bluetoothinfo.h"
#include "hardwareinfo/soundcardinfo.h"
#include "hardwareinfo/networkcardinfo.h"
#include "hardwareinfo/pcinfo.h"
#include "hardwareinfo/cpuinfo.h"
#include "hardwareinfo/memoryinfo.h"
#include "hardwareinfo/baseboardinfo.h"
#include "hardwareinfo/diskinfo.h"
#include "hardwareinfo/monitorinfo.h"
#include "hardwareinfo/keyboardinfo.h"
#include "hardwareinfo/camerainfo.h"
#include "hardwareinfo/batteryinfo.h"
#include "hardwareinfo/cdinfo.h"
#include "hardwareinfo/faninfo.h"

class getDriver : public QObject {
Q_OBJECT
public:
    PcInfo m_pcInfo;
    CpuInfo m_cpuInfo;
    QList<MemoryInfo> m_memoryInfo;
    BaseboardInfo m_baseboardInfo;
    QList<DiskInfo> m_diskInfo;
    QList<MonitorInfo> m_monitorInfo;
    QList<KeyboardInfo> m_keyboardInfo;
    QList<MouseInfo> m_mouseInfo;
    QList<CameraInfo> m_cameraInfo;
    QList<BatteryInfo> m_batteryInfo;
    QList<CDInfo> m_CDInfo;
    QList<FanInfo> m_FanInfo;

    QList<GraphicCardInfo> m_graphicHardwares;
    QList<HardWareInfo> m_scannerHardwares;
    QList<HardWareInfo> m_printerHardwares;
    QList<BluetoothInfo> m_bluetoothHardwares;      // 目前仅获取信息
    QList<SoundCardInfo> m_soundCardHardwares;      // 目前仅获取信息
    QList<NetworkCardInfo> m_networkCardHardwares;  // 目前仅获取信息
    QList<HardWareInfo> m_inputHardwares;
    QList<HardWareInfo> m_videoHardwares;
    QList<HardWareInfo> m_otherHardwares;

    static getDriver *getInstance();

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

    QList<HardWareInfo> getPrinterScanenrHw();

    void addHWinfo(bool flag, HardWareInfo hwInfo);

    void removeHWinfo(HardWareInfo hwInfo);

    QString convertClassToString(QList<HardWareInfo> hardwareList);

    QString convertClassToString(QList<GraphicCardInfo> graphicHardwareList);

    QString convertClassToString(QList<BluetoothInfo> bluetoothHardwareList);

    QString convertClassToString(QList<SoundCardInfo> soundCardHardwareList);

    QString convertClassToString(QList<NetworkCardInfo> networkCardHardwareList);

    QString convertClassToString(PcInfo pcinfo);

    QString convertClassToString(CpuInfo cpuinfo);

    QString convertClassToString(QList<MemoryInfo> memoryInfoList);

    QString convertClassToString(BaseboardInfo baseboardinfo);

    QString convertClassToString(QList<DiskInfo> diskInfolist);

    QString convertClassToString(QList<MonitorInfo> monitorInfolist);

    QString convertClassToString(QList<KeyboardInfo> keyboardInfolist);

    QString convertClassToString(QList<MouseInfo> mouseInfolist);

    QString convertClassToString(QList<CameraInfo> cameraInfolist);

    QString convertClassToString(QList<BatteryInfo> batteryInfolist);

    QString convertClassToString(QList<CDInfo> cdInfolist);

    QString convertClassToString(QList<FanInfo> fanInfolist);

private:
    static getDriver *instance;

    getDriver(QObject *parent = nullptr);

    graphicDriverFinder *m_graphicDriverFinder = nullptr;
    scannerDriverFinder *m_scannerDriverFinder = nullptr;
    printerDriverFinder *m_printerDriverFinder = nullptr;
    bluetoothFinder *m_bluetoothFinder = nullptr;
    SoundCardFinder *m_soundCardFinder = nullptr;
    NetworkCardFinder *m_networkCardFinder = nullptr;
    inputdevicedriverfinder *m_inputDevFinder = nullptr;
    videoDriverFinder *m_videoDriverFinder = nullptr;
    otherDeviceDriverFinder *m_otherDevDriverFinder = nullptr;
    PcIfoGetter *m_pcInfoGetter = nullptr;
    CpuInfoFinder *m_cpuInfoGetter = nullptr;
    MemoryInfoFinder *m_memoryInfoGetter = nullptr;
    BaseboardInfoFinder *m_baseboardInfoGetter = nullptr;
    DiskInfoFinder *m_diskInfoGetter = nullptr;
    MonitorInfoFinder *m_monitorInfoGetter = nullptr;
    KeyboardInfoFinder *m_keyboardInfoGetter = nullptr;
    MouseInfoFinder *m_mouseInfoGetter = nullptr;
    CameraInfoFinder *m_cameraInfoGetter = nullptr;
    BatteryInfoFinder *m_batteryInfoGetter = nullptr;
    CDInfoFinder *m_cdInfoGetter = nullptr;
    FanInfoFinder *m_fanInfoGetter = nullptr;

signals:

    void refreshInfo(DeviceType, QString);

    void addDevInfo(QString);

    void removeDevInfo(QString);
};

#define DRIVER_GETTER getDriver::getInstance()

#endif //GETDRIVER_H

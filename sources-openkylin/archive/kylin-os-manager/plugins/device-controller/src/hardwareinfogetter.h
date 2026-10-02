#ifndef HARDWAREINFOGETTER_H
#define HARDWAREINFOGETTER_H

#include <QObject>
#include "src/hardwareinfo/hardwareinfo.h"
#include "src/hardwareinfo/bluetoothinfo.h"
#include "src/hardwareinfo/graphiccardinfo.h"
#include "src/hardwareinfo/networkcardinfo.h"
#include "src/hardwareinfo/pcinfo.h"
#include "src/hardwareinfo/cpuinfo.h"
#include "src/hardwareinfo/memoryinfo.h"
#include "src/hardwareinfo/diskinfo.h"
#include "src/hardwareinfo/monitorinfo.h"
#include "src/hardwareinfo/baseboardinfo.h"
#include "src/hardwareinfo/soundcardinfo.h"
#include "src/hardwareinfo/keyboardinfo.h"
#include "src/hardwareinfo/mouseinfo.h"
#include "src/hardwareinfo/cdinfo.h"
#include "src/hardwareinfo/camerainfo.h"
#include "src/hardwareinfo/faninfo.h"
#include "src/hardwareinfo/batteryinfo.h"

#include "src/driverinfo/driverinfo.h"
#include "src/driverinfo/drivermodeinfo.h"
#include <QtDBus>
#include <QDBusPendingCall>
#include <QGSettings>

class HardwareInfoGetter : public QObject {
Q_OBJECT
public:
    static HardwareInfoGetter *getInstance();

    PcInfo getPcInfo();

    QList<CpuInfo> getCpuInfo();

    QList<MemoryInfo> getMemoryInfo();

    QList<BaseboardInfo> getBaseboardInfo();

    QList<DiskInfo> getDiskInfo();

    QList<MonitorInfo> getMonitorInfo();

    QList<KeyboardInfo> getKeyboardInfo();

    QList<MouseInfo> getMouseInfo();

    QList<CDInfo> getCDInfo();

    QList<CameraInfo> getCameraInfo();

    QList<FanInfo> getFanInfo();

    QList<BatteryInfo> getBatteryInfo();

    void refreshDiskInfo();

    void refreshMonitorInfo();

    void refreshKeyboardInfo();

    void refreshMouseInfo();

    void refreshBluetoothInfo();

    void refreshNetworkInfo();

    void refreshCdInfo();

    void refreshFlag();

    void refreshServerInfo(DeviceType deviceType);


    int m_getFlag = 0;    // 用于记录上述设备驱动获取一共完成了多少个
    int m_getFailFlag = 0; // 用于记录上述设备驱动获取一共失败了多少个
    bool m_getsucceed = false;

    PcInfo m_pcinfo;
    QList<CpuInfo> m_cpuinfo;
    QList<MemoryInfo> m_memoryinfoList;
    QList<BaseboardInfo> m_baseboardinfo;
    QList<DiskInfo> m_diskinfoList;
    QList<MonitorInfo> m_monitorList;
    QList<KeyboardInfo> m_keyboardInfoList;
    QList<MouseInfo> m_mouseInfoList;
    QList<CDInfo> m_cdInfoList;
    QList<BatteryInfo> m_batteryInfoList;
    QList<FanInfo> m_fanInfoList;
    QList<CameraInfo> m_cameraInfoList;

    QList<GraphicCardInfo> m_graphicHardwares;
    QList<HardWareInfo> m_scannerHardwares;
    QList<HardWareInfo> m_printerHardwares;
    QList<BluetoothInfo> m_bluetoothHardwares;      // 目前仅获取信息
    QList<SoundCardInfo> m_soundCardHardwares;      // 目前仅获取信息
    QList<NetworkCardInfo> m_networkCardHardwares;  // 目前仅获取信息
    QList<HardWareInfo> m_inputHardwares;
    QList<HardWareInfo> m_videoHardwares;
    QList<HardWareInfo> m_otherHardwares;


    // 热插拔后的比较内容
    QList<DiskInfo> m_oldDiskInfoList;
    QList<MouseInfo> m_oldMouseInfoList;
    QList<KeyboardInfo> m_oldKeyboardInfoList;
    QList<MonitorInfo> m_oldMonitorInfoList;
    QList<BluetoothInfo> m_oldBluetoothInfoList;
    QList<NetworkCardInfo> m_oldNetworkInfoList;
    QList<CDInfo> m_oldCdInfoList;

public slots:

    QList<BluetoothInfo> getBluetoothHardwares();

    QList<NetworkCardInfo> getNetworkHardwares();

    QList<SoundCardInfo> getSoundcardHardwares();

    QList<GraphicCardInfo> getGraphicHardwares();

    QList<HardWareInfo> getScannerHardwares();

    QList<HardWareInfo> getPrinterHardwares();

    QList<HardWareInfo> getInputHardwares();

    QList<HardWareInfo> getVideoHardwares();

    QList<HardWareInfo> getOtherHardwares();


private:
    static HardwareInfoGetter *instance;

    QGSettings *m_data = nullptr;

    HardwareInfoGetter(QObject *parent = nullptr);

    QString getInfo(DeviceType type);

    QList<HardWareInfo> convertJsonToHardWareList(const QString &jsonString, DeviceType type);

    void uploadMapMessage(DeviceType type, HardWareInfo result);

    void genMonitorInfoDataFile();

    QList<BluetoothInfo> regetBluetoothHardwares();

    QList<NetworkCardInfo> regetNetworkHardwares();

};

#define g_infogetter HardwareInfoGetter::getInstance()
#endif //HARDWAREINFOGETTER_H

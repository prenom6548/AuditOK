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

#include "src/driverinfo/driverinfo.h"
#include "src/driverinfo/drivermodeinfo.h"
#include <QtDBus>

class HardwareInfoGetter : public QObject {
Q_OBJECT
public:
    static HardwareInfoGetter *getInstance();

    PcInfo getPcInfo();
    CpuInfo getCpuInfo();
    QList<MemoryInfo> getMemoryInfo();
    BaseboardInfo getBaseboardInfo();
    QList<DiskInfo> getDiskInfo();
    QList<MonitorInfo> getMonitorInfo();
    QList<KeyboardInfo> getKeyboardInfo();
    QList<MouseInfo> getMouseInfo();

    void refreshFlag();
    void refreshServerInfo(DeviceType deviceType);


    int m_getFlag = 0;    // 用于记录上述设备驱动获取一共完成了多少个
    int m_getFailFlag = 0; // 用于记录上述设备驱动获取一共失败了多少个
    bool m_getsucceed = false;

    PcInfo m_pcinfo;
    CpuInfo m_cpuinfo;
    QList<MemoryInfo> m_memoryinfoList;
    BaseboardInfo m_baseboardinfo;
    QList<DiskInfo> m_diskinfoList;
    QList<MonitorInfo> m_monitorList;
    QList<KeyboardInfo> m_keyboardInfoList;
    QList<MouseInfo> m_mouseInfoList;

    QList<GraphicCardInfo> m_graphicHardwares;
    QList<HardWareInfo> m_scannerHardwares;
    QList<HardWareInfo> m_printerHardwares;
    QList<BluetoothInfo> m_bluetoothHardwares;      // 目前仅获取信息
    QList<SoundCardInfo> m_soundCardHardwares;      // 目前仅获取信息
    QList<NetworkCardInfo> m_networkCardHardwares;  // 目前仅获取信息
    QList<HardWareInfo> m_inputHardwares;
    QList<HardWareInfo> m_videoHardwares;
    QList<HardWareInfo> m_otherHardwares;

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

    HardwareInfoGetter(QObject *parent = nullptr);

    QString getInfo(DeviceType type);

    QList<HardWareInfo> convertJsonToHardWareList(const QString &jsonString, DeviceType type);

    void uploadMapMessage(DeviceType type, HardWareInfo result);
};
#define g_infogetter HardwareInfoGetter::getInstance()
#endif //HARDWAREINFOGETTER_H

#ifndef GLOBALSIGNAL_H
#define GLOBALSIGNAL_H

#include <QObject>
#include <QSize>
#include "../deviceitem.h"
#include "../uninstallitem.h"

class GlobalSignal : public QObject {
Q_OBJECT
public:
    static GlobalSignal *getInstance();

    bool onUninstalling = false;
    bool onInstalling = false;
signals:

    void themeChangeSignal();

    void getHwInfoComplete();

    void updateInstallContentSucceed();

    void getHwInfoFailed();

    void uncheckCheckBox(int num);   // num = 0 取消安装checkbox、num = 1 取消升级checkbox

    void checkAllChildenCheckboxState(int num);  // 同上

    void switchInstallPage();

    void installDriver(DeviceItem *item, QString driverName);

    void switchUninstallPage();        // 有可删除的驱动，切换到删除界面

    void switchNoUninstallPage();      // 无可删除的驱动，切换到无可删除界面

    void uninstallDriver(UninstallItem *item, QString driverName);

    void uninstallUpdateDriver(DeviceItem *item, QString driverName);

//    void addToUninstallSignal(HardWareInfo hardWareInfo);     // 下载新驱动成功后，发出此信号，添加到删除界面
//
//    void addToinstallSignal(HardWareInfo hardWareInfo);

    void quitEventLoop();

    void reDetectSignal();

    // 热插拔
    void removeItem(QList<HardWareInfo>);

    // 热插拔
    void addItem(QList<HardWareInfo>);

    // 数据库操作
    void insertDataSignal(QStringList);

    void deleteDataSignal(QString);

    void deleteDataFromDriverSignal(QString);

    // 展开、关闭后设置页面长度
    void expandTreeWidget(int count);

    // 跳转硬件信息
    void jumpToHardWareInfoPage();

    // 跳转驱动管理界面
    void jumpToDriverManagerPage();

    // 硬件设备热插拔
    void deviceInserted(QString devtype);

    void deviceRemoved(QString devtype);

    // 显示器热插拔
    void monitorChanged();

    // 启/禁用蓝牙
    void activateBluetooth();

    void deactivateBluetooth();

    // 导出基本信息
    void exportBasicContentSignal();

    // 蓝牙禁用启用信号
    void disableBluetoothSignal();

    void enableBluetoothSignal();

private:
    explicit GlobalSignal(QObject *parent = nullptr);

    static GlobalSignal *instance;

    ~GlobalSignal();
};

#define g_signal GlobalSignal::getInstance()

#endif // GLOBALSIGNAL_H

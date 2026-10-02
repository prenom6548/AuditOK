#ifndef DRIVERINSTALLWIDGET_H
#define DRIVERINSTALLWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QHBoxLayout>
#include <kborderlessbutton.h>

#include "include/custompushbutton.h"
#include "hardwareinfogetter.h"
#include "uninstallitem.h"
#include "deviceitem.h"
#include "driveroperation/aptinstaller.h"
#include "getinfothread.h"
#include "driveroperation/uninstaller.h"

class DriverInstallWidget : public QWidget {
Q_OBJECT
public:
    explicit DriverInstallWidget(QWidget *parent = nullptr);

    DeviceItem *m_item = nullptr;
    UninstallItem *m_uninstallItem = nullptr;
    QEventLoop eventLoop;

private:
    AptInstaller *m_installer = nullptr;
    Uninstaller *m_uninstaller = nullptr;

    int m_installabelNum = 0; // 用于标识是否存在可安装的硬件驱动
    int m_updateabelNum = 0; // 用于标识是否存在可升级的硬件驱动
    int m_notUpdatedNum = 0; // 用于标识无需升级的设备数量
    int m_notFindDriverNum = 0; // 用于标识未适配的设备数量

    QStringList m_devList;  // 用于标识设备，防止在列表中出现多个设备

    QVBoxLayout *m_mainLayout = nullptr;

    QListWidget *m_installableListWidget = nullptr;
    QHBoxLayout *m_installableListWidgetLayout = nullptr;

    QListWidget *m_upgradableListWidget = nullptr;
    QHBoxLayout *m_upgradableListWidgetLayout = nullptr;

    QListWidget *m_notUpdatedListWidget = nullptr;
    QHBoxLayout *m_notUpdatedListWidgetLayout = nullptr;

    QListWidget *m_notFindDriverListWidget = nullptr;
    QHBoxLayout *m_notFindDriverListWidgetLayout = nullptr;

    // 可安装的
    QWidget *m_installableWidget = nullptr;
    QHBoxLayout *m_installableLayout = nullptr;
    QCheckBox *m_installableCheckBox = nullptr;
    QLabel *m_installableLabel = nullptr;
    kdk::KBorderlessButton *m_installButton = nullptr;

    // 可升级的
    QWidget *m_upgradableWidget = nullptr;
    QHBoxLayout *m_upgradableLayout = nullptr;
    QCheckBox *m_upgradableCheckBox = nullptr;
    QLabel *m_upgradableLabel = nullptr;
    kdk::KBorderlessButton *m_updateButton = nullptr;

    // 已安装的
    QWidget *m_notUpdatedWidget = nullptr;
    QHBoxLayout *m_notUpdatedLayout = nullptr;
    QCheckBox *m_notUpdatedCheckBox = nullptr;
    QLabel *m_notUpdatedLabel = nullptr;

    // 未适配
    QWidget *m_notFindDriverWidget = nullptr;
    QHBoxLayout *m_notFindDriverLayout = nullptr;
    QCheckBox *m_notFindDriverCheckBox = nullptr;
    QLabel *m_notFindDriverLabel = nullptr;

    DeviceItem *m_tempupdateitem;
    UninstallItem *m_tempitem;
    QString m_tempDriverName;

    RefreshScannerInfo refreScannerThread;
    RefreshGraphicCardInfo refreshGraphicCardInfo;
    RefreshPrinterInfo refreshPrinterInfo;
    RefreshInputDevInfo refreshInputDevInfo;
    RefreshVideoDevInfo refreshVideoDevInfo;
    RefreshOtherDevInfo refreshOtherDevInfo;


    void initGui();

    void initConnection();

    void caculateAndSetPageSize();

    void checkUpgradable();

    void addGraphicCardContent();

    void addScannerContent();

    void addPrinterContent();

    void addBluetoothContent();

    void addSoundCardContent();

    void addNetworkCardContent();

    void addInputHwContent();

    void addVideoContent();

    void getOtherContent();

    void addDeleteItem(QList<HardWareInfo> hwList);

    void moveToInstalledSlot(DeviceItem *item, HardWareInfo hardWareInfo);

    HardWareInfo updateHwInfo(DeviceItem *item, QString driverName);

    HardWareInfo updateHwInfoForUninstall(DeviceType deviceType, QString driverName);

private slots:

    void themeFontStyle();

    void setContent();

    void setInstallableListWidgetCheck();

    void setUpgradableListWidgetCheck();

    void uncheckCheckBox(int num);

    void checkAllChildenCheckboxStateSlot(int num);

    void installDriverSlot(DeviceItem *item, QString driverName);

    void installSuccessSlot();

    void installFailSlot(QString result);

    void uninstallUpdateSuccessSlot();

    void uninstallSuccessSlot();

    void addNewInstallabelItemSlot(HardWareInfo hardWareInfo);

    void installMultiDriverSlot();

    void updateMultiDriverSlot();

    void removeItemSlot(QList<HardWareInfo> hwInfo);

    void addItemSlot(QList<HardWareInfo> hwInfo);

    void uninstallUpdateDriverSlot(DeviceItem *item, QString driverName);

    void uninstallDriverSlot(UninstallItem *item, QString driverName);

};

#endif //DRIVERINSTALLWIDGET_H

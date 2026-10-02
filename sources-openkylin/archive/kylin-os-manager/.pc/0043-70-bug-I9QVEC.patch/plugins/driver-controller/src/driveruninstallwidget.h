#ifndef DRIVERUNINSTALLWIDGET_H
#define DRIVERUNINSTALLWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>

#include "uninstallitem.h"
#include "driveroperation/uninstaller.h"
#include "getinfothread.h"

class DriverUninstallWidget : public QWidget {
Q_OBJECT
public:
    DriverUninstallWidget(QWidget *parent = nullptr);

    ~DriverUninstallWidget();

    UninstallItem *m_item = nullptr;

private:
    Uninstaller *m_uninstaller = nullptr;

    QStringList m_devList;  // 用于标识设备，防止在列表中出现多个设备

    QLabel *m_alertIconLabel = nullptr;
    QLabel *m_alertLabel = nullptr;
    QListWidget *m_listWidget = nullptr;
    QHBoxLayout *m_labelLayout = nullptr;
    QWidget *m_labelWidget = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    UninstallItem *m_tempitem;
    QString m_tempdriverName;

    RefreshScannerInfo refreScannerThread;
    RefreshGraphicCardInfo refreshGraphicCardInfo;
    RefreshPrinterInfo refreshPrinterInfo;
    RefreshInputDevInfo refreshInputDevInfo;
    RefreshVideoDevInfo refreshVideoDevInfo;
    RefreshOtherDevInfo refreshOtherDevInfo;

    void initGui();

    void initDataBase();

    void initConnection();

    void addGraphicCardContent();

    void addScannerContent();

    void addPrinterContent();

    void addInputHwContent();

    void addVideoContent();

    void getOtherContent();

    HardWareInfo updateHwInfo(UninstallItem *item, QString driverName);

    DriverInfo getGpuDriver(HardWareInfo hwinfo);

    void addDeleteItem(QList<HardWareInfo> hwList);

private slots:

    void setContent();

    void addContent(HardWareInfo hardWareInfo);

    void uninstallDriverSlot(UninstallItem *item, QString driverName);

    void uninstallSuccessSlot();
};


#endif //DRIVERUNINSTALLWIDGET_H

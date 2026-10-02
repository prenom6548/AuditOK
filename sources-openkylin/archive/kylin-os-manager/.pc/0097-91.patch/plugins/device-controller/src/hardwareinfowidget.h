#ifndef HARDWAREINFOWIDGET_H
#define HARDWAREINFOWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "include/text_label.h"
#include "include/customlabel.h"
#include "include/customtreewidget.h"
#include <QTreeWidget>
#include <QHeaderView>
#include <QStackedWidget>
#include <kborderlessbutton.h>
#include <QPainter>

#include "hwwidget.h"
#include "hardwareinfogetter.h"
#include "hardwareinfo/pcinfo.h"
#include "hardwareinfo/cpuinfo.h"
#include "treeitem.h"
#include "getinfothread.h"
#include "basiccontentwidget.h"

class HardWareInfoWidget : public QWidget {
Q_OBJECT

public:
    explicit HardWareInfoWidget(QWidget *parent = nullptr);

    ~HardWareInfoWidget();

public slots:

    void themeFontStyle();

private:
    QGSettings *m_data = nullptr;

    /*********************     PC basic content      *****************/
    QVBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_pcBasicContentLayout = nullptr;
    BasicContentWidget *m_pcBasicContentWidget = nullptr;
    QLabel *m_pcInfoLabel = nullptr;

    QStringList m_basicExportContent;
    // 电脑基本信息
    QLabel *m_hostPic = nullptr;                // 主机名
    QLabel *m_hostnameLabel = nullptr;
    QHBoxLayout *m_hostnameLayout = nullptr;
    QWidget *m_hostnameWidget = nullptr;
    QLabel *m_hostnameLabel2 = nullptr;
    QHBoxLayout *m_hostnameLayout2 = nullptr;
    QWidget *m_hostnameWidget2 = nullptr;

    QLabel *m_sysVersionPic = nullptr;          // 操作系统版本
    QLabel *m_sysVersionLabel = nullptr;
    QHBoxLayout *m_sysVersionLayout = nullptr;
    QWidget *m_sysVersionWidget = nullptr;
    QLabel *m_sysVersionLabel2 = nullptr;
    QHBoxLayout *m_sysVersionLayout2 = nullptr;
    QWidget *m_sysVersionWidget2 = nullptr;

    QLabel *m_kernelVersionPic = nullptr;       // 内核版本
    QLabel *m_kernelVersionLabel = nullptr;
    QHBoxLayout *m_kernelVersionLayout = nullptr;
    QWidget *m_kernelVersionWidget = nullptr;
    QLabel *m_kernelVersionLabel2 = nullptr;
    QHBoxLayout *m_kernelVersionLayout2 = nullptr;
    QWidget *m_kernelVersionWidget2 = nullptr;

    QLabel *m_archPic = nullptr;                // 架构
    QLabel *m_archLabel = nullptr;
    QHBoxLayout *m_archLayout = nullptr;
    QWidget *m_archWidget = nullptr;
    QLabel *m_archLabel2 = nullptr;
    QHBoxLayout *m_archLayout2 = nullptr;
    QWidget *m_archWidget2 = nullptr;

    QLabel *m_sysBitPic = nullptr;              // 系统位数
    QLabel *m_sysBitLabel = nullptr;
    QHBoxLayout *m_sysBitLayout = nullptr;
    QWidget *m_sysBitWidget = nullptr;
    QLabel *m_sysBitLabel2 = nullptr;
    QHBoxLayout *m_sysBitLayout2 = nullptr;
    QWidget *m_sysBitWidget2 = nullptr;

    QLabel *m_manufactorPic = nullptr;          // 制造商
    QLabel *m_manufactorLabel = nullptr;
    QHBoxLayout *m_manufactorLayout = nullptr;
    QWidget *m_manufactorWidget = nullptr;
    QLabel *m_manufactorLabel2 = nullptr;
    QHBoxLayout *m_manufactorLayout2 = nullptr;
    QWidget *m_manufactorWidget2 = nullptr;

    QLabel *m_modelPic = nullptr;               // 型号
    QLabel *m_modelLabel = nullptr;
    QHBoxLayout *m_modelLayout = nullptr;
    QWidget *m_modelWidget = nullptr;
    QLabel *m_modelLabel2 = nullptr;
    QHBoxLayout *m_modelLayout2 = nullptr;
    QWidget *m_modelWidget2 = nullptr;

    QLabel *m_serialPic = nullptr;              // 序列号
    QLabel *m_serialLabel = nullptr;
    QHBoxLayout *m_serialLayout = nullptr;
    QWidget *m_serialWidget = nullptr;
    QLabel *m_serialLabel2 = nullptr;
    QHBoxLayout *m_serialLayout2 = nullptr;
    QWidget *m_serialWidget2 = nullptr;

    /*********************    Hardware Display    *****************/
    int m_hardwareItems = 0;
    QLabel *m_hardwareDisplayLabel = nullptr;
    QVBoxLayout *m_HWDisplayLayout = nullptr;
    QWidget *m_HWDisplayWidget = nullptr;

    CustomTreeWidget *m_TreeWidget = nullptr;

    // CPU
    HwWidget *m_cpuWidget = nullptr;

    // Memory
    QList<HwWidget *> m_memoryWidgetList;

    // GraphicCard
    QList<HwWidget *> m_graphicCardWidgetList;

    // baseboard
    HwWidget *m_baseboardWidget = nullptr;

    // Network Card
    QList<HwWidget *> m_networkCardWidgetList;

    // Disk
    QList<HwWidget *> m_diskWidgetList;

    // Monitor
    QList<HwWidget *> m_monitorWidgetList;

    // Sound Card
    QList<HwWidget *> m_soundCardWidgetList;

    // KeyBoard
    QList<HwWidget *> m_keyboardWidgetList;

    // Mouse
    QList<HwWidget *> m_mouseWidgetList;

    // BlueTooth
    QList<HwWidget *> m_bluetoothWidgetList;

    // Camera
    QList<HwWidget *> m_cameraWidgetList;

    // CD
    QList<HwWidget *> m_cdWidgetList;

    // Battery
    QList<HwWidget *> m_batteryWidgetList;

    // Fan
    QList<HwWidget *> m_fanWidgetList;

    //用于刷新列表
    QList<DiskInfo> m_oldDiskInfoList;   //用于记录刷新前的DiskInfoList
    QList<MouseInfo> m_oldMouseInfoList;
    QList<KeyboardInfo> m_oldKeyboardInfoList;
    QList<MonitorInfo> m_oldMonitorInfoList;
    QList<BluetoothInfo> m_oldBluetoothInfoList;
    QList<NetworkCardInfo> m_oldNetworkInfoList;
    QList<CDInfo> m_oldCdInfoList;

    QClipboard *m_clipboard = nullptr;

    void initGui();

    void initConnection();

    void setBasicContentUi();    // 设定PC基本信息UI

    void setHWContetnUi();

    void getPcInfo();

    void getCpuInfo();

    void getMemoryInfo();

    void getCDInfo();

    void getCameraInfo();

    void getFanInfo();

    void getBatteryInfo();

    void getGraphicCardInfo();

    void getBaseboardInfo();

    void getNetworkCardInfo();

    void getDiskInfo();

    void getMonitorInfo();

    void getSoundCardInfo();

    void getKeyboardInfo();

    void getMouseInfo();

    void getBluetoothInfo();

private:
    void setIconColor();

    QPixmap drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor);

private slots:

    void setContent();

    void deviceAddSlot(QString devtype);

    void deviceRemoveSlot(QString devtype);

    void monitorChangeSlot();

    void handleMonitorItem();

    void addDiskItem();

    void addMouseItem();

    void addKeyboardItem();

    void addMonitorItem();

    void addBluetoothItem();

    void addNetworkItem();

    void addCdItem();

    void removeDiskItem();

    void removeMouseItem();

    void removeKeyboardItem();

    void removeBluetoothItem();

    void removeNetworkItem();

    void removeCdItem();

    void removeMonitorItem();

    void explorBasicContent();

    void removeDisableBluetootItem();
};

#endif // HARDWAREINFOWIDGET_H

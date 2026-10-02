#ifndef HARDWAREINFOWIDGET_H
#define HARDWAREINFOWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QHeaderView>
#include <QStackedWidget>
#include <kysdk/applications/kborderlessbutton.h>

#include "hardwareinfogetter.h"
#include "hardwareinfo/pcinfo.h"
#include "hardwareinfo/cpuinfo.h"

#include "treeitem.h"

class HardWareInfoWidget : public QWidget {
Q_OBJECT

public:
    explicit HardWareInfoWidget(QWidget *parent = nullptr);

    ~HardWareInfoWidget();

public slots:

    void themeFontStyle();

private:
    /*********************     PC basic content      *****************/
    QVBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_pcBasicContentLayout = nullptr;
    QWidget *m_pcBasicContentWidget = nullptr;
    QLabel *m_pcInfoLabel = nullptr;

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
    int m_hardwareItems = 11;
    QLabel *m_hardwareDisplayLabel = nullptr;
    QVBoxLayout *m_HWDisplayLayout = nullptr;
    QWidget *m_HWDisplayWidget = nullptr;

    // CPU
    int cpuItemCount = 0;
    QTreeWidget *m_TreeWidget = nullptr;
    QLabel *m_cpuIconLabel = nullptr;
    QLabel *m_cpuTextLabel = nullptr;
    QLabel *m_cpuModelLabel = nullptr;
    QStackedWidget *m_cpuBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_cpuSpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_cpuSpreadInBtn = nullptr;
    QHBoxLayout *m_cpuLayout = nullptr;
    QWidget *m_cpuWidget = nullptr;
    QTimer *m_timer = nullptr;

    // Memory
    int memoryItemCount = 0;
    QLabel *m_memoryIconLabel = nullptr;
    QLabel *m_memoryTextLabel = nullptr;
    QList<TreeItem *> m_memoryTreeItemList;
    QStackedWidget *m_memoryBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_memorySpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_memorySpreadInBtn = nullptr;
    QHBoxLayout *m_memoryLayout = nullptr;
    QWidget *m_memoryWidget = nullptr;
    QTimer *m_timer2 = nullptr;

    // GraphicCard
    int graphicCardItemCount = 0;
    QLabel *m_graphicCardIconLabel = nullptr;
    QLabel *m_graphicCardTextLabel = nullptr;
    QList<TreeItem *> m_graphicCardTreeItemList;
    QStackedWidget *m_graphicCardBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_graphicCardSpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_graphicCardSpreadInBtn = nullptr;
    QHBoxLayout *m_graphicCardLayout = nullptr;
    QWidget *m_graphicCardWidget = nullptr;

    // baseboard
    int baseboardItemCount = 0;
    QLabel *m_baseboardIconLabel = nullptr;
    QLabel *m_baseboardTextLabel = nullptr;
    QLabel *m_baseboardModelLabel = nullptr;
    QStackedWidget *m_baseboardBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_baseboardSpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_baseboardSpreadInBtn = nullptr;
    QHBoxLayout *m_baseboardLayout = nullptr;
    QWidget *m_baseboardWidget = nullptr;

    // Network Card
    int networkItemCount = 4;
    QLabel *m_networkCardIconLabel = nullptr;
    QLabel *m_networkCardTextLabel = nullptr;
    QList<TreeItem *> m_networkCardTreeItemList;
    QStackedWidget *m_networkCardBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_networkCardSpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_networkCardSpreadInBtn = nullptr;
    QHBoxLayout *m_networkCardLayout = nullptr;
    QWidget *m_networkCardWidget = nullptr;

    // Disk
    int diskItemCount = 0;
    QLabel *m_diskIconLabel = nullptr;
    QLabel *m_diskTextLabel = nullptr;
    QList<TreeItem *> m_diskTreeItemList;
    QStackedWidget *m_diskBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_diskSpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_diskSpreadInBtn = nullptr;
    QHBoxLayout *m_diskLayout = nullptr;
    QWidget *m_diskWidget = nullptr;

    // Monitor
    int monitorItemCount = 0;
    QLabel *m_monitorIconLabel = nullptr;
    QLabel *m_monitorTextLabel = nullptr;
    QList<TreeItem *> m_monitorTreeItemList;
    QStackedWidget *m_monitorBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_monitorSpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_monitorSpreadInBtn = nullptr;
    QHBoxLayout *m_monitorLayout = nullptr;
    QWidget *m_monitorWidget = nullptr;

    // Sound Card
    int soundCardItemCount = 0;
    QLabel *m_soundCardIconLabel = nullptr;
    QLabel *m_soundCardTextLabel = nullptr;
    QLabel *m_soundCardModelLabel = nullptr;
    QList<TreeItem *> m_soundCardTreeItemList;
    QStackedWidget *m_soundCardBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_soundCardSpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_soundCardSpreadInBtn = nullptr;
    QHBoxLayout *m_soundCardLayout = nullptr;
    QWidget *m_soundCardWidget = nullptr;

    // KeyBoard
    int keyboardItemCount = 0;
    QLabel *m_keyboardIconLabel = nullptr;
    QLabel *m_keyboardTextLabel = nullptr;
    QList<TreeItem *> m_keyboardTreeItemList;
    QStackedWidget *m_keyboardBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_keyboardSpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_keyboardSpreadInBtn = nullptr;
    QHBoxLayout *m_keyboardLayout = nullptr;
    QWidget *m_keyboardWidget = nullptr;

    // Mouse
    int mouseItemCount = 0;
    QLabel *m_mouseIconLabel = nullptr;
    QLabel *m_mouseTextLabel = nullptr;
    QList<TreeItem *> m_mouseTreeItemList;
    QStackedWidget *m_mouseBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_mouseSpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_mouseSpreadInBtn = nullptr;
    QHBoxLayout *m_mouseLayout = nullptr;
    QWidget *m_mouseWidget = nullptr;

    // BlueTooth
    int bluetoothItemCount = 0;
    QLabel *m_bluetoothIconLabel = nullptr;
    QLabel *m_bluetoothTextLabel = nullptr;
    QLabel *m_bluetoothModelLabel = nullptr;
    QList<TreeItem *> m_bluetoothTreeItemList;
    QStackedWidget *m_bluetoothBtnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_bluetoothSpreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_bluetoothSpreadInBtn = nullptr;
    QHBoxLayout *m_bluetoothLayout = nullptr;
    QWidget *m_bluetoothWidget = nullptr;

    void initGui();

    void initConnection();

    void setBasicContentUi();    // 设定PC基本信息UI

    void setHWContetnUi();

    void getPcInfo();

    void getCpuInfo();

    void getMemoryInfo();

    void getGraphicCardInfo();

    void getBaseboardInfo();

    void getNetworkCardInfo();

    void getDiskCardInfo();

    void getMonitorInfo();

    void getSoundCardInfo();

    void getKeyboardInfo();

    void getMouseInfo();

    void getBluetoothInfo();

private slots:

    void setContent();

    void setMemoryContent(int i);

    void setGraphicContent(int i);

    void setNetworkCardContent(int i);

    void setDiskContent(int i);

    void setMonitorContent(int i);

    void setKeyboardContent(int i);

    void setMouseContent(int i);

    void refreshCPUInfo(TreeItem *item11, TreeItem *item12, TreeItem *item13);
};

#endif // HARDWAREINFOWIDGET_H

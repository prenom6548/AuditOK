#include "hardwareinfowidget.h"
#include <QDebug>
#include <QIcon>
#include <gsettingmonitor.h>
#include <QAbstractItemView>

#include "include/common.h"
#include "include/globalsignal.h"
#include "treeitem.h"
#include "kom-configure.h"

HardWareInfoWidget::HardWareInfoWidget(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(new QVBoxLayout()),
      m_pcBasicContentLayout(new QVBoxLayout()),
      m_pcBasicContentWidget(new BasicContentWidget),
      m_pcInfoLabel(new QLabel()),
      m_hostPic(new QLabel()),
      m_hostnameLabel(new K::TextLabel),
      m_hostnameLayout(new QHBoxLayout),
      m_hostnameWidget(new QWidget),
      m_hostnameLabel2(new QLabel()),
      m_hostnameLayout2(new QHBoxLayout),
      m_hostnameWidget2(new QWidget),
      m_sysVersionPic(new QLabel()),
      m_sysVersionLabel(new K::TextLabel),
      m_sysVersionLayout(new QHBoxLayout),
      m_sysVersionWidget(new QWidget),
      m_sysVersionLabel2(new QLabel()),
      m_sysVersionLayout2(new QHBoxLayout),
      m_sysVersionWidget2(new QWidget),
      m_sysYearVersionPic(new QLabel()),
      m_sysYearVersionLabel(new K::TextLabel),
      m_sysYearVersionLayout(new QHBoxLayout),
      m_sysYearVersionWidget(new QWidget),
      m_sysYearVersionLabel2(new QLabel()),
      m_sysYearVersionLayout2(new QHBoxLayout),
      m_sysYearVersionWidget2(new QWidget),
      m_kernelVersionPic(new QLabel()),
      m_kernelVersionLabel(new K::TextLabel()),
      m_kernelVersionLayout(new QHBoxLayout),
      m_kernelVersionWidget(new QWidget),
      m_kernelVersionLabel2(new QLabel()),
      m_kernelVersionLayout2(new QHBoxLayout),
      m_kernelVersionWidget2(new QWidget),
      m_archPic(new QLabel()),
      m_archLabel(new K::TextLabel),
      m_archLayout(new QHBoxLayout),
      m_archWidget(new QWidget),
      m_archLabel2(new QLabel()),
      m_archLayout2(new QHBoxLayout),
      m_archWidget2(new QWidget),
      m_sysBitPic(new QLabel()),
      m_sysBitLabel(new K::TextLabel()),
      m_sysBitLayout(new QHBoxLayout),
      m_sysBitWidget(new QWidget),
      m_sysBitLabel2(new QLabel()),
      m_sysBitLayout2(new QHBoxLayout),
      m_sysBitWidget2(new QWidget),
      m_manufactorPic(new QLabel()),
      m_manufactorLabel(new K::TextLabel()),
      m_manufactorLayout(new QHBoxLayout),
      m_manufactorWidget(new QWidget),
      m_manufactorLabel2(new QLabel()),
      m_manufactorLayout2(new QHBoxLayout),
      m_manufactorWidget2(new QWidget),
      m_versionPic(new QLabel()),
      m_versionLabel(new K::TextLabel()),
      m_versionLayout(new QHBoxLayout),
      m_versionWidget(new QWidget),
      m_versionLabel2(new QLabel()),
      m_versionLayout2(new QHBoxLayout),
      m_versionWidget2(new QWidget),
      m_modelPic(new QLabel()),
      m_modelLabel(new K::TextLabel()),
      m_modelLayout(new QHBoxLayout),
      m_modelWidget(new QWidget),
      m_modelLabel2(new QLabel()),
      m_modelLayout2(new QHBoxLayout),
      m_modelWidget2(new QWidget),
      m_serialPic(new QLabel()),
      m_serialLabel(new K::TextLabel()),
      m_serialLayout(new QHBoxLayout),
      m_serialWidget(new QWidget),
      m_serialLabel2(new QLabel()),
      m_serialLayout2(new QHBoxLayout),
      m_serialWidget2(new QWidget),
      m_hardwareDisplayLabel(new QLabel),
      m_HWDisplayLayout(new QVBoxLayout),
      m_HWDisplayWidget(new QWidget),
      m_TreeWidget(new CustomTreeWidget) {
    if (QGSettings::isSchemaInstalled(KYLIN_OS_MANAGER)) {
        m_data = new QGSettings(KYLIN_OS_MANAGER);
    }
    initGui();
    initConnection();
}

HardWareInfoWidget::~HardWareInfoWidget() {

}

void HardWareInfoWidget::initGui() {
    themeFontStyle();        // 设置m_picInfoLabel的字体
    setBasicContentUi();
    setHWContetnUi();

    m_mainLayout->setContentsMargins(40, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_pcInfoLabel);
    m_mainLayout->addWidget(m_pcBasicContentWidget);
    m_mainLayout->addSpacing(30);
    m_mainLayout->addWidget(m_hardwareDisplayLabel);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_HWDisplayWidget);
    m_mainLayout->addStretch();
    this->setLayout(m_mainLayout);
}

void HardWareInfoWidget::setBasicContentUi() {
    m_pcInfoLabel->setText(tr("Computer Basic Information"));

    m_hostPic->setPixmap(QIcon::fromTheme("ukui-zs-host-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_hostPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_hostnameLabel->SetText(tr("Host Name"));

    m_sysVersionPic->setPixmap(getIcon("ukui-zs-neicun-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_sysVersionPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_sysVersionLabel->SetText(tr("Operation System Name"));

    m_sysYearVersionPic->setPixmap(getIcon("ukui-zs-machine-model-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_sysYearVersionPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_sysYearVersionLabel->SetText(tr("Operation System Version"));

    m_kernelVersionPic->setPixmap(getIcon("ukui-zs-kernel-version-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_kernelVersionPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_kernelVersionLabel->SetText(tr("Kernel Version"));

    m_archPic->setPixmap(getIcon("ukui-zs-kernel-architecture-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_archPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_archLabel->SetText(tr("Kernel Architecture"));

    m_sysBitPic->setPixmap(getIcon("ukui-zs-system-bits-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_sysBitPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_sysBitLabel->SetText(tr("System Bits"));

    m_manufactorPic->setPixmap(getIcon("ukui-zs-manufacturer-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_manufactorPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_manufactorLabel->SetText(tr("Manufacturer"));

    m_versionPic->setPixmap(getIcon("ukui-miracast-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_versionPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_versionLabel->SetText(tr("Version"));

    m_modelPic->setPixmap(getIcon("ukui-zs-machine-model-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_modelPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_modelLabel->SetText(tr("Model"));

    m_serialPic->setPixmap(getIcon("ukui-zs-sn-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_serialPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_serialLabel->SetText(tr("Serial"));

    // 布局管理
    m_hostnameLayout->addWidget(m_hostPic);
    m_hostnameLayout->addSpacing(12);
    m_hostnameLayout->addWidget(m_hostnameLabel);
    m_hostnameLayout->setContentsMargins(0, 0, 0, 0);
    m_hostnameLayout->setSpacing(0);
    m_hostnameLayout->setAlignment(Qt::AlignLeft);
    m_hostnameWidget->setLayout(m_hostnameLayout);
    m_hostnameWidget->setFixedWidth(150);
    m_hostnameLayout2->addWidget(m_hostnameWidget);
    m_hostnameLayout2->addSpacing(24);
    m_hostnameLabel2->setFixedHeight(26);
    m_hostnameLayout2->addWidget(m_hostnameLabel2, Qt::AlignLeft);
    m_hostnameLayout2->addStretch();
    m_hostnameLayout2->setContentsMargins(0, 0, 0, 0);
    m_hostnameLayout2->setSpacing(0);
    m_hostnameLayout2->setAlignment(Qt::AlignLeft);
    m_hostnameWidget2->setLayout(m_hostnameLayout2);

    m_sysVersionLayout->addWidget(m_sysVersionPic);
    m_sysVersionLayout->addSpacing(12);
    m_sysVersionLayout->addWidget(m_sysVersionLabel);
    m_sysVersionLayout->setContentsMargins(0, 0, 0, 0);
    m_sysVersionLayout->setSpacing(0);
    m_sysVersionLayout->setAlignment(Qt::AlignLeft);
    m_sysVersionWidget->setLayout(m_sysVersionLayout);
    m_sysVersionWidget->setFixedWidth(150);
    m_sysVersionLayout2->addWidget(m_sysVersionWidget);
    m_sysVersionLayout2->addSpacing(24);
    m_sysVersionLabel2->setFixedHeight(26);
    m_sysVersionLayout2->addWidget(m_sysVersionLabel2, Qt::AlignLeft);
    m_sysVersionLayout2->addStretch();
    m_sysVersionLayout2->setContentsMargins(0, 0, 0, 0);
    m_sysVersionLayout2->setSpacing(0);
    m_sysVersionLayout2->setAlignment(Qt::AlignLeft);
    m_sysVersionWidget2->setLayout(m_sysVersionLayout2);

    m_sysYearVersionLayout->addWidget(m_sysYearVersionPic);
    m_sysYearVersionLayout->addSpacing(12);
    m_sysYearVersionLayout->addWidget(m_sysYearVersionLabel);
    m_sysYearVersionLayout->setContentsMargins(0, 0, 0, 0);
    m_sysYearVersionLayout->setSpacing(0);
    m_sysYearVersionLayout->setAlignment(Qt::AlignLeft);
    m_sysYearVersionWidget->setLayout(m_sysYearVersionLayout);
    m_sysYearVersionWidget->setFixedWidth(150);
    m_sysYearVersionLayout2->addWidget(m_sysYearVersionWidget);
    m_sysYearVersionLayout2->addSpacing(24);
    m_sysYearVersionLabel2->setFixedHeight(26);
    m_sysYearVersionLayout2->addWidget(m_sysYearVersionLabel2, Qt::AlignLeft);
    m_sysYearVersionLayout2->addStretch();
    m_sysYearVersionLayout2->setContentsMargins(0, 0, 0, 0);
    m_sysYearVersionLayout2->setSpacing(0);
    m_sysYearVersionLayout2->setAlignment(Qt::AlignLeft);
    m_sysYearVersionWidget2->setLayout(m_sysYearVersionLayout2);

    m_kernelVersionLayout->addWidget(m_kernelVersionPic);
    m_kernelVersionLayout->addSpacing(12);
    m_kernelVersionLayout->addWidget(m_kernelVersionLabel);
    m_kernelVersionLayout->setContentsMargins(0, 0, 0, 0);
    m_kernelVersionLayout->setSpacing(0);
    m_kernelVersionLayout->setAlignment(Qt::AlignLeft);
    m_kernelVersionWidget->setLayout(m_kernelVersionLayout);
    m_kernelVersionWidget->setFixedWidth(150);
    m_kernelVersionLayout2->addWidget(m_kernelVersionWidget);
    m_kernelVersionLayout2->addSpacing(24);
    m_kernelVersionLabel2->setFixedHeight(26);
    m_kernelVersionLayout2->addWidget(m_kernelVersionLabel2, Qt::AlignLeft);
    m_kernelVersionLayout2->addStretch();
    m_kernelVersionLayout2->setContentsMargins(0, 0, 0, 0);
    m_kernelVersionLayout2->setSpacing(0);
    m_kernelVersionLayout2->setAlignment(Qt::AlignLeft);
    m_kernelVersionWidget2->setLayout(m_kernelVersionLayout2);

    m_archLayout->addWidget(m_archPic);
    m_archLayout->addSpacing(12);
    m_archLayout->addWidget(m_archLabel);
    m_archLayout->setContentsMargins(0, 0, 0, 0);
    m_archLayout->setSpacing(0);
    m_archLayout->setAlignment(Qt::AlignLeft);
    m_archWidget->setLayout(m_archLayout);
    m_archWidget->setFixedWidth(150);
    m_archLayout2->addWidget(m_archWidget);
    m_archLayout2->addSpacing(24);
    m_archLabel2->setFixedHeight(26);
    m_archLayout2->addWidget(m_archLabel2, Qt::AlignLeft);
    m_archLayout2->addStretch();
    m_archLayout2->setContentsMargins(0, 0, 0, 0);
    m_archLayout2->setSpacing(0);
    m_archLayout2->setAlignment(Qt::AlignLeft);
    m_archWidget2->setLayout(m_archLayout2);

    m_sysBitLayout->addWidget(m_sysBitPic);
    m_sysBitLayout->addSpacing(12);
    m_sysBitLayout->addWidget(m_sysBitLabel);
    m_sysBitLayout->setContentsMargins(0, 0, 0, 0);
    m_sysBitLayout->setSpacing(0);
    m_sysBitLayout->setAlignment(Qt::AlignLeft);
    m_sysBitWidget->setLayout(m_sysBitLayout);
    m_sysBitWidget->setFixedWidth(150);
    m_sysBitLayout2->addWidget(m_sysBitWidget);
    m_sysBitLayout2->addSpacing(24);
    m_sysBitLabel2->setFixedHeight(26);
    m_sysBitLayout2->addWidget(m_sysBitLabel2, Qt::AlignLeft);
    m_sysBitLayout2->addStretch();
    m_sysBitLayout2->setContentsMargins(0, 0, 0, 0);
    m_sysBitLayout2->setSpacing(0);
    m_sysBitLayout2->setAlignment(Qt::AlignLeft);
    m_sysBitWidget2->setLayout(m_sysBitLayout2);

    m_manufactorLayout->addWidget(m_manufactorPic);
    m_manufactorLayout->addSpacing(12);
    m_manufactorLayout->addWidget(m_manufactorLabel);
    m_manufactorLayout->setContentsMargins(0, 0, 0, 0);
    m_manufactorLayout->setSpacing(0);
    m_manufactorLayout->setAlignment(Qt::AlignLeft);
    m_manufactorWidget->setLayout(m_manufactorLayout);
    m_manufactorWidget->setFixedWidth(150);
    m_manufactorLayout2->addWidget(m_manufactorWidget);
    m_manufactorLayout2->addSpacing(24);
    m_manufactorLabel2->setFixedHeight(26);
    m_manufactorLayout2->addWidget(m_manufactorLabel2, Qt::AlignLeft);
    m_manufactorLayout2->addStretch();
    m_manufactorLayout2->setContentsMargins(0, 0, 0, 0);
    m_manufactorLayout2->setSpacing(0);
    m_manufactorLayout2->setAlignment(Qt::AlignLeft);
    m_manufactorWidget2->setLayout(m_manufactorLayout2);

    m_versionLayout->addWidget(m_versionPic);
    m_versionLayout->addSpacing(12);
    m_versionLayout->addWidget(m_versionLabel);
    m_versionLayout->setContentsMargins(0, 0, 0, 0);
    m_versionLayout->setSpacing(0);
    m_versionLayout->setAlignment(Qt::AlignLeft);
    m_versionWidget->setLayout(m_versionLayout);
    m_versionWidget->setFixedWidth(150);
    m_versionLayout2->addWidget(m_versionWidget);
    m_versionLayout2->addSpacing(24);
    m_versionLabel2->setFixedHeight(26);
    m_versionLayout2->addWidget(m_versionLabel2, Qt::AlignLeft);
    m_versionLayout2->addStretch();
    m_versionLayout2->setContentsMargins(0, 0, 0, 0);
    m_versionLayout2->setSpacing(0);
    m_versionLayout2->setAlignment(Qt::AlignLeft);
    m_versionWidget2->setLayout(m_versionLayout2);

    m_modelLayout->addWidget(m_modelPic);
    m_modelLayout->addSpacing(12);
    m_modelLayout->addWidget(m_modelLabel);
    m_modelLayout->setContentsMargins(0, 0, 0, 0);
    m_modelLayout->setSpacing(0);
    m_modelLayout->setAlignment(Qt::AlignLeft);
    m_modelWidget->setLayout(m_modelLayout);
    m_modelWidget->setFixedWidth(150);
    m_modelLayout2->addWidget(m_modelWidget);
    m_modelLayout2->addSpacing(24);
    m_modelLabel2->setFixedHeight(26);
    m_modelLayout2->addWidget(m_modelLabel2, Qt::AlignLeft);
    m_modelLayout2->addStretch();
    m_modelLayout2->setContentsMargins(0, 0, 0, 0);
    m_modelLayout2->setSpacing(0);
    m_modelLayout2->setAlignment(Qt::AlignLeft);
    m_modelWidget2->setLayout(m_modelLayout2);

    m_serialLayout->addWidget(m_serialPic);
    m_serialLayout->addSpacing(12);
    m_serialLayout->addWidget(m_serialLabel);
    m_serialLayout->setContentsMargins(0, 0, 0, 0);
    m_serialLayout->setSpacing(0);
    m_serialLayout->setAlignment(Qt::AlignLeft);
    m_serialWidget->setLayout(m_serialLayout);
    m_serialWidget->setFixedWidth(150);
    m_serialLayout2->addWidget(m_serialWidget);
    m_serialLayout2->addSpacing(24);
    m_serialLabel2->setFixedHeight(26);
    m_serialLayout2->addWidget(m_serialLabel2, Qt::AlignLeft);
    m_serialLayout2->addStretch();
    m_serialLayout2->setContentsMargins(0, 0, 0, 0);
    m_serialLayout2->setSpacing(0);
    m_serialLayout2->setAlignment(Qt::AlignLeft);
    m_serialWidget2->setLayout(m_serialLayout2);

    m_pcBasicContentLayout->addWidget(m_pcInfoLabel);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_hostnameWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_sysVersionWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_sysYearVersionWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_kernelVersionWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_archWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_sysBitWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_manufactorWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_versionWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_modelWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_serialWidget2);
    m_pcBasicContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pcBasicContentLayout->setSpacing(0);
    m_pcBasicContentLayout->setAlignment(Qt::AlignLeft);

    m_pcBasicContentWidget->setLayout(m_pcBasicContentLayout);

    setIconColor();
}

void HardWareInfoWidget::setHWContetnUi() {
    m_hardwareDisplayLabel->setText(tr("Hardware Display"));

    m_TreeWidget->header()->setVisible(false);
    m_TreeWidget->setIndentation(0);
    m_TreeWidget->setFrameShape(QTreeWidget::NoFrame);                   //去掉list的边框
    m_TreeWidget->setStyleSheet(".QTreeWidget::item:hover{background:transparent;} QTreeWidget { margin: 0px; }");
    m_TreeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_TreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_TreeWidget->setSelectionMode(QAbstractItemView::NoSelection); // list取消原装的选中功能
    m_TreeWidget->setContentsMargins(0, 0, 0, 0);
    m_TreeWidget->setColumnCount(1);

    // 主体布局
    m_HWDisplayLayout->setContentsMargins(0, 0, 40, 0);
    m_HWDisplayLayout->setSpacing(0);
    m_HWDisplayLayout->addWidget(m_TreeWidget);
    m_HWDisplayWidget->setLayout(m_HWDisplayLayout);
}

void HardWareInfoWidget::setContent() {
    getPcInfo();
    getCpuInfo();
    getMemoryInfo();
    getGraphicCardInfo();
    getBaseboardInfo();
    getNetworkCardInfo();
    getDiskInfo();
    getMonitorInfo();
    getSoundCardInfo();
    getKeyboardInfo();
    getMouseInfo();
    getCDInfo();
    getCameraInfo();
    getFanInfo();
    getBatteryInfo();
    getBluetoothInfo();

    m_TreeWidget->setFixedHeight(m_hardwareItems * TREEWIDGETITEMSIZEHINT.height());
}

void HardWareInfoWidget::initConnection() {
    // gsettings
    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &HardWareInfoWidget::themeFontStyle);
    // 信息获取成功后的信息填充
    connect(g_signal, &GlobalSignal::getHwInfoComplete, this, &HardWareInfoWidget::setContent);
    // treewidget大小变化
    connect(g_signal, &GlobalSignal::expandTreeWidget, [=](int itemCount) {
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (itemCount * TREEWIDGETITEMSIZEHINT.height()));
    });
    connect(g_signal, &GlobalSignal::deviceInserted, this, &HardWareInfoWidget::deviceAddSlot);
    connect(g_signal, &GlobalSignal::deviceRemoved, this, &HardWareInfoWidget::deviceRemoveSlot);
    connect(g_signal, &GlobalSignal::monitorChanged, this, &HardWareInfoWidget::monitorChangeSlot);

    // 禁用启用蓝牙
    connect(g_signal, &GlobalSignal::disableBluetoothSignal, this, &HardWareInfoWidget::disableBluetoothItem);
    connect(g_signal, &GlobalSignal::enableBluetoothSignal, this, &HardWareInfoWidget::enableBluetoothItem);

    // 禁用启用网络
    connect(g_signal, &GlobalSignal::enableWiredNetworkCardSignal, this, &HardWareInfoWidget::enbaleWiredNetworkCardItem);
    connect(g_signal, &GlobalSignal::enableWirelessNetworkCardSignal, this, &HardWareInfoWidget::enableWirelessNetworkCardItem);
}

void HardWareInfoWidget::themeFontStyle() {
    setIconColor();
    float fontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    int size = 18 * fontSize / 15;
    QFont font;
    font.setPointSize(size);
    font.setBold(true);
    m_pcInfoLabel->setFont(font);
    m_hardwareDisplayLabel->setFont(font);

}

void HardWareInfoWidget::getPcInfo() {
    // 设置内容
    m_hostnameLabel2->setText(g_infogetter->m_pcinfo.hostname);
    m_sysVersionLabel2->setText(g_infogetter->m_pcinfo.sysversion);
    m_sysYearVersionLabel2->setText(g_infogetter->m_pcinfo.yearversion);
    m_kernelVersionLabel2->setText(g_infogetter->m_pcinfo.kernelversion);
    m_archLabel2->setText(g_infogetter->m_pcinfo.arch);
    m_sysBitLabel2->setText(g_infogetter->m_pcinfo.sysbits);
    m_manufactorLabel2->setText(g_infogetter->m_pcinfo.manufacturer);
    m_versionLabel2->setText(g_infogetter->m_pcinfo.version);
    m_modelLabel2->setText(g_infogetter->m_pcinfo.model);
    m_serialLabel2->setText(g_infogetter->m_pcinfo.serial);

    m_basicExportContent.append(tr("Host Name") + "\t" + g_infogetter->m_pcinfo.hostname);
    m_basicExportContent.append(tr("Operation System Version") + "\t" + g_infogetter->m_pcinfo.sysversion);
    m_basicExportContent.append(tr("Kernel Version") + "\t" + g_infogetter->m_pcinfo.kernelversion);
    m_basicExportContent.append(tr("Kernel Architecture") + "\t" + g_infogetter->m_pcinfo.arch);
    m_basicExportContent.append(tr("System Bits") + "\t" + g_infogetter->m_pcinfo.sysbits);
    m_basicExportContent.append(tr("Manufacturer") + "\t" + g_infogetter->m_pcinfo.manufacturer);
    m_basicExportContent.append(tr("Model") + "\t" + g_infogetter->m_pcinfo.model);
    m_basicExportContent.append(tr("Serial") + "\t" + g_infogetter->m_pcinfo.serial);

    connect(g_signal, &GlobalSignal::exportBasicContentSignal, this, &HardWareInfoWidget::explorBasicContent);

}

void HardWareInfoWidget::getCpuInfo() {
    if (g_infogetter->m_cpuinfo.length() == 0) {
        return;
    }
    m_cpuWidget = new HwWidget(&(g_infogetter->m_cpuinfo[0]));
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
    rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem);
    m_TreeWidget->setItemWidget(rootItem, 0, m_cpuWidget);

    for (int i = 0; i < m_cpuWidget->m_treeItemList.count(); ++i) {
        m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_cpuWidget->m_treeWidgetItemList.at(i));
        m_TreeWidget->setItemWidget(m_cpuWidget->m_treeWidgetItemList.at(i), 0, m_cpuWidget->m_treeItemList.at(i));
    }
    m_hardwareItems++;
}


void HardWareInfoWidget::getMemoryInfo() {
    for (int j = 0; j < g_infogetter->m_memoryinfoList.length(); ++j) {
        HwWidget *m_memoryWidget = new HwWidget(&(g_infogetter->m_memoryinfoList[j]));
        m_memoryWidget->setAutoFillBackground(true);
        m_memoryWidgetList.append(m_memoryWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_memoryWidget);
        for (int i = 0; i < m_memoryWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_memoryWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_memoryWidget->m_treeWidgetItemList.at(i), 0,
                                        m_memoryWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_MEMORY).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString,QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()){
            continue;
        }
        HwWidget *m_memoryWidget = new HwWidget("Memory", infoMap);
        m_memoryWidget->setAutoFillBackground(true);
        m_memoryWidgetList.append(m_memoryWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_memoryWidget);
        for (int i = 0; i < m_memoryWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_memoryWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_memoryWidget->m_treeWidgetItemList.at(i), 0,
                                        m_memoryWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

}

void HardWareInfoWidget::getGraphicCardInfo() {
    for (int j = 0; j < g_infogetter->m_graphicHardwares.length(); ++j) {
        HwWidget *m_graphicCardWidget = new HwWidget(&(g_infogetter->m_graphicHardwares[j]));
        m_graphicCardWidget->setAutoFillBackground(true);
        m_graphicCardWidgetList.append(m_graphicCardWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_graphicCardWidget);
        for (int i = 0; i < m_graphicCardWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_graphicCardWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_graphicCardWidget->m_treeWidgetItemList.at(i), 0,
                                        m_graphicCardWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_GRAPHICCARD).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString,QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()){
            continue;
        }
        HwWidget *m_graphicCardWidget = new HwWidget("GraphicCard", infoMap);
        m_graphicCardWidget->setAutoFillBackground(true);
        m_graphicCardWidgetList.append(m_graphicCardWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_graphicCardWidget);
        for (int i = 0; i < m_graphicCardWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_graphicCardWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_graphicCardWidget->m_treeWidgetItemList.at(i), 0,
                                        m_graphicCardWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }
}

void HardWareInfoWidget::getBaseboardInfo() {
    if (g_infogetter->m_baseboardinfo.length() == 0) {
        return;
    }
    m_baseboardWidget = new HwWidget(&(g_infogetter->m_baseboardinfo[0]));
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
    rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem);
    m_TreeWidget->setItemWidget(rootItem, 0, m_baseboardWidget);
    for (int i = 0; i < m_baseboardWidget->m_treeItemList.count(); ++i) {
        m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_baseboardWidget->m_treeWidgetItemList.at(i));
        m_TreeWidget->setItemWidget(m_baseboardWidget->m_treeWidgetItemList.at(i), 0,
                                    m_baseboardWidget->m_treeItemList.at(i));
    }
    m_hardwareItems++;
}

void HardWareInfoWidget::getNetworkCardInfo() {
    for (int j = 0; j < g_infogetter->m_networkCardHardwares.length(); ++j) {
        HwWidget *m_networkCardWidget = new HwWidget(&(g_infogetter->m_networkCardHardwares[j]));
        m_networkCardWidget->setAutoFillBackground(true);
        m_networkCardWidgetList.append(m_networkCardWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_networkCardWidget);
        for (int i = 0; i < m_networkCardWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_networkCardWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_networkCardWidget->m_treeWidgetItemList.at(i), 0,
                                        m_networkCardWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_NETWORKCARD).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString,QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()){
            continue;
        }
        HwWidget *m_netWorkCardWidget = new HwWidget("NetworkCard", infoMap);
        m_netWorkCardWidget->setAutoFillBackground(true);
        m_networkCardWidgetList.append(m_netWorkCardWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_netWorkCardWidget);
        for (int i = 0; i < m_netWorkCardWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_netWorkCardWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_netWorkCardWidget->m_treeWidgetItemList.at(i), 0,
                                        m_netWorkCardWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }
}

void HardWareInfoWidget::getDiskInfo() {
    for (int j = 0; j < g_infogetter->m_diskinfoList.length(); ++j) {
        HwWidget *m_diskWidget = new HwWidget(&(g_infogetter->m_diskinfoList[j]));
        m_diskWidget->setAutoFillBackground(true);
        m_diskWidgetList.append(m_diskWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_diskWidget);
        for (int i = 0; i < m_diskWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_diskWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_diskWidget->m_treeWidgetItemList.at(i), 0,
                                        m_diskWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_DISK).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString,QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()){
            continue;
        }
        HwWidget *m_diskWidget = new HwWidget("Disk", infoMap);
        m_diskWidgetList.append(m_diskWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_diskWidget);
        for (int i = 0; i < m_diskWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_diskWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_diskWidget->m_treeWidgetItemList.at(i), 0,
                                        m_diskWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

}

void HardWareInfoWidget::getMonitorInfo() {
    for (int j = 0; j < g_infogetter->m_monitorList.length(); ++j) {
        HwWidget *m_monitorWidget = new HwWidget(&(g_infogetter->m_monitorList[j]));
        m_monitorWidget->setAutoFillBackground(true);
        m_monitorWidgetList.append(m_monitorWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_monitorWidget);
        for (int i = 0; i < m_monitorWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_monitorWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_monitorWidget->m_treeWidgetItemList.at(i), 0,
                                        m_monitorWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_MONITOR).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString,QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()){
            continue;
        }
        HwWidget *m_monitorWidget = new HwWidget("Monitor", infoMap);
        m_monitorWidgetList.append(m_monitorWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_monitorWidget);
        for (int i = 0; i < m_monitorWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_monitorWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_monitorWidget->m_treeWidgetItemList.at(i), 0,
                                        m_monitorWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }
}

void HardWareInfoWidget::getSoundCardInfo() {
    for (int j = 0; j < g_infogetter->m_soundCardHardwares.length(); ++j) {
        HwWidget *m_soundCardWidget = new HwWidget(&(g_infogetter->m_soundCardHardwares[j]));
        m_soundCardWidget->setAutoFillBackground(true);
        m_soundCardWidgetList.append(m_soundCardWidget);

        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_soundCardWidget);
        for (int i = 0; i < m_soundCardWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_soundCardWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_soundCardWidget->m_treeWidgetItemList.at(i), 0,
                                        m_soundCardWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_SOUNDCARD).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString, QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()) {
            continue;
        }
        HwWidget *m_soundCardWidget = new HwWidget("SoundCard", infoMap);
        m_soundCardWidgetList.append(m_soundCardWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_soundCardWidget);
        for (int i = 0; i < m_soundCardWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_soundCardWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_soundCardWidget->m_treeWidgetItemList.at(i), 0,
                                        m_soundCardWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }
}

void HardWareInfoWidget::getKeyboardInfo() {
    for (int j = 0; j < g_infogetter->m_keyboardInfoList.length(); ++j) {
        HwWidget *m_keyboardWidget = new HwWidget(&(g_infogetter->m_keyboardInfoList[j]));
        m_keyboardWidget->setAutoFillBackground(true);
        m_keyboardWidgetList.append(m_keyboardWidget);

        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_keyboardWidget);
        for (int i = 0; i < m_keyboardWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_keyboardWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_keyboardWidget->m_treeWidgetItemList.at(i), 0,
                                        m_keyboardWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_KEYBOARD).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString, QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()) {
            continue;
        }
        HwWidget *m_keyBoardWidget = new HwWidget("Keyboard", infoMap);
        m_keyBoardWidget->setAutoFillBackground(true);
        m_keyboardWidgetList.append(m_keyBoardWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_keyBoardWidget);
        for (int i = 0; i < m_keyBoardWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_keyBoardWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_keyBoardWidget->m_treeWidgetItemList.at(i), 0,
                                        m_keyBoardWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

}

void HardWareInfoWidget::getMouseInfo() {
    for (int j = 0; j < g_infogetter->m_mouseInfoList.length(); ++j) {
        HwWidget *m_mouseWidget = new HwWidget(&(g_infogetter->m_mouseInfoList[j]));
        m_mouseWidget->setAutoFillBackground(true);
        m_mouseWidgetList.append(m_mouseWidget);

        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_mouseWidget);
        for (int i = 0; i < m_mouseWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_mouseWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_mouseWidget->m_treeWidgetItemList.at(i), 0,
                                        m_mouseWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_MOUSE).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString, QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()) {
            continue;
        }
        HwWidget *m_keyBoardWidget = new HwWidget("Mouse", infoMap);
        m_keyBoardWidget->setAutoFillBackground(true);
        m_keyboardWidgetList.append(m_keyBoardWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_keyBoardWidget);
        for (int i = 0; i < m_keyBoardWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_keyBoardWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_keyBoardWidget->m_treeWidgetItemList.at(i), 0,
                                        m_keyBoardWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }
}

void HardWareInfoWidget::getBluetoothInfo() {
    for (int j = 0; j < g_infogetter->m_bluetoothHardwares.length(); ++j) {
        HwWidget *m_bluetoothWidget = new HwWidget(&(g_infogetter->m_bluetoothHardwares[j]));
        m_bluetoothWidget->setAutoFillBackground(true);
        m_bluetoothWidgetList.append(m_bluetoothWidget);

        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_bluetoothWidget);
        for (int i = 0; i < m_bluetoothWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_bluetoothWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_bluetoothWidget->m_treeWidgetItemList.at(i), 0,
                                        m_bluetoothWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_BLUETOOTH).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString, QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()) {
            continue;
        }
        HwWidget *m_bluetoothWidget = new HwWidget("Bluetooth", infoMap);
        m_bluetoothWidget->setAutoFillBackground(true);
        m_bluetoothWidgetList.append(m_bluetoothWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_bluetoothWidget);
        for (int i = 0; i < m_bluetoothWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_bluetoothWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_bluetoothWidget->m_treeWidgetItemList.at(i), 0,
                                        m_bluetoothWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }
}

void HardWareInfoWidget::getCDInfo() {
    for (int j = 0; j < g_infogetter->m_cdInfoList.length(); ++j) {
        HwWidget *m_cdWidget = new HwWidget(&(g_infogetter->m_cdInfoList[j]));
        m_cdWidget->setAutoFillBackground(true);
        m_cdWidgetList.append(m_cdWidget);

        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_cdWidget);
        for (int i = 0; i < m_cdWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_cdWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_cdWidget->m_treeWidgetItemList.at(i), 0, m_cdWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }


    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_CD).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString, QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()) {
            continue;
        }
        HwWidget *m_cdWidget = new HwWidget("Cd", infoMap);
        m_cdWidget->setAutoFillBackground(true);
        m_cdWidgetList.append(m_cdWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_cdWidget);
        for (int i = 0; i < m_cdWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_cdWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_cdWidget->m_treeWidgetItemList.at(i), 0,
                                        m_cdWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }
}

void HardWareInfoWidget::getCameraInfo() {
    for (int j = 0; j < g_infogetter->m_cameraInfoList.length(); ++j) {
        HwWidget *m_cameraWidget = new HwWidget(&(g_infogetter->m_cameraInfoList[j]));
        m_cameraWidget->setAutoFillBackground(true);
        m_cameraWidgetList.append(m_cameraWidget);

        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_cameraWidget);
        for (int i = 0; i < m_cameraWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_cameraWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_cameraWidget->m_treeWidgetItemList.at(i), 0,
                                        m_cameraWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_CAMERA).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString, QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()) {
            continue;
        }
        HwWidget *m_cameraWidget = new HwWidget("Camera", infoMap);
        m_cameraWidget->setAutoFillBackground(true);
        m_cameraWidgetList.append(m_cameraWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_cameraWidget);
        for (int i = 0; i < m_cameraWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_cameraWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_cameraWidget->m_treeWidgetItemList.at(i), 0,
                                        m_cameraWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }
}

void HardWareInfoWidget::getFanInfo() {
    for (int j = 0; j < g_infogetter->m_fanInfoList.length(); ++j) {
        HwWidget *m_fanWidget = new HwWidget(&(g_infogetter->m_fanInfoList[j]));
        m_fanWidget->setAutoFillBackground(true);
        m_fanWidgetList.append(m_fanWidget);

        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_fanWidget);
        for (int i = 0; i < m_fanWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_fanWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_fanWidget->m_treeWidgetItemList.at(i), 0, m_fanWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_FAN).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString, QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()) {
            continue;
        }
        HwWidget *m_fanWidget = new HwWidget("Fan", infoMap);
        m_fanWidget->setAutoFillBackground(true);
        m_fanWidgetList.append(m_fanWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_fanWidget);
        for (int i = 0; i < m_fanWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_fanWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_fanWidget->m_treeWidgetItemList.at(i), 0,
                                        m_fanWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }
}

void HardWareInfoWidget::getBatteryInfo() {
    for (int j = 0; j < g_infogetter->m_batteryInfoList.length(); ++j) {
        HwWidget *m_batteryWidget = new HwWidget(&(g_infogetter->m_batteryInfoList[j]));
        m_batteryWidget->setAutoFillBackground(true);
        m_batteryWidgetList.append(m_batteryWidget);

        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_batteryWidget);
        for (int i = 0; i < m_batteryWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_batteryWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_batteryWidget->m_treeWidgetItemList.at(i), 0,
                                        m_batteryWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }

    // 检查ini中是否需要新增的一整条信息
    kom::Configure conf;
    QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_BATTERY).toStringList().join(",");
    QStringList infoList = info.split("|");
    for (int i = 0; i < infoList.length(); ++i) {
        QMap<QString, QString> infoMap;
        if (infoList.at(i).contains("Add")) {
            QStringList keyValue = infoList.at(i).split(",");
            for (int j = 1; j < keyValue.length(); ++j) {
                QStringList keyAndValue = keyValue[j].split("=");
                infoMap.insert(keyAndValue[0], keyAndValue[1]);
            }
        }
        if (infoMap.isEmpty()) {
            continue;
        }
        HwWidget *m_batteryWidget = new HwWidget("Battery", infoMap);
        m_batteryWidget->setAutoFillBackground(true);
        m_batteryWidgetList.append(m_batteryWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_batteryWidget);
        for (int i = 0; i < m_batteryWidget->m_treeItemList.count(); ++i) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_batteryWidget->m_treeWidgetItemList.at(i));
            m_TreeWidget->setItemWidget(m_batteryWidget->m_treeWidgetItemList.at(i), 0,
                                        m_batteryWidget->m_treeItemList.at(i));
        }
        m_hardwareItems++;
    }
}

void HardWareInfoWidget::setIconColor() {
    QString styleName = kdk::GsettingMonitor::getSystemTheme().toString();
    if (styleName == "ukui-black" || styleName == "ukui-dark") {
        QIcon icon = getIcon("ukui-zs-host-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        QPixmap pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_hostPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "white"));

        icon = getIcon("ukui-zs-neicun-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_sysVersionPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "white"));

        icon = getIcon("ukui-zs-machine-model-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_sysYearVersionPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "white"));

        icon = getIcon("ukui-zs-kernel-version-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_kernelVersionPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "white"));

        icon = getIcon("ukui-zs-kernel-architecture-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_archPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "white"));

        icon = getIcon("ukui-zs-system-bits-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_sysBitPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "white"));

        icon = getIcon("ukui-zs-manufacturer-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_manufactorPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "white"));

        icon = getIcon("ukui-miracast-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_versionPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "white"));

        icon = getIcon("ukui-zs-machine-model-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_modelPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "white"));

        icon = getIcon("ukui-zs-sn-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_serialPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "white"));
    } else {
        QIcon icon = getIcon("ukui-zs-host-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        QPixmap pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_hostPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "black"));

        icon = getIcon("ukui-zs-neicun-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_sysVersionPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "black"));

        icon = getIcon("ukui-zs-machine-model-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_sysYearVersionPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "black"));

        icon = getIcon("ukui-zs-kernel-version-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_kernelVersionPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "black"));

        icon = getIcon("ukui-zs-kernel-architecture-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_archPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "black"));

        icon = getIcon("ukui-zs-system-bits-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_sysBitPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "black"));

        icon = getIcon("ukui-zs-manufacturer-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_manufactorPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "black"));

        icon = getIcon("ukui-miracast-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_versionPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "black"));

        icon = getIcon("ukui-zs-machine-model-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_modelPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "black"));

        icon = getIcon("ukui-zs-sn-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        pixmap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_serialPic->setPixmap(drawSymbolicColoredPixmap(pixmap, "black"));
    }
}

QIcon HardWareInfoWidget::getIcon(QString iconName, QSize iconSize)
{
    QString path = "/usr/share/kylin-os-manager/icons/";

    QIcon icon;
    if (iconName.indexOf("/") != -1) {
        icon.addFile(iconName, iconSize);
        return icon;
    }

    icon = QIcon::fromTheme(iconName);
    if (!icon.isNull())
        return icon;

    QString selfIconPath = path + iconName + ".svg";
    icon.addFile(selfIconPath, iconSize);
    return icon;
}

QPixmap HardWareInfoWidget::drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor) {
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor) {
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

void HardWareInfoWidget::deviceAddSlot(QString devtype) {
    if (devtype == "USB storage device") {
        RefreshDiskvInfo *refreshDiskInfo = new RefreshDiskvInfo();
        connect(refreshDiskInfo, &QThread::finished, this, &HardWareInfoWidget::addDiskItem);
        connect(refreshDiskInfo, &QThread::finished, refreshDiskInfo, &QThread::deleteLater);
        refreshDiskInfo->start();
    } else if (devtype == "Keyboard or mouse") {
        g_infogetter->refreshMouseInfo();
        g_infogetter->refreshKeyboardInfo();
        RefreshKeyboardInfo *refreshKeyboardInfo = new RefreshKeyboardInfo();
        connect(refreshKeyboardInfo, &QThread::finished, this, &HardWareInfoWidget::addKeyboardItem);
        connect(refreshKeyboardInfo, &QThread::finished, refreshKeyboardInfo, &QThread::deleteLater);
        refreshKeyboardInfo->start();
        RefreshMouseInfo *refreshMouseInfo = new RefreshMouseInfo();
        connect(refreshMouseInfo, &QThread::finished, this, &HardWareInfoWidget::addMouseItem);
        connect(refreshMouseInfo, &QThread::finished, refreshMouseInfo, &QThread::deleteLater);
        refreshMouseInfo->start();
    } else if (devtype == "Bluetooth device") {
        RefreshBluetoothInfo *refreshBluetoothInfo = new RefreshBluetoothInfo();
        connect(refreshBluetoothInfo, &QThread::finished, this, &HardWareInfoWidget::addBluetoothItem);
        connect(refreshBluetoothInfo, &QThread::finished, refreshBluetoothInfo, &QThread::deleteLater);
        QTimer::singleShot(2000, [=]() { refreshBluetoothInfo->start(); });
    } else if (devtype == "Network card") {
        RefreshNetworkInfo *refreshNetworkInfo = new RefreshNetworkInfo();
        connect(refreshNetworkInfo, &QThread::finished, this, &HardWareInfoWidget::addNetworkItem);
        connect(refreshNetworkInfo, &QThread::finished, refreshNetworkInfo, &QThread::deleteLater);
        refreshNetworkInfo->start();
    } else if (devtype == "Optical drive") {
        RefreshCdInfo *refreshCdInfo = new RefreshCdInfo();
        connect(refreshCdInfo, &QThread::finished, this, &HardWareInfoWidget::addCdItem);
        connect(refreshCdInfo, &QThread::finished, refreshCdInfo, &QThread::deleteLater);
        refreshCdInfo->start();
    }
}

void HardWareInfoWidget::deviceRemoveSlot(QString devtype) {
    if (devtype == "USB storage device") {
        RefreshDiskvInfo *refreshDiskInfo = new RefreshDiskvInfo();
        connect(refreshDiskInfo, &QThread::finished, this, &HardWareInfoWidget::removeDiskItem);
        connect(refreshDiskInfo, &QThread::finished, refreshDiskInfo, &QThread::deleteLater);
        refreshDiskInfo->start();
    } else if (devtype == "Keyboard or mouse") {

        RefreshKeyboardInfo *refreshKeyboardInfo = new RefreshKeyboardInfo();
        connect(refreshKeyboardInfo, &QThread::finished, this, &HardWareInfoWidget::removeKeyboardItem);
        connect(refreshKeyboardInfo, &QThread::finished, refreshKeyboardInfo, &QThread::deleteLater);
        RefreshMouseInfo *refreshMouseInfo = new RefreshMouseInfo();
        connect(refreshMouseInfo, &QThread::finished, this, &HardWareInfoWidget::removeMouseItem);
        connect(refreshMouseInfo, &QThread::finished, refreshMouseInfo, &QThread::deleteLater);
        refreshKeyboardInfo->start();
        QTimer::singleShot(500, [=]() {    // 由于并行计算，可能导致连续删除两个item时计算item位置错误，所以延时半秒
            refreshMouseInfo->start();
        });
    }else if (devtype == "Bluetooth device") {
        RefreshBluetoothInfo *refreshBluetoothInfo = new RefreshBluetoothInfo();
        connect(refreshBluetoothInfo, &QThread::finished, this, &HardWareInfoWidget::removeBluetoothItem);
        connect(refreshBluetoothInfo, &QThread::finished, refreshBluetoothInfo, &QThread::deleteLater);
        QTimer::singleShot(2000, [=]() { refreshBluetoothInfo->start(); });
    }else if (devtype == "Network card") {
        RefreshNetworkInfo *refreshNetworkInfo = new RefreshNetworkInfo();
        connect(refreshNetworkInfo, &QThread::finished, this, &HardWareInfoWidget::removeNetworkItem);
        connect(refreshNetworkInfo, &QThread::finished, refreshNetworkInfo, &QThread::deleteLater);
        refreshNetworkInfo->start();
    }else if (devtype == "Optical drive") {
        RefreshCdInfo *refreshCdInfo = new RefreshCdInfo();
        connect(refreshCdInfo, &QThread::finished, this, &HardWareInfoWidget::removeCdItem);
        connect(refreshCdInfo, &QThread::finished, refreshCdInfo, &QThread::deleteLater);
        refreshCdInfo->start();
    }
}

void HardWareInfoWidget::addDiskItem() {
    QList<DiskInfo> newDiskInfoList = g_infogetter->m_diskinfoList;
    if (g_infogetter->m_oldDiskInfoList.length() < newDiskInfoList.length()) {
        QStringList nameList;
        for (int i = 0; i < g_infogetter->m_oldDiskInfoList.length(); ++i) {
            nameList.append(g_infogetter->m_oldDiskInfoList[i].name);
        }
        for (int i = 0; i < newDiskInfoList.length(); ++i) {
            if (!nameList.contains(newDiskInfoList[i].name)) {
                //将新加入的设备展示在界面中
                HwWidget *m_diskWidget = new HwWidget(&(newDiskInfoList[i]));
                m_diskWidget->setAutoFillBackground(true);
                m_diskWidgetList.append(m_diskWidget);
                QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
                rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_TreeWidget->addTopLevelItem(rootItem);
                m_TreeWidget->setItemWidget(rootItem, 0, m_diskWidget);
                for (int j = 0; j < m_diskWidget->m_treeItemList.count(); ++j) {
                    m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_diskWidget->m_treeWidgetItemList.at(j));
                    m_TreeWidget->setItemWidget(m_diskWidget->m_treeWidgetItemList.at(j), 0,
                                                m_diskWidget->m_treeItemList.at(j));
                }
                m_hardwareItems++;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() + TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
    m_TreeWidget->customSort();
}

void HardWareInfoWidget::addMouseItem() {
    QList<MouseInfo> newMouseInfoList = g_infogetter->m_mouseInfoList;
    if (g_infogetter->m_oldMouseInfoList.length() < newMouseInfoList.length()) {
        QStringList nameList;
        for (int i = 0; i < g_infogetter->m_oldMouseInfoList.length(); ++i) {
            nameList.append(g_infogetter->m_oldMouseInfoList[i].name);
        }
        for (int i = 0; i < newMouseInfoList.length(); ++i) {
            if (!nameList.contains(newMouseInfoList[i].name)) {
                //将新加入的设备展示在界面中
                HwWidget *m_mouseWidget = new HwWidget(&(newMouseInfoList[i]));
                m_mouseWidget->setAutoFillBackground(true);
                m_mouseWidgetList.append(m_mouseWidget);
                QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
                rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_TreeWidget->addTopLevelItem(rootItem);
                m_TreeWidget->setItemWidget(rootItem, 0, m_mouseWidget);
                for (int j = 0; j < m_mouseWidget->m_treeItemList.count(); ++j) {
                    m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_mouseWidget->m_treeWidgetItemList.at(j));
                    m_TreeWidget->setItemWidget(m_mouseWidget->m_treeWidgetItemList.at(j), 0,
                                                m_mouseWidget->m_treeItemList.at(j));
                }
                m_hardwareItems++;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() + TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
}

void HardWareInfoWidget::addKeyboardItem() {
    QList<KeyboardInfo> newKeyboardInfoList = g_infogetter->m_keyboardInfoList;
    if (g_infogetter->m_oldMouseInfoList.length() < newKeyboardInfoList.length()) {
        QStringList nameList;
        for (int i = 0; i < g_infogetter->m_oldMouseInfoList.length(); ++i) {
            nameList.append(g_infogetter->m_oldMouseInfoList[i].name);
        }
        for (int i = 0; i < newKeyboardInfoList.length(); ++i) {
            if (!nameList.contains(newKeyboardInfoList[i].name)) {
                //将新加入的设备展示在界面中
                HwWidget *m_keyboardWidget = new HwWidget(&(newKeyboardInfoList[i]));
                m_keyboardWidget->setAutoFillBackground(true);
                m_keyboardWidgetList.append(m_keyboardWidget);
                QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
                rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_TreeWidget->addTopLevelItem(rootItem);
                m_TreeWidget->setItemWidget(rootItem, 0, m_keyboardWidget);
                for (int j = 0; j < m_keyboardWidget->m_treeItemList.count(); ++j) {
                    m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_keyboardWidget->m_treeWidgetItemList.at(j));
                    m_TreeWidget->setItemWidget(m_keyboardWidget->m_treeWidgetItemList.at(j), 0,
                                                m_keyboardWidget->m_treeItemList.at(j));
                }
                m_hardwareItems++;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() + TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
}

void HardWareInfoWidget::addBluetoothItem() {
    QList<BluetoothInfo> newBluetoothInfoList = g_infogetter->m_bluetoothHardwares;
    for (int i = 0; i < g_infogetter->m_oldBluetoothInfoList.length(); ++i) {
        //将新加入的设备展示在界面-
        for (int j = m_bluetoothWidgetList.length() - 1; j >= 0; --j) {
            if (m_bluetoothWidgetList[j]->m_bluetoothAddress == g_infogetter->m_oldBluetoothInfoList[i].address) {
                HwWidget *hwWidget = m_bluetoothWidgetList[j];
                hwWidget->setAutoFillBackground(true);
                m_bluetoothWidgetList.removeOne(m_bluetoothWidgetList[j]);
                int x = hwWidget->parent()->findChildren<HwWidget *>().indexOf(hwWidget);
                delete m_TreeWidget->takeTopLevelItem(x);
                m_hardwareItems--;
            }
        }
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - TREEWIDGETITEMSIZEHINT.height());
    }

    for (int i = 0; i < newBluetoothInfoList.length(); ++i) {
        //将新加入的设备展示在界面中
        HwWidget *m_bluetoothWidget = new HwWidget(&(newBluetoothInfoList[i]));
        m_bluetoothWidget->setAutoFillBackground(true);
        m_bluetoothWidgetList.append(m_bluetoothWidget);
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
        rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->addTopLevelItem(rootItem);
        m_TreeWidget->setItemWidget(rootItem, 0, m_bluetoothWidget);
        for (int j = 0; j < m_bluetoothWidget->m_treeItemList.count(); ++j) {
            m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(
                        m_bluetoothWidget->m_treeWidgetItemList.at(j));
            m_TreeWidget->setItemWidget(m_bluetoothWidget->m_treeWidgetItemList.at(j), 0,
                                        m_bluetoothWidget->m_treeItemList.at(j));
        }
        m_hardwareItems++;
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + TREEWIDGETITEMSIZEHINT.height());
    }

    m_TreeWidget->customSort();
}

void HardWareInfoWidget::addNetworkItem() {
    QList<NetworkCardInfo> newNetworkInfoList = g_infogetter->m_networkCardHardwares;
    if (g_infogetter->m_oldNetworkInfoList.length() < newNetworkInfoList.length()) {
        QStringList nameList;
        for (int i = 0; i < g_infogetter->m_oldNetworkInfoList.length(); ++i) {
            nameList.append(g_infogetter->m_oldNetworkInfoList[i].name);
        }
        for (int i = 0; i < newNetworkInfoList.length(); ++i) {
            if (!nameList.contains(newNetworkInfoList[i].name)) {
                //将新加入的设备展示在界面中
                HwWidget *m_networkWidget = new HwWidget(&(newNetworkInfoList[i]));
                m_networkCardWidgetList.append(m_networkWidget);
                QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
                rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_TreeWidget->addTopLevelItem(rootItem);
                m_TreeWidget->setItemWidget(rootItem, 0, m_networkWidget);
                for (int j = 0; j < m_networkWidget->m_treeItemList.count(); ++j) {
                    m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_networkWidget->m_treeWidgetItemList.at(j));
                    m_TreeWidget->setItemWidget(m_networkWidget->m_treeWidgetItemList.at(j), 0,
                                                m_networkWidget->m_treeItemList.at(j));
                }
                m_hardwareItems++;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() + TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
    m_TreeWidget->customSort();
}

void HardWareInfoWidget::addCdItem() {
    QList<CDInfo> newCdInfoList = g_infogetter->m_cdInfoList;
    if (g_infogetter->m_oldCdInfoList.length() < newCdInfoList.length()) {
        QStringList nameList;
        for (int i = 0; i < g_infogetter->m_oldCdInfoList.length(); ++i) {
            nameList.append(g_infogetter->m_oldCdInfoList[i].name);
        }
        for (int i = 0; i < newCdInfoList.length(); ++i) {
            if (!nameList.contains(newCdInfoList[i].name)) {
                //将新加入的设备展示在界面中
                HwWidget *m_cdWidget = new HwWidget(&(newCdInfoList[i]));
                m_cdWidget->setAutoFillBackground(true);
                m_cdWidgetList.append(m_cdWidget);
                QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
                rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_TreeWidget->addTopLevelItem(rootItem);
                m_TreeWidget->setItemWidget(rootItem, 0, m_cdWidget);
                for (int j = 0; j < m_cdWidget->m_treeItemList.count(); ++j) {
                    m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_cdWidget->m_treeWidgetItemList.at(j));
                    m_TreeWidget->setItemWidget(m_cdWidget->m_treeWidgetItemList.at(j), 0,
                                                m_cdWidget->m_treeItemList.at(j));
                }
                m_hardwareItems++;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() + TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
    m_TreeWidget->customSort();
}

void HardWareInfoWidget::removeDiskItem() {
    QList<DiskInfo> newDiskInfoList = g_infogetter->m_diskinfoList;
    if (g_infogetter->m_oldDiskInfoList.length() > newDiskInfoList.length()) {
        QStringList nameList;
        for (int i = 0; i < newDiskInfoList.length(); ++i) {
            nameList.append(newDiskInfoList[i].name);
        }
        for (int i = 0; i < g_infogetter->m_oldDiskInfoList.length(); ++i) {
            if (!nameList.contains(g_infogetter->m_oldDiskInfoList[i].name)) {
                //将新加入的设备展示在界面-
                for (int j = 0; j < m_diskWidgetList.length(); ++j) {
                    if (m_diskWidgetList[j]->getDevName() == g_infogetter->m_oldDiskInfoList[i].name) {
                        HwWidget *hwWidget = m_diskWidgetList[j];
                        m_diskWidgetList[j]->spreadIn();
                        m_diskWidgetList.removeOne(m_diskWidgetList[j]);
                        int x = hwWidget->parent()->findChildren<HwWidget *>().indexOf(hwWidget);
                        delete m_TreeWidget->takeTopLevelItem(x);
                    }
                }
                m_hardwareItems--;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() - TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
}

void HardWareInfoWidget::removeMouseItem() {
    QList<MouseInfo> newMouseInfoList = g_infogetter->m_mouseInfoList;
    if (g_infogetter->m_oldMouseInfoList.length() > newMouseInfoList.length()) {
        QStringList nameList;
        for (int i = 0; i < newMouseInfoList.length(); ++i) {
            nameList.append(newMouseInfoList[i].name);
        }
        for (int i = 0; i < g_infogetter->m_oldMouseInfoList.length(); ++i) {
            if (!nameList.contains(g_infogetter->m_oldMouseInfoList[i].name)) {
                //将新加入的设备展示在界面-
                for (int j = 0; j < m_mouseWidgetList.length(); ++j) {
                    if (m_mouseWidgetList[j]->getDevName() == g_infogetter->m_oldMouseInfoList[i].name) {
                        HwWidget *hwWidget = m_mouseWidgetList[j];
                        m_mouseWidgetList[j]->spreadIn();
                        m_mouseWidgetList.removeOne(m_mouseWidgetList[j]);
                        int x = hwWidget->parent()->findChildren<HwWidget *>().indexOf(hwWidget);
                        delete m_TreeWidget->takeTopLevelItem(x);
                    }
                }
                m_hardwareItems--;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() - TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
}

void HardWareInfoWidget::removeKeyboardItem() {
    QList<KeyboardInfo> newKeyboardInfoList = g_infogetter->m_keyboardInfoList;
    if (g_infogetter->m_oldMouseInfoList.length() > newKeyboardInfoList.length()) {
        QStringList nameList;
        for (int i = 0; i < newKeyboardInfoList.length(); ++i) {
            nameList.append(newKeyboardInfoList[i].name);
        }
        for (int i = 0; i < g_infogetter->m_oldMouseInfoList.length(); ++i) {
            if (!nameList.contains(g_infogetter->m_oldMouseInfoList[i].name)) {
                //将新加入的设备展示在界面-
                for (int j = 0; j < m_keyboardWidgetList.length(); ++j) {
                    if (m_keyboardWidgetList[j]->getDevName() == g_infogetter->m_oldMouseInfoList[i].name) {
                        HwWidget *hwWidget = m_keyboardWidgetList[j];
                        m_keyboardWidgetList[j]->spreadIn();
                        m_keyboardWidgetList.removeOne(m_keyboardWidgetList[j]);
                        int x = hwWidget->parent()->findChildren<HwWidget *>().indexOf(hwWidget);
                        delete m_TreeWidget->takeTopLevelItem(x);
                    }
                }
                m_hardwareItems--;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() - TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
}

void HardWareInfoWidget::removeBluetoothItem() {
    QList<BluetoothInfo> newBluetoothInfoList = g_infogetter->m_bluetoothHardwares;
    if (g_infogetter->m_oldBluetoothInfoList.length() > newBluetoothInfoList.length()) {
        QStringList addressList;
        for (int i = 0; i < newBluetoothInfoList.length(); ++i) {
            addressList.append(newBluetoothInfoList[i].address);
        }
        for (int i = 0; i < g_infogetter->m_oldBluetoothInfoList.length(); ++i) {
            if (!addressList.contains(g_infogetter->m_oldBluetoothInfoList[i].address)) {
                //将新加入的设备展示在界面-
                for (int j = 0; j < m_bluetoothWidgetList.length(); ++j) {
                    if (m_bluetoothWidgetList[j]->m_bluetoothAddress == g_infogetter->m_oldBluetoothInfoList[i].address) {
                        HwWidget *hwWidget = m_bluetoothWidgetList[j];
                        m_bluetoothWidgetList[j]->spreadIn();
                        m_bluetoothWidgetList.removeOne(m_bluetoothWidgetList[j]);
                        int x = hwWidget->parent()->findChildren<HwWidget *>().indexOf(hwWidget);
                        delete m_TreeWidget->takeTopLevelItem(x);
                    }
                }
                m_hardwareItems--;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() - TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
}

void HardWareInfoWidget::removeNetworkItem() {
    QList<NetworkCardInfo> newNetworkInfoList = g_infogetter->m_networkCardHardwares;
    if (g_infogetter->m_oldNetworkInfoList.length() > newNetworkInfoList.length()) {
        QStringList nameList;
        for (int i = 0; i < newNetworkInfoList.length(); ++i) {
            nameList.append(newNetworkInfoList[i].name);
        }
        for (int i = 0; i < g_infogetter->m_oldNetworkInfoList.length(); ++i) {
            if (!nameList.contains(g_infogetter->m_oldNetworkInfoList[i].name)) {
                for (int j = 0; j < m_networkCardWidgetList.length(); ++j) {
                    if (m_networkCardWidgetList[j]->getDevName() == g_infogetter->m_oldNetworkInfoList[i].name) {
                        HwWidget *hwWidget = m_networkCardWidgetList[j];
                        m_networkCardWidgetList[j]->spreadIn();
                        m_networkCardWidgetList.removeOne(m_networkCardWidgetList[j]);
                        int x = hwWidget->parent()->findChildren<HwWidget *>().indexOf(hwWidget);
                        hwWidget->m_timer->stop();
                        delete m_TreeWidget->takeTopLevelItem(x);
                    }
                }
                m_hardwareItems--;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() - TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
}

void HardWareInfoWidget::removeCdItem() {
    QList<CDInfo> newCdInfoList = g_infogetter->m_cdInfoList;
    if (g_infogetter->m_oldCdInfoList.length() > newCdInfoList.length()) {
        QStringList nameList;
        for (int i = 0; i < newCdInfoList.length(); ++i) {
            nameList.append(newCdInfoList[i].name);
        }
        for (int i = 0; i < g_infogetter->m_oldCdInfoList.length(); ++i) {
            if (!nameList.contains(g_infogetter->m_oldCdInfoList[i].name)) {
                //将新加入的设备展示在界面-
                for (int j = 0; j < m_cdWidgetList.length(); ++j) {
                    if (m_cdWidgetList[j]->getDevName() == g_infogetter->m_oldCdInfoList[i].model) {
                        HwWidget *hwWidget = m_cdWidgetList[j];
                        m_cdWidgetList[j]->spreadIn();
                        m_cdWidgetList.removeOne(m_cdWidgetList[j]);
                        int x = hwWidget->parent()->findChildren<HwWidget *>().indexOf(hwWidget);
                        delete m_TreeWidget->takeTopLevelItem(x);
                    }
                }
                m_hardwareItems--;
                m_TreeWidget->setFixedHeight(m_TreeWidget->height() - TREEWIDGETITEMSIZEHINT.height());
            }
        }
    }
}

void HardWareInfoWidget::monitorChangeSlot() {
    g_infogetter->m_oldMonitorInfoList = g_infogetter->m_monitorList;
    RefreshMonitorInfo *refreshMonitorInfo = new RefreshMonitorInfo();
    connect(refreshMonitorInfo, &QThread::finished, this, &HardWareInfoWidget::handleMonitorItem);
    connect(refreshMonitorInfo, &QThread::finished, refreshMonitorInfo, &QThread::deleteLater);
    refreshMonitorInfo->start();
}

void HardWareInfoWidget::removeMonitorItem() {
    QStringList interfaceList;
    for (int i = 0; i < g_infogetter->m_monitorList.length(); ++i) {
        interfaceList.append(g_infogetter->m_monitorList[i].interface);
    }
    for (int i = 0; i < g_infogetter->m_oldMonitorInfoList.length(); ++i) {
        if (!interfaceList.contains(g_infogetter->m_oldMonitorInfoList[i].interface)) {
            //将移除的设备从界面删除
            for (int j = 0; j < m_monitorWidgetList.length(); ++j) {
                if (m_monitorWidgetList[j]->getMonitorInterface() == g_infogetter->m_oldMonitorInfoList[i].interface) {
                    HwWidget *hwWidget = m_monitorWidgetList[j];
                    m_monitorWidgetList[j]->spreadIn();
                    m_monitorWidgetList.removeOne(m_monitorWidgetList[j]);
                    int x = hwWidget->parent()->findChildren<HwWidget *>().indexOf(hwWidget);
                    delete m_TreeWidget->takeTopLevelItem(x);
                }
            }
            m_hardwareItems--;
            m_TreeWidget->setFixedHeight(m_TreeWidget->height() - TREEWIDGETITEMSIZEHINT.height());
        }
    }
}

void HardWareInfoWidget::addMonitorItem() {
    QStringList interfaceList;
    for (int i = 0; i < g_infogetter->m_oldMonitorInfoList.length(); ++i) {
        interfaceList.append(g_infogetter->m_oldMonitorInfoList[i].interface);
    }
    for (int i = 0; i < g_infogetter->m_monitorList.length(); ++i) {
        if (!interfaceList.contains(g_infogetter->m_monitorList[i].interface)) {
            //将新加入的设备展示在界面中
            HwWidget *m_monitorWidget = new HwWidget(&(g_infogetter->m_monitorList[i]));
            m_monitorWidget->setAutoFillBackground(true);
            m_monitorWidgetList.append(m_monitorWidget);
            QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
            rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_TreeWidget->addTopLevelItem(rootItem);
            m_TreeWidget->setItemWidget(rootItem, 0, m_monitorWidget);
            for (int j = 0; j < m_monitorWidget->m_treeItemList.count(); ++j) {
                m_TreeWidget->topLevelItem(m_hardwareItems)->addChild(m_monitorWidget->m_treeWidgetItemList.at(j));
                m_TreeWidget->setItemWidget(m_monitorWidget->m_treeWidgetItemList.at(j), 0,
                                            m_monitorWidget->m_treeItemList.at(j));
            }
            m_hardwareItems++;
            m_TreeWidget->setFixedHeight(m_TreeWidget->height() + TREEWIDGETITEMSIZEHINT.height());
        }
    }
}

void HardWareInfoWidget::handleMonitorItem() {
    QTimer::singleShot(5000, [=](){
        QList<MonitorInfo> newMonitorInfoList = g_infogetter->m_monitorList;
        if (newMonitorInfoList.length() > g_infogetter->m_oldMonitorInfoList.length()) {
            addMonitorItem();
        }
        if (newMonitorInfoList.length() < g_infogetter->m_oldMonitorInfoList.length()) {
            removeMonitorItem();
        }
    });
}

void HardWareInfoWidget::explorBasicContent() {
    m_clipboard = QApplication::clipboard();
    m_clipboard->setText(m_basicExportContent.join("\n"));
}

void HardWareInfoWidget::disableBluetoothItem() {
    for (int i = 0; i < m_bluetoothWidgetList.length(); ++i) {
        m_bluetoothWidgetList[i]->setDisableStatus(true);
    }
}

void HardWareInfoWidget::enableBluetoothItem() {
    for (int i = 0; i < m_bluetoothWidgetList.length(); ++i) {
        m_bluetoothWidgetList[i]->setDisableStatus(false);
    }
}

void HardWareInfoWidget::enbaleWiredNetworkCardItem() {
    for (int i = 0; i < g_infogetter->m_networkCardHardwares.length(); ++i) {
        if(g_infogetter->m_networkCardHardwares[i].name == "wired Network Card"){
            g_infogetter->m_networkCardHardwares.removeAt(i);

            for (int j = 0; j < m_networkCardWidgetList.length(); ++j) {
                if (m_networkCardWidgetList[j]->getDevName() == "wired Network Card") {
                    HwWidget *hwWidget = m_networkCardWidgetList[j];
                    m_networkCardWidgetList[j]->spreadIn();
                    m_networkCardWidgetList.removeOne(m_networkCardWidgetList[j]);
                    int x = hwWidget->parent()->findChildren<HwWidget *>().indexOf(hwWidget);
                    hwWidget->m_timer->stop();
                    delete m_TreeWidget->takeTopLevelItem(x);
                    m_hardwareItems--;
                    m_TreeWidget->setFixedHeight(m_TreeWidget->height() - TREEWIDGETITEMSIZEHINT.height());
                }
            }
        }
    }
    deviceAddSlot("Network card");
}

void HardWareInfoWidget::enableWirelessNetworkCardItem() {
    for (int i = 0; i < g_infogetter->m_networkCardHardwares.length(); ++i) {
        if(g_infogetter->m_networkCardHardwares[i].name == "Wireless Network Card"){
            g_infogetter->m_networkCardHardwares.removeAt(i);

            for (int j = 0; j < m_networkCardWidgetList.length(); ++j) {
                if (m_networkCardWidgetList[j]->getDevName() == "Wireless Network Card") {
                    HwWidget *hwWidget = m_networkCardWidgetList[j];
                    m_networkCardWidgetList[j]->spreadIn();
                    m_networkCardWidgetList.removeOne(m_networkCardWidgetList[j]);
                    int x = hwWidget->parent()->findChildren<HwWidget *>().indexOf(hwWidget);
                    hwWidget->m_timer->stop();
                    delete m_TreeWidget->takeTopLevelItem(x);
                    m_hardwareItems--;
                    m_TreeWidget->setFixedHeight(m_TreeWidget->height() - TREEWIDGETITEMSIZEHINT.height());
                }
            }
        }
    }
    deviceAddSlot("Network card");
}



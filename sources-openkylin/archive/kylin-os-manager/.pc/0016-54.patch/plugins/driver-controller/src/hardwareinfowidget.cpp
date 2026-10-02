#include "hardwareinfowidget.h"
#include <QDebug>
#include <QIcon>
#include <kysdk/applications/gsettingmonitor.h>
#include <QAbstractItemView>

#include "include/common.h"
#include "include/globalsignal.h"
#include "treeitem.h"

HardWareInfoWidget::HardWareInfoWidget(QWidget *parent)
        : QWidget(parent),
          m_mainLayout(new QVBoxLayout()),
          m_pcBasicContentLayout(new QVBoxLayout()),
          m_pcBasicContentWidget(new QWidget),
          m_pcInfoLabel(new QLabel()),
          m_hostPic(new QLabel()),
          m_hostnameLabel(new QLabel()),
          m_hostnameLayout(new QHBoxLayout),
          m_hostnameWidget(new QWidget),
          m_hostnameLabel2(new QLabel()),
          m_hostnameLayout2(new QHBoxLayout),
          m_hostnameWidget2(new QWidget),
          m_sysVersionPic(new QLabel()),
          m_sysVersionLabel(new QLabel()),
          m_sysVersionLayout(new QHBoxLayout),
          m_sysVersionWidget(new QWidget),
          m_sysVersionLabel2(new QLabel()),
          m_sysVersionLayout2(new QHBoxLayout),
          m_sysVersionWidget2(new QWidget),
          m_kernelVersionPic(new QLabel()),
          m_kernelVersionLabel(new QLabel()),
          m_kernelVersionLayout(new QHBoxLayout),
          m_kernelVersionWidget(new QWidget),
          m_kernelVersionLabel2(new QLabel()),
          m_kernelVersionLayout2(new QHBoxLayout),
          m_kernelVersionWidget2(new QWidget),
          m_archPic(new QLabel()),
          m_archLabel(new QLabel()),
          m_archLayout(new QHBoxLayout),
          m_archWidget(new QWidget),
          m_archLabel2(new QLabel()),
          m_archLayout2(new QHBoxLayout),
          m_archWidget2(new QWidget),
          m_sysBitPic(new QLabel()),
          m_sysBitLabel(new QLabel()),
          m_sysBitLayout(new QHBoxLayout),
          m_sysBitWidget(new QWidget),
          m_sysBitLabel2(new QLabel()),
          m_sysBitLayout2(new QHBoxLayout),
          m_sysBitWidget2(new QWidget),
          m_manufactorPic(new QLabel()),
          m_manufactorLabel(new QLabel()),
          m_manufactorLayout(new QHBoxLayout),
          m_manufactorWidget(new QWidget),
          m_manufactorLabel2(new QLabel()),
          m_manufactorLayout2(new QHBoxLayout),
          m_manufactorWidget2(new QWidget),
          m_modelPic(new QLabel()),
          m_modelLabel(new QLabel()),
          m_modelLayout(new QHBoxLayout),
          m_modelWidget(new QWidget),
          m_modelLabel2(new QLabel()),
          m_modelLayout2(new QHBoxLayout),
          m_modelWidget2(new QWidget),
          m_serialPic(new QLabel()),
          m_serialLabel(new QLabel()),
          m_serialLayout(new QHBoxLayout),
          m_serialWidget(new QWidget),
          m_serialLabel2(new QLabel()),
          m_serialLayout2(new QHBoxLayout),
          m_serialWidget2(new QWidget),
          m_hardwareDisplayLabel(new QLabel),
          m_HWDisplayLayout(new QVBoxLayout),
          m_HWDisplayWidget(new QWidget),
          m_TreeWidget(new QTreeWidget),
          m_cpuWidget(new QWidget),
          m_cpuIconLabel(new QLabel),
          m_cpuTextLabel(new QLabel),
          m_cpuModelLabel(new QLabel),
          m_cpuBtnStackedWidget(new QStackedWidget),
          m_cpuSpreadOutBtn(new kdk::KBorderlessButton),
          m_cpuSpreadInBtn(new kdk::KBorderlessButton),
          m_cpuLayout(new QHBoxLayout),
          m_memoryIconLabel(new QLabel),
          m_memoryTextLabel(new QLabel),
          m_memoryBtnStackedWidget(new QStackedWidget),
          m_memorySpreadOutBtn(new kdk::KBorderlessButton),
          m_memorySpreadInBtn(new kdk::KBorderlessButton),
          m_memoryLayout(new QHBoxLayout),
          m_memoryWidget(new QWidget),
          m_graphicCardIconLabel(new QLabel),
          m_graphicCardTextLabel(new QLabel),
          m_graphicCardBtnStackedWidget(new QStackedWidget),
          m_graphicCardSpreadOutBtn(new kdk::KBorderlessButton),
          m_graphicCardSpreadInBtn(new kdk::KBorderlessButton),
          m_graphicCardLayout(new QHBoxLayout),
          m_graphicCardWidget(new QWidget),
          m_baseboardIconLabel(new QLabel),
          m_baseboardTextLabel(new QLabel),
          m_baseboardModelLabel(new QLabel),
          m_baseboardBtnStackedWidget(new QStackedWidget),
          m_baseboardSpreadOutBtn(new kdk::KBorderlessButton),
          m_baseboardSpreadInBtn(new kdk::KBorderlessButton),
          m_baseboardLayout(new QHBoxLayout),
          m_baseboardWidget(new QWidget),
          m_networkCardIconLabel(new QLabel),
          m_networkCardTextLabel(new QLabel),
          m_networkCardBtnStackedWidget(new QStackedWidget),
          m_networkCardSpreadOutBtn(new kdk::KBorderlessButton),
          m_networkCardSpreadInBtn(new kdk::KBorderlessButton),
          m_networkCardLayout(new QHBoxLayout),
          m_networkCardWidget(new QWidget),
          m_diskIconLabel(new QLabel),
          m_diskTextLabel(new QLabel),
          m_diskBtnStackedWidget(new QStackedWidget),
          m_diskSpreadOutBtn(new kdk::KBorderlessButton),
          m_diskSpreadInBtn(new kdk::KBorderlessButton),
          m_diskLayout(new QHBoxLayout),
          m_diskWidget(new QWidget),
          m_monitorIconLabel(new QLabel),
          m_monitorTextLabel(new QLabel),
          m_monitorBtnStackedWidget(new QStackedWidget),
          m_monitorSpreadOutBtn(new kdk::KBorderlessButton),
          m_monitorSpreadInBtn(new kdk::KBorderlessButton),
          m_monitorLayout(new QHBoxLayout),
          m_monitorWidget(new QWidget),
          m_soundCardIconLabel(new QLabel),
          m_soundCardTextLabel(new QLabel),
          m_soundCardModelLabel(new QLabel),
          m_soundCardBtnStackedWidget(new QStackedWidget),
          m_soundCardSpreadOutBtn(new kdk::KBorderlessButton),
          m_soundCardSpreadInBtn(new kdk::KBorderlessButton),
          m_soundCardLayout(new QHBoxLayout),
          m_soundCardWidget(new QWidget),
          m_keyboardIconLabel(new QLabel),
          m_keyboardTextLabel(new QLabel),
          m_keyboardBtnStackedWidget(new QStackedWidget),
          m_keyboardSpreadOutBtn(new kdk::KBorderlessButton),
          m_keyboardSpreadInBtn(new kdk::KBorderlessButton),
          m_keyboardLayout(new QHBoxLayout),
          m_keyboardWidget(new QWidget),
          m_mouseIconLabel(new QLabel),
          m_mouseTextLabel(new QLabel),
          m_mouseBtnStackedWidget(new QStackedWidget),
          m_mouseSpreadOutBtn(new kdk::KBorderlessButton),
          m_mouseSpreadInBtn(new kdk::KBorderlessButton),
          m_mouseLayout(new QHBoxLayout),
          m_mouseWidget(new QWidget),
          m_bluetoothIconLabel(new QLabel),
          m_bluetoothTextLabel(new QLabel),
          m_bluetoothModelLabel(new QLabel),
          m_bluetoothBtnStackedWidget(new QStackedWidget),
          m_bluetoothSpreadOutBtn(new kdk::KBorderlessButton),
          m_bluetoothSpreadInBtn(new kdk::KBorderlessButton),
          m_bluetoothLayout(new QHBoxLayout),
          m_bluetoothWidget(new QWidget) {
    initConnection();
    initGui();
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
    m_mainLayout->addSpacing(8);
    m_mainLayout->addWidget(m_hardwareDisplayLabel);
    m_mainLayout->addWidget(m_HWDisplayWidget);
    m_mainLayout->addStretch();     // 临时的玩意
    this->setLayout(m_mainLayout);
}

void HardWareInfoWidget::setBasicContentUi() {
    m_pcInfoLabel->setText(tr("Computer Basic Information"));

    m_hostPic->setPixmap(QIcon::fromTheme("ukui-zs-host-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_hostPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_hostnameLabel->setText(tr("Host Name"));

    m_sysVersionPic->setPixmap(QIcon::fromTheme("ukui-zs-neicun-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_sysVersionPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_sysVersionLabel->setText(tr("Operation System Version"));

    m_kernelVersionPic->setPixmap(
            QIcon::fromTheme("ukui-zs-kernel-version-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_kernelVersionPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_kernelVersionLabel->setText(tr("Kernel Version"));

    m_archPic->setPixmap(QIcon::fromTheme("ukui-zs-kernel-architecture-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_archPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_archLabel->setText(tr("Kernel Architecture"));

    m_sysBitPic->setPixmap(QIcon::fromTheme("ukui-zs-system-bits-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_sysBitPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_sysBitLabel->setText(tr("System Bits"));

    m_manufactorPic->setPixmap(QIcon::fromTheme("ukui-zs-manufacturer-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_manufactorPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_manufactorLabel->setText(tr("Manufacturer"));

    m_modelPic->setPixmap(QIcon::fromTheme("ukui-zs-machine-model-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_modelPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_modelLabel->setText(tr("Model"));

    m_serialPic->setPixmap(QIcon::fromTheme("ukui-zs-sn-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_serialPic->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_serialLabel->setText(tr("Serial"));

    // 布局管理
    m_hostnameLayout->addWidget(m_hostPic);
    m_hostnameLayout->addSpacing(12);
    m_hostnameLayout->addWidget(m_hostnameLabel);
    m_hostnameLayout->setContentsMargins(0, 0, 0, 0);
    m_hostnameLayout->setSpacing(0);
    m_hostnameLayout->setAlignment(Qt::AlignLeft);
    m_hostnameWidget->setLayout(m_hostnameLayout);
    m_hostnameWidget->setFixedWidth(250);
    m_hostnameLayout2->addWidget(m_hostnameWidget);
    m_hostnameLayout2->addWidget(m_hostnameLabel2);
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
    m_sysVersionWidget->setFixedWidth(250);
    m_sysVersionLayout2->addWidget(m_sysVersionWidget);
    m_sysVersionLayout2->addWidget(m_sysVersionLabel2);
    m_sysVersionLayout2->setContentsMargins(0, 0, 0, 0);
    m_sysVersionLayout2->setSpacing(0);
    m_sysVersionLayout2->setAlignment(Qt::AlignLeft);
    m_sysVersionWidget2->setLayout(m_sysVersionLayout2);

    m_kernelVersionLayout->addWidget(m_kernelVersionPic);
    m_kernelVersionLayout->addSpacing(12);
    m_kernelVersionLayout->addWidget(m_kernelVersionLabel);
    m_kernelVersionLayout->setContentsMargins(0, 0, 0, 0);
    m_kernelVersionLayout->setSpacing(0);
    m_kernelVersionLayout->setAlignment(Qt::AlignLeft);
    m_kernelVersionWidget->setLayout(m_kernelVersionLayout);
    m_kernelVersionWidget->setFixedWidth(250);
    m_kernelVersionLayout2->addWidget(m_kernelVersionWidget);
    m_kernelVersionLayout2->addWidget(m_kernelVersionLabel2);
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
    m_archWidget->setFixedWidth(250);
    m_archLayout2->addWidget(m_archWidget);
    m_archLayout2->addWidget(m_archLabel2);
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
    m_sysBitWidget->setFixedWidth(250);
    m_sysBitLayout2->addWidget(m_sysBitWidget);
    m_sysBitLayout2->addWidget(m_sysBitLabel2);
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
    m_manufactorWidget->setFixedWidth(250);
    m_manufactorLayout2->addWidget(m_manufactorWidget);
    m_manufactorLayout2->addWidget(m_manufactorLabel2);
    m_manufactorLayout2->setContentsMargins(0, 0, 0, 0);
    m_manufactorLayout2->setSpacing(0);
    m_manufactorLayout2->setAlignment(Qt::AlignLeft);
    m_manufactorWidget2->setLayout(m_manufactorLayout2);

    m_modelLayout->addWidget(m_modelPic);
    m_modelLayout->addSpacing(12);
    m_modelLayout->addWidget(m_modelLabel);
    m_modelLayout->setContentsMargins(0, 0, 0, 0);
    m_modelLayout->setSpacing(0);
    m_modelLayout->setAlignment(Qt::AlignLeft);
    m_modelWidget->setLayout(m_modelLayout);
    m_modelWidget->setFixedWidth(250);
    m_modelLayout2->addWidget(m_modelWidget);
    m_modelLayout2->addWidget(m_modelLabel2);
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
    m_serialWidget->setFixedWidth(250);
    m_serialLayout2->addWidget(m_serialWidget);
    m_serialLayout2->addWidget(m_serialLabel2);
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
    m_pcBasicContentLayout->addWidget(m_kernelVersionWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_archWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_sysBitWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_manufactorWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_modelWidget2);
    m_pcBasicContentLayout->addSpacing(18);
    m_pcBasicContentLayout->addWidget(m_serialWidget2);
    m_pcBasicContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pcBasicContentLayout->setSpacing(0);
    m_pcBasicContentLayout->setAlignment(Qt::AlignLeft);

    m_pcBasicContentWidget->setLayout(m_pcBasicContentLayout);
}

void HardWareInfoWidget::setHWContetnUi() {
    m_hardwareDisplayLabel->setText(tr("Hardware Display"));
    // CPU
    m_cpuIconLabel->setPixmap(QIcon::fromTheme("ukui-zs-processor-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_cpuIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_cpuTextLabel->setText(tr("CPU"));
    m_cpuTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_cpuSpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_cpuSpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_cpuBtnStackedWidget->addWidget(m_cpuSpreadOutBtn);
    m_cpuBtnStackedWidget->addWidget(m_cpuSpreadInBtn);
    m_cpuBtnStackedWidget->setCurrentWidget(m_cpuSpreadOutBtn);

    m_cpuLayout->setContentsMargins(0, 0, 0, 0);
    m_cpuLayout->setSpacing(0);
    m_cpuLayout->addWidget(m_cpuIconLabel);
    m_cpuLayout->addSpacing(12);
    m_cpuLayout->addWidget(m_cpuTextLabel);
    m_cpuLayout->addWidget(m_cpuModelLabel, 0, Qt::AlignLeft);
    m_cpuLayout->addWidget(m_cpuBtnStackedWidget, 0, Qt::AlignRight);
    m_cpuWidget->setLayout(m_cpuLayout);

    m_TreeWidget->header()->setVisible(false);
    m_TreeWidget->setIndentation(0);
    m_TreeWidget->setFrameShape(QTreeWidget::NoFrame);                   //去掉list的边框
    m_TreeWidget->setStyleSheet(".QTreeWidget::item:hover{background:transparent;} QTreeWidget { margin: 0px; }");
    m_TreeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_TreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_TreeWidget->setSelectionMode(QAbstractItemView::NoSelection); // list取消原装的选中功能
    m_TreeWidget->setContentsMargins(0, 0, 0, 0);
    m_TreeWidget->setColumnCount(1);

    QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_TreeWidget);
    rootItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem);
    m_TreeWidget->setItemWidget(rootItem, 0, m_cpuWidget);
    m_TreeWidget->setFixedHeight(m_hardwareItems * TREEWIDGETITEMSIZEHINT.height());

    // Memory
    m_memoryIconLabel->setPixmap(QIcon::fromTheme("ukui-zs-memory-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_memoryIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_memoryTextLabel->setText(tr("Memory"));
    m_memoryTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_memorySpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_memorySpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_memoryBtnStackedWidget->addWidget(m_memorySpreadOutBtn);
    m_memoryBtnStackedWidget->addWidget(m_memorySpreadInBtn);
    m_memoryBtnStackedWidget->setCurrentWidget(m_memorySpreadOutBtn);

    m_memoryLayout->setContentsMargins(0, 0, 0, 0);
    m_memoryLayout->setSpacing(0);
    m_memoryLayout->addWidget(m_memoryIconLabel);
    m_memoryLayout->addSpacing(12);
    m_memoryLayout->addWidget(m_memoryTextLabel);
    m_memoryLayout->addWidget(m_memoryBtnStackedWidget, 3, Qt::AlignRight);
    m_memoryWidget->setLayout(m_memoryLayout);

    QTreeWidgetItem *rootItem2 = new QTreeWidgetItem(m_TreeWidget);
    rootItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem2);
    m_TreeWidget->setItemWidget(rootItem2, 0, m_memoryWidget);

    // GraphicCard
    m_graphicCardIconLabel->setPixmap(
            QIcon::fromTheme("ukui-graphics-card-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_graphicCardIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_graphicCardTextLabel->setText(tr("Graphic Card"));
    m_graphicCardTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_graphicCardSpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_graphicCardSpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_graphicCardBtnStackedWidget->addWidget(m_graphicCardSpreadOutBtn);
    m_graphicCardBtnStackedWidget->addWidget(m_graphicCardSpreadInBtn);
    m_graphicCardBtnStackedWidget->setCurrentWidget(m_graphicCardSpreadOutBtn);

    m_graphicCardLayout->setContentsMargins(0, 0, 0, 0);
    m_graphicCardLayout->setSpacing(0);
    m_graphicCardLayout->addWidget(m_graphicCardIconLabel);
    m_graphicCardLayout->addSpacing(12);
    m_graphicCardLayout->addWidget(m_graphicCardTextLabel);
    m_graphicCardLayout->addWidget(m_graphicCardBtnStackedWidget, 3, Qt::AlignRight);
    m_graphicCardWidget->setLayout(m_graphicCardLayout);

    QTreeWidgetItem *rootItem3 = new QTreeWidgetItem(m_TreeWidget);
    rootItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem3);
    m_TreeWidget->setItemWidget(rootItem3, 0, m_graphicCardWidget);

    // baseboard
    m_baseboardIconLabel->setPixmap(
            QIcon::fromTheme("ukui-mother-board-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_baseboardIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_baseboardTextLabel->setText(tr("Mother Board"));
    m_baseboardTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_baseboardSpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_baseboardSpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_baseboardBtnStackedWidget->addWidget(m_baseboardSpreadOutBtn);
    m_baseboardBtnStackedWidget->addWidget(m_baseboardSpreadInBtn);
    m_baseboardBtnStackedWidget->setCurrentWidget(m_baseboardSpreadOutBtn);

    m_baseboardLayout->setContentsMargins(0, 0, 0, 0);
    m_baseboardLayout->setSpacing(0);
    m_baseboardLayout->addWidget(m_baseboardIconLabel);
    m_baseboardLayout->addSpacing(12);
    m_baseboardLayout->addWidget(m_baseboardTextLabel);
    m_baseboardLayout->addWidget(m_baseboardModelLabel);
    m_baseboardLayout->addWidget(m_baseboardBtnStackedWidget, 3, Qt::AlignRight);
    m_baseboardWidget->setLayout(m_baseboardLayout);

    QTreeWidgetItem *rootItem4 = new QTreeWidgetItem(m_TreeWidget);
    rootItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem4);
    m_TreeWidget->setItemWidget(rootItem4, 0, m_baseboardWidget);

    // Network Card
    m_networkCardIconLabel->setPixmap(
            QIcon::fromTheme("ukui-graphics-card-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_networkCardIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_networkCardTextLabel->setText(tr("Network Card"));
    m_networkCardTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_networkCardSpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_networkCardSpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_networkCardBtnStackedWidget->addWidget(m_networkCardSpreadOutBtn);
    m_networkCardBtnStackedWidget->addWidget(m_networkCardSpreadInBtn);
    m_networkCardBtnStackedWidget->setCurrentWidget(m_networkCardSpreadOutBtn);

    m_networkCardLayout->setContentsMargins(0, 0, 0, 0);
    m_networkCardLayout->setSpacing(0);
    m_networkCardLayout->addWidget(m_networkCardIconLabel);
    m_networkCardLayout->addSpacing(12);
    m_networkCardLayout->addWidget(m_networkCardTextLabel);
    m_networkCardLayout->addWidget(m_networkCardBtnStackedWidget, 3, Qt::AlignRight);
    m_networkCardWidget->setLayout(m_networkCardLayout);

    QTreeWidgetItem *rootItem5 = new QTreeWidgetItem(m_TreeWidget);
    rootItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem5);
    m_TreeWidget->setItemWidget(rootItem5, 0, m_networkCardWidget);

    // Disk
    m_diskIconLabel->setPixmap(QIcon::fromTheme("ukui-zs-hard-disk-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_diskIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_diskTextLabel->setText(tr("Disk"));
    m_diskTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_diskSpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_diskSpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_diskBtnStackedWidget->addWidget(m_diskSpreadOutBtn);
    m_diskBtnStackedWidget->addWidget(m_diskSpreadInBtn);
    m_diskBtnStackedWidget->setCurrentWidget(m_diskSpreadOutBtn);

    m_diskLayout->setContentsMargins(0, 0, 0, 0);
    m_diskLayout->setSpacing(0);
    m_diskLayout->addWidget(m_diskIconLabel);
    m_diskLayout->addSpacing(12);
    m_diskLayout->addWidget(m_diskTextLabel);
    m_diskLayout->addWidget(m_diskBtnStackedWidget, 3, Qt::AlignRight);
    m_diskWidget->setLayout(m_diskLayout);

    QTreeWidgetItem *rootItem6 = new QTreeWidgetItem(m_TreeWidget);
    rootItem6->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem6);
    m_TreeWidget->setItemWidget(rootItem6, 0, m_diskWidget);

    // Monitor
    m_monitorIconLabel->setPixmap(QIcon::fromTheme("computer-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_monitorIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_monitorTextLabel->setText(tr("Monitor"));
    m_monitorTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_monitorSpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_monitorSpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_monitorBtnStackedWidget->addWidget(m_monitorSpreadOutBtn);
    m_monitorBtnStackedWidget->addWidget(m_monitorSpreadInBtn);
    m_monitorBtnStackedWidget->setCurrentWidget(m_monitorSpreadOutBtn);

    m_monitorLayout->setContentsMargins(0, 0, 0, 0);
    m_monitorLayout->setSpacing(0);
    m_monitorLayout->addWidget(m_monitorIconLabel);
    m_monitorLayout->addSpacing(12);
    m_monitorLayout->addWidget(m_monitorTextLabel);
    m_monitorLayout->addWidget(m_monitorBtnStackedWidget, 3, Qt::AlignRight);
    m_monitorWidget->setLayout(m_monitorLayout);

    QTreeWidgetItem *rootItem7 = new QTreeWidgetItem(m_TreeWidget);
    rootItem7->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem7);
    m_TreeWidget->setItemWidget(rootItem7, 0, m_monitorWidget);

    // soundCard
    m_soundCardIconLabel->setPixmap(
            QIcon::fromTheme("audio-volume-high-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_soundCardIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_soundCardTextLabel->setText(tr("Sound Card"));
    m_soundCardTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_soundCardSpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_soundCardSpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_soundCardBtnStackedWidget->addWidget(m_soundCardSpreadOutBtn);
    m_soundCardBtnStackedWidget->addWidget(m_soundCardSpreadInBtn);
    m_soundCardBtnStackedWidget->setCurrentWidget(m_soundCardSpreadOutBtn);

    m_soundCardLayout->setContentsMargins(0, 0, 0, 0);
    m_soundCardLayout->setSpacing(0);
    m_soundCardLayout->addWidget(m_soundCardIconLabel);
    m_soundCardLayout->addSpacing(12);
    m_soundCardLayout->addWidget(m_soundCardTextLabel);
    m_soundCardLayout->addWidget(m_soundCardModelLabel);
    m_soundCardLayout->addWidget(m_soundCardBtnStackedWidget, 3, Qt::AlignRight);
    m_soundCardWidget->setLayout(m_soundCardLayout);

    QTreeWidgetItem *rootItem8 = new QTreeWidgetItem(m_TreeWidget);
    rootItem8->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem8);
    m_TreeWidget->setItemWidget(rootItem8, 0, m_soundCardWidget);

    // Keyboard
    m_keyboardIconLabel->setPixmap(QIcon::fromTheme("input-keyboard-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_keyboardIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_keyboardTextLabel->setText(tr("Keyboard"));
    m_keyboardTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_keyboardSpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_keyboardSpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_keyboardBtnStackedWidget->addWidget(m_keyboardSpreadOutBtn);
    m_keyboardBtnStackedWidget->addWidget(m_keyboardSpreadInBtn);
    m_keyboardBtnStackedWidget->setCurrentWidget(m_keyboardSpreadOutBtn);

    m_keyboardLayout->setContentsMargins(0, 0, 0, 0);
    m_keyboardLayout->setSpacing(0);
    m_keyboardLayout->addWidget(m_keyboardIconLabel);
    m_keyboardLayout->addSpacing(12);
    m_keyboardLayout->addWidget(m_keyboardTextLabel);
    m_keyboardLayout->addWidget(m_keyboardBtnStackedWidget, 3, Qt::AlignRight);
    m_keyboardWidget->setLayout(m_keyboardLayout);

    QTreeWidgetItem *rootItem9 = new QTreeWidgetItem(m_TreeWidget);
    rootItem9->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem9);
    m_TreeWidget->setItemWidget(rootItem9, 0, m_keyboardWidget);

    // Mouse
    m_mouseIconLabel->setPixmap(QIcon::fromTheme("input-mouse-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_mouseIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_mouseTextLabel->setText(tr("Mouse"));
    m_mouseTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_mouseSpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_mouseSpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_mouseBtnStackedWidget->addWidget(m_mouseSpreadOutBtn);
    m_mouseBtnStackedWidget->addWidget(m_mouseSpreadInBtn);
    m_mouseBtnStackedWidget->setCurrentWidget(m_mouseSpreadOutBtn);

    m_mouseLayout->setContentsMargins(0, 0, 0, 0);
    m_mouseLayout->setSpacing(0);
    m_mouseLayout->addWidget(m_mouseIconLabel);
    m_mouseLayout->addSpacing(12);
    m_mouseLayout->addWidget(m_mouseTextLabel);
    m_mouseLayout->addWidget(m_mouseBtnStackedWidget, 3, Qt::AlignRight);
    m_mouseWidget->setLayout(m_mouseLayout);

    QTreeWidgetItem *rootItem10 = new QTreeWidgetItem(m_TreeWidget);
    rootItem10->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem10);
    m_TreeWidget->setItemWidget(rootItem10, 0, m_mouseWidget);

    // Bluetooth
    m_bluetoothIconLabel->setPixmap(QIcon::fromTheme("input-keyboard-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
    m_bluetoothIconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);

    m_bluetoothTextLabel->setText(tr("Bluetooth"));
    m_bluetoothTextLabel->setFixedWidth(TREEITEMLABELWIDTH);

    m_bluetoothSpreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_bluetoothSpreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_bluetoothBtnStackedWidget->addWidget(m_bluetoothSpreadOutBtn);
    m_bluetoothBtnStackedWidget->addWidget(m_bluetoothSpreadInBtn);
    m_bluetoothBtnStackedWidget->setCurrentWidget(m_bluetoothSpreadOutBtn);

    m_bluetoothLayout->setContentsMargins(0, 0, 0, 0);
    m_bluetoothLayout->setSpacing(0);
    m_bluetoothLayout->addWidget(m_bluetoothIconLabel);
    m_bluetoothLayout->addSpacing(12);
    m_bluetoothLayout->addWidget(m_bluetoothTextLabel);
    m_bluetoothLayout->addWidget(m_bluetoothModelLabel);
    m_bluetoothLayout->addWidget(m_bluetoothBtnStackedWidget, 3, Qt::AlignRight);
    m_bluetoothWidget->setLayout(m_bluetoothLayout);

    QTreeWidgetItem *rootItem11 = new QTreeWidgetItem(m_TreeWidget);
    rootItem11->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->addTopLevelItem(rootItem11);
    m_TreeWidget->setItemWidget(rootItem11, 0, m_bluetoothWidget);

    // 主体布局
    m_HWDisplayLayout->setContentsMargins(0, 0, 40, 0);
    m_HWDisplayLayout->setSpacing(0);
    m_HWDisplayLayout->addWidget(m_TreeWidget);
    m_HWDisplayWidget->setLayout(m_HWDisplayLayout);
}

void HardWareInfoWidget::initConnection() {
    // gsettings
    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &HardWareInfoWidget::themeFontStyle);
    // 信息获取成功后的信息填充
    connect(g_signal, &GlobalSignal::getHwInfoComplete, this, &HardWareInfoWidget::setContent);

    // CPU Btn
    connect(m_cpuSpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_cpuBtnStackedWidget->setCurrentWidget(m_cpuSpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (cpuItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(0)->setExpanded(true);
    });

    connect(m_cpuSpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_cpuBtnStackedWidget->setCurrentWidget(m_cpuSpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (cpuItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(0)->setExpanded(false);
    });

    // Memory Btn
    connect(m_memorySpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_memoryBtnStackedWidget->setCurrentWidget(m_memorySpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (memoryItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(1)->setExpanded(true);
    });

    connect(m_memorySpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_memoryBtnStackedWidget->setCurrentWidget(m_memorySpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (memoryItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(1)->setExpanded(false);
    });

    // Graphic Card Btn
    connect(m_graphicCardSpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_graphicCardBtnStackedWidget->setCurrentWidget(m_graphicCardSpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (graphicCardItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(2)->setExpanded(true);
    });

    connect(m_graphicCardSpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_graphicCardBtnStackedWidget->setCurrentWidget(m_graphicCardSpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (graphicCardItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(2)->setExpanded(false);
    });

    // BaseBoard Btn
    connect(m_baseboardSpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_baseboardBtnStackedWidget->setCurrentWidget(m_baseboardSpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (baseboardItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(3)->setExpanded(true);
    });

    connect(m_baseboardSpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_baseboardBtnStackedWidget->setCurrentWidget(m_baseboardSpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (baseboardItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(3)->setExpanded(false);
    });

    // Network Btn
    connect(m_networkCardSpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_networkCardBtnStackedWidget->setCurrentWidget(m_networkCardSpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (networkItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(4)->setExpanded(true);
    });

    connect(m_networkCardSpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_networkCardBtnStackedWidget->setCurrentWidget(m_networkCardSpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (networkItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(4)->setExpanded(false);
    });

    // Disk Btn
    connect(m_diskSpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_diskBtnStackedWidget->setCurrentWidget(m_diskSpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (diskItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(5)->setExpanded(true);
    });

    connect(m_diskSpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_diskBtnStackedWidget->setCurrentWidget(m_diskSpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (diskItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(5)->setExpanded(false);
    });

    // Monitor Btn
    connect(m_monitorSpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_monitorBtnStackedWidget->setCurrentWidget(m_monitorSpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (monitorItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(6)->setExpanded(true);
    });

    connect(m_monitorSpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_monitorBtnStackedWidget->setCurrentWidget(m_monitorSpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (monitorItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(6)->setExpanded(false);
    });

    // SoundCard Btn
    connect(m_soundCardSpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_soundCardBtnStackedWidget->setCurrentWidget(m_soundCardSpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (soundCardItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(7)->setExpanded(true);
    });

    connect(m_soundCardSpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_soundCardBtnStackedWidget->setCurrentWidget(m_soundCardSpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (soundCardItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(7)->setExpanded(false);
    });

    // Keyboard Btn
    connect(m_keyboardSpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_keyboardBtnStackedWidget->setCurrentWidget(m_keyboardSpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (keyboardItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(8)->setExpanded(true);
    });

    connect(m_keyboardSpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_keyboardBtnStackedWidget->setCurrentWidget(m_keyboardSpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (keyboardItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(8)->setExpanded(false);
    });

    // Mouse Btn
    connect(m_mouseSpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_mouseBtnStackedWidget->setCurrentWidget(m_mouseSpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (mouseItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(9)->setExpanded(true);
    });

    connect(m_mouseSpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_mouseBtnStackedWidget->setCurrentWidget(m_mouseSpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (mouseItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(9)->setExpanded(false);
    });

    // Bluetooth Btn
    connect(m_bluetoothSpreadOutBtn, &QAbstractButton::clicked, [=]() {
        m_bluetoothBtnStackedWidget->setCurrentWidget(m_bluetoothSpreadInBtn);
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (bluetoothItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(10)->setExpanded(true);
    });

    connect(m_bluetoothSpreadInBtn, &QAbstractButton::clicked, [=]() {
        m_bluetoothBtnStackedWidget->setCurrentWidget(m_bluetoothSpreadOutBtn);
        m_TreeWidget->itemCollapsed(m_TreeWidget->itemAt(0, 0));
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (bluetoothItemCount * TREEWIDGETITEMSIZEHINT.height()));
        m_TreeWidget->topLevelItem(10)->setExpanded(false);
    });
}

void HardWareInfoWidget::themeFontStyle() {
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
    m_kernelVersionLabel2->setText(g_infogetter->m_pcinfo.kernelversion);
    m_archLabel2->setText(g_infogetter->m_pcinfo.arch);
    m_sysBitLabel2->setText(g_infogetter->m_pcinfo.sysbits);
    m_manufactorLabel2->setText(g_infogetter->m_pcinfo.manufacturer);
    m_modelLabel2->setText(g_infogetter->m_pcinfo.model);
    m_serialLabel2->setText(g_infogetter->m_pcinfo.serial);
}

void HardWareInfoWidget::getCpuInfo() {
    // 设置内容
    m_cpuModelLabel->setText(g_infogetter->m_cpuinfo.model);

    TreeItem *item1 = new TreeItem(tr("Manufacturer"), g_infogetter->m_cpuinfo.vendor);
    QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
    childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(0)->addChild(childItem1);
    m_TreeWidget->setItemWidget(childItem1, 0, item1);

    TreeItem *item2 = new TreeItem(tr("Arch"), g_infogetter->m_cpuinfo.arch);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(0)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    TreeItem *item3 = new TreeItem(tr("Cores"), g_infogetter->m_cpuinfo.cores);
    QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
    childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(0)->addChild(childItem3);
    m_TreeWidget->setItemWidget(childItem3, 0, item3);

    TreeItem *item4 = new TreeItem(tr("Online Cores"), g_infogetter->m_cpuinfo.onlinecores);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(0)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);

    TreeItem *item5 = new TreeItem(tr("Thread Per Cores"), g_infogetter->m_cpuinfo.threadprecore);
    QTreeWidgetItem *childItem5 = new QTreeWidgetItem();
    childItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(0)->addChild(childItem5);
    m_TreeWidget->setItemWidget(childItem5, 0, item5);

    TreeItem *item6 = new TreeItem(tr("MAX MHz"), g_infogetter->m_cpuinfo.maxMHz);
    QTreeWidgetItem *childItem6 = new QTreeWidgetItem();
    childItem6->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(0)->addChild(childItem6);
    m_TreeWidget->setItemWidget(childItem6, 0, item6);

    TreeItem *item7 = new TreeItem(tr("L1d Cache"), g_infogetter->m_cpuinfo.L1dcache);
    QTreeWidgetItem *childItem7 = new QTreeWidgetItem();
    childItem7->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(0)->addChild(childItem7);
    m_TreeWidget->setItemWidget(childItem7, 0, item7);

    TreeItem *item8 = new TreeItem(tr("L1i Cache"), g_infogetter->m_cpuinfo.L1icache);
    QTreeWidgetItem *childItem8 = new QTreeWidgetItem();
    childItem8->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(0)->addChild(childItem8);
    m_TreeWidget->setItemWidget(childItem8, 0, item8);

    TreeItem *item9 = new TreeItem(tr("L2 Cache"), g_infogetter->m_cpuinfo.L2cache);
    QTreeWidgetItem *childItem9 = new QTreeWidgetItem();
    childItem9->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(0)->addChild(childItem9);
    m_TreeWidget->setItemWidget(childItem9, 0, item9);

    TreeItem *item10 = new TreeItem(tr("L3 Cache"), g_infogetter->m_cpuinfo.L3cache);
    QTreeWidgetItem *childItem10 = new QTreeWidgetItem();
    childItem10->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(0)->addChild(childItem10);
    m_TreeWidget->setItemWidget(childItem10, 0, item10);

    cpuItemCount = 10;
}


void HardWareInfoWidget::getMemoryInfo() {
    if (g_infogetter->m_memoryinfoList.length() == 0) {
        return;
    }

    for (int i = 0; i < g_infogetter->m_memoryinfoList.length(); ++i) {
        kdk::KBorderlessButton *btn = new kdk::KBorderlessButton();
        btn->setText(g_infogetter->m_memoryinfoList[i].displayname);
        m_memoryLayout->insertWidget(3, btn, 1, Qt::AlignLeft);
        if (i != (g_infogetter->m_memoryinfoList.length() - 1)) {
            QLabel *label = new QLabel();
            label->setText("/");
            m_memoryLayout->insertWidget(3, label);
        }
        connect(btn, &QAbstractButton::clicked, [=]() { setMemoryContent(i); });
    }
    // 默认显示最后一个加入的设备
    int num = g_infogetter->m_memoryinfoList.length() - 1;

    TreeItem *item1 = new TreeItem(tr("Bank Locator"), g_infogetter->m_memoryinfoList[num].banklocator);
    m_memoryTreeItemList.append(item1);
    QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
    childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem1);
    m_TreeWidget->setItemWidget(childItem1, 0, item1);

    TreeItem *item2 = new TreeItem(tr("Total Width"), g_infogetter->m_memoryinfoList[num].totalwidth);
    m_memoryTreeItemList.append(item2);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    TreeItem *item3 = new TreeItem(tr("Size"), g_infogetter->m_memoryinfoList[num].size);
    m_memoryTreeItemList.append(item3);
    QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
    childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem3);
    m_TreeWidget->setItemWidget(childItem3, 0, item3);

    TreeItem *item4 = new TreeItem(tr("Serial Number"), g_infogetter->m_memoryinfoList[num].serial);
    m_memoryTreeItemList.append(item4);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);

    TreeItem *item5 = new TreeItem(tr("Manufacturer"), g_infogetter->m_memoryinfoList[num].verdor);
    m_memoryTreeItemList.append(item5);
    QTreeWidgetItem *childItem5 = new QTreeWidgetItem();
    childItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem5);
    m_TreeWidget->setItemWidget(childItem5, 0, item5);

    TreeItem *item6 = new TreeItem(tr("Data Width"), g_infogetter->m_memoryinfoList[num].datawidth);
    m_memoryTreeItemList.append(item6);
    QTreeWidgetItem *childItem6 = new QTreeWidgetItem();
    childItem6->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem6);
    m_TreeWidget->setItemWidget(childItem6, 0, item6);

    TreeItem *item7 = new TreeItem(tr("Type"), g_infogetter->m_memoryinfoList[num].type);
    m_memoryTreeItemList.append(item7);
    QTreeWidgetItem *childItem7 = new QTreeWidgetItem();
    childItem7->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem7);
    m_TreeWidget->setItemWidget(childItem7, 0, item7);

    TreeItem *item8 = new TreeItem(tr("Speed"), g_infogetter->m_memoryinfoList[num].speed);
    m_memoryTreeItemList.append(item8);
    QTreeWidgetItem *childItem8 = new QTreeWidgetItem();
    childItem8->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem8);
    m_TreeWidget->setItemWidget(childItem8, 0, item8);

    TreeItem *item9 = new TreeItem(tr("Part Number"), g_infogetter->m_memoryinfoList[num].partnumber);
    m_memoryTreeItemList.append(item9);
    QTreeWidgetItem *childItem9 = new QTreeWidgetItem();
    childItem9->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem9);
    m_TreeWidget->setItemWidget(childItem9, 0, item9);

    TreeItem *item10 = new TreeItem(tr("Memory Usage"), "0%");
    m_memoryTreeItemList.append(item10);
    QTreeWidgetItem *childItem10 = new QTreeWidgetItem();
    childItem10->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem10);
    m_TreeWidget->setItemWidget(childItem10, 0, item10);

    memoryItemCount = 10;
}

void HardWareInfoWidget::setMemoryContent(int i) {
    m_memoryTreeItemList[0]->setLabelText(g_infogetter->m_memoryinfoList[i].banklocator);
    m_memoryTreeItemList[1]->setLabelText(g_infogetter->m_memoryinfoList[i].totalwidth);
    m_memoryTreeItemList[2]->setLabelText(g_infogetter->m_memoryinfoList[i].size);
    m_memoryTreeItemList[3]->setLabelText(g_infogetter->m_memoryinfoList[i].serial);
    m_memoryTreeItemList[4]->setLabelText(g_infogetter->m_memoryinfoList[i].verdor);
    m_memoryTreeItemList[5]->setLabelText(g_infogetter->m_memoryinfoList[i].datawidth);
    m_memoryTreeItemList[6]->setLabelText(g_infogetter->m_memoryinfoList[i].type);
    m_memoryTreeItemList[7]->setLabelText(g_infogetter->m_memoryinfoList[i].speed);
    m_memoryTreeItemList[8]->setLabelText(g_infogetter->m_memoryinfoList[i].partnumber);
}

void HardWareInfoWidget::getGraphicCardInfo() {
    if (g_infogetter->m_graphicHardwares.length() == 0) {
        return;
    }

    for (int i = 0; i < g_infogetter->m_graphicHardwares.length(); ++i) {
        kdk::KBorderlessButton *btn = new kdk::KBorderlessButton();
        btn->setText(g_infogetter->m_graphicHardwares[i].model);
        m_graphicCardLayout->insertWidget(3, btn, 1, Qt::AlignLeft);
        if (i != (g_infogetter->m_graphicHardwares.length() - 1)) {
            QLabel *label = new QLabel();
            label->setText("/");
            m_graphicCardLayout->insertWidget(3, label);
        }
        connect(btn, &QAbstractButton::clicked, [=]() { setGraphicContent(i); });
    }
    // 默认显示最后一个加入的设备
    int num = g_infogetter->m_graphicHardwares.length() - 1;

    TreeItem *item1 = new TreeItem(tr("Manufacturer"), g_infogetter->m_graphicHardwares[num].vendor);
    m_graphicCardTreeItemList.append(item1);
    QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
    childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(2)->addChild(childItem1);
    m_TreeWidget->setItemWidget(childItem1, 0, item1);

    TreeItem *item2 = new TreeItem(tr("Sub System Device ID"), g_infogetter->m_graphicHardwares[num].subsystem);
    m_graphicCardTreeItemList.append(item2);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(2)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    TreeItem *item3 = new TreeItem(tr("model"), g_infogetter->m_graphicHardwares[num].model);
    m_graphicCardTreeItemList.append(item3);
    QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
    childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(2)->addChild(childItem3);
    m_TreeWidget->setItemWidget(childItem3, 0, item3);

    TreeItem *item4 = new TreeItem(tr("Memory"), g_infogetter->m_graphicHardwares[num].memorysize);
    m_graphicCardTreeItemList.append(item4);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);

    TreeItem *item5 = new TreeItem(tr("Bus Info"), g_infogetter->m_graphicHardwares[num].businfo);
    m_graphicCardTreeItemList.append(item5);
    QTreeWidgetItem *childItem5 = new QTreeWidgetItem();
    childItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(2)->addChild(childItem5);
    m_TreeWidget->setItemWidget(childItem5, 0, item5);

    TreeItem *item6 = new TreeItem(tr("Clock"), g_infogetter->m_graphicHardwares[num].clock);
    m_graphicCardTreeItemList.append(item6);
    QTreeWidgetItem *childItem6 = new QTreeWidgetItem();
    childItem6->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(2)->addChild(childItem6);
    m_TreeWidget->setItemWidget(childItem6, 0, item6);

    TreeItem *item7 = new TreeItem(tr("Physics ID"), g_infogetter->m_graphicHardwares[num].physid);
    m_graphicCardTreeItemList.append(item7);
    QTreeWidgetItem *childItem7 = new QTreeWidgetItem();
    childItem7->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(2)->addChild(childItem7);
    m_TreeWidget->setItemWidget(childItem7, 0, item7);

    TreeItem *item8 = new TreeItem(tr("Version"), g_infogetter->m_graphicHardwares[num].version);
    m_graphicCardTreeItemList.append(item8);
    QTreeWidgetItem *childItem8 = new QTreeWidgetItem();
    childItem8->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(2)->addChild(childItem8);
    m_TreeWidget->setItemWidget(childItem8, 0, item8);

    TreeItem *item9 = new TreeItem(tr("Driver"), g_infogetter->m_graphicHardwares[num].driver_now);
    m_graphicCardTreeItemList.append(item9);
    QTreeWidgetItem *childItem9 = new QTreeWidgetItem();
    childItem9->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(2)->addChild(childItem9);
    m_TreeWidget->setItemWidget(childItem9, 0, item9);

    TreeItem *item10 = new TreeItem(tr("Width"), g_infogetter->m_graphicHardwares[num].width);
    m_graphicCardTreeItemList.append(item10);
    QTreeWidgetItem *childItem10 = new QTreeWidgetItem();
    childItem10->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(2)->addChild(childItem10);
    m_TreeWidget->setItemWidget(childItem10, 0, item10);

    graphicCardItemCount = 10;
}

void HardWareInfoWidget::setGraphicContent(int i) {
    m_graphicCardTreeItemList[0]->setLabelText(g_infogetter->m_graphicHardwares[i].vendor);
    m_graphicCardTreeItemList[1]->setLabelText(g_infogetter->m_graphicHardwares[i].subsystem);
    m_graphicCardTreeItemList[2]->setLabelText(g_infogetter->m_graphicHardwares[i].model);
    m_graphicCardTreeItemList[3]->setLabelText(g_infogetter->m_graphicHardwares[i].memorysize);
    m_graphicCardTreeItemList[4]->setLabelText(g_infogetter->m_graphicHardwares[i].businfo);
    m_graphicCardTreeItemList[5]->setLabelText(g_infogetter->m_graphicHardwares[i].clock);
    m_graphicCardTreeItemList[6]->setLabelText(g_infogetter->m_graphicHardwares[i].physid);
    m_graphicCardTreeItemList[7]->setLabelText(g_infogetter->m_graphicHardwares[i].version);
    m_graphicCardTreeItemList[8]->setLabelText(g_infogetter->m_graphicHardwares[i].driver_now);
    m_graphicCardTreeItemList[9]->setLabelText(g_infogetter->m_graphicHardwares[i].width);
}

void HardWareInfoWidget::getBaseboardInfo() {

    // 设置内容
    if (g_infogetter->m_baseboardinfo.model != "") {
        m_baseboardModelLabel->setText(g_infogetter->m_baseboardinfo.model);
    }

    if (g_infogetter->m_baseboardinfo.vendor != "") {
        TreeItem *item1 = new TreeItem(tr("Manufacturer"), g_infogetter->m_baseboardinfo.vendor);
        QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
        childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(3)->addChild(childItem1);
        m_TreeWidget->setItemWidget(childItem1, 0, item1);
    }

    if (g_infogetter->m_baseboardinfo.version != "") {
        TreeItem *item2 = new TreeItem(tr("Version"), g_infogetter->m_baseboardinfo.version);
        QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
        childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(3)->addChild(childItem2);
        m_TreeWidget->setItemWidget(childItem2, 0, item2);
    }

    if (g_infogetter->m_baseboardinfo.chassishandle != "") {
        TreeItem *item3 = new TreeItem(tr("Chassis Handle"), g_infogetter->m_baseboardinfo.chassishandle);
        QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
        childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(3)->addChild(childItem3);
        m_TreeWidget->setItemWidget(childItem3, 0, item3);
    }

    if (g_infogetter->m_baseboardinfo.biosvendor != "") {
        TreeItem *item4 = new TreeItem(tr("Bios Vendor"), g_infogetter->m_baseboardinfo.biosvendor);
        QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
        childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(3)->addChild(childItem4);
        m_TreeWidget->setItemWidget(childItem4, 0, item4);
    }

    if (g_infogetter->m_baseboardinfo.biosversion != "") {
        TreeItem *item5 = new TreeItem(tr("Bios Version"), g_infogetter->m_baseboardinfo.biosversion);
        QTreeWidgetItem *childItem5 = new QTreeWidgetItem();
        childItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(3)->addChild(childItem5);
        m_TreeWidget->setItemWidget(childItem5, 0, item5);
    }

    baseboardItemCount = 5;
}

void HardWareInfoWidget::getNetworkCardInfo() {
    if (g_infogetter->m_networkCardHardwares.length() == 0) {
        return;
    }

    for (int i = 0; i < g_infogetter->m_networkCardHardwares.length(); ++i) {
        kdk::KBorderlessButton *btn = new kdk::KBorderlessButton();
        btn->setText(g_infogetter->m_networkCardHardwares[i].name);
        m_networkCardLayout->insertWidget(3, btn, 1, Qt::AlignLeft);
        if (i != (g_infogetter->m_networkCardHardwares.length() - 1)) {
            QLabel *label = new QLabel();
            label->setText("/");
            m_networkCardLayout->insertWidget(3, label);
        }
        connect(btn, &QAbstractButton::clicked, [=]() { setNetworkCardContent(i); });
    }
    // 默认显示最后一个加入的设备
    int num = g_infogetter->m_networkCardHardwares.length() - 1;

    if (g_infogetter->m_networkCardHardwares[num].vendor != "") {
        TreeItem *item1 = new TreeItem(tr("Manufacturer"), g_infogetter->m_networkCardHardwares[num].vendor);
        m_networkCardTreeItemList.append(item1);
        QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
        childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(4)->addChild(childItem1);
        m_TreeWidget->setItemWidget(childItem1, 0, item1);
        networkItemCount++;
    }


    TreeItem *item2 = new TreeItem(tr("Model"), g_infogetter->m_networkCardHardwares[num].model);
    m_networkCardTreeItemList.append(item2);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(4)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    if (g_infogetter->m_networkCardHardwares[num].bus != "") {
        TreeItem *item3 = new TreeItem(tr("Bus"), g_infogetter->m_networkCardHardwares[num].bus);
        m_networkCardTreeItemList.append(item3);
        QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
        childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(4)->addChild(childItem3);
        m_TreeWidget->setItemWidget(childItem3, 0, item3);
        networkItemCount++;
    }

    TreeItem *item4 = new TreeItem(tr("Driver"), g_infogetter->m_networkCardHardwares[num].driver_now);
    m_networkCardTreeItemList.append(item4);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(1)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);

    TreeItem *item5 = new TreeItem(tr("MAC Address"), g_infogetter->m_networkCardHardwares[num].MAC_addr);
    m_networkCardTreeItemList.append(item5);
    QTreeWidgetItem *childItem5 = new QTreeWidgetItem();
    childItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(4)->addChild(childItem5);
    m_TreeWidget->setItemWidget(childItem5, 0, item5);


    TreeItem *item6 = new TreeItem(tr("Type"), tr("Wireless"));  // 无线
    if (g_infogetter->m_networkCardHardwares[num].isWireless != 1) {
        item6->setLabelText(tr("Wired"));     // 有线
    }
    m_networkCardTreeItemList.append(item6);
    QTreeWidgetItem *childItem6 = new QTreeWidgetItem();
    childItem6->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(4)->addChild(childItem6);
    m_TreeWidget->setItemWidget(childItem6, 0, item6);

    if (g_infogetter->m_networkCardHardwares[num].ip_addr != "") {
        TreeItem *item7 = new TreeItem(tr("IP Address"), g_infogetter->m_networkCardHardwares[num].ip_addr);
        m_networkCardTreeItemList.append(item7);
        QTreeWidgetItem *childItem7 = new QTreeWidgetItem();
        childItem7->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(4)->addChild(childItem7);
        m_TreeWidget->setItemWidget(childItem7, 0, item7);
        networkItemCount++;
    }
}

void HardWareInfoWidget::setNetworkCardContent(int i) {
    int childNum = m_TreeWidget->topLevelItem(4)->childCount();
    for (int j = 0; j < childNum; ++j) {
        m_TreeWidget->topLevelItem(4)->takeChildren();
    }
    if (m_TreeWidget->topLevelItem(4)->isExpanded()) {
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() - (networkItemCount * TREEWIDGETITEMSIZEHINT.height()));
    }

    networkItemCount = 4;
    if (g_infogetter->m_networkCardHardwares[i].vendor != "") {
        TreeItem *item1 = new TreeItem(tr("Manufacturer"), g_infogetter->m_networkCardHardwares[i].vendor);
        m_networkCardTreeItemList.append(item1);
        QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
        childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(4)->addChild(childItem1);
        m_TreeWidget->setItemWidget(childItem1, 0, item1);
        networkItemCount++;
    }


    TreeItem *item2 = new TreeItem(tr("Model"), g_infogetter->m_networkCardHardwares[i].model);
    m_networkCardTreeItemList.append(item2);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(4)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    if (g_infogetter->m_networkCardHardwares[i].bus != "") {
        TreeItem *item3 = new TreeItem(tr("Bus"), g_infogetter->m_networkCardHardwares[i].bus);
        m_networkCardTreeItemList.append(item3);
        QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
        childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(4)->addChild(childItem3);
        m_TreeWidget->setItemWidget(childItem3, 0, item3);
        networkItemCount++;
    }

    TreeItem *item4 = new TreeItem(tr("Driver"), g_infogetter->m_networkCardHardwares[i].driver_now);
    m_networkCardTreeItemList.append(item4);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(4)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);

    TreeItem *item5 = new TreeItem(tr("MAC Address"), g_infogetter->m_networkCardHardwares[i].MAC_addr);
    m_networkCardTreeItemList.append(item5);
    QTreeWidgetItem *childItem5 = new QTreeWidgetItem();
    childItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(4)->addChild(childItem5);
    m_TreeWidget->setItemWidget(childItem5, 0, item5);


    TreeItem *item6 = new TreeItem(tr("Type"), tr("Wireless"));  // 无线
    if (g_infogetter->m_networkCardHardwares[i].isWireless != 1) {
        item6->setLabelText(tr("Wired"));     // 有线
    }
    m_networkCardTreeItemList.append(item6);
    QTreeWidgetItem *childItem6 = new QTreeWidgetItem();
    childItem6->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(4)->addChild(childItem6);
    m_TreeWidget->setItemWidget(childItem6, 0, item6);

    if (g_infogetter->m_networkCardHardwares[i].ip_addr != "") {
        TreeItem *item7 = new TreeItem(tr("IP Address"), g_infogetter->m_networkCardHardwares[i].ip_addr);
        m_networkCardTreeItemList.append(item7);
        QTreeWidgetItem *childItem7 = new QTreeWidgetItem();
        childItem7->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
        m_TreeWidget->topLevelItem(4)->addChild(childItem7);
        m_TreeWidget->setItemWidget(childItem7, 0, item7);
        networkItemCount++;
    }

    if (m_TreeWidget->topLevelItem(4)->isExpanded()) {
        m_TreeWidget->setFixedHeight(m_TreeWidget->height() + (networkItemCount * TREEWIDGETITEMSIZEHINT.height()));
    }
}

void HardWareInfoWidget::getDiskCardInfo() {
    if (g_infogetter->m_diskinfoList.length() == 0) {
        return;
    }
    for (int i = 0; i < g_infogetter->m_diskinfoList.length(); ++i) {
        kdk::KBorderlessButton *btn = new kdk::KBorderlessButton();
        btn->setText(QString(("Disk ") + QString::number(i)));
        m_diskLayout->insertWidget(3, btn, 1, Qt::AlignLeft);
        if (i != (g_infogetter->m_diskinfoList.length() - 1)) {
            QLabel *label = new QLabel();
            label->setText("/");
            m_diskLayout->insertWidget(3, label);
        }
        connect(btn, &QAbstractButton::clicked, [=]() { setDiskContent(i); });
    }
    // 默认显示最后一个加入的设备
    int num = g_infogetter->m_diskinfoList.length() - 1;

    TreeItem *item1 = new TreeItem(tr("Model"), g_infogetter->m_diskinfoList[num].model);
    m_diskTreeItemList.append(item1);
    QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
    childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(5)->addChild(childItem1);
    m_TreeWidget->setItemWidget(childItem1, 0, item1);

    TreeItem *item2 = new TreeItem(tr("Size"), g_infogetter->m_diskinfoList[num].size);
    m_diskTreeItemList.append(item2);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(5)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    TreeItem *item3 = new TreeItem(tr("Serial Number"), g_infogetter->m_diskinfoList[num].serial);
    m_diskTreeItemList.append(item3);
    QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
    childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(5)->addChild(childItem3);
    m_TreeWidget->setItemWidget(childItem3, 0, item3);

    TreeItem *item4 = new TreeItem(tr("Disk Name"), g_infogetter->m_diskinfoList[num].name);
    m_diskTreeItemList.append(item4);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(5)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);

    TreeItem *item5 = new TreeItem(tr("Frimwarever Version"), g_infogetter->m_diskinfoList[num].frimwareversion);
    m_diskTreeItemList.append(item5);
    QTreeWidgetItem *childItem5 = new QTreeWidgetItem();
    childItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(5)->addChild(childItem5);
    m_TreeWidget->setItemWidget(childItem5, 0, item5);

    QString typestr;
    if (g_infogetter->m_diskinfoList[num].type == 0) {
        typestr = "SSD";
    } else {
        typestr = "HDD";
    }
    TreeItem *item6 = new TreeItem(tr("Disk Type"), typestr);
    m_diskTreeItemList.append(item6);
    QTreeWidgetItem *childItem6 = new QTreeWidgetItem();
    childItem6->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(5)->addChild(childItem6);
    m_TreeWidget->setItemWidget(childItem6, 0, item6);

    diskItemCount = 6;
}

void HardWareInfoWidget::setDiskContent(int i) {
    m_diskTreeItemList[0]->setLabelText(g_infogetter->m_diskinfoList[i].model);
    m_diskTreeItemList[1]->setLabelText(g_infogetter->m_diskinfoList[i].size);
    m_diskTreeItemList[2]->setLabelText(g_infogetter->m_diskinfoList[i].serial);
    m_diskTreeItemList[3]->setLabelText(g_infogetter->m_diskinfoList[i].name);
    m_diskTreeItemList[4]->setLabelText(g_infogetter->m_diskinfoList[i].frimwareversion);
    QString typestr;
    if (g_infogetter->m_diskinfoList[i].type == 0) {
        typestr = "SSD";
    } else {
        typestr = "HDD";
    }
    m_memoryTreeItemList[5]->setLabelText(typestr);
}

void HardWareInfoWidget::getMonitorInfo() {
    if (g_infogetter->m_monitorList.length() == 0) {
        return;
    }
    for (int i = 0; i < g_infogetter->m_monitorList.length(); ++i) {
        kdk::KBorderlessButton *btn = new kdk::KBorderlessButton();
        btn->setText(QString(("monitor ") + QString::number(i)));
        m_monitorLayout->insertWidget(3, btn, 1, Qt::AlignLeft);
        if (i != (g_infogetter->m_monitorList.length() - 1)) {
            QLabel *label = new QLabel();
            label->setText("/");
            m_monitorLayout->insertWidget(3, label);
        }
        connect(btn, &QAbstractButton::clicked, [=]() { setMonitorContent(i); });
    }
    // 默认显示最后一个加入的设备
    int num = g_infogetter->m_monitorList.length() - 1;

    TreeItem *item1 = new TreeItem(tr("Manufacturer"), g_infogetter->m_monitorList[num].vendor);
    m_monitorTreeItemList.append(item1);
    QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
    childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem1);
    m_TreeWidget->setItemWidget(childItem1, 0, item1);

    TreeItem *item2 = new TreeItem(tr("Model"), g_infogetter->m_monitorList[num].name);
    m_monitorTreeItemList.append(item2);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    TreeItem *item3 = new TreeItem(tr("Monitor Size"), g_infogetter->m_monitorList[num].size);
    m_monitorTreeItemList.append(item3);
    QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
    childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem3);
    m_TreeWidget->setItemWidget(childItem3, 0, item3);

    TreeItem *item4 = new TreeItem(tr("Monitor Ratio"), g_infogetter->m_monitorList[num].ratio);
    m_monitorTreeItemList.append(item4);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);

    TreeItem *item5 = new TreeItem(tr("Resolution"), g_infogetter->m_monitorList[num].resolution);
    m_monitorTreeItemList.append(item5);
    QTreeWidgetItem *childItem5 = new QTreeWidgetItem();
    childItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem5);
    m_TreeWidget->setItemWidget(childItem5, 0, item5);

    TreeItem *item6 = new TreeItem(tr("Max Resolution"), g_infogetter->m_monitorList[num].max_resolution);
    m_monitorTreeItemList.append(item6);
    QTreeWidgetItem *childItem6 = new QTreeWidgetItem();
    childItem6->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem6);
    m_TreeWidget->setItemWidget(childItem6, 0, item6);

    TreeItem *item7 = new TreeItem(tr("Main Monitor"),
                                   (g_infogetter->m_monitorList[num].is_main == "1") ? tr("Yes") : tr("No"));
    m_monitorTreeItemList.append(item7);
    QTreeWidgetItem *childItem7 = new QTreeWidgetItem();
    childItem7->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem7);
    m_TreeWidget->setItemWidget(childItem7, 0, item7);

    TreeItem *item8 = new TreeItem(tr("Interface"), g_infogetter->m_monitorList[num].interface);
    m_monitorTreeItemList.append(item8);
    QTreeWidgetItem *childItem8 = new QTreeWidgetItem();
    childItem8->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem8);
    m_TreeWidget->setItemWidget(childItem8, 0, item8);

    TreeItem *item9 = new TreeItem(tr("Visible Area"), g_infogetter->m_monitorList[num].visible_area);
    m_monitorTreeItemList.append(item9);
    QTreeWidgetItem *childItem9 = new QTreeWidgetItem();
    childItem9->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem9);
    m_TreeWidget->setItemWidget(childItem9, 0, item9);

    TreeItem *item10 = new TreeItem(tr("Production week"), g_infogetter->m_monitorList[num].week);
    m_monitorTreeItemList.append(item10);
    QTreeWidgetItem *childItem10 = new QTreeWidgetItem();
    childItem10->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem10);
    m_TreeWidget->setItemWidget(childItem10, 0, item10);

    TreeItem *item11 = new TreeItem(tr("Production Year"), g_infogetter->m_monitorList[num].year);
    m_monitorTreeItemList.append(item11);
    QTreeWidgetItem *childItem11 = new QTreeWidgetItem();
    childItem11->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem11);
    m_TreeWidget->setItemWidget(childItem11, 0, item11);

    TreeItem *item12 = new TreeItem(tr("Gamma"), g_infogetter->m_monitorList[num].gamma);
    m_monitorTreeItemList.append(item12);
    QTreeWidgetItem *childItem12 = new QTreeWidgetItem();
    childItem12->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(6)->addChild(childItem12);
    m_TreeWidget->setItemWidget(childItem12, 0, item12);

    monitorItemCount = 12;
}

void HardWareInfoWidget::setMonitorContent(int i) {
    m_monitorTreeItemList[0]->setLabelText(g_infogetter->m_monitorList[i].vendor);
    m_monitorTreeItemList[1]->setLabelText(g_infogetter->m_monitorList[i].name);
    m_monitorTreeItemList[2]->setLabelText(g_infogetter->m_monitorList[i].size);
    m_monitorTreeItemList[3]->setLabelText(g_infogetter->m_monitorList[i].ratio);
    m_monitorTreeItemList[4]->setLabelText(g_infogetter->m_monitorList[i].resolution);
    m_monitorTreeItemList[5]->setLabelText(g_infogetter->m_monitorList[i].max_resolution);
    m_monitorTreeItemList[6]->setLabelText((g_infogetter->m_monitorList[i].is_main == "1") ? tr("Yes") : tr("No"));
    m_monitorTreeItemList[7]->setLabelText(g_infogetter->m_monitorList[i].interface);
    m_monitorTreeItemList[8]->setLabelText(g_infogetter->m_monitorList[i].visible_area);
    m_monitorTreeItemList[9]->setLabelText(g_infogetter->m_monitorList[i].week);
    m_monitorTreeItemList[10]->setLabelText(g_infogetter->m_monitorList[i].year);
    m_monitorTreeItemList[11]->setLabelText(g_infogetter->m_monitorList[i].gamma);
}

void HardWareInfoWidget::getSoundCardInfo() {
    if (g_infogetter->m_soundCardHardwares.length() == 0) {
        return;
    }
    // 设置内容
    m_soundCardModelLabel->setText(g_infogetter->m_soundCardHardwares[0].model);

    TreeItem *item1 = new TreeItem(tr("Manufacturer"), g_infogetter->m_soundCardHardwares[0].vendor);
    QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
    childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(7)->addChild(childItem1);
    m_TreeWidget->setItemWidget(childItem1, 0, item1);

    TreeItem *item2 = new TreeItem(tr("Bus Address"), g_infogetter->m_soundCardHardwares[0].bus);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(7)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    TreeItem *item3 = new TreeItem(tr("Clock"), g_infogetter->m_soundCardHardwares[0].clock);
    QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
    childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(7)->addChild(childItem3);
    m_TreeWidget->setItemWidget(childItem3, 0, item3);

    TreeItem *item4 = new TreeItem(tr("Driver"), g_infogetter->m_soundCardHardwares[0].driver_now);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(7)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);

    TreeItem *item5 = new TreeItem(tr("Width"), g_infogetter->m_soundCardHardwares[0].width);
    QTreeWidgetItem *childItem5 = new QTreeWidgetItem();
    childItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(7)->addChild(childItem5);
    m_TreeWidget->setItemWidget(childItem5, 0, item5);

    baseboardItemCount = 5;
}

void HardWareInfoWidget::getKeyboardInfo() {
    if (g_infogetter->m_keyboardInfoList.length() == 0) {
        return;
    }
    for (int i = 0; i < g_infogetter->m_keyboardInfoList.length(); ++i) {
        kdk::KBorderlessButton *btn = new kdk::KBorderlessButton();
        btn->setText(g_infogetter->m_keyboardInfoList[i].name);
        m_keyboardLayout->insertWidget(3, btn, 1, Qt::AlignLeft);
        if (i != (g_infogetter->m_keyboardInfoList.length() - 1)) {
            QLabel *label = new QLabel();
            label->setText("/");
            m_keyboardLayout->insertWidget(3, label);
        }
        connect(btn, &QAbstractButton::clicked, [=]() { setKeyboardContent(i); });
    }
    // 默认显示最后一个加入的设备
    int num = g_infogetter->m_keyboardInfoList.length() - 1;

    TreeItem *item1 = new TreeItem(tr("Manufacturer"), g_infogetter->m_keyboardInfoList[num].vendor);
    m_keyboardTreeItemList.append(item1);
    QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
    childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(8)->addChild(childItem1);
    m_TreeWidget->setItemWidget(childItem1, 0, item1);

    TreeItem *item2 = new TreeItem(tr("Model"), g_infogetter->m_keyboardInfoList[num].model);
    m_keyboardTreeItemList.append(item2);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(8)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    TreeItem *item3 = new TreeItem(tr("Interface"), g_infogetter->m_keyboardInfoList[num].interface);
    m_keyboardTreeItemList.append(item3);
    QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
    childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(8)->addChild(childItem3);
    m_TreeWidget->setItemWidget(childItem3, 0, item3);

    TreeItem *item4 = new TreeItem(tr("Driver"), g_infogetter->m_keyboardInfoList[num].driver);
    m_keyboardTreeItemList.append(item4);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(8)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);


    keyboardItemCount = 4;
}

void HardWareInfoWidget::setKeyboardContent(int i) {
    m_keyboardTreeItemList[0]->setLabelText(g_infogetter->m_keyboardInfoList[i].vendor);
    m_keyboardTreeItemList[1]->setLabelText(g_infogetter->m_keyboardInfoList[i].model);
    m_keyboardTreeItemList[2]->setLabelText(g_infogetter->m_keyboardInfoList[i].interface);
    m_keyboardTreeItemList[3]->setLabelText(g_infogetter->m_keyboardInfoList[i].driver);
}

void HardWareInfoWidget::getMouseInfo() {
    if (g_infogetter->m_mouseInfoList.length() == 0) {
        return;
    }
    for (int i = 0; i < g_infogetter->m_mouseInfoList.length(); ++i) {
        kdk::KBorderlessButton *btn = new kdk::KBorderlessButton();
        btn->setText(g_infogetter->m_mouseInfoList[i].name);
        m_mouseLayout->insertWidget(3, btn, 1, Qt::AlignLeft);
        if (i != (g_infogetter->m_mouseInfoList.length() - 1)) {
            QLabel *label = new QLabel();
            label->setText("/");
            m_mouseLayout->insertWidget(3, label);
        }
        connect(btn, &QAbstractButton::clicked, [=]() { setMouseContent(i); });
    }
    // 默认显示最后一个加入的设备
    int num = g_infogetter->m_mouseInfoList.length() - 1;

    TreeItem *item1 = new TreeItem(tr("Manufacturer"), g_infogetter->m_mouseInfoList[num].vendor);
    m_mouseTreeItemList.append(item1);
    QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
    childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(9)->addChild(childItem1);
    m_TreeWidget->setItemWidget(childItem1, 0, item1);

    TreeItem *item2 = new TreeItem(tr("Model"), g_infogetter->m_mouseInfoList[num].model);
    m_mouseTreeItemList.append(item2);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(9)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    TreeItem *item3 = new TreeItem(tr("Interface"), g_infogetter->m_mouseInfoList[num].interface);
    m_mouseTreeItemList.append(item3);
    QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
    childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(9)->addChild(childItem3);
    m_TreeWidget->setItemWidget(childItem3, 0, item3);

    TreeItem *item4 = new TreeItem(tr("Driver"), g_infogetter->m_mouseInfoList[num].driver);
    m_mouseTreeItemList.append(item4);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(9)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);


    mouseItemCount = 4;
}

void HardWareInfoWidget::setMouseContent(int i) {
    m_mouseTreeItemList[0]->setLabelText(g_infogetter->m_mouseInfoList[i].vendor);
    m_mouseTreeItemList[1]->setLabelText(g_infogetter->m_mouseInfoList[i].model);
    m_mouseTreeItemList[2]->setLabelText(g_infogetter->m_mouseInfoList[i].interface);
    m_mouseTreeItemList[3]->setLabelText(g_infogetter->m_mouseInfoList[i].driver);
}

void HardWareInfoWidget::getBluetoothInfo() {
    if (g_infogetter->m_bluetoothHardwares.length() == 0) {
        return;
    }
    // 设置内容
    m_bluetoothModelLabel->setText(g_infogetter->m_bluetoothHardwares[0].model);

    TreeItem *item1 = new TreeItem(tr("Manufacturer"), g_infogetter->m_bluetoothHardwares[0].vendor);
    QTreeWidgetItem *childItem1 = new QTreeWidgetItem();
    childItem1->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(10)->addChild(childItem1);
    m_TreeWidget->setItemWidget(childItem1, 0, item1);

    TreeItem *item2 = new TreeItem(tr("Driver"), g_infogetter->m_bluetoothHardwares[0].driver_now);
    QTreeWidgetItem *childItem2 = new QTreeWidgetItem();
    childItem2->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(10)->addChild(childItem2);
    m_TreeWidget->setItemWidget(childItem2, 0, item2);

    TreeItem *item3 = new TreeItem(tr("Driver Version"), g_infogetter->m_bluetoothHardwares[0].modeinfo.version);
    QTreeWidgetItem *childItem3 = new QTreeWidgetItem();
    childItem3->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(10)->addChild(childItem3);
    m_TreeWidget->setItemWidget(childItem3, 0, item3);

    TreeItem *item4 = new TreeItem(tr("Link Mode"), g_infogetter->m_bluetoothHardwares[0].link_mode);
    QTreeWidgetItem *childItem4 = new QTreeWidgetItem();
    childItem4->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(10)->addChild(childItem4);
    m_TreeWidget->setItemWidget(childItem4, 0, item4);

    TreeItem *item5 = new TreeItem(tr("Link Policy"), g_infogetter->m_bluetoothHardwares[0].link_policy);
    QTreeWidgetItem *childItem5 = new QTreeWidgetItem();
    childItem5->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
    m_TreeWidget->topLevelItem(10)->addChild(childItem5);
    m_TreeWidget->setItemWidget(childItem5, 0, item5);

    baseboardItemCount = 5;
}

void HardWareInfoWidget::setContent() {
    getPcInfo();
    getCpuInfo();
    getMemoryInfo();
    getGraphicCardInfo();
    getBaseboardInfo();
    getNetworkCardInfo();
    getDiskCardInfo();
    getMonitorInfo();
    getSoundCardInfo();
    getKeyboardInfo();
    getMouseInfo();
    getBluetoothInfo();
}

















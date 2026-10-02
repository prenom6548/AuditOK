#include "driveruninstallwidget.h"
#include <QPixmap>
#include <QPalette>

#include "include/common.h"
#include "include/globalsignal.h"
#include "hardwareinfogetter.h"

static bool driverExist(QString driverName);

DriverUninstallWidget::DriverUninstallWidget(QWidget *parent)
        : QWidget(parent),
          m_alertIconLabel(new QLabel()),
          m_alertLabel(new QLabel()),
          m_listWidget(new QListWidget()),
          m_labelLayout(new QHBoxLayout()),
          m_labelWidget(new QWidget()),
          m_mainLayout(new QVBoxLayout()) {
    initGui();
    initConnection();
}

DriverUninstallWidget::~DriverUninstallWidget() {

}

void DriverUninstallWidget::initGui() {
    m_alertIconLabel->setPixmap(QPixmap(":/warn.png").scaled(DEVICEICONSIZE));

    m_alertLabel->setText(tr("Note that after uninstalling the device driver, your device may not work properly."));
    QPalette palette;
    palette.setColor(QPalette::Text, Qt::red);
    m_alertLabel->setPalette(palette);

    m_listWidget->setSortingEnabled(true);
    m_listWidget->sortItems(Qt::DescendingOrder);
    m_listWidget->setViewMode(QListView::ListMode);
    m_listWidget->setFrameShape(QListWidget::NoFrame);                   //去掉list的边框
    m_listWidget->setStyleSheet(".QListWidget::item:hover{background:transparent;}");
    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection); // list取消原装的选中功能
    m_listWidget->setContentsMargins(0, 0, 0, 0);
    m_listWidget->setSpacing(0);

    m_labelLayout->setContentsMargins(0, 0, 0, 0);
    m_labelLayout->setSpacing(0);
    m_labelLayout->addWidget(m_alertIconLabel);
    m_labelLayout->addSpacing(8);
    m_labelLayout->addWidget(m_alertLabel);
    m_labelWidget->setLayout(m_labelLayout);

    m_mainLayout->setContentsMargins(40, 0, 40, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_labelWidget, 0, Qt::AlignLeft);
    m_mainLayout->addSpacing(16);
    m_mainLayout->addWidget(m_listWidget);
    m_mainLayout->addStretch();

    this->setLayout(m_mainLayout);
}

void DriverUninstallWidget::initConnection() {
    connect(g_signal, &GlobalSignal::getHwInfoComplete, this, &DriverUninstallWidget::setContent);

    // New Driver Installed
    connect(g_signal, &GlobalSignal::addToUninstallSignal, this, &DriverUninstallWidget::addContent);

    // Driver Uninstall
    connect(g_signal, &GlobalSignal::uninstallDriver, this, &DriverUninstallWidget::uninstallDriverSlot);
}

void DriverUninstallWidget::setContent() {
    if(g_infogetter->m_getsucceed){
        addGraphicCardContent();
        addScannerContent();
        addPrinterContent();
        addInputHwContent();
        addVideoContent();
        getOtherContent();

        if (!m_devList.isEmpty()) {
            emit g_signal->switchUninstallPage();
        }else{
            emit g_signal->switchNoUninstallPage();
        }
    }
}

void DriverUninstallWidget::addGraphicCardContent() {
    for (int i = 0; i < g_infogetter->m_graphicHardwares.length(); ++i) {
        if (!g_infogetter->m_graphicHardwares.at(i).upgradeable &&
            g_infogetter->m_graphicHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_graphicHardwares.at(i).model)) {
                bool result = driverExist(g_infogetter->m_graphicHardwares[i].driver_now);
                if (result) {
                    m_devList.append(g_infogetter->m_graphicHardwares.at(i).model);
                    m_item = new UninstallItem(g_infogetter->m_graphicHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_listWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_listWidget->addItem(aItem);
                    m_listWidget->setItemWidget(aItem, m_item);
                }
            }
        }
    }
}

void DriverUninstallWidget::addScannerContent() {
    for (int i = 0; i < g_infogetter->m_scannerHardwares.length(); ++i) {
        if (!g_infogetter->m_scannerHardwares.at(i).upgradeable &&
            g_infogetter->m_scannerHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_scannerHardwares.at(i).model)) {
                bool result = driverExist(g_infogetter->m_scannerHardwares[i].driver_now);
                if (result) {
                    m_devList.append(g_infogetter->m_scannerHardwares.at(i).model);
                    m_item = new UninstallItem(g_infogetter->m_scannerHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_listWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_listWidget->addItem(aItem);
                    m_listWidget->setItemWidget(aItem, m_item);
                }
            }
        }
    }
}

void DriverUninstallWidget::addPrinterContent() {
    for (int i = 0; i < g_infogetter->m_printerHardwares.length(); ++i) {
        if (!g_infogetter->m_printerHardwares.at(i).upgradeable &&
            g_infogetter->m_printerHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_printerHardwares.at(i).model)) {
                bool result = driverExist(g_infogetter->m_printerHardwares[i].driver_now);
                if (result) {
                    m_devList.append(g_infogetter->m_printerHardwares.at(i).model);
                    m_item = new UninstallItem(g_infogetter->m_printerHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_listWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_listWidget->addItem(aItem);
                    m_listWidget->setItemWidget(aItem, m_item);
                }
            }
        }
    }
}

void DriverUninstallWidget::addInputHwContent() {
    for (int i = 0; i < g_infogetter->m_inputHardwares.length(); ++i) {
        if (!g_infogetter->m_inputHardwares.at(i).upgradeable &&
            g_infogetter->m_inputHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_inputHardwares.at(i).model)) {
                bool result = driverExist(g_infogetter->m_inputHardwares[i].driver_now);
                if (result) {
                    m_devList.append(g_infogetter->m_inputHardwares.at(i).model);
                    m_item = new UninstallItem(g_infogetter->m_inputHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_listWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_listWidget->addItem(aItem);
                    m_listWidget->setItemWidget(aItem, m_item);
                }
            }
        }
    }
}

void DriverUninstallWidget::addVideoContent() {
    for (int i = 0; i < g_infogetter->m_videoHardwares.length(); ++i) {
        if (!g_infogetter->m_videoHardwares.at(i).upgradeable &&
            g_infogetter->m_videoHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_videoHardwares.at(i).model)) {
                bool result = driverExist(g_infogetter->m_videoHardwares[i].driver_now);
                if (result) {
                    m_devList.append(g_infogetter->m_videoHardwares.at(i).model);
                    m_item = new UninstallItem(g_infogetter->m_videoHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_listWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_listWidget->addItem(aItem);
                    m_listWidget->setItemWidget(aItem, m_item);
                }
            }
        }
    }
}

void DriverUninstallWidget::getOtherContent() {
    for (int i = 0; i < g_infogetter->m_otherHardwares.length(); ++i) {
        if (!g_infogetter->m_otherHardwares.at(i).upgradeable &&
            g_infogetter->m_otherHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_otherHardwares.at(i).model)) {
                bool result = driverExist(g_infogetter->m_otherHardwares[i].driver_now);
                if (result) {
                    m_devList.append(g_infogetter->m_otherHardwares.at(i).model);
                    m_item = new UninstallItem(g_infogetter->m_otherHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_listWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_listWidget->addItem(aItem);
                    m_listWidget->setItemWidget(aItem, m_item);
                }
            }
        }
    }
}

void DriverUninstallWidget::addContent(HardWareInfo hardWareInfo) {
    m_devList.append(hardWareInfo.model);
    UninstallItem *m_item = new UninstallItem(hardWareInfo, m_listWidget);
    QListWidgetItem *aItem = new QListWidgetItem(m_listWidget);
    aItem->setSizeHint(QSize(0, ITEMHEIGET));
    m_listWidget->addItem(aItem);
    m_listWidget->setItemWidget(aItem, m_item);

    if (m_devList.length() > 0){
        emit g_signal->switchUninstallPage();
    }
}

void DriverUninstallWidget::uninstallDriverSlot(UninstallItem *item, QString driverName) {
    m_uninstaller = new Uninstaller(driverName);

    m_tempitem = item;
    m_tempdriverName = driverName;
    connect(m_uninstaller, &Uninstaller::succeed, this, &DriverUninstallWidget::uninstallSuccessSlot);
    connect(m_uninstaller, &Uninstaller::failed, [=]() {});
    connect(m_uninstaller, &Uninstaller::progressChanged, item, &UninstallItem::changeProgress);

    m_uninstaller->startWorker();
}

HardWareInfo DriverUninstallWidget::updateHwInfo(UninstallItem *item, QString driverName) {
    DeviceType deviceType = item->getDevType();
    HardWareInfo hwinfo;
    switch (deviceType) {
        case DeviceType::SCANNER:
            for (int i = 0; i < g_infogetter->m_scannerHardwares.length(); ++i) {
                if (g_infogetter->m_scannerHardwares[i].driver_now == driverName &&
                    g_infogetter->m_scannerHardwares[i].driverNameList.length() != 0) {
                    g_infogetter->m_scannerHardwares[i].driver_now = "";
                    g_infogetter->m_scannerHardwares[i].driver_version_now = "";
                    g_infogetter->m_scannerHardwares[i].upgradeable = true;
                    hwinfo = g_infogetter->m_scannerHardwares[i];
                    break;
                }
            }
            break;
        case DeviceType::GraphicCard:
            for (int i = 0; i < g_infogetter->m_graphicHardwares.length(); ++i) {
                if (g_infogetter->m_graphicHardwares[i].driver_now == driverName) {
                    g_infogetter->m_graphicHardwares[i].driver_now = "";
                    g_infogetter->m_graphicHardwares[i].driver_version_now = "";
                    g_infogetter->m_graphicHardwares[i].upgradeable = true;
                    hwinfo = g_infogetter->m_graphicHardwares[i];
                    break;
                }
            }
            break;
        case DeviceType::PRINTER:
            for (int i = 0; i < g_infogetter->m_printerHardwares.length(); ++i) {
                if (g_infogetter->m_printerHardwares[i].driver_now == driverName &&
                    g_infogetter->m_printerHardwares[i].driverNameList.length() != 0) {
                    g_infogetter->m_printerHardwares[i].driver_now = "";
                    g_infogetter->m_printerHardwares[i].driver_version_now = "";
                    g_infogetter->m_printerHardwares[i].upgradeable = true;
                    hwinfo = g_infogetter->m_printerHardwares[i];
                    break;
                }
            }
            break;
        case DeviceType::InputDev:
            for (int i = 0; i < g_infogetter->m_inputHardwares.length(); ++i) {
                if (g_infogetter->m_inputHardwares[i].driver_now == driverName) {
                    g_infogetter->m_inputHardwares[i].driver_now = "";
                    g_infogetter->m_inputHardwares[i].driver_version_now = "";
                    g_infogetter->m_inputHardwares[i].upgradeable = true;
                    hwinfo = g_infogetter->m_inputHardwares[i];
                    break;
                }
            }
            break;
        case DeviceType::VideoOrAudio:
            for (int i = 0; i < g_infogetter->m_videoHardwares.length(); ++i) {
                if (g_infogetter->m_videoHardwares[i].driver_now == driverName &&
                    g_infogetter->m_videoHardwares[i].driverNameList.length() != 0) {
                    g_infogetter->m_videoHardwares[i].driver_now = "";
                    g_infogetter->m_videoHardwares[i].driver_version_now = "";
                    g_infogetter->m_videoHardwares[i].upgradeable = true;
                    hwinfo = g_infogetter->m_videoHardwares[i];
                    break;
                }
            }
            break;
        default:
            for (int i = 0; i < g_infogetter->m_otherHardwares.length(); ++i) {
                if (g_infogetter->m_otherHardwares[i].driver_now == driverName &&
                    g_infogetter->m_otherHardwares[i].driverNameList.length() != 0) {
                    g_infogetter->m_otherHardwares[i].driver_now = "";
                    g_infogetter->m_otherHardwares[i].driver_version_now = "";
                    g_infogetter->m_otherHardwares[i].upgradeable = true;
                    hwinfo = g_infogetter->m_otherHardwares[i];
                    break;
                }
            }
            break;
    }
    return hwinfo;
}

void DriverUninstallWidget::uninstallSuccessSlot() {
    // 从统计内容中删除
    m_devList.removeOne(m_tempitem->getDevName());
    // 更新信息
    HardWareInfo hardWareInfo = updateHwInfo(m_tempitem, m_tempdriverName);
    // 删除item
    int x = m_tempitem->parent()->findChildren<UninstallItem *>().indexOf(m_tempitem);
    m_listWidget->takeItem(m_listWidget->count() - 1 - x);

    if(m_listWidget->count() == 0){
        g_signal->switchNoUninstallPage();
    }
    // 添加到可安装页面
    g_signal->addToinstallSignal(hardWareInfo);
}


static bool driverExist(QString driverName) {
    QString cmd = "modinfo " + driverName;
    QProcess proc;
    proc.start(cmd);
    proc.waitForFinished();
    QString output = QString(proc.readAllStandardOutput());
    if (output == "") {
        return true;
    } else {
        return false;
    }
}

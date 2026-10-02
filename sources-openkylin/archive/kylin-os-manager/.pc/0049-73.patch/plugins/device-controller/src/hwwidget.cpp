#include "hwwidget.h"
#include "include/common.h"
#include "include/globalsignal.h"
#include <gsettingmonitor.h>
#include <QDebug>
#include <QMetaObject>
#include <QScreen>
#include "hardwareinfogetter.h"

HwWidget::HwWidget(DeviceInfo *devinfo, QWidget *parent)
        : m_devInfo(devinfo),
          QWidget(parent),
          m_iconLabel(new QLabel),
          m_textLabel(new QLabel),
          m_modelLabel(new K::TextLabel),
          m_activateStatus(new QLabel),
          m_btnStackedWidget(new QStackedWidget),
          m_spreadOutBtn(new kdk::KBorderlessButton),
          m_spreadInBtn(new kdk::KBorderlessButton),
          m_layout(new QHBoxLayout),
          m_timer(new QTimer),
          m_menu(new QMenu),
          m_actionExport(new QAction),
          m_actionActive(new QAction),
          m_actionDeactive(new QAction) {
    if (QGSettings::isSchemaInstalled(KYLIN_OS_MANAGER)) {
        m_data = new QGSettings(KYLIN_OS_MANAGER);
    }
    initGui();
    initConnect();
}

HwWidget::~HwWidget() {

}

QString HwWidget::getDevName() {
    return m_modelLabel->getText();
}

QString HwWidget::getDevType() {
    return m_textLabel->text();
}


void HwWidget::initGui() {
    m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
    m_modelLabel->setMinimumWidth(TREEITEMMODELLABELWIDTH);
    m_activateStatus->setText(tr("Disable"));
    m_spreadOutBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_spreadInBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    m_btnStackedWidget->addWidget(m_spreadOutBtn);
    m_btnStackedWidget->addWidget(m_spreadInBtn);
    m_btnStackedWidget->setCurrentWidget(m_spreadOutBtn);

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_iconLabel);
    m_layout->addSpacing(12);
    m_layout->addWidget(m_textLabel);
    m_layout->addWidget(m_modelLabel, 0, Qt::AlignLeft);
    m_layout->addWidget(m_activateStatus);
    m_layout->addWidget(m_btnStackedWidget, 0, Qt::AlignRight);
    this->setLayout(m_layout);

    m_activateStatus->hide();

    m_actionExport->setText(tr("Export To Clipboard"));
    m_actionActive->setText(tr("Activate"));
    m_actionDeactive->setText(tr("Deactive"));
    setContent();
    setIconColor();
}

void HwWidget::setContent() {
    if (auto cpuInfo = dynamic_cast<CpuInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("ukui-zs-processor-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("ukui-zs-processor-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("CPU"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(cpuInfo->getValue(m_cpuMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("CPU")).arg(cpuInfo->getValue(m_cpuMap.value("1"))));

        for (const QString &key: m_cpuInfo) {
            if (key == "1") {
                continue;
            }
            QString value = cpuInfo->getValue(m_cpuMap.value(key));
            m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
            if (value != "") {
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);

                if (key == QApplication::tr("CPU Current Frequency") || key == QApplication::tr("CPU Usage Rate") ||
                    key == QApplication::tr("CPU Temperature")) {
                    connect(m_timer, &QTimer::timeout, [=]() {
                        item->setLabelText(cpuInfo->getValue(m_cpuMap.value(key)));
                    });
                    m_timer->start(1000);
                }
            }
        }
    } else if (auto memoryInfo = dynamic_cast<MemoryInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("ukui-zs-memory-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("ukui-zs-memory-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Memory"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(memoryInfo->getValue(m_memoryMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Memory")).arg(memoryInfo->getValue(m_memoryMap.value("1"))));

        for (const QString &key: m_memoryInfo) {
            if (key == "1") {
                continue;
            }
            QString value = memoryInfo->getValue(m_memoryMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);

                if (key == QApplication::tr("Memory Usage")) {
                    connect(m_timer, &QTimer::timeout, [=]() {
                        item->setLabelText(memoryInfo->getValue(m_memoryMap.value(key)));
                    });
                    m_timer->start(1000);
                }
            }
        }
    } else if (auto graphicCardInfo = dynamic_cast<GraphicCardInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("ukui-graphics-card-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("ukui-graphics-card-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Graphic Card"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(graphicCardInfo->getValue(m_graphicCardMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Graphic Card")).arg(
                graphicCardInfo->getValue(m_graphicCardMap.value("1"))));

        for (const QString &key: m_graphicCardInfo) {
            if (key == "1") {
                continue;
            }
            QString value = graphicCardInfo->getValue(m_graphicCardMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
    } else if (auto baseboardInfo = dynamic_cast<BaseboardInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("ukui-mother-board-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("ukui-mother-board-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Mother Board"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(baseboardInfo->getValue(m_baseboardMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Mother Board")).arg(
                baseboardInfo->getValue(m_baseboardMap.value("1"))));

        for (const QString &key: m_baseboardInfo) {
            if (key == "1") {
                continue;
            }
            QString value = baseboardInfo->getValue(m_baseboardMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
    } else if (auto networkCardInfo = dynamic_cast<NetworkCardInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("ukui-zs-network-card-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("ukui-zs-network-card-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Network Card"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(networkCardInfo->getValue(m_networkCardMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Network Card")).arg(
                networkCardInfo->getValue(m_networkCardMap.value("1"))));

        for (const QString &key: m_networkCardInfo) {
            if (key == "1") {
                continue;
            }
            QString value = networkCardInfo->getValue(m_networkCardMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);

                if (key == QApplication::tr("Send Bytes") || key == QApplication::tr("Recved Bytes")) {
                    connect(m_timer, &QTimer::timeout, [=]() {
                        item->setLabelText(networkCardInfo->getValue(m_networkCardMap.value(key)));
                    });
                    m_timer->start(5000);
                }
            }
        }

        // 设置右键启用禁用
        m_menu->addAction(m_actionDeactive);
        connect(m_actionActive, &QAction::triggered, this, &HwWidget::actionActiveNetworkCardSlot);
        connect(m_actionDeactive, &QAction::triggered, this, &HwWidget::actionDeactiveNetworkCardSlot);
    } else if (auto diskInfo = dynamic_cast<DiskInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("ukui-zs-hard-disk-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("ukui-zs-hard-disk-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Disk"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(diskInfo->getValue(m_diskMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Disk")).arg(diskInfo->getValue(m_diskMap.value("1"))));

        for (const QString &key: m_diskInfo) {
            if (key == "1") {
                continue;
            }
            QString value = diskInfo->getValue(m_diskMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
    } else if (auto monitorInfo = dynamic_cast<MonitorInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("computer-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("computer-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Monitor"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(monitorInfo->getValue(m_monitorMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Monitor")).arg(monitorInfo->getValue(m_monitorMap.value("1"))));

        for (const QString &key: m_monitorInfo) {
            if (key == "1") {
                continue;
            }
            QString value = monitorInfo->getValue(m_monitorMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);

                if (key == QApplication::tr("Resolution")) {
                    connect(m_timer, &QTimer::timeout, [=]() {
                        item->setLabelText(monitorInfo->getValue(m_monitorMap.value(key)));
                    });
                    m_timer->start(5000);
                }

                if (key == QApplication::tr("Main Monitor")) {
                    connect(g_signal, &GlobalSignal::monitorRefreshed, this, [=]() {
                        for (int i = 0; i < g_infogetter->m_monitorList.length(); ++i) {
                            if (g_infogetter->m_monitorList[i].name == monitorInfo->name) {
                                QTimer::singleShot(2000, [=]() {
                                    item->setLabelText(monitorInfo->getValue(m_monitorMap.value(key)));
                                });
                            }
                        }
                    });
                }
            }
        }
    } else if (auto soundCardInfo = dynamic_cast<SoundCardInfo *>(m_devInfo)) {
        // 设置内容
        m_pixMap = QIcon::fromTheme("audio-volume-high-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("audio-volume-high-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Sound Card"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(soundCardInfo->getValue(m_soundCardMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Sound Card")).arg(
                soundCardInfo->getValue(m_soundCardMap.value("1"))));

        for (const QString &key: m_soundCardInfo) {
            if (key == "1") {
                continue;
            }
            QString value = soundCardInfo->getValue(m_soundCardMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }

        // 设置右键启用禁用
        m_menu->addAction(m_actionDeactive);
        connect(m_actionActive, &QAction::triggered, this, &HwWidget::actionActiveSoundCardSlot);
        connect(m_actionDeactive, &QAction::triggered, this, &HwWidget::actionDeactiveSoundCardSlot);
    } else if (auto keyboardInfo = dynamic_cast<KeyboardInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("input-keyboard-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("input-keyboard-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Keyboard"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(keyboardInfo->getValue(m_keyboardMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Keyboard")).arg(keyboardInfo->getValue(m_keyboardMap.value("1"))));

        for (const QString &key: m_keyboardInfo) {
            if (key == "1") {
                continue;
            }
            QString value = keyboardInfo->getValue(m_keyboardMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
    } else if (auto mouseInfo = dynamic_cast<MouseInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("input-mouse-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("input-mouse-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Mouse"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(mouseInfo->getValue(m_mouseMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Mouse")).arg(mouseInfo->getValue(m_mouseMap.value("1"))));

        for (const QString &key: m_mouseInfo) {
            if (key == "1") {
                continue;
            }
            QString value = mouseInfo->getValue(m_mouseMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
    } else if (auto bluetoothInfo = dynamic_cast<BluetoothInfo *>(m_devInfo)) {
        // 设置展示内容
        m_pixMap = QIcon::fromTheme("bluetooth-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("bluetooth-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Bluetooth"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(bluetoothInfo->getValue(m_bluetoothMap.value("1")));


        m_explorContent.append(QString("%1\t%2").arg(tr("Bluetooth")).arg(
                bluetoothInfo->getValue(m_bluetoothMap.value("1"))));

        for (const QString &key: m_bluetoothInfo) {
            if (key == "1") {
                continue;
            }
            QString value = bluetoothInfo->getValue(m_bluetoothMap.value(key));
            if (key == QObject::tr("Address")) {
                m_bluetoothAddress = value;
            }
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }

        if (m_data->get("bluetooth").toString() == "deactivate") {
            setDisableStatus();
            m_menu->addAction(m_actionActive);
            connect(m_actionActive, &QAction::triggered, g_signal, &GlobalSignal::activateBluetooth);
            connect(g_signal, &GlobalSignal::activateBluetooth, this, &HwWidget::actionActiveBluetoothSlot);
            connect(m_actionDeactive, &QAction::triggered, g_signal, &GlobalSignal::deactivateBluetooth);
            connect(g_signal, &GlobalSignal::deactivateBluetooth, this, &HwWidget::actionDeactiveBluetoothSlot);
        }else{
            m_menu->addAction(m_actionDeactive);
            connect(m_actionActive, &QAction::triggered, g_signal, &GlobalSignal::activateBluetooth);
            connect(g_signal, &GlobalSignal::activateBluetooth, this, &HwWidget::actionActiveBluetoothSlot);
            connect(m_actionDeactive, &QAction::triggered, g_signal, &GlobalSignal::deactivateBluetooth);
            connect(g_signal, &GlobalSignal::deactivateBluetooth, this, &HwWidget::actionDeactiveBluetoothSlot);
        }
    } else if (auto cdInfo = dynamic_cast<CDInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("ukui-burn-cd-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("ukui-burn-cd-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("CD"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(cdInfo->getValue(m_cdInfoMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("CD")).arg(cdInfo->getValue(m_cdInfoMap.value("1"))));

        for (const QString &key: m_cdInfo) {
            if (key == "1") {
                continue;
            }
            QString value = cdInfo->getValue(m_cdInfoMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
    } else if (auto batteryInfo = dynamic_cast<BatteryInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("battery-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("battery-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Battery"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(batteryInfo->getValue(m_batteryInfoMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Battery")).arg(
                batteryInfo->getValue(m_batteryInfoMap.value("1"))));

        for (const QString &key: m_batteryInfo) {
            if (key == "1") {
                continue;
            }
            QString value = batteryInfo->getValue(m_batteryInfoMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
    } else if (auto fanInfo = dynamic_cast<FanInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("ukui-zs-iso-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("ukui-zs-iso-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Fan"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(fanInfo->getValue(m_fanInfoMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Fan")).arg(fanInfo->getValue(m_fanInfoMap.value("1"))));

        for (const QString &key: m_fanInfo) {
            if (key == "1") {
                continue;
            }
            QString value = fanInfo->getValue(m_fanInfoMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
    } else if (auto cameraInfo = dynamic_cast<CameraInfo *>(m_devInfo)) {
        m_pixMap = QIcon::fromTheme("camera-photo-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(QIcon::fromTheme("camera-photo-symbolic").pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Camera"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(cameraInfo->getValue(m_cameraInfoMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Camera")).arg(
                cameraInfo->getValue(m_cameraInfoMap.value("1"))));

        for (const QString &key: m_cameraInfo) {
            if (key == "1") {
                continue;
            }
            QString value = cameraInfo->getValue(m_cameraInfoMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
    }
}

void HwWidget::initConnect() {
    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &HwWidget::setIconColor);

    connect(m_spreadOutBtn, &QPushButton::clicked, [=]() {
        m_btnStackedWidget->setCurrentWidget(m_spreadInBtn);
        if (!m_treeWidgetItemList.isEmpty()) {
            m_treeWidgetItemList.at(0)->parent()->setExpanded(true);
            emit g_signal->expandTreeWidget(itemCount);
        }
    });
    connect(m_spreadInBtn, &QPushButton::clicked, [=]() {
        m_btnStackedWidget->setCurrentWidget(m_spreadOutBtn);
        if (!m_treeWidgetItemList.isEmpty()) {
            m_treeWidgetItemList.at(0)->parent()->setExpanded(false);
            emit g_signal->expandTreeWidget((itemCount * (-1)));
        }
    });
}

void HwWidget::setIconColor() {
    QString styleName = kdk::GsettingMonitor::getSystemTheme().toString();
    if (styleName == "ukui-black" || styleName == "ukui-dark") {
        m_iconLabel->setPixmap(drawSymbolicColoredPixmap(m_pixMap, "white"));
    } else {
        m_iconLabel->setPixmap(drawSymbolicColoredPixmap(m_pixMap, "black"));
    }

    float fontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    QFont font;
    font.setPointSize(fontSize);
    m_textLabel->setFont(font);
}

QPixmap HwWidget::drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor) {
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

void HwWidget::mousePressEvent(QMouseEvent *mouseEvent) {
    if (mouseEvent->button() == Qt::RightButton) {
        if (mouseEvent->type() == QEvent::MouseButtonPress) {
            qDebug() << "右键点击";
            rightClickedMenuRequest(); //右击弹窗:1另存为、2打开文件
        }
    }
}

void HwWidget::rightClickedMenuRequest() {
    connect(m_actionExport, &QAction::triggered, this, &HwWidget::actionExportSlot);
    m_menu->addAction(m_actionExport);
    m_menu->exec(QCursor::pos());
}

void HwWidget::actionExportSlot() {
    m_clipboard = QApplication::clipboard();
    m_clipboard->setText(m_explorContent.join("\n"));
}

void HwWidget::actionActiveBluetoothSlot() {
    m_interface = new QDBusInterface("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon",
                                     "com.kylin.assistant.systemdaemon", QDBusConnection::systemBus());

    if (m_interface && m_interface->isValid()) {
        QDBusPendingCall asyncCall = m_interface->asyncCall("enableBluetooth");
        m_menu->removeAction(m_actionActive);
        m_menu->addAction(m_actionDeactive);

        QString result = m_data->get("bluetooth").toString();
        QString str = "activate";
        m_data->set("bluetooth", str);

        emit g_signal->enableBluetoothSignal();
    } else {
        qWarning() << "call hardwareinfo dbus failed!!";
    }
}

void HwWidget::actionDeactiveBluetoothSlot() {
    m_interface = new QDBusInterface("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon",
                                     "com.kylin.assistant.systemdaemon", QDBusConnection::systemBus());

    if (m_interface && m_interface->isValid()) {
        QDBusPendingCall asyncCall = m_interface->asyncCall("disableBluetooth");
        m_menu->removeAction(m_actionDeactive);
        m_menu->addAction(m_actionActive);

        QString result = m_data->get("bluetooth").toString();
        QString str = "deactivate";
        m_data->set("bluetooth", str);

        emit g_signal->disableBluetoothSignal();
    } else {
        qWarning() << "call hardwareinfo dbus failed!!";
    }
}

void HwWidget::actionActiveSoundCardSlot() {
    m_interface = new QDBusInterface("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon",
                                     "com.kylin.assistant.systemdaemon", QDBusConnection::systemBus());
    auto soundCardInfo = dynamic_cast<SoundCardInfo *>(m_devInfo);
    QString strProduct = soundCardInfo->getValue(m_soundCardMap.value("1"));
    if (m_interface && m_interface->isValid()) {
        QString sysPciPath = soundCardInfo->getValue("bus").replace("pci@", "");;
        QString productAndPci = strProduct + "|" + sysPciPath;
        QDBusReply<QString> reply = m_interface->call("enableAudioAdaptor", productAndPci);
        if (reply.isValid()) {
            m_menu->removeAction(m_actionActive);
            m_menu->addAction(m_actionDeactive);
            qDebug() << "enableSoundCard success" << productAndPci;
        } else {
            qWarning() << "call enableSoundCard dbus failed!!";
        }
    } else {
        qWarning() << "call enableSoundCard dbus failed!!";
    }
}

void HwWidget::actionDeactiveSoundCardSlot() {
    m_interface = new QDBusInterface("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon",
                                     "com.kylin.assistant.systemdaemon", QDBusConnection::systemBus());
    auto soundCardInfo = dynamic_cast<SoundCardInfo *>(m_devInfo);
    QString strProduct = soundCardInfo->getValue(m_soundCardMap.value("1"));
    if (m_interface && m_interface->isValid()) {
        QString sysPciPath = soundCardInfo->getValue("bus").replace("pci@", "");
        QString productAndPci = strProduct + "|" + sysPciPath;
        QDBusReply<QString> reply = m_interface->call("disableNetworkAdaptor", productAndPci);
        if (reply.isValid()) {
            m_menu->removeAction(m_actionDeactive);
            m_menu->addAction(m_actionActive);
            qDebug() << "disableSoundCard success: " << productAndPci;
        } else {
            qWarning() << "call disableSoundCard dbus failed!!";
        }
    } else {
        qWarning() << "call disableSoundCard dbus failed!!";
    }
}

void HwWidget::actionActiveNetworkCardSlot() {
    m_interface = new QDBusInterface("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon",
                                     "com.kylin.assistant.systemdaemon", QDBusConnection::systemBus());
    auto networkCardInfo = dynamic_cast<NetworkCardInfo *>(m_devInfo);
    QString strProduct = networkCardInfo->getValue(m_networkCardMap.value("1"));
    if (m_interface && m_interface->isValid()) {
        QString sysPciPath = m_networkCardPathMap.value(strProduct);
        QString productAndPci = strProduct + "|" + sysPciPath;
        QDBusReply<QString> reply = m_interface->call("enableNetworkAdaptor", productAndPci);
        if (reply.isValid()) {
            m_menu->removeAction(m_actionActive);
            m_menu->addAction(m_actionDeactive);
            qDebug() << "enablenetworkCard success" << productAndPci;

            QString result = m_data->get("networkcard").toString();
            QString str = strProduct + " " + "activate" + "\n";
            if (result == "") {
                m_data->set("networkcard", str);
            } else {
                if (result.contains("\n")) {
                    QStringList results = result.split("\n");
                    for (int i = 0; i < results.length(); ++i) {
                        if (results[i].contains(strProduct)) {
                            results.removeAt(i);
                            results.append(str);
                            break;
                        }
                    }
                    m_data->set("networkcard", results.join(" "));
                }
            }

        } else {
            qWarning() << "call enablenetworkCard dbus failed!!";
        }
    } else {
        qWarning() << "call enablenetworkCard dbus failed!!";
    }
}

void HwWidget::actionDeactiveNetworkCardSlot() {
    m_interface = new QDBusInterface("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon",
                                     "com.kylin.assistant.systemdaemon", QDBusConnection::systemBus());
    auto networkCardInfo = dynamic_cast<NetworkCardInfo *>(m_devInfo);
    QString strProduct = networkCardInfo->getValue(m_networkCardMap.value("1"));
    if (m_interface && m_interface->isValid()) {
        QString sysPciPath = "";
        QProcess p(0);
        p.start("hwinfo --network");
        p.waitForStarted();
        p.waitForFinished();
        QString strTemp = QString::fromLocal8Bit(p.readAllStandardOutput());  //获得输出
        QStringList strList = strTemp.split("\n");
        for (int i = 0; i < strList.length(); i++) {
            if (strList.at(i).contains(strProduct)) {
                //  bus info 获取
                qDebug() << strList.at(i + 1) << endl;
                if (strList.at(i + 1).contains("SysFS Device Link")) {
                    sysPciPath = ((QString) strList.at(i + 1));
                    // /devices/pci0000:00/0000:00:14.3
                    if (sysPciPath.contains("/devices/pci0000:00") and !(sysPciPath.contains("usb"))) {
                        sysPciPath = sysPciPath.replace("  SysFS Device Link: /devices/pci0000:00/", "");
                    }
                }
            }
        }
        if (!m_networkCardPathMap.contains(strProduct)) {
            m_networkCardPathMap.insert(strProduct, sysPciPath);
        }
        QString productAndPci = strProduct + "|" + sysPciPath;
        QDBusReply<QString> reply = m_interface->call("disableNetworkAdaptor", productAndPci);
        if (reply.isValid()) {
            m_menu->removeAction(m_actionDeactive);
            m_menu->addAction(m_actionActive);
            qDebug() << "disablenetworkCard success" << productAndPci;
            QString str = strProduct + " " + "deactivate" + "\n";
            QString result = m_data->get("networkcard").toString();
            if (result == "") {
                m_data->set("networkcard", str);
            } else {
                if (result.contains("\n")) {
                    QStringList results = result.split("\n");
                    for (int i = 0; i < results.length(); ++i) {
                        if (results[i].contains(strProduct)) {
                            results.removeAt(i);
                            results.append(str);
                            break;
                        }
                    }
                    m_data->set("networkcard", results.join(" "));
                }
            }

        } else {
            qWarning() << "call disablenetworkCard dbus failed!!";
        }
    } else {
        qWarning() << "call disablenetworkCard dbus failed!!";
    }
}

HwWidget::HwWidget(QWidget *parent) : QWidget(parent) {

}

void HwWidget::setDisableStatus() {
    m_iconLabel->setDisabled(true);
    m_textLabel->setDisabled(true);
    m_modelLabel->setDisabled(true);
    m_activateStatus->setDisabled(true);
    m_activateStatus->show();
}

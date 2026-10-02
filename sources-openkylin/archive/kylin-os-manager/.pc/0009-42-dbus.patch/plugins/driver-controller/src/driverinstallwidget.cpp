#include "driverinstallwidget.h"
#include <QPushButton>
#include <kysdk/applications/gsettingmonitor.h>
#include <QVersionNumber>

#include "include/globalsignal.h"
#include "include/common.h"

static QString getDriverVersion(QString driverName);

DriverInstallWidget::DriverInstallWidget(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(new QVBoxLayout()),
      m_installableWidget(new QWidget()),
      m_installableLayout(new QHBoxLayout()),
      m_installableCheckBox(new QCheckBox()),
      m_installableLabel(new QLabel()),
      m_installButton(new kdk::KBorderlessButton()),
      m_installableListWidget(new QListWidget()),
      m_installableListWidgetLayout(new QHBoxLayout()),
      m_upgradableWidget(new QWidget()),
      m_upgradableLayout(new QHBoxLayout()),
      m_upgradableCheckBox(new QCheckBox()),
      m_upgradableLabel(new QLabel()),
      m_updateButton(new kdk::KBorderlessButton()),
      m_upgradableListWidget(new QListWidget()),
      m_upgradableListWidgetLayout(new QHBoxLayout()),
      m_notUpdatedWidget(new QWidget()),
      m_notUpdatedLayout(new QHBoxLayout()),
      m_notUpdatedCheckBox(new QCheckBox()),
      m_notUpdatedLabel(new QLabel()),
      m_notUpdatedListWidget(new QListWidget()),
      m_notUpdatedListWidgetLayout(new QHBoxLayout()) {
    initConnection();
    initGui();
}

void DriverInstallWidget::initGui() {
    this->setProperty("useSystemStyleBlur", true);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    QString str = tr("Installable Drivers") + QString("(") + QString::number(m_installabelNum) + QString(tr(")"));
    m_installableLabel->setText(str);
    m_installButton->setText(tr("One-click installation"));

    QString str2 = tr("Upgradable Drivers") + QString("(") + QString::number(m_updateabelNum) + QString(tr(")"));
    m_upgradableLabel->setText(str2);
    m_updateButton->setText(tr("One-click upgrade"));

    QString str3 = tr("Installed Drivers") + QString("(") + QString::number(m_notUpdatedNum) + QString(tr(")"));
    m_notUpdatedLabel->setText(str3);
    m_notUpdatedCheckBox->setEnabled(false);
    themeFontStyle();

    m_installableLayout->setContentsMargins(40, 0, 0, 0);
    m_installableLayout->setSpacing(0);
    m_installableLayout->addWidget(m_installableCheckBox);
    m_installableLayout->addSpacing(4);
    m_installableLayout->addWidget(m_installableLabel);
    m_installableLayout->addStretch();
    m_installableLayout->addWidget(m_installButton);
    m_installableWidget->setLayout(m_installableLayout);

    m_installableListWidget->setSortingEnabled(true);
    m_installableListWidget->sortItems(Qt::DescendingOrder);
    m_installableListWidget->setViewMode(QListView::ListMode);
    m_installableListWidget->setFrameShape(QListWidget::NoFrame);                   //去掉list的边框
    m_installableListWidget->setStyleSheet(".QListWidget::item:hover{background:transparent;}");
    m_installableListWidget->setSelectionMode(QAbstractItemView::NoSelection); // list取消原装的选中功能
    m_installableListWidget->setContentsMargins(0, 0, 0, 0);
    m_installableListWidget->setSpacing(0);

    m_installableListWidgetLayout->setContentsMargins(70, 0, 0, 0);
    m_installableListWidgetLayout->addWidget(m_installableListWidget);

    m_upgradableLayout->setContentsMargins(40, 0, 0, 0);
    m_upgradableLayout->setSpacing(0);
    m_upgradableLayout->addWidget(m_upgradableCheckBox);
    m_upgradableLayout->addSpacing(4);
    m_upgradableLayout->addWidget(m_upgradableLabel);
    m_upgradableLayout->addStretch();
    m_upgradableLayout->addWidget(m_updateButton);
    m_upgradableWidget->setLayout(m_upgradableLayout);

    m_upgradableListWidget->setSortingEnabled(true);
    m_upgradableListWidget->sortItems(Qt::DescendingOrder);
    m_upgradableListWidget->setViewMode(QListView::ListMode);
    m_upgradableListWidget->setFrameShape(QListWidget::NoFrame);                   //去掉list的边框
    m_upgradableListWidget->setStyleSheet(".QListWidget::item:hover{background:transparent;}");
    m_upgradableListWidget->setSelectionMode(QAbstractItemView::NoSelection); // list取消原装的选中功能
    m_upgradableListWidget->setContentsMargins(0, 0, 0, 0);
    m_upgradableListWidget->setSpacing(0);

    m_upgradableListWidgetLayout->setContentsMargins(70, 0, 0, 0);
    m_upgradableListWidgetLayout->addWidget(m_upgradableListWidget);

    m_notUpdatedLayout->setContentsMargins(40, 0, 0, 0);
    m_notUpdatedLayout->setSpacing(0);
    m_notUpdatedLayout->addWidget(m_notUpdatedCheckBox);
    m_notUpdatedLayout->addSpacing(4);
    m_notUpdatedLayout->addWidget(m_notUpdatedLabel);
    m_notUpdatedLayout->addStretch();
    m_notUpdatedWidget->setLayout(m_notUpdatedLayout);

    m_notUpdatedListWidget->setSortingEnabled(true);
    m_notUpdatedListWidget->sortItems(Qt::DescendingOrder);
    m_notUpdatedListWidget->setViewMode(QListView::ListMode);
    m_notUpdatedListWidget->setFrameShape(QListWidget::NoFrame);                   //去掉list的边框
    m_notUpdatedListWidget->setStyleSheet(".QListWidget::item:hover{background:transparent;}");
    m_notUpdatedListWidget->setSelectionMode(QAbstractItemView::NoSelection); // list取消原装的选中功能
    m_notUpdatedListWidget->setContentsMargins(0, 0, 0, 0);
    m_notUpdatedListWidget->setSpacing(0);

    m_notUpdatedListWidgetLayout->setContentsMargins(70, 0, 0, 0);
    m_notUpdatedListWidgetLayout->addWidget(m_notUpdatedListWidget);

    m_mainLayout->setContentsMargins(0, 0, 40, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_installableWidget);
    m_mainLayout->addLayout(m_installableListWidgetLayout);
    m_mainLayout->addWidget(m_upgradableWidget);
    m_mainLayout->addLayout(m_upgradableListWidgetLayout);
    m_mainLayout->addWidget(m_notUpdatedWidget);
    m_mainLayout->addLayout(m_notUpdatedListWidgetLayout);
    this->setLayout(m_mainLayout);
}

void DriverInstallWidget::initConnection() {

    connect(g_signal, &GlobalSignal::getHwInfoComplete, this, &DriverInstallWidget::setContent);
    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &DriverInstallWidget::themeFontStyle);

    // 一键安装
    connect(m_installButton, &QPushButton::clicked, this, &DriverInstallWidget::installMultiDriverSlot);

    // checkbox
    connect(m_installableCheckBox, &QCheckBox::clicked, this, &DriverInstallWidget::setInstallableListWidgetCheck);
    connect(m_upgradableCheckBox, &QCheckBox::clicked, this, &DriverInstallWidget::setUpgradableListWidgetCheck);
    connect(g_signal, &GlobalSignal::uncheckCheckBox, this, &DriverInstallWidget::uncheckCheckBox);
    connect(g_signal, &GlobalSignal::checkAllChildenCheckboxState, this,
            &DriverInstallWidget::checkAllChildenCheckboxStateSlot);

    // Driver Install
    connect(g_signal, &GlobalSignal::installDriver, this, &DriverInstallWidget::installDriverSlot);

    // 卸载后添加到可安装列
    connect(g_signal, &GlobalSignal::addToinstallSignal, this, &DriverInstallWidget::addNewInstallabelItemSlot);
}

void DriverInstallWidget::setContent() {
    checkUpgradable();

    addGraphicCardContent();
    addScannerContent();
    addPrinterContent();
    addBluetoothContent();
    addSoundCardContent();
    addNetworkCardContent();
    addInputHwContent();
    addVideoContent();
    getOtherContent();

    caculateAndSetPageSize();

    emit g_signal->switchInstallPage();    // 由检测页面切换到安装页面
}

void DriverInstallWidget::caculateAndSetPageSize() {
    QString str = tr("Installable Drivers") + QString("(") + QString::number(m_installabelNum) + QString(tr(")"));
    m_installableLabel->setText(str);

    QString str2 = tr("Upgradable Drivers") + QString("(") + QString::number(m_updateabelNum) + QString(tr(")"));
    m_upgradableLabel->setText(str2);

    QString str3 = tr("Installed Drivers") + QString("(") + QString::number(m_notUpdatedNum) + QString(tr(")"));
    m_notUpdatedLabel->setText(str3);

    int installableListHeight = (ITEMHEIGET) * m_installabelNum;
    m_installableListWidget->setFixedHeight(installableListHeight);

    int upgradableListHeight = (ITEMHEIGET) * m_updateabelNum;
    m_upgradableListWidget->setFixedHeight(upgradableListHeight);

    int notUpdatedListHeight = (ITEMHEIGET) * m_notUpdatedNum;
    m_notUpdatedListWidget->setFixedHeight(notUpdatedListHeight);

    if (m_installabelNum == 0) {
        m_installableWidget->hide();
        m_installableListWidget->hide();
    } else {
        m_installableWidget->show();
        m_installableListWidget->show();
    }

    if (m_updateabelNum == 0) {
        m_upgradableWidget->hide();
        m_upgradableListWidget->hide();
    } else {
        m_upgradableWidget->show();
        m_upgradableListWidget->show();
    }

    if (m_notUpdatedNum == 0) {
        m_notUpdatedWidget->hide();
        m_notUpdatedListWidget->hide();
    } else {
        m_notUpdatedWidget->show();
        m_notUpdatedListWidget->show();
    }
}

void DriverInstallWidget::checkUpgradable() {
    for (int i = 0; i < g_infogetter->m_graphicHardwares.length(); ++i) {
        if (g_infogetter->m_graphicHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_graphicHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            };
        } else {
            m_notUpdatedNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_scannerHardwares.length(); ++i) {
        if (g_infogetter->m_scannerHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_scannerHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else {
            m_notUpdatedNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_printerHardwares.length(); ++i) {
        if (g_infogetter->m_printerHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_printerHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else {
            m_notUpdatedNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_bluetoothHardwares.length(); ++i) {
        if (g_infogetter->m_bluetoothHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_bluetoothHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else {
            m_notUpdatedNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_soundCardHardwares.length(); ++i) {
        if (g_infogetter->m_soundCardHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_soundCardHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else {
            m_notUpdatedNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_networkCardHardwares.length(); ++i) {
        if (g_infogetter->m_networkCardHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_networkCardHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else {
            m_notUpdatedNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_inputHardwares.length(); ++i) {
        if (g_infogetter->m_inputHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_inputHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else {
            m_notUpdatedNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_videoHardwares.length(); ++i) {
        if (g_infogetter->m_videoHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_videoHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else {
            m_notUpdatedNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_otherHardwares.length(); ++i) {
        if (g_infogetter->m_otherHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_otherHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else {
            m_notUpdatedNum++;
        }
    }
}

void DriverInstallWidget::themeFontStyle() {
    float fontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    int size = 18 * fontSize / 15;
    QFont font;
    font.setPointSize(size);
    font.setBold(true);
    m_installableLabel->setFont(font);
    m_upgradableLabel->setFont(font);
    m_notUpdatedLabel->setFont(font);
}


void DriverInstallWidget::addGraphicCardContent() {
    for (int i = 0; i < g_infogetter->m_graphicHardwares.length(); ++i) {
        if (g_infogetter->m_graphicHardwares.at(i).upgradeable &&
                g_infogetter->m_graphicHardwares.at(i).driver_now == "") {
            if (!m_devList.contains(g_infogetter->m_graphicHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_graphicHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_graphicHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_installableListWidget->addItem(aItem);
                m_installableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_installabelNum--;
            }
        } else if (g_infogetter->m_graphicHardwares.at(i).upgradeable &&
                   g_infogetter->m_graphicHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_graphicHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_graphicHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_graphicHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_upgradableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_upgradableListWidget->addItem(aItem);
                m_upgradableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_updateabelNum--;
            }
        } else {
            if (g_infogetter->m_graphicHardwares.at(i).driverNameList.length() == 0 &&
                    g_infogetter->m_graphicHardwares.at(i).driver_now == "") {
                m_notUpdatedNum--;
                continue;
            }
            if (!m_devList.contains(g_infogetter->m_graphicHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_graphicHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_graphicHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_notUpdatedListWidget->addItem(aItem);
                m_notUpdatedListWidget->setItemWidget(aItem, m_item);
            } else {
                m_notUpdatedNum--;
            }
        }
    }
}

void DriverInstallWidget::addScannerContent() {
    for (int i = 0; i < g_infogetter->m_scannerHardwares.length(); ++i) {
        if (g_infogetter->m_scannerHardwares.at(i).upgradeable &&
                g_infogetter->m_scannerHardwares.at(i).driver_now == "") {
            if (!m_devList.contains(g_infogetter->m_scannerHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_scannerHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_scannerHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_installableListWidget->addItem(aItem);
                m_installableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_installabelNum--;
            }
        } else if (g_infogetter->m_scannerHardwares.at(i).upgradeable &&
                   g_infogetter->m_scannerHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_scannerHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_scannerHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_scannerHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_upgradableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_upgradableListWidget->addItem(aItem);
                m_upgradableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_updateabelNum--;
            }
        } else {
            if (g_infogetter->m_scannerHardwares.at(i).driverNameList.length() == 0 &&
                    g_infogetter->m_scannerHardwares.at(i).driver_now == "") {
                m_notUpdatedNum--;
                continue;
            }
            if (!m_devList.contains(g_infogetter->m_scannerHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_scannerHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_scannerHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_notUpdatedListWidget->addItem(aItem);
                m_notUpdatedListWidget->setItemWidget(aItem, m_item);
            } else {
                m_notUpdatedNum--;
            }
        }
    }
}

void DriverInstallWidget::addPrinterContent() {
    for (int i = 0; i < g_infogetter->m_printerHardwares.length(); ++i) {
        if (g_infogetter->m_printerHardwares.at(i).upgradeable &&
                g_infogetter->m_printerHardwares.at(i).driver_now == "") {
            if (!m_devList.contains(g_infogetter->m_printerHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_printerHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_printerHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_installableListWidget->addItem(aItem);
                m_installableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_installabelNum--;
            }
        } else if (g_infogetter->m_printerHardwares.at(i).upgradeable &&
                   g_infogetter->m_printerHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_printerHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_printerHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_printerHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_upgradableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_upgradableListWidget->addItem(aItem);
                m_upgradableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_updateabelNum--;
            }
        } else {
            if (g_infogetter->m_printerHardwares.at(i).driverNameList.length() == 0 &&
                    g_infogetter->m_printerHardwares.at(i).driver_now == "") {
                m_notUpdatedNum--;
                continue;
            }
            if (!m_devList.contains(g_infogetter->m_printerHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_printerHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_printerHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_notUpdatedListWidget->addItem(aItem);
                m_notUpdatedListWidget->setItemWidget(aItem, m_item);
            } else {
                m_notUpdatedNum--;
            }
        }
    }
}

void DriverInstallWidget::addBluetoothContent() {
    for (int i = 0; i < g_infogetter->m_bluetoothHardwares.length(); ++i) {
        if (g_infogetter->m_bluetoothHardwares.at(i).upgradeable &&
                g_infogetter->m_bluetoothHardwares.at(i).driver_now == "") {
            if (!m_devList.contains(g_infogetter->m_bluetoothHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_bluetoothHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_bluetoothHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_installableListWidget->addItem(aItem);
                m_installableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_installabelNum--;
            }
        } else if (g_infogetter->m_bluetoothHardwares.at(i).upgradeable &&
                   g_infogetter->m_bluetoothHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_bluetoothHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_bluetoothHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_bluetoothHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_upgradableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_upgradableListWidget->addItem(aItem);
                m_upgradableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_updateabelNum--;
            }
        } else {
            if (g_infogetter->m_bluetoothHardwares.at(i).driverNameList.length() == 0 &&
                    g_infogetter->m_bluetoothHardwares.at(i).driver_now == "") {
                m_notUpdatedNum--;
                continue;
            }
            if (!m_devList.contains(g_infogetter->m_bluetoothHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_bluetoothHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_bluetoothHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_notUpdatedListWidget->addItem(aItem);
                m_notUpdatedListWidget->setItemWidget(aItem, m_item);
            } else {
                m_notUpdatedNum--;
            }
        }
    }
}

void DriverInstallWidget::addSoundCardContent() {
    for (int i = 0; i < g_infogetter->m_soundCardHardwares.length(); ++i) {
        if (g_infogetter->m_soundCardHardwares.at(i).upgradeable &&
                g_infogetter->m_soundCardHardwares.at(i).driver_now == "") {
            if (!m_devList.contains(g_infogetter->m_soundCardHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_soundCardHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_soundCardHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_installableListWidget->addItem(aItem);
                m_installableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_installabelNum--;
            }
        } else if (g_infogetter->m_soundCardHardwares.at(i).upgradeable &&
                   g_infogetter->m_soundCardHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_soundCardHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_soundCardHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_soundCardHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_upgradableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_upgradableListWidget->addItem(aItem);
                m_upgradableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_updateabelNum--;
            }
        } else {
            if (g_infogetter->m_soundCardHardwares.at(i).driverNameList.length() == 0 &&
                    g_infogetter->m_soundCardHardwares.at(i).driver_now == "") {
                m_notUpdatedNum--;
                continue;
            }
            if (!m_devList.contains(g_infogetter->m_soundCardHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_soundCardHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_soundCardHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_notUpdatedListWidget->addItem(aItem);
                m_notUpdatedListWidget->setItemWidget(aItem, m_item);
            } else {
                m_notUpdatedNum--;
            }
        }
    }
}

void DriverInstallWidget::addNetworkCardContent() {
    for (int i = 0; i < g_infogetter->m_networkCardHardwares.length(); ++i) {
        if (g_infogetter->m_networkCardHardwares.at(i).upgradeable &&
                g_infogetter->m_networkCardHardwares.at(i).driver_now == "") {
            if (!m_devList.contains(g_infogetter->m_networkCardHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_networkCardHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_networkCardHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_installableListWidget->addItem(aItem);
                m_installableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_installabelNum--;
            }
        } else if (g_infogetter->m_networkCardHardwares.at(i).upgradeable &&
                   g_infogetter->m_networkCardHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_networkCardHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_networkCardHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_networkCardHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_upgradableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_upgradableListWidget->addItem(aItem);
                m_upgradableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_updateabelNum--;
            }
        } else {
            if (g_infogetter->m_networkCardHardwares.at(i).driverNameList.length() == 0 &&
                    g_infogetter->m_networkCardHardwares.at(i).driver_now == "") {
                m_notUpdatedNum--;
                continue;
            }
            if (!m_devList.contains(g_infogetter->m_networkCardHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_networkCardHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_networkCardHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_notUpdatedListWidget->addItem(aItem);
                m_notUpdatedListWidget->setItemWidget(aItem, m_item);
            } else {
                m_notUpdatedNum--;
            }
        }
    }
}

void DriverInstallWidget::addInputHwContent() {
    for (int i = 0; i < g_infogetter->m_inputHardwares.length(); ++i) {
        if (g_infogetter->m_inputHardwares.at(i).upgradeable && g_infogetter->m_inputHardwares.at(i).driver_now == "") {
            if (!m_devList.contains(g_infogetter->m_inputHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_inputHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_inputHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_installableListWidget->addItem(aItem);
                m_installableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_installabelNum--;
            }
        } else if (g_infogetter->m_inputHardwares.at(i).upgradeable &&
                   g_infogetter->m_inputHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_inputHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_inputHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_inputHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_upgradableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_upgradableListWidget->addItem(aItem);
                m_upgradableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_updateabelNum--;
            }
        } else {
            if (g_infogetter->m_inputHardwares.at(i).driverNameList.length() == 0 &&
                    g_infogetter->m_inputHardwares.at(i).driver_now == "") {
                m_notUpdatedNum--;
                continue;
            }
            if (!m_devList.contains(g_infogetter->m_inputHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_inputHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_inputHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_notUpdatedListWidget->addItem(aItem);
                m_notUpdatedListWidget->setItemWidget(aItem, m_item);
            } else {
                m_notUpdatedNum--;
            }
        }
    }
}

void DriverInstallWidget::addVideoContent() {
    for (int i = 0; i < g_infogetter->m_videoHardwares.length(); ++i) {
        if (g_infogetter->m_videoHardwares.at(i).upgradeable && g_infogetter->m_videoHardwares.at(i).driver_now == "") {
            if (!m_devList.contains(g_infogetter->m_videoHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_videoHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_videoHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_installableListWidget->addItem(aItem);
                m_installableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_installabelNum--;
            }
        } else if (g_infogetter->m_videoHardwares.at(i).upgradeable &&
                   g_infogetter->m_videoHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_videoHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_videoHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_videoHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_upgradableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_upgradableListWidget->addItem(aItem);
                m_upgradableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_updateabelNum--;
            }
        } else {
            if (g_infogetter->m_videoHardwares.at(i).driverNameList.length() == 0 &&
                    g_infogetter->m_videoHardwares.at(i).driver_now == "") {
                m_notUpdatedNum--;
                continue;
            }
            if (!m_devList.contains(g_infogetter->m_videoHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_videoHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_videoHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_notUpdatedListWidget->addItem(aItem);
                m_notUpdatedListWidget->setItemWidget(aItem, m_item);
            } else {
                m_notUpdatedNum--;
            }
        }
    }
}

void DriverInstallWidget::getOtherContent() {
    for (int i = 0; i < g_infogetter->m_otherHardwares.length(); ++i) {
        if (g_infogetter->m_otherHardwares.at(i).upgradeable && g_infogetter->m_otherHardwares.at(i).driver_now == "") {
            if (!m_devList.contains(g_infogetter->m_otherHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_otherHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_otherHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_installableListWidget->addItem(aItem);
                m_installableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_installabelNum--;
            }
        } else if (g_infogetter->m_otherHardwares.at(i).upgradeable &&
                   g_infogetter->m_otherHardwares.at(i).driver_now != "") {
            if (!m_devList.contains(g_infogetter->m_otherHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_otherHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_otherHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_upgradableListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_upgradableListWidget->addItem(aItem);
                m_upgradableListWidget->setItemWidget(aItem, m_item);
            } else {
                m_updateabelNum--;
            }
        } else {
            if (g_infogetter->m_otherHardwares.at(i).driverNameList.length() == 0 &&
                    g_infogetter->m_otherHardwares.at(i).driver_now == "") {
                m_notUpdatedNum--;
                continue;
            }
            if (!m_devList.contains(g_infogetter->m_otherHardwares.at(i).model)) {
                m_devList.append(g_infogetter->m_otherHardwares.at(i).model);
                m_item = new DeviceItem(g_infogetter->m_otherHardwares[i]);
                QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
                aItem->setSizeHint(QSize(0, ITEMHEIGET));
                m_notUpdatedListWidget->addItem(aItem);
                m_notUpdatedListWidget->setItemWidget(aItem, m_item);
            } else {
                m_notUpdatedNum--;
            }
        }
    }
}

void DriverInstallWidget::setInstallableListWidgetCheck() {
    if (m_installableCheckBox->isChecked()) {
        for (int i = 0; i < m_installableListWidget->count(); i++) {
            DeviceItem *itemWidget = m_installableListWidget->findChildren<DeviceItem *>().at(i);
            itemWidget->checkTheBox();
        }
    } else {
        for (int i = 0; i < m_installableListWidget->count(); i++) {
            DeviceItem *itemWidget = m_installableListWidget->findChildren<DeviceItem *>().at(i);
            itemWidget->unCheckTheBox();
        }
    }
}

void DriverInstallWidget::setUpgradableListWidgetCheck() {
    if (m_upgradableCheckBox->isChecked()) {
        for (int i = 0; i < m_upgradableListWidget->count(); i++) {
            DeviceItem *itemWidget = m_upgradableListWidget->findChildren<DeviceItem *>().at(i);
            itemWidget->checkTheBox();
        }
    } else {
        for (int i = 0; i < m_upgradableListWidget->count(); i++) {
            DeviceItem *itemWidget = m_upgradableListWidget->findChildren<DeviceItem *>().at(i);
            itemWidget->unCheckTheBox();
        }
    }
}

void DriverInstallWidget::uncheckCheckBox(int num) {
    if (num == 0) {
        m_installableCheckBox->setCheckState(Qt::Unchecked);
    } else {
        m_upgradableCheckBox->setCheckState(Qt::Unchecked);
    }
}

void DriverInstallWidget::checkAllChildenCheckboxStateSlot(int num) {
    if (num == 0) {
        bool check = true;
        for (int i = 0; i < m_installableListWidget->count(); i++) {
            DeviceItem *itemWidget = m_installableListWidget->findChildren<DeviceItem *>().at(i);
            check = check && itemWidget->m_isChecked;
        }
        if (check) {
            m_installableCheckBox->setCheckState(Qt::Checked);
        } else {
            m_installableCheckBox->setCheckState(Qt::Unchecked);
        }
    } else {
        bool check = true;
        for (int i = 0; i < m_upgradableListWidget->count(); i++) {
            DeviceItem *itemWidget = m_upgradableCheckBox->findChildren<DeviceItem *>().at(i);
            check = check && itemWidget->m_isChecked;
            if (check) {
                m_upgradableCheckBox->setCheckState(Qt::Checked);
            } else {
                m_upgradableCheckBox->setCheckState(Qt::Unchecked);
            }
        }
    }
}

void DriverInstallWidget::installDriverSlot(DeviceItem *item, QString driverName) {
    m_installer = new AptInstaller(driverName);

    m_tempitem = item;
    m_tempDriverName = driverName;
    connect(m_installer, &AptInstaller::succeed, this, &DriverInstallWidget::installSuccessSlot);
    connect(m_installer, &AptInstaller::failed, [=]() {g_signal->quitEventLoop();});
    connect(m_installer, &AptInstaller::progressChanged, item, &DeviceItem::changeProgress);

    m_installer->startWorker();
}

void DriverInstallWidget::installSuccessSlot() {
    g_signal->quitEventLoop();
    // 更新信息
    HardWareInfo hardWareInfo = updateHwInfo(m_tempitem, m_tempDriverName);
    // 加入可卸载列表
    emit g_signal->addToUninstallSignal(hardWareInfo);
    // 将设备条目从可下载移动至已下载
    moveToInstalledSlot(m_tempitem, hardWareInfo);

}

void DriverInstallWidget::moveToInstalledSlot(DeviceItem *item, HardWareInfo hardWareInfo) {
    m_installabelNum--;
    m_notUpdatedNum++;
    caculateAndSetPageSize();

    // 从可安装列表中移除
    int x = item->parent()->findChildren<DeviceItem *>().indexOf(item);
    m_installableListWidget->takeItem(m_installableListWidget->count() - 1 - x);

    DeviceItem *m_item2 = new DeviceItem(hardWareInfo, m_notUpdatedListWidget);
    QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
    aItem->setSizeHint(QSize(0, ITEMHEIGET));
    m_notUpdatedListWidget->addItem(aItem);
    m_notUpdatedListWidget->setItemWidget(aItem, m_item2);
}

void DriverInstallWidget::addNewInstallabelItemSlot(HardWareInfo hardWareInfo) {
    m_installabelNum++;
    m_notUpdatedNum--;
    caculateAndSetPageSize();

    // 加入可安装列表
    DeviceItem *item = new DeviceItem(hardWareInfo, m_installableListWidget);
    QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
    aItem->setSizeHint(QSize(0, ITEMHEIGET));
    m_installableListWidget->addItem(aItem);
    m_installableListWidget->setItemWidget(aItem, item);

    // 从已安装列表中删除
    for (int i = 0; i < m_notUpdatedListWidget->count(); ++i) {
        DeviceItem *item = static_cast<DeviceItem *>(m_notUpdatedListWidget->itemWidget(
                                                         m_notUpdatedListWidget->item(i)));
        if (item->getDevName() == hardWareInfo.model) {
            int x = item->parent()->findChildren<DeviceItem *>().indexOf(item);
            m_notUpdatedListWidget->takeItem(m_notUpdatedListWidget->count() - 1 - x);
        }
    }
}

void DriverInstallWidget::installMultiDriverSlot() {
    QStringList checkedItem;         // 记录被选中的是哪几个
    for (int i = 0; i < m_installableListWidget->count(); ++i) {
        DeviceItem *item = static_cast<DeviceItem *>(m_installableListWidget->itemWidget(
                                                         m_installableListWidget->item(i)));
        if (item->m_isChecked) {
            checkedItem.append(item->getDevName());
        }
    }
    if (checkedItem.length() == 0) {
        return;
    }
    connect(g_signal, &GlobalSignal::quitEventLoop, [=](){eventLoop.quit();});
    for (int i = 0; i < m_installableListWidget->count(); ++i) {
        for (int j = 0; j < checkedItem.length(); ++j) {
            if(static_cast<DeviceItem *>(m_installableListWidget->itemWidget(m_installableListWidget->item(i)))->getDevName() == checkedItem[j]){
                static_cast<DeviceItem *>(m_installableListWidget->itemWidget(m_installableListWidget->item(i)))->clickInstall();
                eventLoop.exec();
            }
        }
    }
}

HardWareInfo DriverInstallWidget::updateHwInfo(DeviceItem *item, QString driverName) {
    QString driverVersion = getDriverVersion(driverName);
    QString deviceName = item->getDevName();
    DeviceType deviceType = item->getDevType();
    HardWareInfo hwinfo;
    switch (deviceType) {
    case DeviceType::SCANNER:
        for (int i = 0; i < g_infogetter->m_scannerHardwares.length(); ++i) {
            if (g_infogetter->m_scannerHardwares[i].model == deviceName &&
                    g_infogetter->m_scannerHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_scannerHardwares[i].driver_now = driverName;
                g_infogetter->m_scannerHardwares[i].driver_version_now = driverVersion;
                g_infogetter->m_scannerHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_scannerHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::GraphicCard:
        for (int i = 0; i < g_infogetter->m_graphicHardwares.length(); ++i) {
            if (g_infogetter->m_graphicHardwares[i].model == deviceName &&
                    g_infogetter->m_graphicHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_graphicHardwares[i].driver_now = driverName;
                g_infogetter->m_graphicHardwares[i].driver_version_now = driverVersion;
                g_infogetter->m_graphicHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_graphicHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::SoundCard:
        for (int i = 0; i < g_infogetter->m_soundCardHardwares.length(); ++i) {
            if (g_infogetter->m_soundCardHardwares[i].model == deviceName &&
                    g_infogetter->m_soundCardHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_soundCardHardwares[i].driver_now = driverName;
                g_infogetter->m_soundCardHardwares[i].driver_version_now = driverVersion;
                g_infogetter->m_soundCardHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_soundCardHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::NetworkCard:
        for (int i = 0; i < g_infogetter->m_networkCardHardwares.length(); ++i) {
            if (&g_infogetter->m_networkCardHardwares[i].model == deviceName &&
                    g_infogetter->m_networkCardHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_networkCardHardwares[i].driver_now = driverName;
                g_infogetter->m_networkCardHardwares[i].driver_version_now = driverVersion;
                g_infogetter->m_networkCardHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_networkCardHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::Bluetooth:
        for (int i = 0; i < g_infogetter->m_bluetoothHardwares.length(); ++i) {
            if (&g_infogetter->m_bluetoothHardwares[i].model == deviceName &&
                    g_infogetter->m_bluetoothHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_bluetoothHardwares[i].driver_now = driverName;
                g_infogetter->m_bluetoothHardwares[i].driver_version_now = driverVersion;
                g_infogetter->m_bluetoothHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_bluetoothHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::PRINTER:
        for (int i = 0; i < g_infogetter->m_printerHardwares.length(); ++i) {
            if (&g_infogetter->m_printerHardwares[i].model == deviceName &&
                    g_infogetter->m_printerHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_printerHardwares[i].driver_now = driverName;
                g_infogetter->m_printerHardwares[i].driver_version_now = driverVersion;
                g_infogetter->m_printerHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_printerHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::InputDev:
        for (int i = 0; i < g_infogetter->m_inputHardwares.length(); ++i) {
            if (&g_infogetter->m_inputHardwares[i].model == deviceName &&
                    g_infogetter->m_inputHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_inputHardwares[i].driver_now = driverName;
                g_infogetter->m_inputHardwares[i].driver_version_now = driverVersion;
                g_infogetter->m_inputHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_inputHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::VideoOrAudio:
        for (int i = 0; i < g_infogetter->m_videoHardwares.length(); ++i) {
            if (&g_infogetter->m_videoHardwares[i].model == deviceName &&
                    g_infogetter->m_videoHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_videoHardwares[i].driver_now = driverName;
                g_infogetter->m_videoHardwares[i].driver_version_now = driverVersion;
                g_infogetter->m_videoHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_videoHardwares[i];
                break;
            }
        }
        break;
    default:
        for (int i = 0; i < g_infogetter->m_otherHardwares.length(); ++i) {
            if (g_infogetter->m_otherHardwares[i].model == deviceName &&
                    g_infogetter->m_otherHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_otherHardwares[i].driver_now = driverName;
                g_infogetter->m_otherHardwares[i].driver_version_now = driverVersion;
                g_infogetter->m_otherHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_otherHardwares[i];
                break;
            }
        }
        break;
    }
    return hwinfo;
}

static QString getDriverVersion(QString driverName) {
    QString cmd = "apt-cache show " + driverName;
    QProcess proc;
    proc.start(cmd);
    proc.waitForFinished();
    QStringList output = QString(proc.readAllStandardOutput()).split("\n");
    QString result = "";
    for (int i = 0; i < output.length(); ++i) {
        if (QString(output[i]).contains("Version: ")) {
            QString version = QString(output[i]).split("Version: ")[1];
            version.remove("\n");
            if (result == "") {
                result = version;
            } else {
                if (QVersionNumber::fromString(result) < QVersionNumber::fromString(version)) {
                    result = version;
                }
            }
        }
    }
    return result;
}



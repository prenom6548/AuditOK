#include "driverinstallwidget.h"
#include <QPushButton>
#include <gsettingmonitor.h>
#include <QVersionNumber>
#include "include/globalsignal.h"
#include "include/common.h"
#include "database/drivermanagerdatabase.h"

static QString getDriverVersion(QString driverName);

static QString getPackageVersion(const QString &packageName);

static QString getRetFromCommand(const QStringList &command);

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
      m_notUpdatedListWidgetLayout(new QHBoxLayout()),
      m_notFindDriverWidget(new QWidget()),
      m_notFindDriverLayout(new QHBoxLayout()),
      m_notFindDriverCheckBox(new QCheckBox()),
      m_notFindDriverLabel(new QLabel()),
      m_notFindDriverListWidget(new QListWidget()),
      m_notFindDriverListWidgetLayout(new QHBoxLayout()) {
    initConnection();
    initGui();
}

void DriverInstallWidget::initGui() {
    this->setProperty("useSystemStyleBlur", true);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    QString str = tr("Installable Drivers") + QString(tr("(")) + QString::number(m_installabelNum) + QString(tr(")"));
    m_installableLabel->setText(str);
    m_installButton->setText(tr("One-click installation"));

    QString str2 = tr("Upgradable Drivers") + QString(tr("(")) + QString::number(m_updateabelNum) + QString(tr(")"));
    m_upgradableLabel->setText(str2);
    m_updateButton->setText(tr("One-click upgrade"));

    QString str3 = tr("Installed Drivers") + QString(tr("(")) + QString::number(m_notUpdatedNum) + QString(tr(")"));
    m_notUpdatedLabel->setText(str3);
    m_notUpdatedCheckBox->setEnabled(false);
    m_notFindDriverCheckBox->setEnabled(false);
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

    m_notFindDriverLayout->setContentsMargins(40, 0, 0, 0);
    m_notFindDriverLayout->setSpacing(0);
    m_notFindDriverLayout->addWidget(m_notFindDriverCheckBox);
    m_notFindDriverLayout->addSpacing(4);
    m_notFindDriverLayout->addWidget(m_notFindDriverLabel);
    m_notFindDriverLayout->addStretch();
    m_notFindDriverWidget->setLayout(m_notFindDriverLayout);

    m_notFindDriverListWidget->setSortingEnabled(true);
    m_notFindDriverListWidget->sortItems(Qt::DescendingOrder);
    m_notFindDriverListWidget->setViewMode(QListView::ListMode);
    m_notFindDriverListWidget->setFrameShape(QListWidget::NoFrame);                   //去掉list的边框
    m_notFindDriverListWidget->setStyleSheet(".QListWidget::item:hover{background:transparent;}");
    m_notFindDriverListWidget->setSelectionMode(QAbstractItemView::NoSelection); // list取消原装的选中功能
    m_notFindDriverListWidget->setContentsMargins(0, 0, 0, 0);
    m_notFindDriverListWidget->setSpacing(0);

    m_notFindDriverListWidgetLayout->setContentsMargins(70, 0, 0, 0);
    m_notFindDriverListWidgetLayout->addWidget(m_notFindDriverListWidget);

    m_mainLayout->setContentsMargins(0, 0, 40, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_installableWidget);
    m_mainLayout->addLayout(m_installableListWidgetLayout);
    m_mainLayout->addWidget(m_upgradableWidget);
    m_mainLayout->addLayout(m_upgradableListWidgetLayout);
    m_mainLayout->addWidget(m_notUpdatedWidget);
    m_mainLayout->addLayout(m_notUpdatedListWidgetLayout);
    m_mainLayout->addWidget(m_notFindDriverWidget);
    m_mainLayout->addLayout(m_notFindDriverListWidgetLayout);
    m_mainLayout->addStretch();
    this->setLayout(m_mainLayout);
}

void DriverInstallWidget::initConnection() {

    connect(g_signal, &GlobalSignal::getHwInfoComplete, [=]() {
        QTimer::singleShot(100, this, [=]() {
            DATABASE_INSTANCE.initDatabase();
            setContent();
        });
    });

    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &DriverInstallWidget::themeFontStyle);

    // 一键安装
    connect(m_installButton, &QPushButton::clicked, this, &DriverInstallWidget::installMultiDriverSlot);

    // 一键升级
    connect(m_updateButton, &QPushButton::clicked, this, &DriverInstallWidget::updateMultiDriverSlot);

    // checkbox
    connect(m_installableCheckBox, &QCheckBox::clicked, this, &DriverInstallWidget::setInstallableListWidgetCheck);
    connect(m_upgradableCheckBox, &QCheckBox::clicked, this, &DriverInstallWidget::setUpgradableListWidgetCheck);
    connect(g_signal, &GlobalSignal::uncheckCheckBox, this, &DriverInstallWidget::uncheckCheckBox);
    connect(g_signal, &GlobalSignal::checkAllChildenCheckboxState, this,
            &DriverInstallWidget::checkAllChildenCheckboxStateSlot);

    // Driver Install
    connect(g_signal, &GlobalSignal::installDriver, this, &DriverInstallWidget::installDriverSlot);
    connect(g_signal, &GlobalSignal::uninstallUpdateDriver, this, &DriverInstallWidget::uninstallUpdateDriverSlot);
    connect(g_signal, &GlobalSignal::uninstallDriver, this, &DriverInstallWidget::uninstallDriverSlot);

    // 卸载后添加到可安装列
    //    connect(g_signal, &GlobalSignal::addToinstallSignal, this, &DriverInstallWidget::addNewInstallabelItemSlot);

    // 热插拔后的增删Item
    connect(g_signal, &GlobalSignal::removeItem, this, &DriverInstallWidget::removeItemSlot);
    connect(g_signal, &GlobalSignal::addItem, this, &DriverInstallWidget::addItemSlot);
}

void DriverInstallWidget::setContent() {
    if (g_infogetter->m_getsucceed) {
        checkUpgradable();

        addGraphicCardContent();
        addPrinterContent();
        addScannerContent();
        addBluetoothContent();
        addSoundCardContent();
        addNetworkCardContent();
        addInputHwContent();
        addVideoContent();
        getOtherContent();
        QList<HardWareInfo> devList = DATABASE_INSTANCE.getDataList();
//        addDeleteItem(devList);
        caculateAndSetPageSize();
        emit g_signal->updateInstallContentSucceed();    // 由检测页面切换到安装页面

    }
    emit g_signal->switchInstallPage();    // 由检测页面切换到安装页面
}

void DriverInstallWidget::caculateAndSetPageSize() {
    QString str = tr("Installable Drivers") + QString(tr("(")) + QString::number(m_installabelNum) + QString(tr(")"));
    m_installableLabel->setText(str);

    QString str2 = tr("Upgradable Drivers") + QString(tr("(")) + QString::number(m_updateabelNum) + QString(tr(")"));
    m_upgradableLabel->setText(str2);

    QString str3 = tr("Installed Drivers") + QString(tr("(")) + QString::number(m_notUpdatedNum) + QString(tr(")"));
    m_notUpdatedLabel->setText(str3);

    QString str4 =
            tr("Unrecognized Device") + QString(tr("(")) + QString::number(m_notFindDriverNum) + QString(tr(")"));
    m_notFindDriverLabel->setText(str4);

    int installableListHeight = (ITEMHEIGET) * m_installabelNum;
    m_installableListWidget->setFixedHeight(installableListHeight);

    int upgradableListHeight = (ITEMHEIGET) * m_updateabelNum;
    m_upgradableListWidget->setFixedHeight(upgradableListHeight);

    int notUpdatedListHeight = (ITEMHEIGET) * m_notUpdatedNum;
    m_notUpdatedListWidget->setFixedHeight(notUpdatedListHeight);

    int notFindDriverListHeight = (ITEMHEIGET) * m_notFindDriverNum;
    m_notFindDriverListWidget->setFixedHeight(notFindDriverListHeight);

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

    if (m_notFindDriverNum == 0) {
        m_notFindDriverWidget->hide();
        m_notFindDriverListWidget->hide();
    } else {
        m_notFindDriverWidget->show();
        m_notFindDriverListWidget->show();
    }
}

void DriverInstallWidget::checkUpgradable() {
    m_installabelNum = 0; // 用于标识是否存在可安装的硬件驱动
    m_updateabelNum = 0; // 用于标识是否存在可升级的硬件驱动
    m_notUpdatedNum = 0; // 用于标识无需升级的设备数量
    m_notFindDriverNum = 0;

    m_devList.clear();
    for (int i = 0; i < m_installableListWidget->count(); ++i) {
        m_installableListWidget->takeItem(i);
    }
    for (int i = 0; i < m_upgradableListWidget->count(); ++i) {
        m_upgradableListWidget->takeItem(i);
    }
    for (int i = 0; i < m_notUpdatedListWidget->count(); ++i) {
        m_notUpdatedListWidget->takeItem(i);
    }
    for (int i = 0; i < m_notFindDriverListWidget->count(); ++i) {
        m_notFindDriverListWidget->takeItem(i);
    }


    for (int i = 0; i < g_infogetter->m_graphicHardwares.length(); ++i) {
        if (g_infogetter->m_graphicHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_graphicHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            };
        } else if (g_infogetter->m_graphicHardwares.at(i).driver_now != "") {
            m_notUpdatedNum++;
        } else {
            m_notFindDriverNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_scannerHardwares.length(); ++i) {
        if (g_infogetter->m_scannerHardwares.at(i).isOnline == false) {
            continue;
        }
        if (g_infogetter->m_scannerHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_scannerHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else if (g_infogetter->m_scannerHardwares.at(i).driver_now != "") {
            m_notUpdatedNum++;
        } else {
            m_notFindDriverNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_printerHardwares.length(); ++i) {
        if (g_infogetter->m_printerHardwares.at(i).isOnline == false) {
            continue;
        }
        if (g_infogetter->m_printerHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_printerHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else if (g_infogetter->m_printerHardwares.at(i).driver_now != "") {
            m_notUpdatedNum++;
        } else {
            m_notFindDriverNum++;
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
        } else if (g_infogetter->m_inputHardwares.at(i).driver_now != "") {
            m_notUpdatedNum++;
        } else {
            m_notFindDriverNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_videoHardwares.length(); ++i) {
        if (g_infogetter->m_videoHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_videoHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else if (g_infogetter->m_videoHardwares.at(i).driver_now != "") {
            m_notUpdatedNum++;
        } else {
            m_notFindDriverNum++;
        }
    }

    for (int i = 0; i < g_infogetter->m_otherHardwares.length(); ++i) {
        if (g_infogetter->m_otherHardwares.at(i).upgradeable == true) {
            if (g_infogetter->m_otherHardwares.at(i).driver_now == "") {
                m_installabelNum++;
            } else {
                m_updateabelNum++;
            }
        } else if (g_infogetter->m_otherHardwares.at(i).driver_now != "") {
            m_notUpdatedNum++;
        } else {
            m_notFindDriverNum++;
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
    m_notFindDriverLabel->setFont(font);
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
                if (!m_devList.contains(g_infogetter->m_graphicHardwares.at(i).model)) {
                    m_notFindDriverNum--;
                } else {
                    m_notFindDriverNum--;
                }
            } else {
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
}

void DriverInstallWidget::addScannerContent() {
    for (int i = 0; i < g_infogetter->m_scannerHardwares.length(); ++i) {
        if (!g_infogetter->m_scannerHardwares.at(i).isOnline) {
            continue;
        }
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
                g_infogetter->m_scannerHardwares.at(i).driver_now == "" ||
                (g_infogetter->m_scannerHardwares.at(i).upgradeable ==
                 false && g_infogetter->m_scannerHardwares.at(i).driverNameList.length() != 0 &&
                 g_infogetter->m_scannerHardwares.at(i).driver_now == "")) {
                if (!m_devList.contains(g_infogetter->m_scannerHardwares.at(i).model)) {
                    m_devList.append(g_infogetter->m_scannerHardwares.at(i).model);
                    m_item = new DeviceItem(g_infogetter->m_scannerHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_notFindDriverListWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_notFindDriverListWidget->addItem(aItem);
                    m_notFindDriverListWidget->setItemWidget(aItem, m_item);
                } else {
                    m_notFindDriverNum--;
                }
            } else {
                // 需要确定该设备的驱动是否为设备管理安装的
                QList<HardWareInfo> devList = DATABASE_INSTANCE.getDataList();
                if (devList.length() == 0) {
                    m_notUpdatedNum--;
                }
                for (int j = 0; j < devList.length(); ++j) {
                    if (devList[j].model == g_infogetter->m_scannerHardwares.at(i).model) {
                        if (devList[j].driver_now == g_infogetter->m_scannerHardwares.at(i).driver_now &&
                            devList[j].driver_version_now ==
                            g_infogetter->m_scannerHardwares.at(i).driver_version_now) {
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
                        } else {
                            m_notUpdatedNum--;
                            continue;
                        }
                    }
                }
            }
        }
    }
}

void DriverInstallWidget::addPrinterContent() {
    for (int i = 0; i < g_infogetter->m_printerHardwares.length(); ++i) {
        if (!g_infogetter->m_printerHardwares.at(i).isOnline) {
            continue;
        }
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
            if ((g_infogetter->m_printerHardwares.at(i).driverNameList.length() == 0 &&
                 g_infogetter->m_printerHardwares.at(i).driver_now == "") ||
                (g_infogetter->m_printerHardwares.at(i).upgradeable ==
                 false && g_infogetter->m_printerHardwares.at(i).driverNameList.length() != 0 &&
                 g_infogetter->m_printerHardwares.at(i).driver_now == "")) {
                if (!m_devList.contains(g_infogetter->m_printerHardwares.at(i).model)) {
                    m_devList.append(g_infogetter->m_printerHardwares.at(i).model);
                    m_item = new DeviceItem(g_infogetter->m_printerHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_notFindDriverListWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_notFindDriverListWidget->addItem(aItem);
                    m_notFindDriverListWidget->setItemWidget(aItem, m_item);
                } else {
                    m_notFindDriverNum--;
                }
            } else {
                // 需要确定该设备的驱动是否为设备管理安装的
                QList<HardWareInfo> devList = DATABASE_INSTANCE.getDataList();
                if (devList.length() == 0) {
                    m_notUpdatedNum--;
                }
                for (int j = 0; j < devList.length(); ++j) {
                    if (devList[j].model == g_infogetter->m_printerHardwares.at(i).model) {
                        if (devList[j].driver_now == g_infogetter->m_printerHardwares.at(i).driver_now &&
                                devList[j].driver_version_now == g_infogetter->m_printerHardwares.at(i).driver_version_now) {
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
                        } else {
                            m_notUpdatedNum--;
                            continue;
                        }
                    }
                }
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
                if (!m_devList.contains(g_infogetter->m_inputHardwares.at(i).model)) {
                    m_devList.append(g_infogetter->m_inputHardwares.at(i).model);
                    m_item = new DeviceItem(g_infogetter->m_inputHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_notFindDriverListWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_notFindDriverListWidget->addItem(aItem);
                    m_notFindDriverListWidget->setItemWidget(aItem, m_item);
                } else {
                    m_notFindDriverNum--;
                }
            } else {
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
                if (!m_devList.contains(g_infogetter->m_videoHardwares.at(i).model)) {
                    m_devList.append(g_infogetter->m_videoHardwares.at(i).model);
                    m_item = new DeviceItem(g_infogetter->m_videoHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_notFindDriverListWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_notFindDriverListWidget->addItem(aItem);
                    m_notFindDriverListWidget->setItemWidget(aItem, m_item);
                } else {
                    m_notFindDriverNum--;
                }
            } else {
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
                if (!m_devList.contains(g_infogetter->m_otherHardwares.at(i).model)) {
                    m_devList.append(g_infogetter->m_otherHardwares.at(i).model);
                    m_item = new DeviceItem(g_infogetter->m_otherHardwares[i]);
                    QListWidgetItem *aItem = new QListWidgetItem(m_notFindDriverListWidget);
                    aItem->setSizeHint(QSize(0, ITEMHEIGET));
                    m_notFindDriverListWidget->addItem(aItem);
                    m_notFindDriverListWidget->setItemWidget(aItem, m_item);
                } else {
                    m_notFindDriverNum--;
                }
            } else {
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
}

void DriverInstallWidget::addDeleteItem(QList<HardWareInfo> hwList) {
    for (int i = 0; i < hwList.length(); ++i) {
        QString driver = hwList.at(i).driver_now;
        QStringList drivers;
        if(driver.contains(";")){
            drivers = driver.split(";");
            drivers.removeLast();
        }
        QString driverVersion = "";
        for (int j = 0; j < drivers.length(); ++j) {
            QString version = getDriverVersion(drivers[j]).trimmed();
            if(version == ""){
                g_signal->deleteDataFromDriverSignal(driver);   // 删除数据库中的条目
                return;
            }
            driverVersion += version + ";";
        }

        if ((!m_devList.contains(hwList.at(i).model)) && driverVersion == hwList.at(i).driver_version_now.trimmed()) {
            m_devList.append(hwList.at(i).model);
            m_uninstallItem = new UninstallItem(hwList[i]);
            QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
            aItem->setSizeHint(QSize(0, ITEMHEIGET));
            m_notUpdatedListWidget->addItem(aItem);
            m_notUpdatedListWidget->setItemWidget(aItem, m_uninstallItem);
            m_notUpdatedNum++;
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
            DeviceItem *itemWidget = m_upgradableListWidget->findChildren<DeviceItem *>().at(i);
            check = check && itemWidget->m_isChecked;
            if (check) {
                m_upgradableCheckBox->setCheckState(Qt::Checked);
            } else {
                m_upgradableCheckBox->setCheckState(Qt::Unchecked);
            }
        }
    }
}

void DriverInstallWidget::installDriverSlot(DeviceItem *item, QStringList driverName) {
    m_installer = new AptInstaller(driverName);

    m_tempupdateitem = item;
    m_tempDriverName = driverName;
    connect(m_installer, &AptInstaller::succeed, this, &DriverInstallWidget::installSuccessSlot);
    connect(m_installer, &AptInstaller::succeed, item, &DeviceItem::installSucceed);
    connect(m_installer, &AptInstaller::failed, this, &DriverInstallWidget::installFailSlot);
    connect(m_installer, &AptInstaller::failed, item, &DeviceItem::installFailed);
    connect(m_installer, &AptInstaller::progressChanged, item, &DeviceItem::changeProgress);

    m_installer->startWorker();
}

void DriverInstallWidget::installSuccessSlot() {
    g_signal->onInstalling = false;
    emit g_signal->quitEventLoop();
    m_installableCheckBox->setCheckState(Qt::CheckState::Unchecked);
    // 更新信息
    HardWareInfo hardWareInfo = updateHwInfo(m_tempupdateitem, m_tempDriverName);
    // 将设备条目从可下载移动至已下载
    moveToInstalledSlot(m_tempupdateitem, hardWareInfo);
}

void DriverInstallWidget::installFailSlot(QString result) {
    g_signal->onInstalling = false;
    emit g_signal->quitEventLoop();
    m_tempupdateitem->installFailed();
    QMessageBox::information(this, tr("Install Failed"), result);
}

void DriverInstallWidget::moveToInstalledSlot(DeviceItem *item, HardWareInfo hardWareInfo) {
    if (item->getBtnText() == tr("Update")) {
        m_updateabelNum--;
        m_notUpdatedNum++;
        // 从可升级列表中移除
        caculateAndSetPageSize();
        int x = item->parent()->findChildren<DeviceItem *>().indexOf(item);
        m_upgradableListWidget->takeItem(m_upgradableListWidget->count() - 1 - x);
    } else if (item->getBtnText() == tr("Install")) {
        m_installabelNum--;
        m_notUpdatedNum++;
        // 从可安装列表中移除
        caculateAndSetPageSize();
        int x = item->parent()->findChildren<DeviceItem *>().indexOf(item);
        m_installableListWidget->takeItem(m_installableListWidget->count() - 1 - x);
    }

    // 检查已安装列表中是否有不同的设备有同样的包，但不同的版本，如果有则需移动到可安装中，因为该包已经被覆盖
    for (int i = 0; i < m_notUpdatedListWidget->count(); ++i) {
        DeviceItem *item3 = static_cast<DeviceItem *>(m_notUpdatedListWidget->itemWidget(m_notUpdatedListWidget->item(i)));
        if (item3->getDriverName() == hardWareInfo.driver_now.split("=").first()) {
            // 将该设备加入到可安装列表中
            HardWareInfo hwinfo = item3->getHardwareInfo();
            addNewInstallabelItemSlot(hwinfo);
        }
    }

    // 加入已安装列表
    DeviceItem *m_item2 = new DeviceItem(hardWareInfo, m_notUpdatedListWidget);
    QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
    aItem->setSizeHint(QSize(0, ITEMHEIGET));
    m_notUpdatedListWidget->addItem(aItem);
    m_notUpdatedListWidget->setItemWidget(aItem, m_item2);

}

void DriverInstallWidget::addNewInstallabelItemSlot(HardWareInfo hardWareInfo) {
    // 从已安装列表中删除
    bool dev_exist_flag = false;   // 标识当前设备是否在已安装列表中，true则表明该设备usb在线
    for (int i = 0; i < m_notUpdatedListWidget->count(); ++i) {
        DeviceItem *item = static_cast<DeviceItem *>(m_notUpdatedListWidget->itemWidget(
                                                         m_notUpdatedListWidget->item(i)));
        if (item->getDevName() == hardWareInfo.model) {
            dev_exist_flag = true;
            int x = item->parent()->findChildren<DeviceItem *>().indexOf(item);
            m_notUpdatedListWidget->takeItem(m_notUpdatedListWidget->count() - 1 - x);
            m_installabelNum++;
            m_notUpdatedNum--;
        }
    }
    // 从可升级列表中删除
    for (int i = 0; i < m_upgradableListWidget->count(); ++i) {
        DeviceItem *item = static_cast<DeviceItem *>(m_upgradableListWidget->itemWidget(
                                                         m_upgradableListWidget->item(i)));
        if (item->getDevName() == hardWareInfo.model) {
            dev_exist_flag = true;
            int x = item->parent()->findChildren<DeviceItem *>().indexOf(item);
            m_upgradableListWidget->takeItem(m_upgradableListWidget->count() - 1 - x);
            m_installabelNum++;
            m_updateabelNum--;
        }
    }

    //如果该设备USB在线则在可安装中加入该设备，否则无需加入
    if (dev_exist_flag) {
        caculateAndSetPageSize();
        HardWareInfo hwinfo;
        // 加入可安装列表
        switch (hardWareInfo.type) {
        case DeviceType::PRINTER:
            for (int i = 0; i < g_infogetter->m_printerHardwares.length(); ++i) {
                if (g_infogetter->m_printerHardwares.at(i).PID == hardWareInfo.PID &&
                        g_infogetter->m_printerHardwares.at(i).VID == hardWareInfo.VID) {
                    hwinfo = g_infogetter->m_printerHardwares.at(i);
                    hwinfo.upgradeable = true;
                }
            }
            break;
        case DeviceType::SCANNER:
            for (int i = 0; i < g_infogetter->m_scannerHardwares.length(); ++i) {
                if (g_infogetter->m_scannerHardwares.at(i).PID == hardWareInfo.PID &&
                        g_infogetter->m_scannerHardwares.at(i).VID == hardWareInfo.VID) {
                    hwinfo = g_infogetter->m_scannerHardwares.at(i);
                    hwinfo.upgradeable = true;
                }
            }
            break;
        case DeviceType::GraphicCard:
            for (int i = 0; i < g_infogetter->m_graphicHardwares.length(); ++i) {
                if (g_infogetter->m_graphicHardwares.at(i).PID == hardWareInfo.PID &&
                        g_infogetter->m_graphicHardwares.at(i).VID == hardWareInfo.VID) {
                    hwinfo = g_infogetter->m_graphicHardwares.at(i);
                }
            }
            break;
        case DeviceType::InputDev:
            for (int i = 0; i < g_infogetter->m_inputHardwares.length(); ++i) {
                if (g_infogetter->m_inputHardwares.at(i).PID == hardWareInfo.PID &&
                        g_infogetter->m_inputHardwares.at(i).VID == hardWareInfo.VID) {
                    hwinfo = g_infogetter->m_inputHardwares.at(i);
                }
            }
            break;
        case DeviceType::VideoOrAudio:
            for (int i = 0; i < g_infogetter->m_videoHardwares.length(); ++i) {
                if (g_infogetter->m_videoHardwares.at(i).PID == hardWareInfo.PID &&
                        g_infogetter->m_videoHardwares.at(i).VID == hardWareInfo.VID) {
                    hwinfo = g_infogetter->m_videoHardwares.at(i);
                }
            }
            break;
        case DeviceType::Other:
            for (int i = 0; i < g_infogetter->m_otherHardwares.length(); ++i) {
                if (g_infogetter->m_otherHardwares.at(i).PID == hardWareInfo.PID &&
                        g_infogetter->m_otherHardwares.at(i).VID == hardWareInfo.VID) {
                    hwinfo = g_infogetter->m_otherHardwares.at(i);
                }
            }
            break;
        default:
            break;
        }
        DeviceItem *item = new DeviceItem(hwinfo, m_installableListWidget);
        QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
        aItem->setSizeHint(QSize(0, ITEMHEIGET));
        m_installableListWidget->addItem(aItem);
        m_installableListWidget->setItemWidget(aItem, item);
    }
}

void DriverInstallWidget::installMultiDriverSlot() {
    if (g_signal->onUninstalling == true) {
        QString result = tr(
                    "Uninstalling drivers. Please wait for the uninstallation to complete before clicking again.");
        QMessageBox::information(this, tr("Uninstall Failed"), result);
        return;
    } else if (g_signal->onInstalling == true) {
        QString result = tr("Installing drivers. Please wait for the installation to complete before clicking again.");
        QMessageBox::information(this, tr("Install Failed"), result);
        return;
    }

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

    // 将所有的要安装的驱动设置为待安装状态
    for (int i = 0; i < m_installableListWidget->count(); ++i) {
        for (int j = 0; j < checkedItem.length(); ++j) {
            if (static_cast<DeviceItem *>(m_installableListWidget->itemWidget(
                                              m_installableListWidget->item(i)))->getDevName() == checkedItem[j]) {
                static_cast<DeviceItem *>(m_installableListWidget->itemWidget(
                                              m_installableListWidget->item(i)))->toLoadingPage();
            }
        }
    }

    connect(g_signal, &GlobalSignal::quitEventLoop, [=]() { eventLoop.quit(); });
    for (int i = 0; i < m_installableListWidget->count(); ++i) {
        for (int j = 0; j < checkedItem.length(); ++j) {
            if (static_cast<DeviceItem *>(m_installableListWidget->itemWidget(
                                              m_installableListWidget->item(i)))->getDevName() == checkedItem[j]) {
                static_cast<DeviceItem *>(m_installableListWidget->itemWidget(
                                              m_installableListWidget->item(i)))->clickInstall();
                eventLoop.exec();
            }
        }
    }
}

void DriverInstallWidget::updateMultiDriverSlot() {
    if (g_signal->onUninstalling == true) {
        QString result = tr(
                    "Uninstalling drivers. Please wait for the uninstallation to complete before clicking again.");
        QMessageBox::information(this, tr("Uninstall Failed"), result);
        return;
    } else if (g_signal->onInstalling == true) {
        QString result = tr("Installing drivers. Please wait for the installation to complete before clicking again.");
        QMessageBox::information(this, tr("Install Failed"), result);
        return;
    }

    QStringList checkedItem;         // 记录被选中的是哪几个
    for (int i = 0; i < m_upgradableListWidget->count(); ++i) {
        DeviceItem *item = static_cast<DeviceItem *>(m_upgradableListWidget->itemWidget(
                                                         m_upgradableListWidget->item(i)));
        if (item->m_isChecked) {
            checkedItem.append(item->getDevName());
        }
    }
    if (checkedItem.length() == 0) {
        return;
    }

    for (int i = 0; i < m_upgradableListWidget->count(); ++i) {
        for (int j = 0; j < checkedItem.length(); ++j) {
            if (static_cast<DeviceItem *>(m_upgradableListWidget->itemWidget(
                                              m_upgradableListWidget->item(i)))->getDevName() == checkedItem[j]) {
                static_cast<DeviceItem *>(m_upgradableListWidget->itemWidget(
                                              m_upgradableListWidget->item(i)))->toLoadingPage();
            }
        }
    }

    connect(g_signal, &GlobalSignal::quitEventLoop, [=]() { eventLoop.quit(); });
    for (int i = 0; i < m_upgradableListWidget->count(); ++i) {
        for (int j = 0; j < checkedItem.length(); ++j) {
            if (static_cast<DeviceItem *>(m_upgradableListWidget->itemWidget(
                                              m_upgradableListWidget->item(i)))->getDevName() == checkedItem[j]) {
                static_cast<DeviceItem *>(m_upgradableListWidget->itemWidget(
                                              m_upgradableListWidget->item(i)))->clickInstall();
                eventLoop.exec();
            }
        }
    }
}

HardWareInfo DriverInstallWidget::updateHwInfo(DeviceItem *item, QStringList driverName) {
    QStringList driverVersionlist;
    QString drivername = "";
    QString driverversion = "";
    for (int i = 0; i < driverName.length(); ++i) {
        driverVersionlist.append(getDriverVersion(QString(driverName[i]).split("=")[0]));
        drivername += QString(driverName[i]).split("=")[0] + ";";
    }
    for (int i = 0; i < driverVersionlist.length(); ++i) {
        driverversion += driverVersionlist[i] + ";";
    }
    QString deviceName = item->getDevName();
    DeviceType deviceType = item->getDevType();
    QString type = "";
    HardWareInfo hwinfo;
    switch (deviceType) {
    case DeviceType::SCANNER:
        type = "scanner";
        // 更新服务端数据
        refreScannerThread.start();
        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_scannerHardwares.length(); ++i) {
            if (g_infogetter->m_scannerHardwares[i].model == deviceName &&
                    g_infogetter->m_scannerHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_scannerHardwares[i].driver_now = drivername;
                g_infogetter->m_scannerHardwares[i].driver_version_now = driverversion;
                g_infogetter->m_scannerHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_scannerHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::GraphicCard:
        type = "graphiccard";
        // 更新服务端数据
        refreshGraphicCardInfo.start();
        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_graphicHardwares.length(); ++i) {
            if (g_infogetter->m_graphicHardwares[i].model == deviceName &&
                    g_infogetter->m_graphicHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_graphicHardwares[i].driver_now = drivername;
                g_infogetter->m_graphicHardwares[i].driver_version_now = driverversion;
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
                g_infogetter->m_soundCardHardwares[i].driver_now = drivername;
                g_infogetter->m_soundCardHardwares[i].driver_version_now = driverversion;
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
                g_infogetter->m_networkCardHardwares[i].driver_now = drivername;
                g_infogetter->m_networkCardHardwares[i].driver_version_now = driverversion;
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
                g_infogetter->m_bluetoothHardwares[i].driver_now = drivername;
                g_infogetter->m_bluetoothHardwares[i].driver_version_now = driverversion;
                g_infogetter->m_bluetoothHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_bluetoothHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::PRINTER:
        type = "printer";
        // 更新服务端数据
        refreshPrinterInfo.start();
        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_printerHardwares.length(); ++i) {
            if (&g_infogetter->m_printerHardwares[i].model == deviceName &&
                    g_infogetter->m_printerHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_printerHardwares[i].driver_now = drivername;
                g_infogetter->m_printerHardwares[i].driver_version_now = driverversion;
                g_infogetter->m_printerHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_printerHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::InputDev:
        // 更新服务端数据
        refreshInputDevInfo.start();
        for (int i = 0; i < g_infogetter->m_inputHardwares.length(); ++i) {
            if (&g_infogetter->m_inputHardwares[i].model == deviceName &&
                    g_infogetter->m_inputHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_inputHardwares[i].driver_now = drivername;
                g_infogetter->m_inputHardwares[i].driver_version_now = driverversion;
                g_infogetter->m_inputHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_inputHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::VideoOrAudio:
        // 更新服务端数据
        refreshVideoDevInfo.start();
        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_videoHardwares.length(); ++i) {
            if (&g_infogetter->m_videoHardwares[i].model == deviceName &&
                    g_infogetter->m_videoHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_videoHardwares[i].driver_now = drivername;
                g_infogetter->m_videoHardwares[i].driver_version_now = driverversion;
                g_infogetter->m_videoHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_videoHardwares[i];
                break;
            }
        }
        break;
    default:
        // 更新服务端数据
        refreshOtherDevInfo.start();
        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_otherHardwares.length(); ++i) {
            if (g_infogetter->m_otherHardwares[i].model == deviceName &&
                    g_infogetter->m_otherHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_otherHardwares[i].driver_now = drivername;
                g_infogetter->m_otherHardwares[i].driver_version_now = driverversion;
                g_infogetter->m_otherHardwares[i].upgradeable = false;
                hwinfo = g_infogetter->m_otherHardwares[i];
                break;
            }
        }
        break;
    }

    // 写入数据库
    if(DATABASE_INSTANCE.initDatabase() == DB_OP_SUCC){
        QStringList data;
        data.append(type);
        data.append(hwinfo.model);
        data.append(hwinfo.driver_now);
        data.append(hwinfo.driver_version_now);
        data.append("driverSize");
        g_signal->insertDataSignal(data);
    }

    return hwinfo;
}

HardWareInfo DriverInstallWidget::updateHwInfoForUninstall(DeviceType deviceType, QStringList driverName) {
    HardWareInfo hwinfo;
    bool contains = true;
    switch (deviceType) {
    case DeviceType::SCANNER:
        // 更新服务端数据
        refreScannerThread.start();
        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_scannerHardwares.length(); ++i) {
            for (int j = 0; j < driverName.length(); ++j) {
                if(!g_infogetter->m_scannerHardwares[i].driver_now.contains(driverName[j])){
                    contains = false;
                }
            }

            if (contains &&
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
        // 更新服务端数据
        refreshGraphicCardInfo.start();
        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_graphicHardwares.length(); ++i) {
            for (int j = 0; j < driverName.length(); ++j) {
                if(!g_infogetter->m_graphicHardwares[i].driver_now.contains(driverName[j])){
                    contains = false;
                }
            }

            if (contains) {
                g_infogetter->m_graphicHardwares[i].driver_now = "";
                g_infogetter->m_graphicHardwares[i].driver_version_now = "";
                g_infogetter->m_graphicHardwares[i].upgradeable = true;
                hwinfo = g_infogetter->m_graphicHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::PRINTER:
        // 更新服务端数据
        refreshPrinterInfo.start();
        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_printerHardwares.length(); ++i) {
            for (int j = 0; j < driverName.length(); ++j) {
                if(!g_infogetter->m_printerHardwares[i].driver_now.contains(driverName[j])){
                    contains = false;
                }
            }

            if (contains &&
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
        // 更新服务端数据
        refreshInputDevInfo.start();

        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_inputHardwares.length(); ++i) {
            for (int j = 0; j < driverName.length(); ++j) {
                if(!g_infogetter->m_inputHardwares[i].driver_now.contains(driverName[j])){
                    contains = false;
                }
            }
            if (contains) {
                g_infogetter->m_inputHardwares[i].driver_now = "";
                g_infogetter->m_inputHardwares[i].driver_version_now = "";
                g_infogetter->m_inputHardwares[i].upgradeable = true;
                hwinfo = g_infogetter->m_inputHardwares[i];
                break;
            }
        }
        break;
    case DeviceType::VideoOrAudio:
        // 更新服务端数据
        refreshVideoDevInfo.start();
        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_videoHardwares.length(); ++i) {
            for (int j = 0; j < driverName.length(); ++j) {
                if(!g_infogetter->m_videoHardwares[i].driver_now.contains(driverName[j])){
                    contains = false;
                }
            }
            if (contains && g_infogetter->m_videoHardwares[i].driverNameList.length() != 0) {
                g_infogetter->m_videoHardwares[i].driver_now = "";
                g_infogetter->m_videoHardwares[i].driver_version_now = "";
                g_infogetter->m_videoHardwares[i].upgradeable = true;
                hwinfo = g_infogetter->m_videoHardwares[i];
                break;
            }
        }
        break;
    default:
        // 更新服务端数据
        refreshOtherDevInfo.start();
        // 更新本地数据
        for (int i = 0; i < g_infogetter->m_otherHardwares.length(); ++i) {
            for (int j = 0; j < driverName.length(); ++j) {
                if(!g_infogetter->m_otherHardwares[i].driver_now.contains(driverName[j])){
                    contains = false;
                }
            }
            if (contains && g_infogetter->m_otherHardwares[i].driverNameList.length() != 0) {
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

void DriverInstallWidget::removeItemSlot(QList<HardWareInfo> hwInfo) {
    for (int i = 0; i < hwInfo.length(); ++i) {
        if(hwInfo[i].type == DeviceType::PRINTER){
            for (int j = 0; j < g_infogetter->m_printerHardwares.length(); ++j) {
                if(g_infogetter->m_printerHardwares[j].model == hwInfo[i].model){
                    g_infogetter->m_printerHardwares.removeAt(j);
                }
            }
        }

        if(hwInfo[i].type == DeviceType::SCANNER){
            for (int j = 0; j < g_infogetter->m_scannerHardwares.length(); ++j) {
                if(g_infogetter->m_scannerHardwares[j].model == hwInfo[i].model){
                    g_infogetter->m_scannerHardwares.removeAt(j);
                }
            }
        }
    }

    DeviceItem *item;
    for (int i = 0; i < m_installableListWidget->count(); ++i) {
        item = static_cast<DeviceItem *>(m_installableListWidget->itemWidget(
                                             m_installableListWidget->item(i)));
        if (item->getDevPID() == hwInfo.at(0).PID && item->getDevVID() == hwInfo.at(0).VID) {
            m_devList.removeOne(hwInfo.at(0).model);
            m_installabelNum--;
            m_installableListWidget->takeItem(i);
            caculateAndSetPageSize();
            return;
        }
    }

    for (int i = 0; i < m_upgradableListWidget->count(); ++i) {
        item = static_cast<DeviceItem *>(m_upgradableListWidget->itemWidget(
                                             m_upgradableListWidget->item(i)));
        if (item->getDevPID() == hwInfo.at(0).PID && item->getDevVID() == hwInfo.at(0).VID) {
            m_devList.removeOne(hwInfo.at(0).model);
            m_updateabelNum--;
            m_upgradableListWidget->takeItem(i);
            caculateAndSetPageSize();
            return;
        }
    }

    for (int i = 0; i < m_notUpdatedListWidget->count(); ++i) {
        item = static_cast<DeviceItem *>(m_notUpdatedListWidget->itemWidget(
                                             m_notUpdatedListWidget->item(i)));
        if (item->getDevPID() == hwInfo.at(0).PID && item->getDevVID() == hwInfo.at(0).VID) {
            m_devList.removeOne(hwInfo.at(0).model);
            m_notUpdatedNum--;
            m_notUpdatedListWidget->takeItem(i);
            caculateAndSetPageSize();
            return;
        }
    }

    for (int i = 0; i < m_notFindDriverListWidget->count(); ++i) {
        item = static_cast<DeviceItem *>(m_notFindDriverListWidget->itemWidget(
                                             m_notFindDriverListWidget->item(i)));
        if (item->getDevPID() == hwInfo.at(0).PID && item->getDevVID() == hwInfo.at(0).VID) {
            m_devList.removeOne(hwInfo.at(0).model);
            m_notFindDriverNum--;
            m_notFindDriverListWidget->takeItem(i);
            caculateAndSetPageSize();
            return;
        }
    }
}

void DriverInstallWidget::addItemSlot(QList<HardWareInfo> hwInfo) {
    for (int i = 0; i < hwInfo.length(); ++i) {
        if(hwInfo[i].type == DeviceType::PRINTER){
            g_infogetter->m_printerHardwares.append(hwInfo);

        }

        if(hwInfo[i].type == DeviceType::SCANNER){
            g_infogetter->m_scannerHardwares.append(hwInfo);
        }
    }

    if (hwInfo.at(0).upgradeable == true && hwInfo.at(0).driver_now == "") {
        // 在可安装栏添加
        m_installabelNum++;
        if (!m_devList.contains(hwInfo.at(0).model)) {
            m_devList.append(hwInfo.at(0).model);
            m_item = new DeviceItem(hwInfo.at(0));
            QListWidgetItem *aItem = new QListWidgetItem(m_installableListWidget);
            aItem->setSizeHint(QSize(0, ITEMHEIGET));
            m_installableListWidget->addItem(aItem);
            m_installableListWidget->setItemWidget(aItem, m_item);
            caculateAndSetPageSize();
        }
        return;
    }

    if (hwInfo.at(0).upgradeable == true && hwInfo.at(0).driver_now != "") {
        // 在可升级栏添加
        m_updateabelNum++;
        if (!m_devList.contains(hwInfo.at(0).model)) {
            m_devList.append(hwInfo.at(0).model);
            m_item = new DeviceItem(hwInfo.at(0));
            QListWidgetItem *aItem = new QListWidgetItem(m_upgradableListWidget);
            aItem->setSizeHint(QSize(0, ITEMHEIGET));
            m_upgradableListWidget->addItem(aItem);
            m_upgradableListWidget->setItemWidget(aItem, m_item);
            caculateAndSetPageSize();
        }
        return;
    }

    if (hwInfo.at(0).upgradeable == false && hwInfo.at(0).driver_now != "") {
        // 需要确定该设备的驱动是否为设备管理安装的
        QList<HardWareInfo> devList = DATABASE_INSTANCE.getDataList();
        if (devList.length() == 0) {
            return;
        }
        for (int j = 0; j < devList.length(); ++j) {
            if (devList[j].model == hwInfo.at(0).model) {
                if (devList[j].driver_now == hwInfo.at(0).driver_now &&
                        hwInfo.at(0).driver_version_now.contains(devList[j].driver_version_now)) {
                    if (!m_devList.contains(hwInfo.at(0).model)) {
                        m_notUpdatedNum++;
                        m_devList.append(hwInfo.at(0).model);
                        m_item = new DeviceItem(hwInfo[0]);
                        QListWidgetItem *aItem = new QListWidgetItem(m_notUpdatedListWidget);
                        aItem->setSizeHint(QSize(0, ITEMHEIGET));
                        m_notUpdatedListWidget->addItem(aItem);
                        m_notUpdatedListWidget->setItemWidget(aItem, m_item);
                        caculateAndSetPageSize();
                    }
                } else {
                    continue;
                }
            }
        }
        return;
    }

    if (hwInfo.at(0).upgradeable == false && hwInfo.at(0).driver_now == "") {
        // 在无可安装驱动栏添加
        m_notFindDriverNum++;
        if (!m_devList.contains(hwInfo.at(0).model)) {
            m_devList.append(hwInfo.at(0).model);
            if (hwInfo[0].type != DeviceType::NONE) {
            } else {
                hwInfo[0].type = DeviceType::Other;
            }
            m_item = new DeviceItem(hwInfo.at(0));
            QListWidgetItem *aItem = new QListWidgetItem(m_notFindDriverListWidget);
            aItem->setSizeHint(QSize(0, ITEMHEIGET));
            m_notFindDriverListWidget->addItem(aItem);
            m_notFindDriverListWidget->setItemWidget(aItem, m_item);
            caculateAndSetPageSize();
        }
        return;
    }
}

void DriverInstallWidget::uninstallUpdateDriverSlot(DeviceItem *item, QStringList driverName) {
    m_uninstaller = new Uninstaller(driverName);

    m_tempupdateitem = item;
    m_tempDriverName = driverName;
    connect(m_uninstaller, &Uninstaller::succeed, this, &DriverInstallWidget::uninstallUpdateSuccessSlot);
    connect(m_uninstaller, &Uninstaller::failed, [=]() {
        g_signal->onUninstalling = false;
        item->setToInstallPage();
    });
    connect(m_uninstaller, &Uninstaller::progressChanged, item, &DeviceItem::changeProgress);

    m_uninstaller->startWorker();
}

void DriverInstallWidget::uninstallUpdateSuccessSlot() {
    g_signal->onUninstalling = false;
    // 从统计内容中删除
    m_devList.removeOne(m_tempupdateitem->getDevName());
    // 更新信息
    HardWareInfo hardWareInfo = updateHwInfoForUninstall(m_tempupdateitem->getDevType(), m_tempDriverName);
    // 删除数据库内容
    g_signal->deleteDataSignal(hardWareInfo.model);
    // 添加到可安装页面
    addNewInstallabelItemSlot(hardWareInfo);
}

void DriverInstallWidget::uninstallDriverSlot(UninstallItem *item, QStringList driverName) {
    m_uninstaller = new Uninstaller(driverName);

    m_tempitem = item;
    m_tempDriverName = driverName;
    connect(m_uninstaller, &Uninstaller::succeed, this, &DriverInstallWidget::uninstallSuccessSlot);
    connect(m_uninstaller, &Uninstaller::failed, [=]() { g_signal->onUninstalling = false; });
    connect(m_uninstaller, &Uninstaller::progressChanged, item, &UninstallItem::changeProgress);

    m_uninstaller->startWorker();
}

void DriverInstallWidget::uninstallSuccessSlot() {
    g_signal->onUninstalling = false;
    // 从统计内容中删除
    m_devList.removeOne(m_tempitem->getDevName());
    // 更新信息
    HardWareInfo hardWareInfo = updateHwInfoForUninstall(m_tempitem->getDevType(), m_tempDriverName);
    // 删除数据库内容
    g_signal->deleteDataSignal(hardWareInfo.model);
    // 添加到可安装页面
    addNewInstallabelItemSlot(hardWareInfo);
}

static QString getDriverVersion(QString driverName) {
    // 构建 dpkg 命令
    QStringList arguments;
    arguments << "-s" << driverName;

    // 启动 dpkg 命令
    QProcess process;
    process.start("dpkg", arguments);
    process.waitForFinished();

    // 读取 dpkg 命令的输出
    QByteArray result = process.readAllStandardOutput();

    // 在输出中查找版本信息
    QString versionString = "Version:";
    int versionIndex = result.indexOf(versionString);
    if (versionIndex != -1) {
        // 提取版本号
        QString version = QString::fromUtf8(result.mid(versionIndex + versionString.length()).trimmed()).split("\n")[0];
        return version;
    } else {
        qDebug() << "Package version not found.";
        return QString();
    }
}

static QString getPackageVersion(const QString &packageName) {
    QString res{QString()};
    if (packageName.isEmpty()) {
        return res;
    }
    QString cmd = QString("/usr/bin/dpkg-query -W -f='${Version}\\n' %1").arg(packageName);
    res = getRetFromCommand(cmd.split(" "));
    return res;
}

static QString getRetFromCommand(const QStringList &command) {
    QProcess proc;
    QStringList options;
    options << "-c" << command.join(" ");
    proc.closeWriteChannel();
    proc.start("bash", options);
    if (!proc.waitForFinished())
        return "";
    QString res = QString(proc.readAll());
    proc.close();
    if (res.right(1) == "\n")
        res.chop(1);
    return res;
}
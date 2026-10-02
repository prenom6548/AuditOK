#include "hwwidget.h"
#include <QDebug>
#include <QMetaObject>
#include <QScreen>
// #include <libkydevicesec_bluetooth.h>
#include "include/common.h"
#include "include/globalsignal.h"
#include <gsettingmonitor.h>
#include "hardwareinfogetter.h"
#include "kom-configure.h"
#include "include/hwinfostorage.h"

HwWidget::HwWidget(DeviceInfo *devinfo, QWidget *parent)
        : m_devInfo(devinfo),
          QWidget(parent),
          m_iconLabel(new QLabel),
          m_textLabel(new QLabel),
          m_modelLabel(new K::TextLabel),
          m_activeStackWidget(new QStackedWidget),
          m_activateStatus(new QLabel),
          m_deactiveTips(new QLabel),
          m_btnStackedWidget(new QStackedWidget),
          m_spreadOutBtn(new kdk::KBorderlessButton),
          m_spreadInBtn(new kdk::KBorderlessButton),
          m_layout(new QHBoxLayout),
          m_timer(new QTimer(this)),
          m_menu(new QMenu),
          m_actionExport(new QAction),
          m_actionActive(new QAction),
          m_actionDeactive(new QAction) {
    if (QGSettings::isSchemaInstalled(KYLIN_OS_MANAGER)) {
        m_data = new QGSettings(KYLIN_OS_MANAGER);
    }
    initGui();
    setContent();
    setIconColor();
    initConnect();
}

HwWidget::HwWidget(QString devType, QMap<QString, QString> devInfo, QWidget *parent)
        : QWidget(parent),
          m_iconLabel(new QLabel),
          m_textLabel(new QLabel),
          m_modelLabel(new K::TextLabel),
          m_activeStackWidget(new QStackedWidget),
          m_activateStatus(new QLabel),
          m_deactiveTips(new QLabel),
          m_btnStackedWidget(new QStackedWidget),
          m_spreadOutBtn(new kdk::KBorderlessButton),
          m_spreadInBtn(new kdk::KBorderlessButton),
          m_layout(new QHBoxLayout),
          m_timer(new QTimer(this)),
          m_menu(new QMenu),
          m_actionExport(new QAction),
          m_actionActive(new QAction),
          m_actionDeactive(new QAction){
    initGui();
    setContent(devType, devInfo);
    setIconColor();
    initConnect();
}



HwWidget::~HwWidget() {
    for (auto conn: m_connections) {
        disconnect(conn);
    }
}

QString HwWidget::getDevName() {
    return m_modelLabel->getText();
}

QString HwWidget::getDevType() {
    return m_textLabel->text();
}

QString HwWidget::getMonitorInterface() {
    return m_monitorInterface;
}


void HwWidget::initGui() {
    m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
    m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
    m_modelLabel->setMinimumWidth(TREEITEMMODELLABELWIDTH);
    m_activateStatus->setText(tr("Disable"));
    m_deactiveTips->setPixmap(getIcon("preferences-system-details-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
            HARDWAREINFOWIDGETBUTTONSIZE));
    m_deactiveTips->setToolTip(tr("This device can be disabled/enabled"));
    m_deactiveTips->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_activeStackWidget->addWidget(m_activateStatus);
    m_activeStackWidget->addWidget(m_deactiveTips);
    m_activeStackWidget->setContentsMargins(22, 0, 0, 0);

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
    m_layout->addWidget(m_activeStackWidget, 0, Qt::AlignRight);
    m_layout->addWidget(m_btnStackedWidget, 0, Qt::AlignRight);
    this->setLayout(m_layout);

    m_activeStackWidget->hide();

    m_actionExport->setText(tr("Export To Clipboard"));
    m_actionActive->setText(tr("Activate"));
    m_actionDeactive->setText(tr("Deactive"));
}

void HwWidget::setContent() {
    if (auto cpuInfo = dynamic_cast<CpuInfo *>(m_devInfo)) {
        m_pixMap = getIcon("ukui-zs-processor-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(getIcon("ukui-zs-processor-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("CPU"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(cpuInfo->getValue(HwInfoSingleton::instance().m_cpuMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("CPU")).arg(cpuInfo->getValue(HwInfoSingleton::instance().m_cpuMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_CPU).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(cpuInfo->model == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Graphic Card")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_cpuMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            cpuInfo->setValue(HwInfoSingleton::instance().m_cpuMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_cpuInfo) {
            if (key == "1") {
                continue;
            }
            QString value = cpuInfo->getValue(HwInfoSingleton::instance().m_cpuMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);

                if (key == QApplication::tr("CPU Current Frequency") || key == QApplication::tr("CPU Usage Rate") ||
                    key == QApplication::tr("CPU Temperature")) {
                    connect(m_timer, &QTimer::timeout, [=]() {
                        QString value = cpuInfo->getValue(HwInfoSingleton::instance().m_cpuMap.value(key));
                        item->setLabelText(value);
                        for (int i = 0; i < m_explorContent.length(); ++i) {
                            if (m_explorContent.at(i).split("\t").at(0) == key) {
                                m_explorContent[i] = QString("%1\t%2").arg(key).arg(value);
                            }
                        }
                    });
                    m_timer->start(1000);
                }
            }
        }
        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto memoryInfo = dynamic_cast<MemoryInfo *>(m_devInfo)) {
        m_pixMap = getIcon("ukui-zs-memory-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("ukui-zs-memory-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Memory"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(memoryInfo->getValue(HwInfoSingleton::instance().m_memoryMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Memory")).arg(memoryInfo->getValue(HwInfoSingleton::instance().m_memoryMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_MEMORY).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(memoryInfo->displayname == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Graphic Card")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_memoryMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            memoryInfo->setValue(HwInfoSingleton::instance().m_memoryMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_memoryInfo) {
            if (key == "1") {
                continue;
            }
            QString value = memoryInfo->getValue(HwInfoSingleton::instance().m_memoryMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);

                if (key == QApplication::tr("Memory Usage")) {
                    connect(m_timer, &QTimer::timeout, [=]() {
                        item->setLabelText(memoryInfo->getValue(HwInfoSingleton::instance().m_memoryMap.value(key)));
                    });
                    m_timer->start(1000);
                }
            }
        }

        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }

    } else if (auto graphicCardInfo = dynamic_cast<GraphicCardInfo *>(m_devInfo)) {
        m_pixMap = getIcon("ukui-graphics-card-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(getIcon("ukui-graphics-card-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Graphic Card"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(graphicCardInfo->getValue(HwInfoSingleton::instance().m_graphicCardMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Graphic Card")).arg(
                graphicCardInfo->getValue(HwInfoSingleton::instance().m_graphicCardMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_GRAPHICCARD).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(graphicCardInfo->model == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Graphic Card")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_graphicCardMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            graphicCardInfo->setValue(HwInfoSingleton::instance().m_graphicCardMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_graphicCardInfo) {
            if (key == "1") {
                continue;
            }
            QString value = graphicCardInfo->getValue(HwInfoSingleton::instance().m_graphicCardMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);

                if (key == QApplication::tr("GDDR Size")) {
                    connect(m_timer, &QTimer::timeout, [=]() {
                        item->setLabelText(graphicCardInfo->getValue(HwInfoSingleton::instance().m_graphicCardMap.value(key)));
                    });
                    m_timer->start(1000);
                }
            }
        }
        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto baseboardInfo = dynamic_cast<BaseboardInfo *>(m_devInfo)) {
        m_pixMap = getIcon("ukui-mother-board-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(getIcon("ukui-mother-board-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Mother Board"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(baseboardInfo->getValue(HwInfoSingleton::instance().m_baseboardMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Mother Board")).arg(
                baseboardInfo->getValue(HwInfoSingleton::instance().m_baseboardMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_BASEBOARD).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(baseboardInfo->model == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Mother Board")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_baseboardMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            baseboardInfo->setValue(HwInfoSingleton::instance().m_baseboardMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_baseboardInfo) {
            if (key == "1") {
                continue;
            }
            QString value = baseboardInfo->getValue(HwInfoSingleton::instance().m_baseboardMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }

        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }

    } else if (auto networkCardInfo = dynamic_cast<NetworkCardInfo *>(m_devInfo)) {
        NetworkCardInfo *mynetwork = new NetworkCardInfo(networkCardInfo);
        m_devInfo = mynetwork;
        m_pixMap = getIcon("ukui-zs-network-card-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(getIcon("ukui-zs-network-card-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Network Card"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        if (networkCardInfo->getValue(HwInfoSingleton::instance().m_networkCardMap.value("1")).trimmed() == "") {
            return;
        }
        m_modelLabel->SetText(networkCardInfo->getValue(HwInfoSingleton::instance().m_networkCardMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Network Card")).arg(
                networkCardInfo->getValue(HwInfoSingleton::instance().m_networkCardMap.value("1"))));
        m_activeStackWidget->show();

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_NETWORKCARD).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(networkCardInfo->name == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Network Card")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_networkCardMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            networkCardInfo->setValue(HwInfoSingleton::instance().m_networkCardMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_networkCardInfo) {
            if (key == "1") {
                continue;
            }
            QString value = networkCardInfo->getValue(HwInfoSingleton::instance().m_networkCardMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }

        // 设置右键启用禁用
        if (networkCardInfo->isWireless == 1) {
            if (m_data->get("wirelessnetworkcard").toString() == "deactivate") {
                setDisableStatus(true);
                m_menu->addAction(m_actionActive);
            }
            if (m_data->get("wirelessnetworkcard").toString() == "activate") {
                setDisableStatus(false);
                m_menu->addAction(m_actionDeactive);
            }
        } else {
            if (m_data->get("wireldnetworkcard").toString() == "deactivate") {
                setDisableStatus(true);
                m_menu->addAction(m_actionActive);
            }
            if (m_data->get("wireldnetworkcard").toString() == "activate") {
                setDisableStatus(false);
                m_menu->addAction(m_actionDeactive);
            }
        }

        m_isWireless = networkCardInfo->isWireless;
        connect(m_actionActive, &QAction::triggered, this, &HwWidget::actionActiveNetworkCardSlot);
        connect(m_actionDeactive, &QAction::triggered, this, &HwWidget::actionDeactiveNetworkCardSlot);
        if (networkCardInfo->isWireless == 1) {
            connect(g_signal, &GlobalSignal::enableWirelessNetworkCardSignal, this, [=]() {
                m_menu->removeAction(m_actionActive);
                m_menu->addAction(m_actionDeactive);
                setDisableStatus(false);
            });
            connect(g_signal, &GlobalSignal::disableWirelessNetworkCardSignal, this, [=]() {
                m_menu->removeAction(m_actionDeactive);
                m_menu->addAction(m_actionActive);
                setDisableStatus(true);
            });
        } else {
            connect(g_signal, &GlobalSignal::enableWiredNetworkCardSignal, this, [=]() {
                m_menu->removeAction(m_actionDeactive);
                m_menu->addAction(m_actionActive);
                setDisableStatus(false);
            });
            connect(g_signal, &GlobalSignal::disableWiredNetworkCardSignal, this, [=]() {
                m_menu->removeAction(m_actionActive);
                m_menu->addAction(m_actionDeactive);
                setDisableStatus(true);
            });
        }

        connect(m_timer, &QTimer::timeout, this, &HwWidget::setNetworkStatus);
        m_timer->start(2000);

        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto diskInfo = dynamic_cast<DiskInfo *>(m_devInfo)) {
        m_pixMap = getIcon("ukui-zs-hard-disk-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(getIcon("ukui-zs-hard-disk-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Disk"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(diskInfo->getValue(HwInfoSingleton::instance().m_diskMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Disk")).arg(diskInfo->getValue(HwInfoSingleton::instance().m_diskMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_DISK).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(diskInfo->model == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Disk")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_diskMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            diskInfo->setValue(HwInfoSingleton::instance().m_diskMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_diskInfo) {
            if (key == "1") {
                continue;
            }
            QString value = diskInfo->getValue(HwInfoSingleton::instance().m_diskMap.value(key));

            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }

        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto monitorInfo = dynamic_cast<MonitorInfo *>(m_devInfo)) {
        m_pixMap = getIcon("computer-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("computer-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Monitor"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(monitorInfo->getValue(HwInfoSingleton::instance().m_monitorMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Monitor")).arg(monitorInfo->getValue(HwInfoSingleton::instance().m_monitorMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_MONITOR).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(monitorInfo->name == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Disk")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_monitorMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            monitorInfo->setValue(HwInfoSingleton::instance().m_monitorMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_monitorInfo) {
            if (key == "1") {
                continue;
            }
            QString value = monitorInfo->getValue(HwInfoSingleton::instance().m_monitorMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);

                QString interface = monitorInfo->getValue(HwInfoSingleton::instance().m_monitorMap.value(QObject::tr("Interface")));
                m_monitorInterface = interface;
                m_screens = QApplication::screens();
                QScreen *screen = nullptr;
                for (int i = 0; i < m_screens.length(); ++i) {
                    if (m_screens[i]->name() == interface) {
                        screen = m_screens[i];
                    }
                }

                if (key == QApplication::tr("Resolution")) {
                    auto conn = connect(m_timer, &QTimer::timeout, [=]() {
                        m_screens = QApplication::screens();
                        if (screen != nullptr && m_screens.contains(screen)) {
                            item->setLabelText(getResolution(screen->name()));
                        }
                    });
                    m_connections.append(conn);
                }

                if (key == QApplication::tr("Main Monitor")) {
                    auto conn = connect(m_timer, &QTimer::timeout, this, [=]() {
                        m_screens = QApplication::screens();
                        if (screen == QGuiApplication::primaryScreen() && screen != nullptr &&
                            m_screens.contains(screen)) {
                            item->setLabelText(QApplication::tr("Yes"));
                        } else {
                            item->setLabelText(QApplication::tr("No"));
                        }
                    });
                    m_connections.append(conn);
                }
            }
        }
        m_timer->start(5000);

        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto soundCardInfo = dynamic_cast<SoundCardInfo *>(m_devInfo)) {
        // 设置内容
        m_pixMap = getIcon("audio-volume-high-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(getIcon("audio-volume-high-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Sound Card"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(soundCardInfo->getValue(HwInfoSingleton::instance().m_soundCardMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Sound Card")).arg(
                soundCardInfo->getValue(HwInfoSingleton::instance().m_soundCardMap.value("1"))));
        m_activeStackWidget->show();

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_SOUNDCARD).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(soundCardInfo->model == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Disk")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_soundCardMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            soundCardInfo->setValue(HwInfoSingleton::instance().m_soundCardMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key:HwInfoSingleton::instance().m_soundCardInfo) {
            if (key == "1") {
                continue;
            }
            QString value = soundCardInfo->getValue(HwInfoSingleton::instance().m_soundCardMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }

        // 设置右键启用禁用
        if (m_data->get("soundcard").toString() == "activate") {
            m_menu->addAction(m_actionDeactive);
            setDisableStatus(false);
            connect(m_actionActive, &QAction::triggered, this, &HwWidget::actionActiveSoundCardSlot);
            connect(m_actionDeactive, &QAction::triggered, this, &HwWidget::actionDeactiveSoundCardSlot);
        } else {
            m_menu->addAction(m_actionActive);
            setDisableStatus(true);
            connect(m_actionActive, &QAction::triggered, this, &HwWidget::actionActiveSoundCardSlot);
            connect(m_actionDeactive, &QAction::triggered, this, &HwWidget::actionDeactiveSoundCardSlot);
        }

        connect(m_timer, &QTimer::timeout, [=]() {
            if (!soundCardInfo->checkStatus()) {
                setDisableStatus(false);
                m_data->set("soundcard", "activate");
                if (m_menu->actions().contains(m_actionActive)) {
                    m_menu->removeAction(m_actionActive);
                    m_menu->addAction(m_actionDeactive);
                }
            } else {
                setDisableStatus(true);
                m_data->set("soundcard", "deactivate");
                if (m_menu->actions().contains(m_actionDeactive)) {
                    m_menu->removeAction(m_actionDeactive);
                    m_menu->addAction(m_actionActive);
                }
            }
        });
        m_timer->start(2000);

        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto keyboardInfo = dynamic_cast<KeyboardInfo *>(m_devInfo)) {
        m_pixMap = getIcon("input-keyboard-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("input-keyboard-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Keyboard"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(keyboardInfo->getValue(HwInfoSingleton::instance().m_keyboardMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Keyboard")).arg(keyboardInfo->getValue(HwInfoSingleton::instance().m_keyboardMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_KEYBOARD).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(keyboardInfo->name == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Keyboard")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_keyboardMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            keyboardInfo->setValue(HwInfoSingleton::instance().m_keyboardMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_keyboardInfo) {
            if (key == "1") {
                continue;
            }
            QString value = keyboardInfo->getValue(HwInfoSingleton::instance().m_keyboardMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto mouseInfo = dynamic_cast<MouseInfo *>(m_devInfo)) {
        m_pixMap = getIcon("input-mouse-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("input-mouse-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Mouse"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(mouseInfo->getValue(HwInfoSingleton::instance().m_mouseMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Mouse")).arg(mouseInfo->getValue(HwInfoSingleton::instance().m_mouseMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_MOUSE).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(mouseInfo->name == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Mouse")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_mouseMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            mouseInfo->setValue(HwInfoSingleton::instance().m_mouseMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_mouseInfo) {
            if (key == "1") {
                continue;
            }
            QString value = mouseInfo->getValue(HwInfoSingleton::instance().m_mouseMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto bluetoothInfo = dynamic_cast<BluetoothInfo *>(m_devInfo)) {
        BluetoothInfo *mybluetooth = new BluetoothInfo(bluetoothInfo);
        m_devInfo = mybluetooth;
        // 设置展示内容
        m_pixMap = getIcon("bluetooth-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("bluetooth-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Bluetooth"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(bluetoothInfo->getValue(HwInfoSingleton::instance().m_bluetoothMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Bluetooth")).arg(
                bluetoothInfo->getValue(HwInfoSingleton::instance().m_bluetoothMap.value("1"))));
        m_activeStackWidget->show();

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_BLUETOOTH).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(bluetoothInfo->model == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Bluetooth")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_bluetoothMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            bluetoothInfo->setValue(HwInfoSingleton::instance().m_bluetoothMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_bluetoothInfo) {
            if (key == "1") {
                continue;
            }
            QString value = bluetoothInfo->getValue(HwInfoSingleton::instance().m_bluetoothMap.value(key));
            if (key == QObject::tr("Address")) {
                m_bluetoothAddress = value;
            }
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }

        if (m_data->get("bluetooth").toString() == "deactivate") {
            setDisableStatus(true);
            m_menu->addAction(m_actionActive);
            connect(m_actionActive, &QAction::triggered, g_signal, &GlobalSignal::activateBluetooth);
            connect(g_signal, &GlobalSignal::activateBluetooth, this, &HwWidget::actionActiveBluetoothSlot);
            connect(m_actionDeactive, &QAction::triggered, g_signal, &GlobalSignal::deactivateBluetooth);
            connect(g_signal, &GlobalSignal::deactivateBluetooth, this, &HwWidget::actionDeactiveBluetoothSlot);
        } else {
            setDisableStatus(false);
            m_menu->addAction(m_actionDeactive);
            connect(m_actionActive, &QAction::triggered, g_signal, &GlobalSignal::activateBluetooth);
            connect(g_signal, &GlobalSignal::activateBluetooth, this, &HwWidget::actionActiveBluetoothSlot);
            connect(m_actionDeactive, &QAction::triggered, g_signal, &GlobalSignal::deactivateBluetooth);
            connect(g_signal, &GlobalSignal::deactivateBluetooth, this, &HwWidget::actionDeactiveBluetoothSlot);
        }

        connect(m_timer, &QTimer::timeout, this, &HwWidget::setBluetoothStatus);
        m_timer->start(2000);

        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto cdInfo = dynamic_cast<CDInfo *>(m_devInfo)) {
        m_pixMap = getIcon("ukui-burn-cd-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("ukui-burn-cd-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("CD"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(cdInfo->getValue(HwInfoSingleton::instance().m_cdInfoMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("CD")).arg(cdInfo->getValue(HwInfoSingleton::instance().m_cdInfoMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_CD).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(cdInfo->model == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("CD")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_cdInfoMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            cdInfo->setValue(HwInfoSingleton::instance().m_cdInfoMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_cdInfo) {
            if (key == "1") {
                continue;
            }
            QString value = cdInfo->getValue(HwInfoSingleton::instance().m_cdInfoMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }

        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto batteryInfo = dynamic_cast<BatteryInfo *>(m_devInfo)) {
        m_pixMap = getIcon("battery-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("battery-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Battery"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(batteryInfo->getValue(HwInfoSingleton::instance().m_batteryInfoMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Battery")).arg(
                batteryInfo->getValue(HwInfoSingleton::instance().m_batteryInfoMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_BATTERY).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(batteryInfo->model == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Battery")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_batteryInfoMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            batteryInfo->setValue(HwInfoSingleton::instance().m_batteryInfoMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_batteryInfo) {
            if (key == "1") {
                continue;
            }
            QString value = batteryInfo->getValue(HwInfoSingleton::instance().m_batteryInfoMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);

                if (key == QApplication::tr("Energy") || key == QApplication::tr("Time To Empty") ||
                    key == QApplication::tr("Percentage") || key == QApplication::tr("State")) {
                    connect(m_timer, &QTimer::timeout, [=]() {
                        item->setLabelText(batteryInfo->getValue(HwInfoSingleton::instance().m_batteryInfoMap.value(key)));
                    });
                    m_timer->start(3000);
                }

            }
        }

        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto fanInfo = dynamic_cast<FanInfo *>(m_devInfo)) {
        m_pixMap = getIcon("ukui-zs-iso-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("ukui-zs-iso-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Fan"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(fanInfo->getValue(HwInfoSingleton::instance().m_fanInfoMap.value("1")));
        m_explorContent.append(
                QString("%1\t%2").arg(tr("Fan")).arg(fanInfo->getValue(HwInfoSingleton::instance().m_fanInfoMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_FAN).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(fanInfo->name == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Fan")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_fanInfoMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            fanInfo->setValue(HwInfoSingleton::instance().m_fanInfoMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_fanInfo) {
            if (key == "1") {
                continue;
            }
            QString value = fanInfo->getValue(HwInfoSingleton::instance().m_fanInfoMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }

        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    } else if (auto cameraInfo = dynamic_cast<CameraInfo *>(m_devInfo)) {
        m_pixMap = getIcon("camera-photo-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("camera-photo-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Camera"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(cameraInfo->getValue(HwInfoSingleton::instance().m_cameraInfoMap.value("1")));
        m_explorContent.append(QString("%1\t%2").arg(tr("Camera")).arg(
                cameraInfo->getValue(HwInfoSingleton::instance().m_cameraInfoMap.value("1"))));

        //从ini中获取内容需要修改的内容
        QMap<QString, QString> additionMap;
        kom::Configure conf;
        QString info = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_CAMERA).toStringList().join(",");
        QStringList infolist = info.split("|");
        for (int i = 0; i < infolist.length(); ++i) {
            if(infolist.at(i).contains("Change")) {
                QStringList infos = infolist.at(i).split(",");
                if(cameraInfo->name == infos[1]){
                    for (int j = 2; j < infos.length(); ++j) {
                        QStringList keyAndValue = infos[j].split("=");
                        if (keyAndValue[0] == "Name"){
                            m_modelLabel->SetText(keyAndValue[1]);
                            m_explorContent.clear();
                            m_explorContent.append(QString("%1\t%2").arg(tr("Camera")).arg(keyAndValue[1]));
                            continue;
                        }
                        if(HwInfoSingleton::instance().m_cameraInfoMap.keys().contains(keyAndValue[0])){
                            // 如果map中存在，说明是修改某一项的值
                            cameraInfo->setValue(HwInfoSingleton::instance().m_cameraInfoMap.value(keyAndValue[0]), keyAndValue[1]);
                        }else{
                            // 如果map中不存在，说明是添加一项
                            additionMap.insert(keyAndValue[0], keyAndValue[1]);
                        }
                    }
                }
            }
        }

        for (const QString &key: HwInfoSingleton::instance().m_cameraInfo) {
            if (key == "1") {
                continue;
            }
            QString value = cameraInfo->getValue(HwInfoSingleton::instance().m_cameraInfoMap.value(key));
            if (value != "") {
                m_explorContent.append(QString("%1\t%2").arg(key).arg(value));
                itemCount++;
                TreeItem *item = new TreeItem(key, value, this);
                item->setAutoFillBackground(true);
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
                m_treeItemList.append(item);
                m_treeWidgetItemList.append(childItem);
            }
        }
        // 添加用户自定义的显示内容
        for (int i = 0; i < additionMap.keys().length(); ++i) {
            m_explorContent.append(QString("%1\t%2").arg(additionMap.keys()[i]).arg(additionMap.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(additionMap.keys()[i], additionMap.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
    }
}

void HwWidget::setContent(QString devType, QMap<QString, QString> devInfo){
    if (devType == "Memory"){
        m_pixMap = getIcon("ukui-zs-memory-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("ukui-zs-memory-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Memory"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Memory")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }
    // 显卡
    if (devType == "GraphicCard"){
        m_pixMap = getIcon("ukui-graphics-card-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(getIcon("ukui-graphics-card-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Graphic Card"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Graphic Card")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 网卡
    if (devType == "NetworkCard"){
        m_pixMap = getIcon("ukui-zs-network-card-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(getIcon("ukui-zs-network-card-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Network Card"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Network Card")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 硬盘
    if (devType == "Disk"){
        m_pixMap = getIcon("ukui-zs-hard-disk-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(getIcon("ukui-zs-hard-disk-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Disk"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Disk")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 显示器
    if (devType == "Monitor"){
        m_pixMap = getIcon("computer-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("computer-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Monitor"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Monitor")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 声卡
    if (devType == "SoundCard"){
        m_pixMap = getIcon("audio-volume-high-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(getIcon("audio-volume-high-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Sound Card"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Sound Card")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 键盘
    if (devType == "Keyboard"){
        m_pixMap = getIcon("input-keyboard-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(
                HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("input-keyboard-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Keyboard"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Keyboard")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 鼠标
    if (devType == "Mouse"){
        m_pixMap = getIcon("input-mouse-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("input-mouse-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Mouse"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Mouse")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 蓝牙
    if (devType == "Bluetooth"){
        m_pixMap = getIcon("bluetooth-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("bluetooth-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Bluetooth"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Bluetooth")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 光驱
    if (devType == "Cd"){
        m_pixMap = getIcon("ukui-burn-cd-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("ukui-burn-cd-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("CD"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("CD")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 电池
    if (devType == "Battery"){
        m_pixMap = getIcon("battery-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("battery-sylmbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Battery"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Battery")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 光驱
    if (devType == "Fan"){
        m_pixMap = getIcon("ukui-zs-iso-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("ukui-zs-iso-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Fan"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Fan")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
    }

    // 摄像头
    if (devType == "Camera"){
        m_pixMap = getIcon("camera-photo-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_iconLabel->setPixmap(
                getIcon("camera-photo-symbolic", HARDWAREINFOWIDGETBUTTONSIZE).pixmap(HARDWAREINFOWIDGETBUTTONSIZE));
        m_iconLabel->setFixedSize(HARDWAREINFOWIDGETBUTTONSIZE);
        m_textLabel->setText(tr("Camera"));
        m_textLabel->setFixedWidth(TREEITEMLABELWIDTH);
        m_modelLabel->SetText(devInfo.value("Name"));
        m_explorContent.append(QString("%1\t%2").arg(tr("Camera")).arg(devInfo.value("Name")));
        for (int i = 0; i < devInfo.values().length(); ++i) {
            if (devInfo.keys()[i] == "Name") {
                continue;
            }
            m_explorContent.append(QString("%1\t%2").arg(devInfo.keys()[i]).arg(devInfo.values()[i]));
            itemCount++;
            TreeItem *item = new TreeItem(devInfo.keys()[i], devInfo.values()[i], this);
            QTreeWidgetItem *childItem = new QTreeWidgetItem();
            childItem->setSizeHint(0, TREEWIDGETITEMSIZEHINT);
            m_treeItemList.append(item);
            m_treeWidgetItemList.append(childItem);
        }
        return;
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

        QIcon icon = getIcon("preferences-system-details-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        QPixmap pixMap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_deactiveTips->setPixmap(drawSymbolicColoredPixmap(pixMap, "white"));

    } else {
        m_iconLabel->setPixmap(drawSymbolicColoredPixmap(m_pixMap, "black"));

        QIcon icon = getIcon("preferences-system-details-symbolic", HARDWAREINFOWIDGETBUTTONSIZE);
        QPixmap pixMap = icon.pixmap(HARDWAREINFOWIDGETBUTTONSIZE);
        m_deactiveTips->setPixmap(drawSymbolicColoredPixmap(pixMap, "black"));
    }

    float fontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    QFont font;
    font.setPointSize(fontSize);
    m_textLabel->setFont(font);
    m_activateStatus->setFont(font);
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

QIcon HwWidget::getIcon(QString iconName, QSize iconSize) {
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
    // e84825e03eecc296ead1fbf90ec63592f7e90e6e  commit ID  在这个版本修改了之前调用工具箱dbs的禁用启用策略
    m_menu->removeAction(m_actionActive);
    m_menu->addAction(m_actionDeactive);
    QString result = m_data->get("bluetooth").toString();
    QString str = "activate";
    m_data->set("bluetooth", str);
    //kdk_device_set_bluetooth_status(KDK_BLUETOOTH_ENABLE);
    emit g_signal->enableBluetoothSignal();
}

void HwWidget::actionDeactiveBluetoothSlot() {
    // e84825e03eecc296ead1fbf90ec63592f7e90e6e  commit ID  在这个版本修改了之前调用工具箱dbs的禁用启用策略
    m_menu->removeAction(m_actionDeactive);
    m_menu->addAction(m_actionActive);
    QString result = m_data->get("bluetooth").toString();
    QString str = "deactivate";
    m_data->set("bluetooth", str);
    //kdk_device_set_bluetooth_status(KDK_BLUETOOTH_DISABLE);
    emit g_signal->disableBluetoothSignal();
}

void HwWidget::actionActiveSoundCardSlot() {
    auto soundCardInfo = dynamic_cast<SoundCardInfo *>(m_devInfo);
    soundCardInfo->enableSoundCard();
    QString str = "activate";
    m_data->set("soundcard", str);
    setDisableStatus(false);
    m_menu->removeAction(m_actionActive);
    m_menu->addAction(m_actionDeactive);
}

void HwWidget::actionDeactiveSoundCardSlot() {
    auto soundCardInfo = dynamic_cast<SoundCardInfo *>(m_devInfo);
    soundCardInfo->disableSoundCard();
    QString str = "deactivate";
    m_data->set("soundcard", str);
    setDisableStatus(true);
    m_menu->removeAction(m_actionDeactive);
    m_menu->addAction(m_actionActive);
}

void HwWidget::actionActiveNetworkCardSlot() {
    if (m_isWireless == 1) {
        QDBusConnection bus = QDBusConnection::systemBus();
        QDBusInterface manager(DBUS_SERVICENAME, DBUS_DRIVER_INFO_TOOL, DBUS_DRIVER_INFO_INTERFACE_TOOL, bus);
        manager.call("enableWirelessNetworkCard");
        QString str = "activate";
        m_data->set("wirelessnetworkcard", str);
        emit g_signal->enableWirelessNetworkCardSignal();
    } else {
        QDBusConnection bus = QDBusConnection::systemBus();
        QDBusInterface manager(DBUS_SERVICENAME, DBUS_DRIVER_INFO_TOOL, DBUS_DRIVER_INFO_INTERFACE_TOOL, bus);
        manager.call("enableWiredNetworkCard");
        QString str = "activate";
        m_data->set("wireldnetworkcard", str);
        emit g_signal->enableWiredNetworkCardSignal();
    }
    m_menu->removeAction(m_actionActive);
    m_menu->addAction(m_actionDeactive);
}

void HwWidget::actionDeactiveNetworkCardSlot() {
    if (m_isWireless == 1) {
        QDBusConnection bus = QDBusConnection::systemBus();
        QDBusInterface manager(DBUS_SERVICENAME, DBUS_DRIVER_INFO_TOOL, DBUS_DRIVER_INFO_INTERFACE_TOOL, bus);
        manager.call("disableWirelessNetworkCard");
        QString str = "deactivate";
        m_data->set("wirelessnetworkcard", str);
        emit g_signal->disableWirelessNetworkCardSignal();
    } else {
        QDBusConnection bus = QDBusConnection::systemBus();
        QDBusInterface manager(DBUS_SERVICENAME, DBUS_DRIVER_INFO_TOOL, DBUS_DRIVER_INFO_INTERFACE_TOOL, bus);
        manager.call("disableWiredNetworkCard");
        QString str = "deactivate";
        m_data->set("wireldnetworkcard", str);
        emit g_signal->disableWiredNetworkCardSignal();
    }
    m_menu->removeAction(m_actionDeactive);
    m_menu->addAction(m_actionActive);
}

HwWidget::HwWidget(QWidget *parent) : QWidget(parent) {

}

void HwWidget::setDisableStatus(bool status) {
    m_iconLabel->setDisabled(status);
    m_textLabel->setDisabled(status);
    m_modelLabel->setDisabled(status);
    m_activateStatus->setDisabled(status);
    if (status) {
        m_activeStackWidget->setCurrentIndex(0);
    } else {
        m_activeStackWidget->setCurrentIndex(1);
    }
}


void HwWidget::spreadIn() {
    if (m_btnStackedWidget->currentWidget() == m_spreadInBtn) {
        this->m_spreadInBtn->click();
    }
}

QString HwWidget::getResolution(QString interface) {
    QMap<QString, QString> devlist;
    QProcess process;
    process.start("xrandr --prop");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');

    QString currentDisplay;
    QString res;
    bool isCurrentMode = false;

    for (const QString &line: lines) {
        if (line.contains(" connected")) {
            // 解析当前显示器名称
            QStringList parts = line.split(' ');
            currentDisplay = parts[0];
            isCurrentMode = false;
        } else if (!currentDisplay.isEmpty() && line.startsWith("   ")) {
            // 解析当前显示器的分辨率和刷新率
            QStringList parts = line.simplified().split(' ');
            if (parts.size() >= 2) {
                QString refreshRate;
                for (int i = 1; i < parts.length(); ++i) {
                    if (QString(parts[i]).contains("*")) {
                        res = parts[0];
                        refreshRate = parts[i];
                    }
                    if (refreshRate.contains("+")) {
                        refreshRate.remove("+");
                    }
                }
                bool hasAsterisk = refreshRate.contains('*');

                // 如果刷新率带有 '*'，表示当前模式
                if (hasAsterisk) {
                    refreshRate = refreshRate.remove('*');
                    devlist[currentDisplay] = refreshRate;
                    isCurrentMode = true;
                } else if (!isCurrentMode && !devlist.contains(currentDisplay)) {
                    // 如果没有 '*' 并且还没有记录当前显示器的刷新率
                    devlist[currentDisplay] = refreshRate;
                }
            }
        }
    }

    QList<QScreen *> screens = QGuiApplication::screens();
            foreach (QScreen *screen, screens) {
            QString interface = screen->name();
            if (devlist.contains(interface)) {
                QString resolution = res
                                     + "@"
                                     + devlist.value(interface) + "Hz";
                return resolution;
            }
        }

    return QString(); // 如果没有匹配到则返回空字符串
}

void HwWidget::setBluetoothStatus() {
    auto bluetoothInfo = dynamic_cast<BluetoothInfo *>(m_devInfo);
    if (bluetoothInfo->checkStatus()) {
        setDisableStatus(false);
        m_data->set("bluetooth", "activate");
        if (m_menu->actions().contains(m_actionActive)) {
            m_menu->removeAction(m_actionActive);
            m_menu->addAction(m_actionDeactive);
        }
    } else {
        setDisableStatus(true);
        m_data->set("bluetooth", "deactivate");
        if (m_menu->actions().contains(m_actionDeactive)) {
            m_menu->removeAction(m_actionDeactive);
            m_menu->addAction(m_actionActive);
        }
    }
}

void HwWidget::setNetworkStatus() {
    auto networkCardInfo = dynamic_cast<NetworkCardInfo *>(m_devInfo);
    if (networkCardInfo->isWireless == 1) {
        if (networkCardInfo->checkStatus()) {
            setDisableStatus(false);
            m_data->set("wirelessnetworkcard", "activate");
            if (m_menu->actions().contains(m_actionActive)) {
                m_menu->removeAction(m_actionActive);
                m_menu->addAction(m_actionDeactive);
            }
        } else {
            setDisableStatus(true);
            m_data->set("wirelessnetworkcard", "deactivate");
            if (m_menu->actions().contains(m_actionDeactive)) {
                m_menu->removeAction(m_actionDeactive);
                m_menu->addAction(m_actionActive);
            }
        }
    } else {
        if (networkCardInfo->checkStatus()) {
            setDisableStatus(false);
            m_data->set("wireldnetworkcard", "activate");
            if (m_menu->actions().contains(m_actionActive)) {
                m_menu->removeAction(m_actionActive);
                m_menu->addAction(m_actionDeactive);
            }
        } else {
            setDisableStatus(true);
            m_data->set("wireldnetworkcard", "deactivate");
            if (m_menu->actions().contains(m_actionDeactive)) {
                m_menu->removeAction(m_actionDeactive);
                m_menu->addAction(m_actionActive);
            }
        }
    }
}


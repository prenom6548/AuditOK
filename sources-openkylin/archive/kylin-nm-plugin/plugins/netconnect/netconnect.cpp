/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "netconnect.h"
#include "colorpalette.h"

#include <QGSettings>
#include <QProcess>
#include <QTimer>
#include <QtDBus>
#include <QDir>
#include <QDebug>
#include <QtAlgorithms>

const QString KLanSymbolic      = "network-wired-connected-symbolic";
const QString NoNetSymbolic     = "network-wired-disconnected-symbolic";


#define NO_MARGINS 0,0,0,0
#define TOP_MARGINS 0,8,0,0
#define MAIN_LAYOUT_MARGINS 0,0,0,0
#define MAIN_LAYOUT_SPACING 0
#define TITLE_FRAME_HEIGHT 50     //TabWidget的tab和widget有间隙，和设计稿看起来一致就不能设为设计稿里的高度
#define TITLE_LAYOUT_MARGINS 24,0,24,0
#define SPACING 8
#define REFRESH_NETWORKSPEED_TIMER 1000

#define SYSTEM_DBUS_SERVICE     "com.kylin.network.qt.systemdbus"
#define SYSTEM_DBUS_PATH        "/"
#define SYSTEM_DBUS_INTERFACE   "com.kylin.network.interface"


bool checkMapValveContainsTrue(QMap<QString, bool> map) {
    if (map.isEmpty()) {
        return false;
    }
    QMap<QString, bool>::iterator iter;
    for (iter = map.begin(); iter != map.end(); iter++) {
        if (iter.value()) {
            return true;
        }
    }
    return false;
}

NetConnect::NetConnect() :  mFirstLoad(true) , m_fileWatcher(nullptr), m_translator(nullptr){
    qRegisterMetaType<KyDeviceState>("KyDeviceState");
    qRegisterMetaType<KyConnectSetting>("KyConnectSetting");
    QLocale local;
    QString locale = local.name();
    QTranslator* translator = new QTranslator(this);
    if (translator->load("/usr/share/kylin-nm-plugin/netconnect/"+ locale + ".qm")) {
        QApplication::installTranslator(translator);
    } else {
        qWarning() << "Translations load fail";
    }
    m_fileWatcher = new QFileSystemWatcher(this);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &NetConnect::onConfigFileChanged);
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged, this, &NetConnect::onConfigFileChanged);
}

void NetConnect::onConfigFileChanged(const QString &path)
{
    const QString configPath = oobeInstallerConfigPath();
    const QString configDir = QDir::homePath();

    if (path == configPath) {
        qDebug() << "Config file changed, reloading language...";
        loadLanguageFromConfig();
    } else if (path == configDir) {
        QFileInfo fi(configPath);
        if (fi.exists()) {
            qDebug() << "Config directory changed and config exists, reloading language...";
            loadLanguageFromConfig();
        }
    }

    if (!m_fileWatcher->directories().contains(configDir)) {
        m_fileWatcher->addPath(configDir);
    }
    if (!m_fileWatcher->files().contains(configPath) && QFile::exists(configPath)) {
        m_fileWatcher->addPath(configPath);
    }
}

void NetConnect::loadLanguageFromConfig()
{
    const QString configPath = oobeInstallerConfigPath();
    QFile configFile(configPath);
    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&configFile);
        QString languageCode;
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith("language=")) {
                languageCode = line.mid(9); 
                break;
            }
        }
        configFile.close();

        if (!languageCode.isEmpty()) {
            if (m_translator) {
                QApplication::removeTranslator(m_translator);
                delete m_translator;
                m_translator = nullptr;
            }

            // 加载新的翻译器
            m_translator = new QTranslator(this);
            QString translationPath = QString("/usr/share/kylin-nm-plugin/netconnect/%1.qm").arg(languageCode);
            if (m_translator->load(translationPath)) {
                QApplication::installTranslator(m_translator);
                qDebug() << "Language reloaded for locale:" << languageCode;
                retranslateUi();
            } else {
                qWarning() << "Failed to load translation for locale:" << languageCode;
                delete m_translator;
                m_translator = nullptr;
            }
        }
    } else {
        qWarning() << "Failed to open config file:" << configPath;
    }
}

void NetConnect::retranslateUi()
{
    if (m_titleLabel) m_titleLabel->setText(tr("LAN"));
    if (m_settingsLabel) m_settingsLabel->setText(tr("Settings"));
    Q_EMIT languageChanged();
}


NetConnect::~NetConnect() {

    qDebug() << Q_FUNC_INFO << __LINE__;
    if (nullptr != thread)
    {
        thread->quit();
        thread->wait();
        delete thread;
    }

    if (nullptr != pluginWidget ) {
        delete pluginWidget;
    }

    qDebug() << Q_FUNC_INFO << __LINE__;
}

QWidget *NetConnect::pluginUi() {

    // OOBE模式下读取家目录下 ky-installer.cfg，获取 language 参数并加载对应翻译
    if (OOBE == m_pluginType) {
        const QString configPath = oobeInstallerConfigPath();
        const QString configDir = QDir::homePath();

        loadLanguageFromConfig(); // 初始化加载语言

        if (!m_fileWatcher->files().contains(configPath) && QFile::exists(configPath)) {
            m_fileWatcher->addPath(configPath);
            qDebug() << "Watching config file for changes:" << configPath;
        }
        if (!m_fileWatcher->directories().contains(configDir)) {
            m_fileWatcher->addPath(configDir);
            qDebug() << "Watching config directory for changes:" << configDir;
        }
    }

    if (mFirstLoad) {
        mFirstLoad = false;

        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);

        initSystemDbus();
        initUi();
        initComponent();
        initConnect();
        if(OOBE == m_pluginType){
            hideTitle();
        }
        // 如果是OOBE模式，在初始化后显示所有网络连接
        if (OOBE == m_pluginType) {
            showLayout(m_scrollAreaLayout);
        }
    }
    return pluginWidget;
}

void NetConnect::setPluginType(PluginType type, bool useSwitch)
{
    m_pluginType = type;
    if (type == SIMPLE) {
        m_isSimpleMode = true;
    } else {
        m_isSimpleMode = false;
    }
    m_useSwitch = useSwitch;
}

bool NetConnect::checkPluginIsHidden()
{
    return true;  //to show this plugin
#if 0
    if (nullptr != m_wiredSwitch) {
        return m_wiredSwitch->isEnabled();
    }
    return false;
#endif
}

void NetConnect::initSystemDbus()
{
    //add netconnect swtich signal
    m_pSysBusIntfs = new QDBusInterface(SYSTEM_DBUS_SERVICE,
                                         SYSTEM_DBUS_PATH,
                                         SYSTEM_DBUS_INTERFACE,
                                         QDBusConnection::systemBus());
    if (m_pSysBusIntfs->isValid()) {
        //bind manager swtich signal
        QDBusConnection::systemBus().connect(SYSTEM_DBUS_SERVICE,
                                              SYSTEM_DBUS_PATH,
                                              SYSTEM_DBUS_INTERFACE,
                                              "sysWiredMainSwitchChanged",
                                              this,
                                              SLOT(onWiredMainSwitchChanged(bool)));

    }
}

void NetConnect::initUi()
{
    thread = new QThread;
    manager = new KyNetworkManager();
    manager->moveToThread(thread);
    connect(thread, &QThread::started, manager, &KyNetworkManager::kylinNetworkManagerInit);
    connect(thread, &QThread::finished, manager, &KyNetworkManager::deleteLater);
    connect(thread, &QThread::finished, [=](){
        qDebug() << "NetConnect thread quit";
    });
    thread->start();

    while (!manager->isInitFinished()) {
        ::usleep(1000);
    }

    m_mainLayout = new QVBoxLayout(pluginWidget);
    if (OOBE == m_pluginType) {
        // 在OOBE模式下使用更简洁的布局，更符合设计稿
        m_mainLayout->setContentsMargins(0, 0, 0, 0);
        m_mainLayout->setSpacing(0);
    } else {
        m_mainLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
        m_mainLayout->setSpacing(MAIN_LAYOUT_SPACING);
    }
    pluginWidget->setLayout(m_mainLayout);

    m_titleFrame = new QFrame(pluginWidget);
    if (OOBE == m_pluginType) {
        // m_titleFrame->setFixedHeight(0);  // 在OOBE模式下隐藏标题
        m_titleFrame->setVisible(false);
    } else {
        m_titleFrame->setFixedHeight(53);
    }
    m_titleLayout = new QHBoxLayout(m_titleFrame);
    m_titleLayout->setContentsMargins(TITLE_LAYOUT_MARGINS);
    m_titleLabel = new QLabel(m_titleFrame);
    m_titleLabel->setText(tr("LAN"));
    m_wiredSwitch = new KSwitchButton(pluginWidget);
    m_wiredSwitch->installEventFilter(this);
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_wiredSwitch);
    m_titleDivider = new Divider(pluginWidget);
    m_titleDivider->hide();

    m_scrollFrame = new QFrame(pluginWidget);
    if (OOBE == m_pluginType) {
        // 在OOBE模式下，让滚动区域占满整个可用空间
        // m_scrollFrame->setMinimumHeight(0);
        // 使用标准色板而非样式表设置背景
        QPalette standardPalette = getDefaultPalette(true);
        QPalette palette = m_scrollFrame->palette();
        palette.setColor(QPalette::Window, standardPalette.color(QPalette::Window)); // 使用标准色板中的窗口色
        m_scrollFrame->setAutoFillBackground(true);
        m_scrollFrame->setPalette(palette);
    } else {
        if (!m_isSimpleMode) {
            m_scrollFrame->setMinimumHeight(330);
        } else {
            m_scrollFrame->setMinimumHeight(383);
        }
    }
    m_scrollLayout = new QVBoxLayout(m_scrollFrame);
    m_scrollLayout->setContentsMargins(0,0,0,0);
    m_scrollLayout->setSpacing(0);
    m_scrollFrame->setLayout(m_scrollLayout);

    m_scrollArea = new QScrollArea(m_scrollFrame);
    m_scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setBackgroundRole(QPalette::Base);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_scrollLayout->addWidget(m_scrollArea);

    m_listWidget = new QWidget(pluginWidget);
    if (OOBE == m_pluginType) {
        // 在OOBE模式下，让列表宽度自适应
        // m_listWidget->setFixedWidth(0);
        m_listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    } else {
        m_listWidget->setFixedWidth(420);
    }
    m_scrollAreaLayout = new QVBoxLayout(m_listWidget);
    if (OOBE == m_pluginType) {
        // 在OOBE模式下，使用更紧凑的间距
        m_scrollAreaLayout->setSpacing(0);
        m_scrollAreaLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        m_scrollAreaLayout->setSpacing(MAIN_LAYOUT_SPACING);
        m_scrollAreaLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    }
    m_scrollAreaLayout->setAlignment(Qt::AlignTop);
    m_listWidget->setLayout(m_scrollAreaLayout);

    m_scrollArea->setWidget(m_listWidget);

    m_settingsDivider = new Divider(pluginWidget);
    m_settingsFrame = new QFrame(pluginWidget);
    m_settingsFrame->setFixedHeight(TITLE_FRAME_HEIGHT);

    m_settingsLayout = new QHBoxLayout(m_settingsFrame);
    m_settingsLayout->setContentsMargins(TITLE_LAYOUT_MARGINS);
    m_settingsLabel = new KBorderlessButton(m_settingsFrame);
    m_settingsLabel->setCursor(Qt::PointingHandCursor);
    m_settingsLabel->setText(tr("Settings"));

    m_settingsLayout->addWidget(m_settingsLabel);
    m_settingsLayout->addStretch();
    m_settingsFrame->setLayout(m_settingsLayout);

    m_mainLayout->addWidget(m_titleFrame);
    m_mainLayout->addWidget(m_titleDivider);
    m_mainLayout->addWidget(m_scrollFrame);
    if (!m_isSimpleMode && (OOBE != m_pluginType)) {
        m_mainLayout->addStretch();
        m_mainLayout->addWidget(m_settingsDivider);
        m_mainLayout->addWidget(m_settingsFrame);
    } else {
        m_settingsDivider->hide();
        m_settingsFrame->hide();
    }

    QPalette pal = m_scrollArea->palette();
    pal.setBrush(QPalette::Base, QColor(0,0,0,0));     //背景透明
    m_scrollArea->setPalette(pal);

    showRate();
}

void NetConnect::showDesktopNotify(const QString &message, QString soundName)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QStringList actions;  //跳转动作
    actions.append("default");
    actions.append("kylin-nm");          //默认动作：点击消息体时打开麒麟录音
    QMap<QString, QVariant> hints;
    if (!soundName.isEmpty()) {
        hints.insert("sound-name",soundName); //添加声音
    }
    hints.insert("desktop-entry","kylin-nm");

    QList<QVariant> args;
    args<<(tr("Kylin NM"))
         <<((unsigned int) 0)
         <<QString("gnome-dev-ethernet")
         <<tr("kylin network applet desktop message") //显示的是什么类型的信息
         <<message //显示的具体信息
         <<actions
         <<hints
         <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}

// bool NetConnect::eventFilter(QObject *w, QEvent *e) {
// //     if (w == m_wiredSwitch && e->type() == QEvent::MouseButtonRelease) {
// //         if (!m_wiredSwitch->isEnabled()) {
// // //            showDesktopNotify(tr("No ethernet device avaliable"));
// //             qDebug() << "No ethernet device avaliable";
// //         } else {
// //             Q_EMIT setWiredEnabled(!m_wiredSwitch->isChecked());
// //         }
// //         return true;
// //     }
//     return QObject::eventFilter(w,e);
// }

void NetConnect::initComponent() {

    getDeviceStatusMap(deviceStatusMap);
    initNet();
    setSwitchStatus();

    connect(m_settingsLabel, &KBorderlessButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        runExternalApp();
    });
}

void NetConnect::initConnect()
{
    connect(manager, &KyNetworkManager::wiredStateChange, this, &NetConnect::onActiveConnectionChanged);

    //connect(manager, &KyNetworkManager::deviceStateChange, this, &NetConnect::onDeviceStatusChanged);
    connect(manager, &KyNetworkManager::carrierChange, this, &NetConnect::onDeviceCarrierChange);

    connect(manager, &KyNetworkManager::wiredConnectionAdd, this, &NetConnect::onLanAdd);
    connect(manager, &KyNetworkManager::wiredConnectionUpdate, this, &NetConnect::updateLanInfo);
    connect(manager, &KyNetworkManager::connectionRemove, this, &NetConnect::onLanRemove);

    connect(manager, &KyNetworkManager::wiredDeviceAdd, this, &NetConnect::onDeviceAdd);
    connect(manager, &KyNetworkManager::deviceRemove, this, &NetConnect::onDeviceRemove);
    connect(manager, &KyNetworkManager::wiredDeviceUpdate, this, &NetConnect::onDeviceNameChanged);

    connect(manager, &KyNetworkManager::deviceManagedChange, this, &NetConnect::onDeviceManagedChange);
    connect(manager, &KyNetworkManager::wiredEnabledChanged, this, &NetConnect::onWiredEnabledChanged);


    connect(this, &NetConnect::setWiredDeviceEnable, manager, &KyNetworkManager::onSetWiredDeviceEnable);
    connect(this, &NetConnect::setWiredEnabled, manager, &KyNetworkManager::onSetWiredEnabled);
    connect(this, &NetConnect::activateConnection, manager, &KyNetworkManager::onActivateConnection);
    connect(this, &NetConnect::deactivateConnection, manager, &KyNetworkManager::onDeactivateConnection);
    connect(this, &NetConnect::deleteConnect, manager, &KyNetworkManager::onDeleteConnect);
    connect(this, &NetConnect::updateIpv4AndIpv6SettingInfo, manager, &KyNetworkManager::onUpdateIpv4AndIpv6SettingInfo);
    connect(this, &NetConnect::createWiredConnect, manager, &KyNetworkManager::onCreateWiredConnect);
    //btn
    connect(m_wiredSwitch, &KSwitchButton::clicked, this, &NetConnect::onWiredEnabledChanged);


}
//获取网卡列表
void NetConnect::getDeviceStatusMap(QMap<QString, bool> &map)
{
    map.clear();
    QStringList wiredDevList;
    wiredDevList.clear();
    manager->getNetworkDeviceList(DEVICE_TYPE_ETHERNET, wiredDevList);

    qDebug() << wiredDevList;
    if (!wiredDevList.isEmpty()) {
        for (int i = 0; i < wiredDevList.size(); ++i) {

            QString devName = wiredDevList.at(i);qDebug() << devName;
            map.insert(devName, manager->getDeviceManaged(devName));
        }
    }
}

//lanUpdate
void NetConnect::updateLanInfo(QString deviceName, QString connectUuid, QString connectName, QString connectPath)
{
    QMap<QString, ItemFrame *>::iterator devIter;
    QMap<QString, LanItem *>::iterator itemIter;
    for (devIter = deviceFrameMap.begin(); devIter != deviceFrameMap.end(); devIter++) {
        // 全部show出来
        for (itemIter = devIter.value()->itemMap.begin(); itemIter != devIter.value()->itemMap.end(); itemIter++) {
            if(itemIter.value()->getName() != connectName){
                itemIter.value()->show();
            }
        }
    }
    //设备归属变化 || 有线名称变化
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        if (deviceName.isEmpty()) {
            //变为无指定网卡，所有列表都要添加
            if (!iter.value()->itemMap.contains(connectUuid)) {
                QStringList lanInfo;
                lanInfo << connectName << connectUuid << connectPath;
                addOneLanFrame(iter.value(), iter.key(), lanInfo);
            } else {
                if (iter.value()->itemMap[connectUuid]->getName() != connectName) {
                    qDebug() << "[NetConnect]" << iter.key()
                             << iter.value()->itemMap[connectUuid]->getName() << "change to" << connectName;
                    iter.value()->itemMap[connectUuid]->setName(connectName);
                    deviceFrameMap[iter.key()]->lanItemLayout->removeWidget(iter.value()->itemMap[connectUuid]);
                    int index = getInsertPos(connectName, deviceFrameMap[iter.key()]->lanItemLayout);
                    deviceFrameMap[iter.key()]->lanItemLayout->insertWidget(index,iter.value()->itemMap[connectUuid]);
                }
            }
        } else {
            if (iter.key() != deviceName) {
                qDebug() << "[NetConnect]" << connectUuid << " not belongs to " << iter.key();
                removeOneLanFrame(iter.value(), deviceName, connectUuid);
            } else {
                if (!iter.value()->itemMap.contains(connectUuid)) {
                    qDebug() << "[NetConnect]" << connectUuid << " now belongs to " << deviceName;
                    QStringList lanInfo;
                    lanInfo << connectName << connectUuid << connectPath;
                    addOneLanFrame(iter.value(), deviceName, lanInfo);
                } else {
                    qDebug() << "[NetConnect]" << deviceName
                             << iter.value()->itemMap[connectUuid]->getName() << "change to" << connectName;
                    if (iter.value()->itemMap[connectUuid]->getName() != connectName) {
                        iter.value()->itemMap[connectUuid]->setName(connectName);
                        deviceFrameMap[iter.key()]->lanItemLayout->removeWidget(iter.value()->itemMap[connectUuid]);
                        int index = getInsertPos(connectName, deviceFrameMap[iter.key()]->lanItemLayout);
                        deviceFrameMap[iter.key()]->lanItemLayout->insertWidget(index,iter.value()->itemMap[connectUuid]);
                    }
                }
            }
        }
    }
}


//总开关
void NetConnect::setSwitchStatus()
{
    getDeviceStatusMap(deviceStatusMap);

    if (deviceStatusMap.isEmpty()) {
        m_wiredSwitch->setChecked(false);
        m_wiredSwitch->setEnabled(false);
#if 0
        Q_EMIT updatePluginHidden(false);
#endif
    } else {
        m_wiredSwitch->setEnabled(true);

        bool status = getWiredMainSwitch();
        // 如果是OOBE模式，则开关始终显示为开启状态
        if (OOBE == m_pluginType) {
            m_wiredSwitch->setChecked(true);
        } else {
            m_wiredSwitch->setChecked(status);
        }

        // 如果是OOBE模式，则不受开关影响显示所有有线网络连接项
        if (OOBE == m_pluginType) {
            showLayout(m_scrollAreaLayout);
            m_titleDivider->hide();
        } else {
                showLayout(m_scrollAreaLayout);
                m_titleDivider->hide();
        }
#if 0
        Q_EMIT updatePluginHidden(true);
#endif
    }
}

//总体隐藏
void NetConnect::hideLayout(QVBoxLayout * layout) {
    for (int i = layout->layout()->count()-1; i >= 0; --i)
    {
        QLayoutItem *it = layout->layout()->itemAt(i);
        ItemFrame *itemFrame = qobject_cast<ItemFrame *>(it->widget());
        itemFrame->hide();
    }
}

//总体显示
void NetConnect::showLayout(QVBoxLayout * layout) {
    for (int i = layout->layout()->count()-1; i >= 0; --i)
    {
        QLayoutItem *it = layout->layout()->itemAt(i);
        ItemFrame *itemFrame = qobject_cast<ItemFrame *>(it->widget());
        // 如果是OOBE模式，则不受开关影响显示所有有线网络连接项
        if (OOBE == m_pluginType) {
            itemFrame->show();
        } else {
            if (deviceStatusMap[itemFrame->m_deviceName]) {
                itemFrame->show();
            } else {
                itemFrame->hide();
            }
        }
    }
}

//初始化
void NetConnect::initNet()
{
    //先构建每个设备的列表头
    QStringList deviceList = deviceStatusMap.keys();
    for (int i = 0; i < deviceList.size(); ++i) {
        addDeviceFrame(deviceList.at(i), deviceStatusMap[deviceList.at(i)]);
    }
    //再填充每个设备的列表
    for (int i = 0; i < deviceList.size(); ++i) {
        initNetListFromDevice(deviceList.at(i));
    }
    m_setNetSpeedTimer->start(REFRESH_NETWORKSPEED_TIMER);
}

void NetConnect::runExternalApp()
{
    QProcess process;
    process.startDetached("ukui-control-center -m netconnect");
}

//激活
void NetConnect::activeConnect(QString uuid, QString deviceName)
{
    if (manager) {
        bool isCarriered = manager->getWiredDeviceIsCarriered(deviceName);
        if (!isCarriered){
            showDesktopNotify(tr("Wired Device not carried"), "networkwrong");
        }
    }
    Q_EMIT activateConnection(uuid, deviceName);

}

//断开
void NetConnect::deActiveConnect(QString uuid)
{
    Q_EMIT deactivateConnection(uuid);
}

//初始化设备列表
void NetConnect::initNetListFromDevice(QString deviceName)
{
    qDebug() << "[NetConnect]initNetListFromDevice " << deviceName;
    if (!deviceFrameMap.contains(deviceName)) {
        qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " not exist";
        return;
    }
    QMap<QString,QList<KyWiredItem>> wiredMap;
    manager->getWiredList(wiredMap);

    QList<QString> activateUuidList;
    QList<QString> activateDeviceList;
    for (auto it = wiredMap.begin(); it != wiredMap.end(); ++it) {
        // 获取wiredMap里面的设备名称，将getActiveConnectionList里面所有的连接名称保存
        QList<KyActivateItem> activeConnections;
        manager->getActiveConnectionList(it.key(), CONNECT_TYPE_WIRED, activeConnections);
        for (const auto& connection : activeConnections) {
            qDebug() << "Active connection name: " << connection.m_uuid;
            activateUuidList.append(connection.m_uuid);
            activateDeviceList.append(it.key());
        }
    }

    if (wiredMap.size() == 0) {
        qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " list empty";
        return;
    }

    QMap<QString,QList<KyWiredItem>>::iterator iter;
    for (iter = wiredMap.begin(); iter != wiredMap.end(); iter++) {
        if (deviceName == iter.key()) {
            QList<KyWiredItem> lanListInfo = iter.value();
            //处理列表
            for (int i = 0; i < lanListInfo.length(); i++) {
                // 先检查是否已存在此UUID的连接，避免重复添加
                bool exists = false;
                QMap<QString, ItemFrame *>::iterator devIter;
                for (devIter = deviceFrameMap.begin(); devIter != deviceFrameMap.end(); devIter++) {
                    if (devIter.value()->itemMap.contains(lanListInfo.at(i).m_connectUuid)) {
                        exists = true;
                        break;
                    }
                }
                
                if (!exists) {
                    // 判断lanListInfo.at(i).m_connectUuid是否在activeConnections
                    addLanItem(deviceFrameMap[deviceName], deviceName, lanListInfo.at(i));
                    // 如果判断网卡是否连接，未连接的同名连接进行移除，在初始化的时候
                    if(activateUuidList.contains(lanListInfo.at(i).m_connectUuid)&&!activateDeviceList.contains(deviceName)){
                        // 隐藏item
                        deviceFrameMap[deviceName]->itemMap[lanListInfo.at(i).m_connectUuid]->hide();
                    }
                }
            }
        }
    }
    QList<KyActivateItem> activateList;
    manager->getActiveConnectionList(deviceName, CONNECT_TYPE_WIRED, activateList);
    if (activateList.size() != 0) {
        onActiveConnectionChanged(deviceName, activateList.at(0).m_uuid, activateList.at(0).m_connStatus);
    }
}

//初始化时添加一个项
void NetConnect::addLanItem(ItemFrame *frame, QString devName, KyWiredItem item)
{
    if (frame == nullptr) {
        return;
    }

    // 检查是否已存在相同UUID的项，避免重复添加
    if (deviceFrameMap[devName]->itemMap.contains(item.m_connectUuid)) {
        qDebug() << "[NetConnect]Item with UUID already exists, skipping: " << item.m_connectUuid;
        return;
    }

    LanItem * lanItem = new LanItem(m_isSimpleMode, m_pluginType == OOBE, pluginWidget);

    QIcon searchIcon = QIcon::fromTheme(NoNetSymbolic);
    lanItem->setItemIcon(searchIcon);
    lanItem->setName(item.m_connectName);

    lanItem->setUuid(item.m_connectUuid);
    lanItem->setPath(item.m_connectPath);

    //todo show detail page
    connect(lanItem, &LanItem::infoButtonClick, this, [=](QString uuid) {
        onShowDetailPage(devName, uuid);
    });

    lanItem->setStatus(false);

    connect(lanItem, &LanItem::itemClick, this, [=] {
        if (lanItem->getStatus() || lanItem->getIsLoading()) {
            deActiveConnect(lanItem->getUuid());
        } else {
            activeConnect(lanItem->getUuid(), devName);
        }
    });

    //记录到deviceFrame的itemMap中
    deviceFrameMap[devName]->itemMap.insert(item.m_connectUuid, lanItem);
    frame->lanItemLayout->addWidget(lanItem);
}

//增加设备
void NetConnect::addDeviceFrame(QString devName, bool isEnable)
{
    // 防止重复添加相同的设备
    if (deviceFrameMap.contains(devName)) {
        qDebug() << "[NetConnect]Device already exists, skipping: " << devName;
        return;
    }
    
    ItemFrame *itemFrame = new ItemFrame(devName, pluginWidget);
    m_scrollAreaLayout->addWidget(itemFrame);
    itemFrame->deviceFrame->deviceLabel->setText(devName);
    
    // 在OOBE模式下，如果只有一个网卡，则隐藏设备名称和分割线
    if (OOBE == m_pluginType && deviceStatusMap.size() == 1) {
        itemFrame->deviceFrame->hide();
        itemFrame->m_divider->hide();
    }
    
    if (isEnable) {
        itemFrame->show();
    } else {
        itemFrame->hide();
    }
    deviceFrameMap.insert(devName, itemFrame);
    deviceStatusMap.insert(devName, true);
}

//减少设备
void NetConnect::removeDeviceFrame(QString devName)
{
    if (deviceFrameMap.contains(devName)) {
        ItemFrame *item = deviceFrameMap[devName];
        if (item->lanItemFrame->layout() != NULL) {
            QLayoutItem* layoutItem;
            while ((layoutItem = item->lanItemFrame->layout()->takeAt(0)) != NULL) {
                delete layoutItem->widget();
                delete layoutItem;
                layoutItem = nullptr;
            }
            item->itemMap.clear();
        }
        delete item;
        item = nullptr;
        deviceFrameMap.remove(devName);
    }
}

//device add or remove=================================
#if 0
void NetConnect::onDeviceStatusChanged()
{
    qDebug()<<"[NetConnect]onDeviceStatusChanged";
    QEventLoop eventloop;
    QTimer::singleShot(300, &eventloop, SLOT(quit()));
    eventloop.exec();
    QStringList list;
    QMap<QString, bool> map;
    getDeviceStatusMap(map);
    list = map.keys();

    QStringList removeList;
    QMap<QString, bool> addMap;

    //remove的设备
    for (int i = 0; i< deviceStatusMap.keys().size(); ++i) {
        if (!list.contains(deviceStatusMap.keys().at(i))) {
            qDebug() << "[NetConnect]onDeviceStatusChanged " << deviceStatusMap.keys().at(i) << "was removed";
            removeList << deviceStatusMap.keys().at(i);
        }
    }

    //add的设备
    for (int i = 0; i< list.size(); ++i) {
        if (!deviceStatusMap.keys().contains(list.at(i))) {
            qDebug() << "[NetConnect]onDeviceStatusChanged " << list.at(i) << "was add, init status" << map[list.at(i)];
            addMap.insert(list.at(i),map[list.at(i)]);
        }
    }

    for (int i = 0; i < removeList.size(); ++i) {
        removeDeviceFrame(removeList.at(i));
    }

    QStringList addList = addMap.keys();
    for (int i = 0; i < addList.size(); ++i) {
        qDebug() << "add a device " << addList.at(i) << "status" << map[addList.at(i)];
        addDeviceFrame(addList.at(i));
        initNetListFromDevice(addList.at(i));
    }
    deviceStatusMap = map;
    if (deviceStatusMap.isEmpty()) {
        wiredSwitch->setDisabledFlag(true);
        wiredSwitch->setChecked(false);
    } else {
        wiredSwitch->setDisabledFlag(false);
        setSwitchStatus();
    }

}
#endif
void NetConnect::onDeviceNameChanged(QString oldName, QString newName)
{
    if (!deviceFrameMap.contains(oldName) || !deviceStatusMap.contains(oldName)) {
        qDebug() << "[NetConnect]onDeviceNameChanged no such device " << oldName;
        return;
    }

    if (deviceFrameMap.contains(newName) && deviceStatusMap.contains(newName)) {
        qDebug() << "[NetConnect]onDeviceNameChanged already has device " << newName;
        return;
    }

    qDebug() << "[NetConnect]onDeviceNameChanged " << oldName << "change to" << newName;

    //shan chu chong jian
    removeDeviceFrame(oldName);
    removeDeviceFrame(newName);

    getDeviceStatusMap(deviceStatusMap);

    addDeviceFrame(newName, manager->getDeviceManaged(newName));
    initNetListFromDevice(newName);
}

//lan add===============================================================
void NetConnect::onLanAdd(QString devName, QString connectUuid, QString connectName, QString connectPath)
{
    qDebug() << "onLanAdd";

    if (!devName.isEmpty() && !deviceStatusMap.contains(devName)) {
        return;
    }

    QStringList lanInfo;
    lanInfo << connectName << connectUuid << connectPath;

    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        if (devName.isEmpty()) {
            addOneLanFrame(iter.value(), iter.key(), lanInfo);
        } else if (devName == iter.key()) {
            addOneLanFrame(iter.value(), devName, lanInfo);
            break;
        }
    }
}

//wifi remove =============================================================
void NetConnect::onLanRemove(QString lanPath)
{
    qDebug() << "onLanRemove";
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        QMap<QString, LanItem *>::iterator itemIter;
        for (itemIter = iter.value()->itemMap.begin(); itemIter != iter.value()->itemMap.end(); itemIter++) {
            if (itemIter.value()->getPath() == lanPath) {
               qDebug()<<"[NetConnect]lan remove " << lanPath << " find in " << itemIter.value()->getName();
               QString key = itemIter.key();
               iter.value()->lanItemLayout->removeWidget(itemIter.value());
               delete itemIter.value();
               iter.value()->itemMap.remove(key);
               break;
            }
        }
    }
}

//增加一项
void NetConnect::addOneLanFrame(ItemFrame *frame, QString deviceName, QStringList infoList)
{
    if (nullptr == frame) {
        return;
    }

    QString connUuid = infoList.at(1);
    
    // 检查全局deviceFrameMap中是否已存在此UUID的项，避免跨设备重复
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        if (iter.value()->itemMap.contains(connUuid)) {
            qDebug() << "[NetConnect]Item with UUID already exists globally, skipping: " << connUuid;
            return;
        }
    }

    if (frame->itemMap.contains(connUuid)) {
            qDebug() << "[NetConnect]Already exist a lan " << connUuid << " in " << deviceName;
            return;
    }

    qDebug() << "[NetConnect]addOneLanFrame" << deviceName << infoList.at(0);
    QString connName = infoList.at(0);
    QString connDbusPath = infoList.at(2);
    LanItem * lanItem = new LanItem(m_isSimpleMode, m_pluginType == OOBE, pluginWidget);

    QIcon searchIcon = QIcon::fromTheme(NoNetSymbolic);
    lanItem->setItemIcon(searchIcon);
    lanItem->setName(connName);

    lanItem->setUuid(connUuid);
    lanItem->setPath(connDbusPath);

    // todo open landetail page
    if (!m_isSimpleMode) {
    connect(lanItem, &LanItem::infoButtonClick, this, [=](QString uuid) {
        onShowDetailPage(deviceName, uuid);
    });
    }

    lanItem->setStatus(false);

    connect(lanItem, &LanItem::itemClick, this, [=] {
        if (lanItem->getStatus() || lanItem->getIsLoading()) {
            deActiveConnect(lanItem->getUuid());
        } else {
            activeConnect(lanItem->getUuid(), deviceName);
        }
    });

    //记录到deviceFrame的itemMap中
    deviceFrameMap[deviceName]->itemMap.insert(connUuid, lanItem);
    int index = getInsertPos(connName, deviceFrameMap[deviceName]->lanItemLayout);
    qDebug()<<"[NetConnect]addOneLanFrame " << connName << " to " << deviceName << " list at pos:" << index;
    frame->lanItemLayout->insertWidget(index, lanItem);
}

void NetConnect::removeOneLanFrame(ItemFrame *frame, QString deviceName, QString uuid)
{
    if (nullptr == frame) {
        return;
    }

    if (!frame->itemMap.contains(uuid)) {
            qDebug() << "[NetConnect]not exist a lan " << uuid << " in " << deviceName;
            return;
    }

   qDebug()<<"[NetConnect]removeOneLanFrame " << uuid << " find in " << deviceName;

   frame->lanItemLayout->removeWidget(frame->itemMap[uuid]);
   delete frame->itemMap[uuid];
   frame->itemMap.remove(uuid);
}

//activeconnect status change
void NetConnect::onActiveConnectionChanged(QString deviceName, QString uuid, KyConnectState status)
{
    if (uuid.isEmpty()) {
        qDebug() << "[NetConnect]onActiveConnectionChanged but uuid is empty";
        return;
    }
    qDebug() << "[NetConnect]onActiveConnectionChanged " << deviceName << uuid << status;
    LanItem * item= nullptr;
    if (deviceName.isEmpty()) {
        if (status != CONNECT_STATE_DEACTIVATED) {
            return;
        }
        //断开时 设备为空 说明此有线未指定设备 添加到所有列表中
        QStringList infoList;
        QMap<QString, ItemFrame *>::iterator iters;
        for (iters = deviceFrameMap.begin(); iters != deviceFrameMap.end(); iters++) {
            if (iters.value()->itemMap.contains(uuid)) {
                item = iters.value()->itemMap[uuid];
                //为断开则重新插入
                deviceFrameMap[iters.key()]->lanItemLayout->removeWidget(item);
                int index = getInsertPos(item->getName(), deviceFrameMap[iters.key()]->lanItemLayout);
                qDebug() << "[NetConnect]reinsert" << item->getName() << "pos" << index << "in" << iters.key() << "because status changes to deactive";
                deviceFrameMap[iters.key()]->lanItemLayout->insertWidget(index,item);
                itemActiveConnectionStatusChanged(item, status);
            }
        }
        //添加到所有列表中
        if (!infoList.isEmpty()) {
            QMap<QString, ItemFrame *>::iterator iter;
            for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
                if (!iter.value()->itemMap.contains(uuid)) {
                    addOneLanFrame(iter.value(), iter.key(), infoList);
                }
            }
        }
    } else {
        if (deviceFrameMap.contains(deviceName)) {
            if (deviceFrameMap[deviceName]->itemMap.contains(uuid)) {
                item = deviceFrameMap[deviceName]->itemMap[uuid];
                if (status == CONNECT_STATE_ACTIVATED) {
                    //为已连接则放到第一个
                    QMap<QString,QList<KyWiredItem>> wiredMap;
                    manager->getWiredList(wiredMap);

                    QList<QString> activateUuidList;
                    for (auto it = wiredMap.begin(); it != wiredMap.end(); ++it) {
                        // 获取wiredMap里面的设备名称，将getActiveConnectionList里面所有的连接名称保存
                        QList<KyActivateItem> activeConnections;
                        manager->getActiveConnectionList(it.key(), CONNECT_TYPE_WIRED, activeConnections);
                        for (const auto& connection : activeConnections) {
                            qDebug() << "Active connection name: " << connection.m_uuid;
                            activateUuidList.append(connection.m_uuid);
                        }
                    }
                    // 根据当前连接的名称隐藏所有网卡下同名的连接
                    QMap<QString, ItemFrame *>::iterator biter;
                    for (biter = deviceFrameMap.begin(); biter != deviceFrameMap.end(); biter++) {
                         if (biter.value()->itemMap.contains(uuid)) {
                            QString key = biter.key();
                            LanItem *itemTmp = deviceFrameMap[key]->itemMap[uuid];
                            // 如果同名连接为当前网卡则移除，其余网卡下则隐藏
                            if(deviceName !=key){
                                itemTmp->hide();
                            }else{
                                deviceFrameMap[key]->lanItemLayout->removeWidget(itemTmp);
                            }
                         }
                    }
                    deviceFrameMap[deviceName]->lanItemLayout->insertWidget(0,item);
                    deviceFrameMap[deviceName]->startNetSpeedThread();
                } else if (status == CONNECT_STATE_DEACTIVATED) {
                    //为断开则重新插入
                    deviceFrameMap[deviceName]->lanItemLayout->removeWidget(item);
                    int index = getInsertPos(item->getName(), deviceFrameMap[deviceName]->lanItemLayout);
                    qDebug() << "[NetConnect]reinsert" << item->getName() << "pos" << index << "in" << deviceName << "because status changes to deactive";
                    deviceFrameMap[deviceName]->lanItemLayout->insertWidget(index,item);
                    deviceFrameMap[deviceName]->stopNetSpeedThread();
                }
                itemActiveConnectionStatusChanged(item, status);
            }
        } else {
            if (status == CONNECT_STATE_ACTIVATED || status == CONNECT_STATE_DEACTIVATED) {
                //虚拟网卡处理
                QMap<QString, ItemFrame *>::iterator iters;
                for (iters = deviceFrameMap.begin(); iters != deviceFrameMap.end(); iters++) {
                    if (iters.value()->itemMap.contains(uuid)) {
                        removeOneLanFrame(iters.value(), iters.key(), uuid);
                     }
                }
            }
        }
    }
}

void NetConnect::itemActiveConnectionStatusChanged(LanItem *item, KyConnectState status)
{
    if (status == CONNECT_STATE_ACTIVATING) {
        item->startLoading();
    } else if (status == CONNECT_STATE_ACTIVATED) {
        item->stopLoading();
        QIcon searchIcon = QIcon::fromTheme(KLanSymbolic);
        item->setItemIcon(searchIcon);
        item->setStatus(true);
    } else if (status == CONNECT_STATE_DEACTIVATING) {
        item->startLoading();
    } else {
        item->stopLoading();
        QIcon searchIcon = QIcon::fromTheme(NoNetSymbolic);
        item->setItemIcon(searchIcon);
        item->setStatus(false);
    }
}

void NetConnect::showRate()
{
    m_setNetSpeedTimer = new QTimer(this);
    m_setNetSpeedTimer->setTimerType(Qt::PreciseTimer);
    connect(m_setNetSpeedTimer, &QTimer::timeout,  [&]() {
        onSetNetSpeed();
    });
}

int NetConnect::getInsertPos(QString connName, QVBoxLayout* layout)
{
//    qDebug() << "[NetConnect]getInsertPos" << connName << deviceName;
//    int index = 0;
//    QMap<QString,QList<KyWiredItem>> wiredMap;
//    manager->getWiredList(wiredMap);

//    if (!wiredMap.contains(deviceName)) {
//        return 0;
//    }

//    QList<KyActivateItem> activateList;
//    QString name;
//    manager->getActiveConnectionList(deviceName, CONNECT_TYPE_WIRED, activateList);
//    if (activateList.size() != 0) {
//        name = activateList.at(0).m_connName;
//    }

//    bool isFind = false;
//    for (;index < wiredMap[deviceName].size(); ++index) {
//        if (wiredMap[deviceName].at(index).m_connectName == connName) {
//            break;
//        } else if (wiredMap[deviceName].at(index).m_connectName == name) {
//            isFind = true;
//        }
//    }
//    if (!isFind) {
//        index++;
//    }
//    return index;
    int index = 0;
    for (; index < layout->count(); ++index)
    {
        QLayoutItem *it = layout->layout()->itemAt(index);
        LanItem *item = qobject_cast<LanItem *>(it->widget());
        if (QString::compare(connName, item->getName(), Qt::CaseInsensitive) > 0) {
            qDebug() << "compare" << connName << item->getName() << index;
            continue;
        } else {
            if (item->getStatus()) {
                continue;
            }
            break;
        }
    }
    return index;
}

void NetConnect::onDeviceCarrierChange(QString deviceName, bool pluged)
{

}

void NetConnect::onDeviceManagedChange(QString deviceName, bool managed)
{
    getDeviceStatusMap(deviceStatusMap);
    if (deviceFrameMap.contains(deviceName)) {
        if (managed) {
            deviceFrameMap[deviceName]->show();
        } else {
            deviceFrameMap[deviceName]->hide();
        }
    }
    setSwitchStatus();
}

void NetConnect::onWiredEnabledChanged(bool enabled)
{
    qDebug() << Q_FUNC_INFO << __LINE__ << enabled;
    // if (m_wiredSwitch->isChecked() == enabled) {
    //     qDebug() << Q_FUNC_INFO << __LINE__ << "return";
    //     return;
    // }

    qWarning() << Q_FUNC_INFO << __LINE__ << deviceFrameMap.keys();

    if (0 == deviceFrameMap.keys().count()) {
        showDesktopNotify(tr("No ethernet device avaliable"), "networkwrong");
    }

    if (m_pSysBusIntfs == nullptr || !m_pSysBusIntfs->isValid())
    {
        qWarning() << Q_FUNC_INFO << __LINE__ << "m_pSysBusIntfs is error";
        return;
    }

    m_pSysBusIntfs->call(QStringLiteral("setWiredMainSwitch"), enabled);
    setSwitchStatus();
    // Q_EMIT setWiredEnabled(enabled);

    if (!enabled)
    {
        QMap<QString, ItemFrame *>::iterator iter1;
        for (iter1 = deviceFrameMap.begin(); iter1 != deviceFrameMap.end(); iter1++)
        {
            QMap<QString, LanItem *>::iterator iter2;
            for (iter2 = iter1.value()->itemMap.begin();
                 iter2 != iter1.value()->itemMap.end();
                 iter2++)
            {
                if (iter2.value()->getStatus()) {
                    deActiveConnect(iter2.value()->getUuid());
                }
            }
        }
    } else {
        QDBusReply<QVariantMap> reply = m_pSysBusIntfs->call("getNmConfig", "/etc/kylin-nm/netSwitch.conf", "Lan_Connect");

        if (!reply.isValid()) {
            qWarning() << "Call failed:" << reply.error().name() << reply.error().message();
            return;
        }

        QVariantMap connectMap = reply.value();
        qWarning() << "Connect map:" << connectMap;

        for (auto iter = connectMap.begin(); iter != connectMap.end(); ++iter) {
            QString deviceName = iter.key();
            // 使用 QVariant 转换值
            QString connectUuid = iter.value().toString();
            qWarning() << "deviceName:" << deviceName << "connectUuid:" << connectUuid;

            if (deviceName.isEmpty() || connectUuid.isEmpty()) {
                continue;
            }

            /* 添加网卡状态检查：只有当网卡是启用状态时才进行自动连接 */
            if (deviceFrameMap.contains(deviceName) && manager->getDeviceManaged(deviceName)) {
                qDebug() << Q_FUNC_INFO << __LINE__ << "Device" << deviceName << "is enabled, activating connection:" << connectUuid;
                activeConnect(connectUuid,deviceName);
            } else {
                qDebug() << Q_FUNC_INFO << __LINE__ << "Device" << deviceName << "is disabled, skipping auto-connect for connection:" << connectUuid;
            }
        }
    }
}

void NetConnect::onDeviceAdd(QString deviceName)
{
    addDeviceFrame(deviceName, manager->getDeviceManaged(deviceName));
    initNetListFromDevice(deviceName);
    setSwitchStatus();
}

void NetConnect::onDeviceRemove(QString deviceName)
{
    if (!deviceFrameMap.contains(deviceName) || !deviceStatusMap.contains(deviceName)) {
        return;
    }
    removeDeviceFrame(deviceName);
    setSwitchStatus();
}

void NetConnect::onSetNetSpeed()
{
    if (deviceFrameMap.isEmpty()) {
        return;
    }
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        if(nullptr == iter.value() || iter.value()->lanItemLayout->count() <= 0) {
            continue;
        }
        iter.value()->updateNetSpeed();
    }
}

int getPerfixLength(QString text)
{
    int length = 0;
    QStringList list= text.split(":");
    for (int i = 0; i < list.size(); ++i) {
        QString temp = list.at(i);
        if (temp.isEmpty()) {
            continue;
        }
        bool ok;
        unsigned int val = temp.toUInt(&ok, 16);
        temp = temp.setNum(val,2);
        for(int j = 0; j < temp.length(); ++j) {
            if (temp.at(j) == "1") {
                length++;
            }
        }
    }
    return length;
}

void NetConnect::getDetailConfInfo(QString devName, QString uuid, ConInfo &conInfo)
{
    KyDetailInfo detailInfo;
    manager->getWiredConnectDetail(devName, uuid, detailInfo);

    conInfo.strMac = detailInfo.strMac;
    conInfo.strBandWidth = detailInfo.strBandWidth;
    conInfo.strDynamicIpv4 = detailInfo.strDynamicIpv4;
    conInfo.strDynamicIpv6 = detailInfo.strDynamicIpv6;
    conInfo.strDynamicIpv4Dns = detailInfo.strDynamicIpv4Dns;
    conInfo.isAutoConnect = detailInfo.isAutoConnect;

    KyConnectSetting connectSetting;
    manager->getConnectIpInfo(uuid, connectSetting);
    conInfo.strConName = connectSetting.m_connectName;
    conInfo.ipv4ConfigType = connectSetting.m_ipv4ConfigIpType;
    conInfo.ipv6ConfigType = connectSetting.m_ipv6ConfigIpType;

    if (connectSetting.m_ipv4ConfigIpType == CONFIG_IP_MANUAL) {
        if (connectSetting.m_ipv4Address.size() > 0) {
            conInfo.strIPV4Address = connectSetting.m_ipv4Address.at(0).ip().toString();
            conInfo.strIPV4NetMask = connectSetting.m_ipv4Address.at(0).netmask().toString();
            conInfo.strIPV4GateWay = connectSetting.m_ipv4Address.at(0).gateway().toString();
        }
        if (connectSetting.m_ipv4Dns.size() == 1) {
            conInfo.strIPV4FirDns = connectSetting.m_ipv4Dns.at(0).toString();
        } else if (connectSetting.m_ipv4Dns.size() > 1) {
            conInfo.strIPV4FirDns = connectSetting.m_ipv4Dns.at(0).toString();
            conInfo.strIPV4SecDns = connectSetting.m_ipv4Dns.at(1).toString();
        }
    }

    if (connectSetting.m_ipv6ConfigIpType == CONFIG_IP_MANUAL) {
        if (connectSetting.m_ipv6Address.size() > 0) {
            conInfo.strIPV6Address = connectSetting.m_ipv6Address.at(0).ip().toString();
            conInfo.iIPV6Prefix = getPerfixLength(connectSetting.m_ipv6Address.at(0).netmask().toString());
            conInfo.strIPV6GateWay = connectSetting.m_ipv6Address.at(0).gateway().toString();
        }

        if (connectSetting.m_ipv6Dns.size() == 1) {
            conInfo.strIPV6FirDns = connectSetting.m_ipv6Dns.at(0).toString();
        } else if (connectSetting.m_ipv6Dns.size() > 1) {
            conInfo.strIPV6FirDns = connectSetting.m_ipv6Dns.at(0).toString();
            conInfo.strIPV6SecDns = connectSetting.m_ipv6Dns.at(1).toString();
        }
    }

    if (conInfo.strDynamicIpv4.isEmpty() && conInfo.strDynamicIpv6.isEmpty() &&  conInfo.strDynamicIpv4Dns.isEmpty()) {
        conInfo.strDynamicIpv4 = conInfo.strIPV4Address.isEmpty() ? tr("Auto") : conInfo.strIPV4Address;
        conInfo.strDynamicIpv6 = conInfo.strIPV6Address.isEmpty() ? tr("Auto") : conInfo.strIPV6Address;
        conInfo.strDynamicIpv4Dns = conInfo.strIPV4FirDns.isEmpty() ? tr("Auto") : conInfo.strIPV4FirDns;
    }
}

void NetConnect::onUpdateSettings(QString devName, QString uuid)
{
    NetDetail * netDetail =
        qobject_cast<NetDetail *>(sender());

    KyConnectSetting connectSetting;
    manager->getConnectIpInfo(uuid, connectSetting);
    netDetail->updateConnect(connectSetting);
    Q_EMIT updateIpv4AndIpv6SettingInfo(uuid, connectSetting);

    QList<KyActivateItem> connectItemList;
    manager->getActiveConnectionList(devName,CONNECT_TYPE_WIRED, connectItemList);

    bool isActived = false;
    for (int i = 0; i < connectItemList.size(); ++i) {
        if (connectItemList.at(i).m_uuid == uuid) {
            isActived = true;
            break;
        }
    }

    if (isActived) {
        usleep(10000);
        activeConnect(uuid, devName);
    }

    netDetail->close();
}

void NetConnect::onShowDetailPage(QString devName, QString uuid)
{
    if (m_netdtail != nullptr) {
        disconnect(m_netdtail);
        m_netdtail->close();
        delete m_netdtail;
        m_netdtail = nullptr;
    }
    ConInfo info;
    getDetailConfInfo(devName, uuid, info);
    m_netdtail = new NetDetail(m_pluginType, devName, info.strConName, uuid, false, info, parentWidget);
    m_netdtail->show();
    m_netdtail->raise();
    connect(m_netdtail, &NetDetail::deleteConnect, this, &NetConnect::deleteConnect);
    connect(m_netdtail, &NetDetail::needUpdate, this, &NetConnect::onUpdateSettings);
    connect(m_netdtail, &NetDetail::detailPageClose, this, [&](QString devName, QString uuid) {
        disconnect(m_netdtail);
        delete m_netdtail;
        m_netdtail = nullptr;
    });
}

void NetConnect::hideTitle()
{
    m_wiredSwitch->hide();
    // m_titleLabel->hide();
    m_settingsLabel->hide();
}

//===================== wired network swtich (neet systemdbus)
bool NetConnect::getWiredMainSwitch()
{
    qDebug() << Q_FUNC_INFO << __LINE__ ;

    if (m_pSysBusIntfs && m_pSysBusIntfs->isValid())
    {
        qDebug() << Q_FUNC_INFO << __LINE__ ;
        QDBusReply <bool> reply = m_pSysBusIntfs->call("getWiredMainSwitch");
        if (reply.isValid()) {
            bool status = reply.value();
            qDebug() << Q_FUNC_INFO << status << __LINE__ ;
            return status;
        } else {
            qWarning () << "switch init dbus reply invalid";
            return false;
        }
    }
    return false;
}

void NetConnect::onWiredMainSwitchSLot(bool checked)
{
    if (!m_wiredSwitch->isCheckable()) {
        return;
    }
    if (m_pSysBusIntfs == nullptr || !m_pSysBusIntfs->isValid())
        return;

    m_pSysBusIntfs->call(QStringLiteral("setWiredMainSwitch"), checked);
}
void NetConnect::onWiredMainSwitchChanged(bool state)
{
    qDebug() << Q_FUNC_INFO << __LINE__ << state;
    setSwitchStatus();
}

QString NetConnect::oobeInstallerConfigPath()
{
    return QDir(QDir::homePath()).filePath(QStringLiteral("ky-installer.cfg"));
}


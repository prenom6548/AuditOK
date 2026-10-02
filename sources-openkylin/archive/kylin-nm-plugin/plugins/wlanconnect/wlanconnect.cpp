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
#include "wlanconnect.h"

#include "oobecheckstyle.h"

#include <QGSettings>
#include <QProcess>
#include <QTimer>
#include <QtDBus>
#include <QDir>
#include <QDebug>
#include <QtAlgorithms>
#include <QMessageBox>

#define SCANTIMER  20 * 1000
#define UPDATETIMER 10 * 1000
#define REFRESH_NETWORKSPEED_TIMER 1000

#define SPACING 8

const QString KWifiSymbolic     = "network-wireless-signal-excellent-symbolic";
const QString KWifiLockSymbolic = "network-wireless-signal-excellent-secure-symbolic";
const QString KWifiGood         = "network-wireless-signal-good-symbolic";
const QString KWifiLockGood     = "network-wireless-signal-good-secure-symbolic";
const QString KWifiOK           = "network-wireless-signal-ok-symbolic";
const QString KWifiLockOK       = "network-wireless-signal-ok-secure-symbolic";
const QString KWifiLow          = "network-wireless-signal-weak-symbolic";
const QString KWifiLockLow      = "network-wireless-signal-weak-secure-symbolic";
const QString KWifiNone         = "network-wireless-signal-none-symbolic";
const QString KWifiLockNone     = "network-wireless-signal-none-secure-symbolic";

const QString KApSymbolic       = "network-wireless-hotspot-symbolic";

const QString IsApConnection    = "1";

#define NO_MARGINS 0,0,0,0
#define TOP_MARGINS 0,8,0,0
#define MAIN_LAYOUT_MARGINS 0,0,0,0
#define MAIN_LAYOUT_SPACING 0
#define TITLE_FRAME_HEIGHT 53     //TabWidget的tab和widget有间隙，和设计稿看起来一致就不能设为设计稿里的高度
#define TITLE_LAYOUT_MARGINS 24,0,24,0

enum E_KylinDeviceType{
    KEYLIN_NC_NONE= 0, //无
    KEYLIN_NC_WIRED=1ul<<0,//有线
    KEYLIN_NC_WIRELESS=1ul<<1,//无线

    KEYLIN_NC_ALL=KEYLIN_NC_WIRED|KEYLIN_NC_WIRELESS,
};


bool intThan(int sign1, int sign2)
{
    return sign1 < sign2;
}

void WlanConnect::showDesktopNotify(const QString &message, QString soundName)
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

WlanConnect::WlanConnect() :  m_firstLoad(true), m_fileWatcher(nullptr), m_translator(nullptr)
{
    //dbus类型注册
    qRegisterMetaType<KyWirelessConnectSetting>("KyWirelessConnectSetting");
    qRegisterMetaType<KySecuType>("KySecuType");
    qRegisterMetaType<QList<KyActivateItem>>("QList<KyActivateItem>");
    qRegisterMetaType<QList<KyWirelessNetItem>>("QList<KyWirelessNetItem>");
    qRegisterMetaType<KyEapMethodPeapInfo>("KyEapMethodPeapInfo");
    qRegisterMetaType<KyEapMethodTtlsInfo>("KyEapMethodTtlsInfo");
    qRegisterMetaType<KyEapMethodLeapInfo>("KyEapMethodLeapInfo");
    qRegisterMetaType<KyEapMethodPwdInfo>("KyEapMethodPwdInfo");
    qRegisterMetaType<KyEapMethodFastInfo>("KyEapMethodFastInfo");
    qRegisterMetaType<KyDeviceState>("KyDeviceState");
    qRegisterMetaType<NetworkManager::Device::StateChangeReason>("NetworkManager::Device::StateChangeReason");


    m_fileWatcher = new QFileSystemWatcher(this);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &WlanConnect::onConfigFileChanged);
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged, this, &WlanConnect::onConfigFileChanged);
    
    QLocale local;
    QString locale = local.name();
    QTranslator* translator = new QTranslator(this);
    if (translator->load("/usr/share/kylin-nm-plugin/wlanconnect/wlan_" + locale)) {
        QApplication::installTranslator(translator);
    } else {
        qWarning() << "Translations load fail";
    }
}

WlanConnect::~WlanConnect()
{
    qDebug() << "~WlanConnect 1";

    thread->quit();
    thread->wait();
    delete thread;
    if (pluginWidget != nullptr) {
        delete pluginWidget;
    }
    keyAgentUnregister();
    qDebug() << "~WlanConnect 2";
}

QWidget *WlanConnect::pluginUi() {
    if (m_firstLoad) {
        m_firstLoad = false;
        
        // OOBE模式下读取家目录下 ky-installer.cfg，获取 language 参数并加载对应翻译
        if (OOBE == m_pluginType) {
            const QString configPath = oobeInstallerConfigPath();
            const QString configDir = QDir::homePath();
            
            loadLanguageFromConfig(); // 初始化加载语言
            
            if (!m_fileWatcher->files().contains(configPath) && QFile::exists(configPath)) {
                m_fileWatcher->addPath(configPath);
                qDebug() << "WLAN: Watching config file for changes:" << configPath;
            }
            if (!m_fileWatcher->directories().contains(configDir)) {
                m_fileWatcher->addPath(configDir);
                qDebug() << "WLAN: Watching config directory for changes:" << configDir;
            }
        }

        kylinAgentInit();
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        initUi();
        initConnect();
        initComponent();
        if(msettingLoad){
            hideTitle();
        }
        initNetCtrl();
    }
    return pluginWidget;
}

void WlanConnect::onConfigFileChanged(const QString &path)
{
    const QString configPath = oobeInstallerConfigPath();
    const QString configDir = QDir::homePath();

    if (path == configPath) {
        qDebug() << "WLAN: Config file changed, reloading language...";
        loadLanguageFromConfig();
    } else if (path == configDir) {
        QFileInfo fi(configPath);
        if (fi.exists()) {
            qDebug() << "WLAN: Config directory changed and config exists, reloading language...";
            loadLanguageFromConfig();
        }
    }

    // 确保监视器始终监视配置文件和目录
    if (!m_fileWatcher->directories().contains(configDir)) {
        m_fileWatcher->addPath(configDir);
    }
    if (!m_fileWatcher->files().contains(configPath) && QFile::exists(configPath)) {
        m_fileWatcher->addPath(configPath);
    }
}

void WlanConnect::loadLanguageFromConfig()
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
            // 移除旧的翻译器
            if (m_translator) {
                QApplication::removeTranslator(m_translator);
                delete m_translator;
                m_translator = nullptr;
            }

            // 加载新的翻译器 - 注意：WLAN插件翻译文件路径与有线插件不同
            m_translator = new QTranslator(this);
            QString translationPath = QString("/usr/share/kylin-nm-plugin/wlanconnect/wlan_%1").arg(languageCode);
            if (m_translator->load(translationPath)) {
                QApplication::installTranslator(m_translator);
                qDebug() << "WLAN: Language reloaded for locale:" << languageCode;
                retranslateUi();
            } else {
                qWarning() << "WLAN: Failed to load translation for locale:" << languageCode;
                delete m_translator;
                m_translator = nullptr;
            }
        }
    } else {
        qWarning() << "WLAN: Failed to open config file:" << configPath;
    }
}

void WlanConnect::retranslateUi()
{
    Q_EMIT languageChanged();
}

void WlanConnect::setParentWidget(QWidget* widget){
    if (parentWidget != nullptr) {
        return;
    }
    parentWidget = widget;
}

void WlanConnect::setWidgetVisable(bool visable) {
    if (pluginWidget == nullptr) {
        return;
    }

    if (deviceFrameMap.isEmpty()) {
        return;
    }

    if (visable) {
        return;
    }

    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        QMap<QString, WlanItem *>::iterator itemIter;
        for (itemIter = iter.value()->itemMap.begin(); itemIter != iter.value()->itemMap.end(); itemIter++) {
            itemIter.value()->setExpend(false);
        }
    }
}

bool WlanConnect::checkInputStauts() {
    return (getExpandedItem() != nullptr);
}

void WlanConnect::setInputStauts() {
    WlanItem* item = getExpandedItem();
    if (item != nullptr) {
        item->setExpend(true);
    }
}

void WlanConnect::setPluginType(PluginType type, bool useSwitch)
{
    m_pluginType = type;
    Q_UNUSED(useSwitch)
    if (type == SIMPLE) {
        m_isSimpleMode = true;
    } else {
        m_isSimpleMode = false;
    }
    if (type == OOBE){
        msettingLoad = true;
        m_isSimpleMode = true;
    }
}

bool WlanConnect::checkPluginIsHidden()
{
    return true;  //to show this plugin
#if 0
    if (nullptr != m_wirelessSwitch) {
        return m_wirelessSwitch->isEnabled();
    }
    return false;
#endif
}

void WlanConnect::initUi()
{
    thread = new QThread;
    manager = new KyNetworkManager();
    manager->moveToThread(thread);
    connect(thread, &QThread::started, manager, &KyNetworkManager::kylinNetworkManagerInit);
    connect(thread, &QThread::finished, manager, &KyNetworkManager::deleteLater);
    connect(thread, &QThread::finished, [=](){
        qDebug() << "WlanConnect thread quit";
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
        m_titleFrame->setFixedHeight(0);  // 在OOBE模式下隐藏标题
        m_titleFrame->setVisible(false);
    } else {
        m_titleFrame->setFixedHeight(TITLE_FRAME_HEIGHT);
    }
    m_titleLayout = new QHBoxLayout(m_titleFrame);
    m_titleLayout->setContentsMargins(TITLE_LAYOUT_MARGINS);
    m_titleLabel = new QLabel(m_titleFrame);
    m_titleLabel->setText(tr("WLAN"));
    m_wirelessSwitch = new KSwitchButton(pluginWidget);
    m_wirelessSwitch->installEventFilter(this);
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_wirelessSwitch);
    m_titleDivider = new Divider(pluginWidget);
    m_titleDivider->hide();

    m_scrollFrame = new QFrame(pluginWidget);
    if (OOBE == m_pluginType) {
        m_scrollFrame->setStyleSheet("QFrame { background-color: transparent; }");
    } else {
        if (m_isSimpleMode) {
            m_scrollFrame->setMinimumHeight(383);
        } else {
            m_scrollFrame->setMinimumHeight(330);
        }
    }
    if (msettingLoad){
        m_scrollFrame->setFixedHeight(700);
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


bool WlanConnect::eventFilter(QObject *w, QEvent *e) {
    if (w == m_wirelessSwitch && e->type() == QEvent::MouseButtonRelease) {
        if (!m_wirelessSwitch->isCheckable()) {
           showDesktopNotify(tr("No wireless device avaliable"), "networkwrong");
        } else {
            Q_EMIT setWirelessNetworkEnabled(!m_wirelessSwitch->isChecked());
        }
        return true;
    }
    return QObject::eventFilter(w,e);
}

void WlanConnect::initComponent() {

    m_wirelessSwitch->blockSignals(true);
    m_wirelessSwitch->setChecked(manager->getWirelessEnabled());
    m_wirelessSwitch->blockSignals(false);

    getDeviceList(deviceList);
    if (deviceList.isEmpty()) {
        qDebug() << "[WlanConnect]no device exist when init, set switch disable";
        m_wirelessSwitch->setChecked(false);
        m_wirelessSwitch->setCheckable(false);
        m_wirelessSwitch->setEnabled(false);
        qDebug() << "m_wirelessSwitch  setEnabled setCheckable setChecked" << false;
        qDebug() << "m_wirelessSwitch  setCheckable setChecked" << false;
    }

    initNet();

    if (!m_wirelessSwitch->isChecked() || deviceList.isEmpty()) {
        hideLayout(m_scrollAreaLayout);
        m_titleDivider->show();
    }

    connect(m_settingsLabel, &KBorderlessButton::clicked, this, [=]() {
        runExternalApp();
    });
}

void WlanConnect::initConnect()
{
    connect(manager, &KyNetworkManager::wirelessStateChange, this, &WlanConnect::onActiveConnectionChanged);
    connect(manager, &KyNetworkManager::deviceStateChange, this, &WlanConnect::onDeviceStatusChanged);
    connect(manager, &KyNetworkManager::wifiEnabledChanged, this, &WlanConnect::onWifiEnabledChanged);
    connect(manager, &KyNetworkManager::secuTypeChange, this, &WlanConnect::onNetworkSecuTypeChange);
    connect(manager, &KyNetworkManager::wirelessConnectionRemove, this, &WlanConnect::onWirelessConnectionRemove);
    connect(manager, &KyNetworkManager::wirelessConnectionAdd, this, &WlanConnect::onWirelessConnectionAdd);
    connect(manager, &KyNetworkManager::wirelessConnectionUpdate, this, &WlanConnect::onWirelessConnectionUpdate);
    connect(manager, &KyNetworkManager::wirelessDeviceAdd, this, &WlanConnect::onWirelessDeviceAdd);
    connect(manager, &KyNetworkManager::deviceRemove, this, &WlanConnect::onWirelessDeviceRemove);
    connect(manager, &KyNetworkManager::wirelessDeviceNameUpdate, this, &WlanConnect::onDeviceNameChanged);
    connect(manager, &KyNetworkManager::wifiNetworkAdd, this, &WlanConnect::onNetworkAdd);
    connect(manager, &KyNetworkManager::wifiNetworkRemove, this, &WlanConnect::onNetworkRemove);
    connect(manager, &KyNetworkManager::wifiNetworkUpdate, this, &WlanConnect::onNetworkUpdate);
    connect(manager, &KyNetworkManager::updateWifiList, this, &WlanConnect::updateList);
    connect(manager, &KyNetworkManager::deviceManagedChange, this, &WlanConnect::onDeviceManagedChange);

    connect(this, &WlanConnect::requestWirelessScan, manager, &KyNetworkManager::onRequestWirelessScan);
    connect(this, &WlanConnect::activateConnection, manager, &KyNetworkManager::onActivateConnection);
    connect(this, &WlanConnect::deactivateConnection, manager, &KyNetworkManager::onDeactivateConnection);
    connect(this,&WlanConnect::addAndActivateNormalWifi, manager, &KyNetworkManager::onAddAndActivateNormalWifi);
    connect(this, &WlanConnect::setWirelessNetworkEnabled, manager, &KyNetworkManager::onSetWirelessNetworkEnabled);
    connect(this, &WlanConnect::deleteConnect, manager, &KyNetworkManager::onDeleteConnect);

    //定时20s扫描
    m_scanTimer = new QTimer(this);
    m_scanTimer->start(SCANTIMER);
    connect(m_scanTimer, &QTimer::timeout, this, &WlanConnect::reScan);

    //10s重新排序
    //    m_updateTimer = new QTimer(this);
    //    m_updateTimer->start(UPDATETIMER);
    //    connect(m_updateTimer, &QTimer::timeout, this, &WlanConnect::updateList);

    reScan();
}

void WlanConnect::reScan()
{
    Q_EMIT requestWirelessScan();
}

//更新列表顺序 ->从so中重新拿 然后插+更新icon
void WlanConnect::updateList(QString devName, QList<KyActivateItem> connectItemList, QList<KyWirelessNetItem> list)
{
    if (!m_wirelessSwitch->isChecked()) {
        return;
    }

    if (!deviceFrameMap.keys().contains(devName)) {
        return;
    }

    resortWifiList(deviceFrameMap[devName], connectItemList, list);
}

void WlanConnect::resortWifiList(ItemFrame *frame, QList<KyActivateItem> connectItemList, QList<KyWirelessNetItem> list)
{
    qDebug() << "resortWifiList" << frame->deviceFrame->deviceLabel->text();
    if(nullptr == frame || frame->lanItemLayout->count() <= 0) {
        return;
    }
    int frameIndex = 0;
    QString activateSsid = "";
    for (int i = 0; i < connectItemList.size(); ++i) {
        if (connectItemList.at(i).m_connStatus == CONNECT_STATE_ACTIVATED) {
            activateSsid = connectItemList.at(i).m_ssid;
            for (int i = 0; i < list.size(); ++i) {
                if (list.at(i).m_NetSsid == activateSsid) {
                    updateIcon(frame->itemMap[list.at(i).m_NetSsid],
                            list.at(i).m_signalStrength,
                            list.at(i).m_secuType,
                            list.at(i).m_isApConnection);
                    break;
                }
            }
            frameIndex++;
            qDebug() << "resortWifiList no" << frameIndex << activateSsid << list.at(i).m_signalStrength;
            break;
        }
    }


    for (int listIndex = 0; listIndex < list.size(); ++listIndex) {
        if (frameIndex > frame->lanItemLayout->count() - 1) {
            return;
        }
        if (frame->itemMap.contains(list.at(listIndex).m_NetSsid) && list.at(listIndex).m_NetSsid != activateSsid) {
            frame->lanItemLayout->removeWidget(frame->itemMap[list.at(listIndex).m_NetSsid]);
            frame->lanItemLayout->insertWidget(frameIndex, frame->itemMap[list.at(listIndex).m_NetSsid]);
            if (!frame->itemMap[list.at(listIndex).m_NetSsid]->getIsLoading()) {
                updateIcon(frame->itemMap[list.at(listIndex).m_NetSsid],
                        list.at(listIndex).m_signalStrength,
                        list.at(listIndex).m_secuType,
                        list.at(listIndex).m_isApConnection);
            }
            frameIndex++;
            qDebug() << "resortWifiList no" << frameIndex << list.at(listIndex).m_NetSsid << list.at(listIndex).m_signalStrength;
        } else {
            qDebug() << "not find " << list.at(listIndex).m_NetSsid << " in current list, ignore";
        }
    }
    qDebug() << "resort finish";
}

void WlanConnect::updateIcon(WlanItem *item, int signalStrength, QString security, bool isApConnection)
{
    qDebug() << "updateIcon" << item->getName();

    int sign = setSignal(signalStrength);
    bool isLock = true;
    if (security.isEmpty()) {
        isLock = false;
    } else {
        isLock = true;
    }

    QString iconamePath;
    if (isApConnection) {
        iconamePath = KApSymbolic;
    } else {
        iconamePath = wifiIcon(isLock, sign);
    }
    QIcon searchIcon = QIcon::fromTheme(iconamePath);

    item->setItemIcon(searchIcon);
    qDebug() << "updateIcon" << item->getName() << " finish";
}

//device status change
void WlanConnect::onDeviceStatusChanged()
{

}

void WlanConnect::onDeviceNameChanged(QString oldName, QString newName)
{
    qDebug() << "============onDeviceNameChanged" << oldName << newName;
    if (!deviceFrameMap.contains(oldName) || !deviceList.contains(oldName)) {
        qWarning() << "[WlanConnect]onDeviceNameChanged no such device " << oldName;
        return;
    }

    if (deviceFrameMap.contains(newName) && deviceList.contains(newName)) {
        qWarning() << "[WlanConnect]onDeviceNameChanged already has device " << newName;
        return;
    }
    removeDeviceFrame(oldName);

    getDeviceList(deviceList);
    if (deviceList.contains(newName)) {
        addDeviceFrame(newName);
        initNetListFromDevice(newName);
        setSwitchStatus(manager->getWirelessEnabled());
    }
}

//activeconnect status change
void WlanConnect::onActiveConnectionChanged(QString deviceName, QString ssid, QString uuid, KyConnectState status)
{
    qDebug() << "onActiveConnectionChanged" << ssid << status;
    if (!m_wirelessSwitch->isChecked()) {
        return;
    }
    if (uuid.isEmpty()) {
        return;
    }
    WlanItem * item= nullptr;
    //device ssid 有可能均为空
    if (deviceName.isEmpty() || ssid.isEmpty()) {
        if (status == CONNECT_STATE_ACTIVATING || status == CONNECT_STATE_ACTIVATED) {
            return;
        }
        QMap<QString, ItemFrame*>::iterator iter;
        for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
            if (uuid == iter.value()->uuid) {
                QMap<QString, WlanItem*>::iterator itemIter;
                for (itemIter = iter.value()->itemMap.begin(); itemIter != iter.value()->itemMap.end(); itemIter++) {
                    if (itemIter.value()->getUuid() == uuid ) {
                        item = itemIter.value();
                        break;
                    }
                }
                break;
            }
        }
    } else {
        if (!deviceFrameMap.contains(deviceName)) {
            return;
        }
        for (int i = 0; i < deviceFrameMap[deviceName]->itemMap.size(); ++i) {
            if (deviceFrameMap[deviceName]->itemMap.contains(ssid)) {
                item = deviceFrameMap[deviceName]->itemMap[ssid];
                if (status == CONNECT_STATE_ACTIVATED || status == CONNECT_STATE_ACTIVATING) {
                    deviceFrameMap[deviceName]->itemMap[ssid]->setUuid(uuid);
                    deviceFrameMap[deviceName]->uuid = uuid;
                    if (status == CONNECT_STATE_ACTIVATED) {
                        deviceFrameMap[deviceName]->lanItemLayout->removeWidget(item);
                        deviceFrameMap[deviceName]->lanItemLayout->insertWidget(0,item);
                    }
                } else if (status == CONNECT_STATE_DEACTIVATED) {
                    deviceFrameMap[deviceName]->uuid.clear();
                }
                break;
            }
        }
    }

    if (nullptr != item) {
        // When a connection is starting or has become active, collapse other
        // expanded items and reset their active status so only the new item
        // shows as active.
        if (status == CONNECT_STATE_ACTIVATING || status == CONNECT_STATE_ACTIVATED) {
            clearOtherActiveItems(deviceName, uuid);
        }

        itemActiveConnectionStatusChanged(item, status);
        if (status == CONNECT_STATE_ACTIVATED ||
                status == CONNECT_STATE_DEACTIVATED) {
            QList<KyWirelessNetItem> list;
            manager->getWifiNetworkList(deviceName, list);
            for (int i = 0; i < list.size(); ++i) {
                if (list.at(i).m_NetSsid == ssid) {
                    updateIcon(item,list.at(i).m_signalStrength,list.at(i).m_secuType,list.at(i).m_isApConnection);
                    break;
                }
            }
        }
    }
}

//wifi add==============================================================
void WlanConnect::onNetworkAdd(QString deviceName, KyWirelessNetItem wlanInfo)
{
    qDebug()<<"[WlanConnect]onNetworkAdd "<< deviceName << " " << wlanInfo.m_NetSsid;
    if(!m_wirelessSwitch->isChecked() || deviceName.isEmpty()) {
        return;
    }

    if (!deviceList.contains(deviceName)) {
        qDebug() << "[WlanConnect]onNetworkAdd not contain " << deviceName << "then add";
        deviceList.append(deviceName);
        addDeviceFrame(deviceName);
        onNetworkAdd(deviceName, wlanInfo);
        return;
    }

    insertOneWlanFrame(deviceFrameMap[deviceName], deviceName, wlanInfo);
}

//wifi remove =============================================================
void WlanConnect::onNetworkRemove(QString deviceName, QString wlannName)
{
    //当前无此设备 忽略
    if (deviceName.isEmpty() || !deviceFrameMap.contains(deviceName)) {
        qWarning() << "[WlanConnect]recieve network remove,but no such device:" << deviceName;
        return;
    }
    qDebug()<<"[WlanConnect]Wifi remove device:" << deviceName << ",wlan name:" << wlannName;
    removeOneWlanFrame(deviceFrameMap[deviceName], deviceName, wlannName);
}

//wifi secu Update =============================================================
void WlanConnect::onNetworkUpdate(QString deviceName, QString wlannName, KyWirelessNetItem item)
{
    //当前无此设备 忽略
    if (deviceName.isEmpty() || !deviceFrameMap.contains(deviceName)) {
        qWarning() << "[WlanConnect]recieve network update,but no such device:" << deviceName;
        return;
    }
    qDebug()<<"[WlanConnect]Wifi update device:" << deviceName << ",wlan name:" << wlannName;

    bool isFind = false;
    QMap<QString, WlanItem*>::iterator itemIter;
    for (itemIter = deviceFrameMap[deviceName]->itemMap.begin(); itemIter != deviceFrameMap[deviceName]->itemMap.end(); ++itemIter) {
        if (itemIter.value()->getName() == wlannName) {
            itemIter.value()->setSecuType(item.m_secuType);
            isFind = true;
            break;
        }
    }

    if (!isFind) {
        insertOneWlanFrame(deviceFrameMap[deviceName], deviceName, item);
    }
}

//获取设备列表=======================================================
void WlanConnect::getDeviceList(QStringList &list)
{
    // Query network manager for wifi devices
    manager->getNetworkDeviceList(DEVICE_TYPE_WIFI, list);

    // Remove duplicate entries while preserving the original order.
    // `manager->getNetworkDeviceList` may return duplicates in some environments,
    // so perform a simple dedupe here to make downstream logic simpler.
    QStringList uniqueList;
    QSet<QString> seen;
    for (const QString &dev : list) {
        if (!seen.contains(dev)) {
            seen.insert(dev);
            uniqueList.append(dev);
        }
    }
    list = uniqueList;
}

//设置开关
void WlanConnect::setSwitchStatus(bool status)
{
    m_wirelessSwitch->setCheckable(true);
    m_wirelessSwitch->blockSignals(true);
    m_wirelessSwitch->setChecked(status);
    m_wirelessSwitch->blockSignals(false);
    if (!status) {
        hideLayout(m_scrollAreaLayout);
        m_titleDivider->show();
    } else {
        showLayout(m_scrollAreaLayout);
        m_titleDivider->hide();
    }
}

//初始化整体列表和单设备列表
void WlanConnect::initNet() {
    //先构建每个设备的列表头
    qDebug() << deviceList <<deviceList.size();
    for (int i = 0; i < deviceList.size(); ++i) {
        addDeviceFrame(deviceList.at(i));
    }
    //再填充每个设备的列表
    for (int i = 0; i < deviceList.size(); ++i) {
        initNetListFromDevice(deviceList.at(i));
    }
    m_setNetSpeedTimer->start(REFRESH_NETWORKSPEED_TIMER);
}

//初始化设备列表
void WlanConnect::initNetListFromDevice(QString deviceName)
{
    qDebug() << "[WlanConnect]initNetListFromDevice " << deviceName;
    if (!m_wirelessSwitch->isChecked()) {
        qWarning() << "[WlanConnect]initNetListFromDevice " << deviceName << " switch off";
        return;
    }
    if (!deviceFrameMap.contains(deviceName)) {
        qWarning() << "[WlanConnect]initNetListFromDevice " << deviceName << " not exist";
        return;
    }
    QList<KyWirelessNetItem> list;
    manager->getWifiNetworkList(deviceName, list);

    for (int i = 0; i < list.size(); ++i) {
        KyWirelessNetItem item = list.at(i);
        addOneWlanFrame(deviceFrameMap[deviceName], deviceName, item);
    }
    QList<KyActivateItem> activateList;
    manager->getActiveConnectionList(deviceName, CONNECT_TYPE_WIRELESS, activateList);
    if (activateList.size() != 0) {
        onActiveConnectionChanged(deviceName,activateList.at(0).m_ssid, activateList.at(0).m_uuid, activateList.at(0).m_connStatus);
    }
}

//高级设置
void WlanConnect::runExternalApp() {
    QProcess process;
    process.startDetached("ukui-control-center -m wlanconnect");
}

//根据信号强度分级+安全性分图标
QString WlanConnect::wifiIcon(bool isLock, int strength) {
    switch (strength) {
    case 1:
        return isLock ? KWifiLockSymbolic : KWifiSymbolic;
    case 2:
        return isLock ? KWifiLockGood : KWifiGood;
    case 3:
        return isLock ? KWifiLockOK : KWifiOK;
    case 4:
        return isLock ? KWifiLockLow : KWifiLow;
    case 5:
        return isLock ? KWifiLockNone : KWifiNone;
    default:
        return "";
    }
}

//根据信号强度分级
int WlanConnect::setSignal(int signal) {
    int signalLv = 0;

    if (signal > 75) {
        signalLv = 1;
    } else if (signal > 55 && signal <= 75) {
        signalLv = 2;
    } else if (signal > 35 && signal <= 55) {
        signalLv = 3;
    } else if (signal > 15 && signal  <= 35) {
        signalLv = 4;
    } else if (signal <= 15) {
        signalLv = 5;
    }
    return signalLv;
}

//隐藏
void WlanConnect::hideLayout(QVBoxLayout * layout) {
    for (int i = layout->layout()->count()-1; i >= 0; --i) {
        QLayoutItem *it = layout->layout()->itemAt(i);
        ItemFrame *itemFrame = qobject_cast<ItemFrame *>(it->widget());
        itemFrame->hide();
    }
}

//显示
void WlanConnect::showLayout(QVBoxLayout * layout) {
    for (int i = layout->layout()->count()-1; i >= 0; --i) {
        QLayoutItem *it = layout->layout()->itemAt(i);
        ItemFrame *itemFrame = qobject_cast<ItemFrame *>(it->widget());
        itemFrame->show();
    }
}

//增加设备
void WlanConnect::addDeviceFrame(QString devName)
{
    // 防止重复添加相同的设备
    if (deviceFrameMap.contains(devName)) {
        qDebug() << "[WlanConnect]Device already exists, skipping: " << devName;
        return;
    }
    
    qDebug() << "[WlanConnect]addDeviceFrame " << devName;
    ItemFrame *itemFrame = new ItemFrame(devName, pluginWidget);
    m_scrollAreaLayout->addWidget(itemFrame);
    
    // 在OOBE模式下，如果只有一个网卡，则不设置设备名称，直接隐藏设备框架
    if (m_pluginType == OOBE && deviceList.size() == 1) {
        itemFrame->deviceFrame->hide();
        itemFrame->m_divider->hide();
    } else {
        // 非OOBE模式或多个网卡时，才显示设备名称
        itemFrame->deviceFrame->deviceLabel->setText(devName);
    }
    
    deviceFrameMap.insert(devName, itemFrame);
    if (deviceList.indexOf(devName) < 0) {
        deviceList << devName;
    }
    itemFrame->addNetItem->setHidden(m_hideAddbutton);// m_hiddenItem 管控需要还原
    connect(itemFrame, &ItemFrame::addNetItemClick , [=](){
        //加入网络
        itemFrame->showJoinPage(parentWidget, m_pluginType == OOBE);
    });

    connect(itemFrame, &ItemFrame::connectHideNormalConnect, manager, &KyNetworkManager::onAddAndActivateNormalWifi);
    connect(itemFrame, &ItemFrame::connectHideTtlsConnect, manager, &KyNetworkManager::onAddAndActiveWirelessEnterPriseTtlsConnect);
    connect(itemFrame, &ItemFrame::connectHidePeapConnect, manager, &KyNetworkManager::onAddAndActiveWirelessEnterPrisePeapConnect);
}

//减少设备
void WlanConnect::removeDeviceFrame(QString devName)
{
    qDebug() << "[WlanConnect]removeDeviceFrame " << devName;
    if (deviceFrameMap.contains(devName)) {
        ItemFrame *item = deviceFrameMap[devName];
        if (item->lanItemFrame->layout() != NULL) {
            QLayoutItem* layoutItem;
            while ((layoutItem = item->lanItemFrame->layout()->takeAt(0)) != NULL) {
                delete layoutItem->widget();
                delete layoutItem;
            }
            item->itemMap.clear();
        }
        delete item;
        item = nullptr;
        item->disconnect(this);
        deviceFrameMap.remove(devName);
    }
//    getDeviceList(deviceList);
    deviceList.removeOne(devName);
}

//增加ap
void WlanConnect::addOneWlanFrame(ItemFrame *frame, QString deviceName, KyWirelessNetItem &wlanInfo)
{
    if (nullptr == frame) {
        return;
    }

    if (frame->itemMap.contains(wlanInfo.m_NetSsid)) {
        qDebug() << "[WlanConnect]Already exist a wifi " << wlanInfo.m_NetSsid << " in " << deviceName;
        return;
    }
    //设置单项的信息
    int sign = setSignal(wlanInfo.m_signalStrength);
    WlanItem * wlanItem = new WlanItem(m_pluginType, pluginWidget);
    QString iconamePath;
    if (wlanInfo.m_isApConnection) {
        iconamePath = KApSymbolic;
    } else {
        iconamePath = wifiIcon(!wlanInfo.m_secuType.isEmpty(), sign);
    }

    QIcon searchIcon = QIcon::fromTheme(iconamePath);
    wlanItem->setItemIcon(searchIcon);
    wlanItem->setName(wlanInfo.m_NetSsid);
    wlanItem->setSecuType(wlanInfo.m_secuType);
    wlanItem->setUuid(wlanInfo.m_connectUuid);
    wlanItem->setPath(wlanInfo.m_dbusPath);
    wlanItem->setInterface(deviceName);

    connect(wlanItem, &WlanItem::infoButtonClick, this, [=]{
        //todo 详情页
    });

    connect(wlanItem, &WlanItem::itemClick, this, [=] {
        if (wlanItem->getStatus() || wlanItem->getIsLoading()) {
            Q_EMIT deactivateConnection(wlanItem->getUuid());
        } else {
            if (wlanItem->getExpend()) {
                wlanItem->setExpend(false);
            } else {
                //先看加密类型 再看是否已有配置
                if (!wlanItem->getUuid().isEmpty()) {
                    activateConnection(wlanItem->getUuid(), wlanItem->getInterface());
                } else {
                    if (wlanItem->getSecuType().isEmpty()) {
                        KyWirelessConnectSetting sett;
                        KySecuType secuType = KySecuType::NONE;
                        sett.m_ssid = wlanItem->getName();
                        sett.m_ifaceName = wlanItem->getInterface();
                        Q_EMIT addAndActivateNormalWifi(sett, secuType);
                    } else {
                        if (wlanItem->getSecuType().indexOf("802.1X") != -1) {
                            //todo 企业网页面
                            wlanItem->showEnterPricePage(deviceName, parentWidget);
                        } else {
                            wlanItem->setExpend(true);
                            setOtherItemExpandedFalse(deviceName, wlanInfo.m_NetSsid);
                        }
                    }
                }
            }
        }
    });
    connect(wlanItem, &WlanItem::connectButtonClick, this, [=](QString password, bool isAutoConnect) {
        KySecuType secuType;
        if (wlanItem->getSecuType().indexOf("WPA3")) {
            secuType = KySecuType::WPA3_PERSONAL;
        } else {
            secuType = KySecuType::WPA_AND_WPA2_PERSONAL;
        }
        KyWirelessConnectSetting sett;
        sett.m_ssid = wlanItem->getName();
        sett.m_psk = password;
        sett.m_isAutoConnect = isAutoConnect;
        sett.m_ifaceName = wlanItem->getInterface();
        Q_EMIT addAndActivateNormalWifi(sett, secuType);
        wlanItem->setExpend(false);
    });
    connect(wlanItem, &WlanItem::connectButtonClickForAgent, this, [=](const QString &reqId, const QString &pwd, bool autoConnect) {
        m_agent->provideWirelessSecret(reqId, pwd, autoConnect);
        wlanItem->setExpend(false);
    });

    connect(wlanItem, &WlanItem::connectPeapConnect, manager, &KyNetworkManager::onAddAndActiveWirelessEnterPrisePeapConnect);
    connect(wlanItem, &WlanItem::connectTtlsConnect, manager, &KyNetworkManager::onAddAndActiveWirelessEnterPriseTtlsConnect);
    connect(wlanItem, &WlanItem::passwordFocusIn, this, &WlanConnect::needShowVirtualKeyboard);
    connect(wlanItem, &WlanItem::connectLeapConnect, manager, &KyNetworkManager::onAddAndActiveWirelessEnterPriseLeapConnect);
    connect(wlanItem, &WlanItem::connectPwdConnect, manager, &KyNetworkManager::onAddAndActiveWirelessEnterPrisePwdConnect);
    connect(wlanItem, &WlanItem::connectFastConnect, manager, &KyNetworkManager::onAddAndActiveWirelessEnterPriseFastConnect);
    //记录到deviceFrame的itemMap中
    deviceFrameMap[deviceName]->itemMap.insert(wlanInfo.m_NetSsid, wlanItem);

    qDebug()<<"add " << wlanInfo.m_NetSsid << " to " << deviceName << " list";
    frame->lanItemLayout->addWidget(wlanItem);
}

//减少ap
void WlanConnect::removeOneWlanFrame(ItemFrame *frame, QString deviceName, QString ssid)
{
    if (nullptr == frame) {
        return;
    }

    if (frame->itemMap.contains(ssid)) {
        qDebug() << "[WlanConnect]removeOneWlanFrame " << deviceName << ssid;
        frame->lanItemLayout->removeWidget(frame->itemMap[ssid]);
        delete frame->itemMap[ssid];
        frame->itemMap.remove(ssid);
    }
}

void WlanConnect::insertOneWlanFrame(ItemFrame *frame, QString devName, KyWirelessNetItem &wlanInfo)
{
    addOneWlanFrame(frame, devName, wlanInfo);
}

void WlanConnect::itemActiveConnectionStatusChanged(WlanItem *item, int status)
{
    if (status == CONNECT_STATE_ACTIVATING) {
        item->startLoading();
    } else if (status == CONNECT_STATE_ACTIVATED) {
        item->stopLoading();
        item->setStatus(true);
    } else if (status == CONNECT_STATE_DEACTIVATING) {
        item->startLoading();
    } else if (status == CONNECT_STATE_DEACTIVATED) {
        item->stopLoading();
        item->setStatus(false);
    }
}

void WlanConnect::onNetworkSecuTypeChange(QString deviceName, QString ssid, QString securityType)
{
    Q_UNUSED(deviceName)
    Q_UNUSED(ssid)
    Q_UNUSED(securityType)
}

void WlanConnect::onWifiEnabledChanged(bool status)
{
    qDebug() << "[WlanConnect]onWifiEnabledChanged status:" << status;
    setSwitchStatus(status);
    
    // WiFi开启时，主动刷新网络列表，解决信号竞态导致列表为空的问题
    if (status) {
        // 延迟刷新，等待设备状态稳定
        QTimer::singleShot(500, this, [this]() {
            if (!m_wirelessSwitch->isChecked()) {
                return;
            }
            qDebug() << "[WlanConnect]onWifiEnabledChanged refresh wifi list";
            // 重新初始化每个设备的网络列表
            for (const QString &deviceName : deviceList) {
                if (deviceFrameMap.contains(deviceName)) {
                    ItemFrame *frame = deviceFrameMap[deviceName];
                    // 只有当列表为空时才重新加载
                    if (frame->itemMap.isEmpty()) {
                        initNetListFromDevice(deviceName);
                    }
                }
            }
            // 触发扫描
            reScan();
        });
    }
}

void WlanConnect::onDeviceManagedChange(QString deviceName, bool status)
{
    qDebug() << "[WlanConnect]onDeviceManagedChange device:" << deviceName << "managed:" << status;
    // 只有当设备被取消管理时才关闭开关，避免与wifiEnabledChanged冲突
    // 设备被管理时不主动开启开关，应由wifiEnabledChanged信号控制
    if (!status && deviceList.contains(deviceName)) {
        // 检查是否还有其他可用设备
        bool hasOtherManagedDevice = false;
        for (const QString &dev : deviceList) {
            if (dev != deviceName && manager->getDeviceState(dev) >= DEVICE_STATE_UNAVAILABLE) {
                hasOtherManagedDevice = true;
                break;
            }
        }
        if (!hasOtherManagedDevice) {
            setSwitchStatus(false);
        }
    }
}

void WlanConnect::onWirelessConnectionRemove(QString deviceName, QString ssid)
{
    qDebug() << "onWirelessConnectionRemove" << deviceName << ssid;
    WlanItem *item = findItem(deviceName,ssid);
    if (nullptr == item) {
        return;
    }

    if (item->getStatus()) {
        onActiveConnectionChanged(deviceName, ssid, item->getUuid(), CONNECT_STATE_DEACTIVATED);
    }
    item->setUuid("");
    item->setPath("");
}

void WlanConnect::onWirelessConnectionAdd(QString deviceName, QString ssid, QString uuid, QString dbusPath)
{
    qDebug() << "onWirelessConnectionAdd" << deviceName << ssid;
    WlanItem *item = findItem(deviceName,ssid);
    if (nullptr == item) {
        return;
    }
    if (!item->getUuid().isEmpty()
            && !item->getPath().isEmpty())
        item->setUuid(uuid);
    item->setPath(dbusPath);
}

void WlanConnect::onWirelessConnectionUpdate(QString deviceName, QString ssid, QString uuid, QString dbusPath, KySecuType connectSecuType)
{

}

void WlanConnect::onWirelessDeviceAdd(QString deviceName)
{
    addDeviceFrame(deviceName);
    setSwitchStatus(manager->getWirelessEnabled());
}

void WlanConnect::onWirelessDeviceRemove(QString deviceName)
{
    if (!deviceFrameMap.contains(deviceName) || deviceList.indexOf(deviceName) < 0) {
        return;
    }
    removeDeviceFrame(deviceName);
    if (deviceList.isEmpty()) {
        setSwitchStatus(false);
        m_wirelessSwitch->setCheckable(false);
    }
}

void WlanConnect::onSetNetSpeed()
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

WlanItem* WlanConnect::findItem(QString devName, QString ssid)
{
    if (!deviceFrameMap.contains(devName)) {
        return nullptr;
    }

    if (!deviceFrameMap[devName]->itemMap.contains(ssid)) {
        return nullptr;
    }

    return deviceFrameMap[devName]->itemMap[ssid];
}

void WlanConnect::setOtherItemExpandedFalse(QString devName, QString ssid)
{
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        if (iter.key() != devName) {
            QMap<QString, WlanItem *>::iterator itemIter;
            for (itemIter = iter.value()->itemMap.begin(); itemIter != iter.value()->itemMap.end(); itemIter++) {
                itemIter.value()->setExpend(false);
            }
        } else {
            QMap<QString, WlanItem *>::iterator itemIter;
            for (itemIter = iter.value()->itemMap.begin(); itemIter != iter.value()->itemMap.end(); itemIter++) {
                if (itemIter.value()->getName() != ssid) {
                    itemIter.value()->setExpend(false);
                }
            }
        }
    }
}

WlanItem* WlanConnect::getExpandedItem()
{
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        QMap<QString, WlanItem *>::iterator itemIter;
        for (itemIter = iter.value()->itemMap.begin(); itemIter != iter.value()->itemMap.end(); itemIter++) {
            if (itemIter.value()->getExpend()) {
                return itemIter.value();
            }
        }
    }
    return nullptr;
}

void WlanConnect::clearOtherActiveItems(const QString &devName, const QString &uuid)
{
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        QMap<QString, WlanItem *>::iterator itemIter;
        for (itemIter = iter.value()->itemMap.begin(); itemIter != iter.value()->itemMap.end(); itemIter++) {
            WlanItem *itm = itemIter.value();
            // If uuid provided, skip that item
            if (!uuid.isEmpty() && itm->getUuid() == uuid) {
                continue;
            }
            // collapse any expanded item
            if (itm->getExpend()) {
                itm->setExpend(false);
            }
            // clear active status (this will update the RadioItemButton icon)
            if (itm->getStatus()) {
                itm->setStatus(false);
            }
        }
    }
}

bool WlanConnect::getWirelessDeviceUseable()
{
    for (auto devname : deviceList) {
        qDebug() << devname << (KyDeviceState)manager->getDeviceState(devname);
        if (manager->getDeviceState(devname) >= DEVICE_STATE_DISCONNECTED) {
            return true;
        }
    }
    return false;
}

void WlanConnect::showRate()
{
    m_setNetSpeedTimer = new QTimer(this);
    m_setNetSpeedTimer->setTimerType(Qt::PreciseTimer);
    connect(m_setNetSpeedTimer, &QTimer::timeout,  [&]() {
        onSetNetSpeed();
    });
}

bool WlanConnect::getWirelessDeviceAvailable()
{
    for (auto devname : deviceList) {
        qDebug() << devname << (KyDeviceState)manager->getDeviceState(devname);
        if (manager->getDeviceState(devname) >= DEVICE_STATE_UNAVAILABLE) {
            return true;
        }
    }
    return false;
}

void WlanConnect::hideTitle()
{
    m_wirelessSwitch->hide();
    m_settingsLabel->hide();
    m_titleFrame->hide();
}

void WlanConnect::keyAgentRegister()
{
    if (m_agent) {
        return;
    }
    m_agent = new KylinAgent(this);
    if (m_agent) {
        m_agent->setParentWidget(parentWidget);
        m_agent->startKylinAgent();
        connect(m_agent, &KylinAgent::requestWirelessSecret,
                this, &WlanConnect::onRequestWirelessSecret);
        connect(m_agent, &KylinAgent::requestEnterpriseSecret,
                this, &WlanConnect::onRequestEnterpriseSecret);
        qDebug() << "keyAgentRegister startKylinAgent";
    }
}

void WlanConnect::keyAgentUnregister()
{
    qDebug() << "keyAgentUnregister";
    if (m_agent) {
        delete m_agent;
        m_agent = nullptr;
    }
}

/*弹窗时机应该自洽，原方案两个包之间会通过dbus交互，登录锁屏前端容易崩，直接使用后端锁屏与否的信号来决定是否在桌面注册弹窗*/
void WlanConnect::kylinAgentInit()
{
    bool lockState = true;

    QDBusInterface dbusInterface("org.ukui.ScreenSaver",
                                 "/",
                                 "org.ukui.ScreenSaver",
                                 QDBusConnection::sessionBus());
    if (!dbusInterface.isValid()) {
        qWarning()<<Q_FUNC_INFO<<__LINE__<<"dbusInterface error!";
        lockState=true;
    }
    else {
        dbusInterface.setTimeout(2000);
        QDBusMessage result = dbusInterface.call("GetLockState");
        if(result.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "GetLockState error:" << result.errorMessage();
            lockState=true;
        }
        else {
            lockState = result.arguments().at( 0 ).toBool();
        }
    }

    if (lockState) {
        keyAgentRegister();
    } else {
        keyAgentUnregister();
    }

    QDBusConnection::sessionBus().connect("org.ukui.ScreenSaver",
                                          "/",
                                          "org.ukui.ScreenSaver",
                                          "lock",
                                          this,
                                          SLOT(keyAgentRegister()));

    QDBusConnection::sessionBus().connect("org.ukui.ScreenSaver",
                                          "/",
                                          "org.ukui.ScreenSaver",
                                          "unlock",
                                          this,
                                          SLOT(keyAgentUnregister()));

    qDebug()<<Q_FUNC_INFO<<__LINE__<<"kylin agent init success!"<<lockState;
}

/*显示添加按钮 未管控*/
void WlanConnect::updateAddButtonShow(bool netctlEnable)
{
    QMap<QString, ItemFrame*>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        iter.value()->addNetItem->setHidden(netctlEnable);
    }
    m_hideAddbutton=netctlEnable;
}

void WlanConnect::updateNetCtrl(QString modName,QVariantMap value)
{
    bool enable=false;

    if(modName!="Connect") return;

    qInfo()<<"[WlanPage]"<<modName<<value;
    for (auto it = value.cbegin(); it != value.cend(); ++it) {
        QString key = it.key();
        QVariant value = it.value();
        if(key==QString("addConnectCtrol")) {
            enable=((value.toUInt()&KEYLIN_NC_WIRELESS)==KEYLIN_NC_WIRELESS)? true:false;
            updateAddButtonShow(enable);
        }
    }
    return;
}

void WlanConnect::initNetCtrl()
{
    QVariantMap map;
    int errCode=0;
    QString netCtrlConnectName="Connect";
    QDBusInterface dbusInterface("com.kylin.networkCtrol",
                                 "/com/kylin/networkCtrol",
                                 "com.kylin.networkCtrol",
                                 QDBusConnection::systemBus());
    if (!dbusInterface.isValid()) {
        qWarning()<<Q_FUNC_INFO<<__LINE__<<"dbusInterface error!";
    } else {
        dbusInterface.setTimeout(2000);
        QDBusMessage result = dbusInterface.call("getNetContrlRule",netCtrlConnectName);
        if(result.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "[WlanPage]getNetContrlRule error:" << result.errorMessage();
        } else {
            if( result.arguments().size()>=2) {
                const QDBusArgument &dbusArg1st = result.arguments().at( 0 ).value<QDBusArgument>();
                dbusArg1st >> map;
                errCode = result.arguments().at( 1 ).toInt();
                qInfo()<<"[WlanPage]"<<map<<errCode;
                if(errCode==0) updateNetCtrl(netCtrlConnectName,map);
                map.clear();
            }
        }
    }

    QDBusConnection::systemBus().connect("com.kylin.networkCtrol",
                                         "/com/kylin/networkCtrol",
                                         "com.kylin.networkCtrol",
                                         "sigNetContrlRuleChanged",
                                         this,
                                         SLOT(updateNetCtrl(QString ,QVariantMap)));

    qInfo()<<"[WlanPage] initNetCtrl success";
    return;
}

void WlanConnect::onRequestWirelessSecret(const QString &ssid, const QString &password, const QString &devName, const QString &settingName, uint flags, const QString &requestId)
{
    // 通过设备名和SSID找到对应的WlanItem
    WlanItem *item = findItem(devName, ssid);
    if (!item) {
        qWarning() << "onRequestWirelessSecret: cannot find item" << devName << ssid;
        m_agent->cancelSecretRequest(requestId);
        return;
    }
    qDebug()<<Q_FUNC_INFO<<__LINE__<<ssid<<devName<<flags<<requestId<<settingName;

    // 如果当前有其他item展开且不是当前item，先收起并取消其请求
    WlanItem* expanded = getExpandedItem();
    if (expanded && expanded != item) {
        expanded->setExpend(false);
    }

    item->setPasswordError(password);  // 显示错误状态，并回填密码
    item->setRequestId(requestId);  // 需要在WlanItem中添加setRequestId方法
    item->setExpend(true);
}

void WlanConnect::onRequestEnterpriseSecret(const QString &ssid, const QString &devName, const QString &settingName, uint flags, const QVariantMap &currentSettings, const QString &requestId)
{
    // 创建EnterpriseWlanPage，并预填currentSettings中的信息
    bool isOobe = (m_pluginType == OOBE);
    bool isLockScreen = (m_pluginType == SIMPLE);
    EnterpriseWlanPage *page = new EnterpriseWlanPage(ssid, devName, isOobe, isLockScreen, parentWidget);
    page->setPreFillInfo(currentSettings);

    // 连接page的信号到回传
    connect(page, &EnterpriseWlanPage::reConnectPeapConnect, this,
       [this, requestId](KyEapMethodPeapInfo info, KyWirelessConnectSetting connSetting) {
           QVariantMap secretMap;
           secretMap[IDENTITY_KEY] = info.userName;
           secretMap[PASSWORD_KEY] = info.userPWD;
           secretMap["phase2-auth"] = info.phase2AuthMethod;
           // 这里需要根据settingName决定使用哪个键，但暂时用ENTERPRICE_NAME
           m_agent->provideEnterpriseSecret(requestId, secretMap);
    });
    connect(page, &EnterpriseWlanPage::reConnectTtlsConnect, this,
       [this, requestId](KyEapMethodTtlsInfo info, KyWirelessConnectSetting connSetting) {
           QVariantMap secretMap;
           secretMap[IDENTITY_KEY] = info.userName;
           secretMap[PASSWORD_KEY] = info.userPWD;
           m_agent->provideEnterpriseSecret(requestId, secretMap);
    });
    connect(page, &EnterpriseWlanPage::reConnectLeapConnect, this,
       [this, requestId](KyEapMethodLeapInfo info, KyWirelessConnectSetting connSetting){
           QVariantMap secrets;
           secrets[LEAP_USER_KEY] = info.m_userName;
           secrets[LEAP_PASSWORD_KEY] = info.m_userPwd;
           m_agent->provideEnterpriseSecret(requestId, secrets);
    });
    connect(page, &EnterpriseWlanPage::reConnectPwdConnect, this,
       [this, requestId](KyEapMethodPwdInfo info, KyWirelessConnectSetting connSetting){
           QVariantMap secrets;
           secrets[IDENTITY_KEY] = info.m_userName;
           secrets[PASSWORD_KEY] = info.m_userPwd;
           m_agent->provideEnterpriseSecret(requestId, secrets);
    });
    connect(page, &EnterpriseWlanPage::reConnectFastConnect, this,
       [this, requestId](KyEapMethodFastInfo info, KyWirelessConnectSetting connSetting){
           QVariantMap secrets;
           secrets[IDENTITY_KEY] = info.m_userName;
           secrets[PASSWORD_KEY] = info.m_userPwd;
           m_agent->provideEnterpriseSecret(requestId, secrets);
    });

    // 如果页面被关闭，取消请求
    connect(page, &EnterpriseWlanPage::destroyed, this, [this, requestId]() {
        m_agent->cancelSecretRequest(requestId);
    });

    page->show();

}

QString WlanConnect::oobeInstallerConfigPath()
{
    return QDir(QDir::homePath()).filePath(QStringLiteral("ky-installer.cfg"));
}

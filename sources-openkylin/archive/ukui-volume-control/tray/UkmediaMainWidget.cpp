/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "UkmediaMainWidget.h"

#include <QTableWidget>
#include <QGuiApplication>
#include <QScreen>
#include <kwindowsystem.h>
#include <kwindowsystem_export.h>
#include <QDebug>

#include "TrayClientMethod.h"
#include "DBusClient.h"
#include "XatomHelper.h"

UkmediaMainWidget::UkmediaMainWidget(QWidget* parent) : QWidget(parent)
{
    getDesktopEnvironmentVersion();

    initUi();

    initSlots();

    initClientManager();

    initData();

    setFocusPolicy(Qt::NoFocus);

    setProperty("useStyleWindowManager", false); // 禁用拖动
}

void UkmediaMainWidget::initUi()
{
    switch (m_version) {
    case DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI2:
        m_pMainWid = new Ukui2MainWindow(m_sysVolumeSliderItemMap, m_appVolumeSliderItemMap);
        m_pTrayIcon = std::make_shared<UkuiVersionTwoTrayIcon>(new UkmediaTrayIcon(), m_pMainWid);
        break;
    case DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI3:
    case DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI4:
        m_pMainWid = new Ukui2MainWindow(m_sysVolumeSliderItemMap, m_appVolumeSliderItemMap);
        m_pTrayIcon = std::make_shared<UkuiVersionTwoTrayIcon>(new UkmediaTrayIcon(), m_pMainWid);
//        m_pMainWid = new Ukui4MainWindow(m_sysVolumeSliderItemMap, m_appVolumeSliderItemMap, m_devicePortItemMap);
//        m_pTrayIcon = std::make_shared<UkuiVersion4TrayIcon>(new UkmediaTrayIcon(), m_pMainWid);
        break;
    case DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI5:

        break;
    default:
        break;
    }

    m_pTrayIcon->initTray();
    m_pTrayIcon->initSlot();
    m_pTrayIcon->setVisible(true);

    m_pMainWid->initUi();
    m_pMainWid->initSlots();
    m_pMainWid->setParentWidget(this);

    const KWindowInfo info(this->winId(), NET::WMState);
    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager)) {
        KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
    }

    removeHeader(this);
}

void UkmediaMainWidget::initSystemDeviceList()
{
    QDBusMessage replyList = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getAvailablePortList",
                                                                      EnumToInt(NodeType::OUTPUT));

    if (replyList.arguments().isEmpty()) {
        qDebug() << "返回参数为空";
        return;
    }

    qDebug() << " getAvailablePortList: " << replyList;
    const QDBusArgument& dbusArgs = replyList.arguments().at(0).value<QDBusArgument>();

    QList<ClientDeviceInfo> deviceList;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientDeviceInfo info;
        dbusArgs >> info;
        deviceList.push_back(info);
    }
    dbusArgs.endArray();

    for (const auto& v : deviceList) {
        TrayClientMethod::getInstance().addDeviceItem(EnumToInt(NodeType::OUTPUT), v);
    }
}

void UkmediaMainWidget::initCurrentDefaultDevice()
{
    ClientDefaultDeviceInfo outputInfo = getDefaultDevice(1, "");
    ClientDefaultDeviceInfo inputInfo = getDefaultDevice(0, "");

    int index = -1;
    for (const auto& it : m_devicePortItemMap) {
        if (outputInfo.cardName == it.second->getCardName()  && outputInfo.activePortName == it.second->getPortName()) {
            index = it.first;
        }
    }

    TrayClientMethod::getInstance().updateDescription(inputInfo.activePortDesc, outputInfo.activePortDesc);
    TrayClientMethod::getInstance().setCurrentIndex(index);
}

void UkmediaMainWidget::initAppVolumeList()
{
    QDBusMessage replyList = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getSinkInputList");

    if (replyList.arguments().isEmpty()) {
        qDebug() << "arguments is nullptr...";
        return;
    }

    const QDBusArgument& siArgs = replyList.arguments().at(0).value<QDBusArgument>();

    QList<StreamInfo> streamList;
    siArgs.beginArray();
    while (!siArgs.atEnd()) {
        StreamInfo info;
        siArgs >> info;
        streamList.push_back(info);
    }
    siArgs.endArray();


    replyList = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                         AUDIO_OBJECT_PATH,
                                                         AUDIO_INTERFACE,
                                                         "getSourceOutputList");

    if (replyList.arguments().isEmpty()) {
        qDebug() << "arguments is nullptr...";
        return;
    }

    const QDBusArgument& soArgs = replyList.arguments().at(0).value<QDBusArgument>();
    soArgs.beginArray();
    while (!siArgs.atEnd()) {
        StreamInfo info;
        soArgs >> info;
        streamList.push_back(info);
    }
    soArgs.endArray();

    for (const auto& s : streamList) {

        if ("filter" != s.role) {
            TrayClientMethod::getInstance().addAppItem(s.index, s.iconName, s.name);
        }
    }
}

void UkmediaMainWidget::initSystemVolumeSlider()
{
    for (const auto& [k, v] : m_sysVolumeSliderItemMap) {
        switch (IntToEnum<VolumeSliderType>(k)) {
        case VolumeSliderType::VOLUME_SLIDER_MINI_OUTPUT:
        case VolumeSliderType::VOLUME_SLIDER_DEVICE_OUTPUT:
        case VolumeSliderType::VOLUME_SLIDER_APP_OUTPUT: {
            QDBusReply<bool> muted = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                              AUDIO_OBJECT_PATH,
                                                                              AUDIO_INTERFACE,
                                                                              "getMute",
                                                                              EnumToInt(NodeType::OUTPUT), "");

            QDBusReply<int> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                             AUDIO_OBJECT_PATH,
                                                                             AUDIO_INTERFACE,
                                                                             "getVolume",
                                                                             EnumToInt(NodeType::OUTPUT), "");
            TrayClientMethod::getInstance().setValue(EnumToInt(NodeType::OUTPUT), k, reply);

            QString iconStr;
            if (muted) {
                iconStr = iconNameOutputs[0];
            }
            else if(reply <= 0){
                iconStr = iconNameOutputs[0];
            }
            else if (reply > 0 && reply <= 33) {
                iconStr = iconNameOutputs[1];
            }
            else if (reply >33 && reply <= 66) {
                iconStr = iconNameOutputs[2];
            }
            else {
                iconStr = iconNameOutputs[3];
            }

            m_pTrayIcon->setIcon(QIcon::fromTheme(iconStr));
            break;
        }
        case VolumeSliderType::VOLUME_SLIDER_DEVICE_INPUT: {

            bool muted = getMute(EnumToInt(NodeType::INPUT), "");
            int volume = getVolume(EnumToInt(NodeType::INPUT), "");


            TrayClientMethod::getInstance().setValue(EnumToInt(NodeType::INPUT), k, volume);
        }
        default:
            break;
        }
    }
}

void UkmediaMainWidget::initAppVolumeSlider()
{
    for (const auto& [k, v] : m_appVolumeSliderItemMap) {
        QDBusReply<int> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                         AUDIO_OBJECT_PATH,
                                                                         AUDIO_INTERFACE,
                                                                         "getVolume",
                                                                         EnumToInt(NodeType::SINK_INPUT), "");
        TrayClientMethod::getInstance().setValue(EnumToInt(NodeType::SINK_INPUT), k, reply);
    }

}

void UkmediaMainWidget::initData()
{
    if (DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI2 == m_version) {
        static_cast<Ukui2MainWindow*>(m_pMainWid)->setStackWidgetCurrentIndex(0);
    }
    else {
        m_pMainWid->setSize(QSize(420, 476));
    }

    initSystemDeviceList();
    initAppVolumeList();
    initCurrentDefaultDevice();
    initSystemVolumeSlider();
    initAppVolumeSlider();
}

void UkmediaMainWidget::initSlots()
{
    if (m_pMainWid) {
        m_pMainWid->addVolumeObserver([this](int type, int value) {
            setVolume(type, "", value);
        });

        m_pMainWid->addMuteObserver([this](int type, const QString& name) {
            setMute(type, name, !getMute(type, name));
        });

        m_pMainWid->addDeviceObserver([this](int type, const QString& portName, const QString& cardName) {
            setDefaultDevice(type, portName, cardName);
        });

        m_pMainWid->addTrayIconTriggerObserver([this]() {

            if (this->isHidden()) {
                setGeometry(caculatePosition());
                show();
            }
            else {
                hide();
            }
        });

        m_pMainWid->addSizeChangedObserver([this](const QSize& size) {
            resize(size);
            setGeometry(caculatePosition());
        });
    }

    connect(&DBusClient::getInstance(), SIGNAL(volumeChangedSignal(int, int, const QDBusVariant&)), this, SLOT(volumeChangedSlots(int, int, const QDBusVariant&)));
    connect(&DBusClient::getInstance(), SIGNAL(deviceChangedSignal(int, const QString&, const QString&)), this, SLOT(deviceChangedSlots(int, const QString&, const QString&)));
    connect(&DBusClient::getInstance(), SIGNAL(deviceAdjustSignal(int)), this, SLOT(deviceAdjustSlots(int)));
    connect(&DBusClient::getInstance(), SIGNAL(settingsChangedSignal(const QString&, const QDBusVariant&)), this, SLOT(settingsChangedSlots(const QString&, const QDBusVariant&)));
    connect(&DBusClient::getInstance(), SIGNAL(addStreamSignal(int, int, const QString&, const QString&)), this, SLOT(addStreamSlots(int, int, const QString&, const QString&)));
    connect(&DBusClient::getInstance(), SIGNAL(removeStreamSignal(int)), this, SLOT(removeStreamSlots(int)));
}

void UkmediaMainWidget::initClientManager()
{
    m_pClientManager = std::make_shared<TrayClientManager>(m_sysVolumeSliderItemMap, m_appVolumeSliderItemMap, m_pMainWid);
    DBusClient::getInstance().initDbusConnect();
    TrayClientMethod::getInstance().setManager(m_pClientManager);
}

QRect UkmediaMainWidget::caculatePosition()
{
    QRect availableGeo = QGuiApplication::screenAt(QCursor::pos())->geometry();
    int x, y;
    int margin = 8;

    int panelType = 0; // PanelSettings::getInstance().getPanelType();
    // 经典任务栏
    if (0 == panelType) {
        switch (m_panelPosition) {
        case PanelPosition::PANEL_POSITION_TOP: {
            x = availableGeo.x() + availableGeo.width() - this->width() - margin;
            y = availableGeo.y() + m_panelSize + margin;
            break;
        }
        case PanelPosition::PANEL_POSITION_BOTTOM: {
            x = availableGeo.x() + availableGeo.width() - this->width() - margin;
            y = availableGeo.y() + availableGeo.height() - m_panelSize - this->height() - margin;
            break;
        }
        case PanelPosition::PANEL_POSITION_LEFT: {
            x = availableGeo.x() + m_panelSize + margin;
            y = availableGeo.y() + availableGeo.height() - this->height() - margin;
            break;
        }
        case PanelPosition::PANEL_POSITION_RIGHT: {
            x = availableGeo.x() + availableGeo.width() - m_panelSize - this->width() - margin;
            y = availableGeo.y() + availableGeo.height() - this->height() - margin;
            break;
        }
        default:
            break;
        }
    }
    else if (1 == panelType) {

    }

    return QRect(x, y, this->width(), this->height());
}

void UkmediaMainWidget::removeHeader(QWidget* widget)
{
    if(!widget)
        return;

    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
        installEventFilter(this);
    }
    else {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(widget->winId(), hints);
    }
}


void UkmediaMainWidget::volumeChangedSlots(int type, int idx, const QDBusVariant& v)
{
    TrayClientMethod::getInstance().setValue(type, idx, v.variant().toInt());
}

void UkmediaMainWidget::deviceChangedSlots(int type, const QString& portName, const QString& cardName)
{
    switch (IntToEnum<NodeType>(type)) {
    case NodeType::OUTPUT: {
        int index = -1;
        for (const auto& it : m_devicePortItemMap) {
            qDebug() << "UkmediaMainWidget::deviceChangedSlot, type: " << type
                     << " cardName:" << cardName << " it.second->getCardName(): " << it.second->getCardName() << " \n"
                     << " portName: " << portName << " it.second->getPortName(): " << it.second->getPortName();
            if (cardName == it.second->getCardName()  && portName == it.second->getPortName()) {
                index = it.first;
                break;
            }
        }
        qDebug() << "setcurrent index: " << index;
        TrayClientMethod::getInstance().setCurrentIndex(index);
        break;
    }
    case NodeType::INPUT:
    case NodeType::SINK_INPUT:
    case NodeType::SOURCE_OUTPUT:
    default:
        break;
    }
}

void UkmediaMainWidget::deviceAdjustSlots(int type)
{
    switch (IntToEnum<NodeType>(type)) {
    case NodeType::OUTPUT: {
        // 暂时先删除在添加
//        for (const auto& it : m_devicePortItemMap) {
//            TrayClientMethod::getInstance().removeDeviceItem(it.second->getPortName(), it.second->getCardName());
//        }
//        for (auto it = m_devicePortItemMap.begin(); it != m_devicePortItemMap.end(); ) {
//            it = TrayClientMethod::getInstance().removeDeviceItem(it->second->getPortName(), it->second->getCardName());
//        }

        TrayClientMethod::getInstance().removeDeviceItem("", "");
        initSystemDeviceList();

        break;
    }
    case NodeType::INPUT:
    case NodeType::SINK_INPUT:
    case NodeType::SOURCE_OUTPUT:
    default:
        break;
    }
}

void UkmediaMainWidget::settingsChangedSlots(const QString& key, const QDBusVariant& v)
{
    if (EVENT_SOUND_KEY == key) {
//        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_ALERT), v.variant().toBool());
    }
    else if (THEME_NAME_KEY == key) {

    }
    else if (CUSTOM_THEME_KEY == key) {

    }
    else if (MONO_AUDIO_KEY == key) {
//        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_MONO), v.variant().toBool());
        DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                 AUDIO_OBJECT_PATH,
                                                 AUDIO_INTERFACE,
                                                 "LoadModule",
                                                 "module-mono", "");
    }
    else if (NOISE_REDUCTION_KEY == key) {
//        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL), v.variant().toBool());
    }
    else if (LOOPBACK_KEY == key) {
//        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_LOOPBACK), v.variant().toBool());
    }
    else if (VOLUME_BOOST_KEY == key) {
//        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST), v.variant().toBool());
    }
    else if (VOLUME_BOOST_VOLUME_KEY == key) {

    }
    else if (STARTUP_MUSIC_KEY == key) {
//        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_STARTUP), v.variant().toBool());
    }
    else if (NOTIFY_GENERAL_KEY == key) {

    }
    else if (VOLUME_CHANGED_KEY == key) {

    }
    else if (ALERT_VOLUME_KEY == key) {

    }
}

void UkmediaMainWidget::addStreamSlots(int idx, int value, const QString& iconName, const QString& descName)
{
    TrayClientMethod::getInstance().addAppItem(idx, iconName, descName);
}

void UkmediaMainWidget::removeStreamSlots(int idx)
{
    TrayClientMethod::getInstance().removeAppItem(idx);
}

DesktopEnvironmentVersion UkmediaMainWidget::getDesktopEnvironmentVersion()
{
    QDBusReply<int> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                     AUDIO_OBJECT_PATH,
                                                                     AUDIO_INTERFACE,
                                                                     "getDesktopEnvironmentVersion");

    return m_version = IntToEnum<DesktopEnvironmentVersion>(reply);
}

int UkmediaMainWidget::getVolume(int type, const QString& name)
{
    QDBusReply<int> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                     AUDIO_OBJECT_PATH,
                                                                     AUDIO_INTERFACE,
                                                                     "getVolume",
                                                                     type, name);

    return reply;
}

void UkmediaMainWidget::setVolume(int type, const QString& name, int vaule)
{
    qDebug() << "UkmediaMainWidget::setVolume"
             << " type: " << type << " vaule: " << vaule;

    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setVolume",
                                             type, name, vaule);
}

bool UkmediaMainWidget::getMute(int type, const QString& name)
{
    QDBusReply<bool> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getMute",
                                                                      type, name);

    return reply;
}

void UkmediaMainWidget::setMute(int type, const QString& name, bool muted)
{
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setMute",
                                             type, name, muted);
}

ClientDefaultDeviceInfo UkmediaMainWidget::getDefaultDevice(int type, const QString& name) const
{
    QDBusMessage replyList = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getDefaultDevice",
                                                                      type, name);

    if (replyList.arguments().isEmpty()) {
        qDebug() << "arguments is nullptr...";
        return {};
    }

    const QDBusArgument& siArgs = replyList.arguments().at(0).value<QDBusArgument>();

    ClientDefaultDeviceInfo info;
    siArgs >> info;

    return info;
}

void UkmediaMainWidget::setDefaultDevice(int type, const QString& portName, const QString& cardName)
{
    switch (IntToEnum<NodeType>(type)) {
    case NodeType::OUTPUT:

        break;
    case NodeType::INPUT:
    case NodeType::SINK_INPUT:
    case NodeType::SOURCE_OUTPUT:
    default:
        qDebug() << type << " type is not expect";
        return ;
    }

    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setDefaultDevice",
                                             type, portName, cardName);
}

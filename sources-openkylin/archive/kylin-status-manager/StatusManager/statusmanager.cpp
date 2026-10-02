/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "statusmanager.h"

#include <QSettings>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QJsonObject>
#include <QJsonArray>

#include <kysdk/kysdk-system/libkysysinfo.h>
#include <ukui/ukuiinputgatherclient.h>

#include "keyboardmanager/keyboardmanager.h"
#include "KmreAppManager/kmremanager.h"
#include "DeviceModeManager/devicemodemanager.h"
#include "devicerotationmanager.h"
#include "ConfigurationManager/configurationmanager.h"

StatusManager::StatusManager(QObject *parent)
    : QObject(parent)
    , m_keyboardManager(new KeyboardManager(this))
    , m_kmreManager(new KmreManager())
    , m_deviceModeManager(new DeviceModeManager(this))
    , m_deviceRotationManager(new DeviceRotationManager())
    , m_inputGatherClient(new UKUIInputGatherClient())
    , m_configurationManager(ConfigurationManager::GetInstance())
{
    initDeviceModeManager();
    connectSignals();
    initMachineType();
    initInputGatherProxy();
    initKeyboardAvailable();
    restoreDeviceStatus();
}

void StatusManager::initDeviceModeManager()
{
    m_deviceModeManager->setShouldShowNotificationCallback(
                [this]() {
        return m_kmreManager->kmreWindowExists();
    }
    );

    m_deviceModeManager->setShowNotificationCallback(
                [this]() {
        QString notificationText = m_requestToSwitchModeToTablet ? "Tablet" : "PC";
        m_kmreManager->showModeSwitchPromptWindow(notificationText);
    }
    );

    m_deviceModeManager->setCheckFirstDeviceModeSwitchFunction(
                [this]() {
        if (!isOpenkylinSystem() || !m_configurationManager->isFirstSwitchDeviceMode()) {
            return false;
        }
        m_configurationManager->setFirstSwitchDeviceModeToOff();
        return true;
    }
    );
}

void StatusManager::restoreDeviceStatus()
{
    restoreAutoSwitchStatus();
    restoreDeviceMode();
    restoreAutoRotateStatus();
    restoreDeviceOrientation();
}

void StatusManager::notifyStartupFinished()
{
    QtConcurrent::run(this, &StatusManager::notifySessionStartupFinished);
}

void StatusManager::setTabletModeEnabled(bool enabled)
{
    m_deviceModeManager->setTabletModeEnabled(enabled);
}

bool StatusManager::isTabletModeEnabled() const
{
    return m_deviceModeManager->isTabletModeEnabled();
}

bool StatusManager::isClassicalMode() const
{
    return m_deviceModeManager->isClassicalMode();
}

void StatusManager::setClassicalMode(bool classicalMode)
{
    m_deviceModeManager->setClassicalMode(classicalMode);
}

bool StatusManager::isImmersiveMode() const
{
    return m_deviceModeManager->isImmersiveMode();
}

void StatusManager::setImmersiveMode(bool immersiveMode)
{
    m_deviceModeManager->setImmersiveMode(immersiveMode);
}

QMap<QString, QVariant> StatusManager::getLiteMode() const
{
    qDebug()<<m_deviceModeManager->getLiteMode();
    return m_deviceModeManager->getLiteMode();
}

void StatusManager::notifySessionStartupFinished()
{
    QDBusInterface sessioninterface("org.gnome.SessionManager",
                                    "/org/gnome/SessionManager",
                                    "org.gnome.SessionManager",
                                    QDBusConnection::sessionBus());
    if (!sessioninterface.isValid()) {
        qDebug() << "Call to session DBUS interface failed!!!";
        return;
    }

    //调用session的接口，通知管理服务已经启动
    QDBusReply<QString> reply = sessioninterface.call("startupfinished",
                                                      "kylin-status-manager",
                                                      "startupfinished");
}

void StatusManager::connectSignals()
{
    connectKmreManagerSignals();
    connectDeviceModeManagerSignals();
    connectKeyboardSignals();
    connectDeviceRotateManagerSignals();
}

void StatusManager::connectKmreManagerSignals()
{
    connect(m_kmreManager, &KmreManager::kmreAppEnableAutoRotate,
            this, &StatusManager::setAutoRotateEnabled);
    connect(m_kmreManager, &KmreManager::kmreAppRestoreAutoRotateStatus,
            this, &StatusManager::restoreAutoRotateStatus);

    connect(m_kmreManager, &KmreManager::canSwitchMode,
            this, &StatusManager::handleKmreOperationSignal);
}

void StatusManager::connectDeviceModeManagerSignals()
{
    /*由设备模式确认此值是否需要保存，需要保存则发送信号由status中转给configmanager*/
    connect(m_deviceModeManager, &DeviceModeManager::autoSwitchEnabledChanged,
            m_configurationManager, &ConfigurationManager::setAutoSwitchEnabled);

    connect(m_deviceModeManager, &DeviceModeManager::tabletModeChanged,
            m_configurationManager, &ConfigurationManager::setTabletMode);

    connect(m_deviceModeManager, &DeviceModeManager::isTabletModeEnabledChanged,
            m_configurationManager, &ConfigurationManager::setTabletModeEnabled);

    connect(m_deviceModeManager, &DeviceModeManager::isClassicalModeChanged,
            m_configurationManager, &ConfigurationManager::setClassicalMode);

    connect(m_deviceModeManager, &DeviceModeManager::isImmersiveModeChanged,
            m_configurationManager, &ConfigurationManager::setImmersiveMode);

    /*转发信号给dbusmanager，发送dbus信号*/
    connect(m_deviceModeManager, &DeviceModeManager::isClassicalModeChanged,
            this, &StatusManager::isClassicalModeChanged);

    connect(m_deviceModeManager, &DeviceModeManager::isImmersiveModeChanged,
            this, &StatusManager::isImmersiveModeChanged);

    connect(m_deviceModeManager, &DeviceModeManager::tabletModeChanged,
            this, &StatusManager::tabletModeChanged);

    connect(m_deviceModeManager, &DeviceModeManager::autoSwitchEnabledChanged,
            this, &StatusManager::autoSwitchEnabledChanged);

    connect(m_deviceModeManager, &DeviceModeManager::isTabletModeEnabledChanged,
            this, &StatusManager::isTabletModeEnabledChanged);

    connect(m_deviceModeManager, &DeviceModeManager::tabletModeChanged,
            this, &StatusManager::restoreAutoRotateStatus);

    connect(m_deviceModeManager, &DeviceModeManager::tabletModeChanged, this, [this](){
        if (!isAutoRotateEnabled()) {
            /*
                此处的1100是因为模式切换的时候，需要播放切换动画，如果立刻旋转屏幕方向的话，动画就播放混乱了
                两者之间有冲突，动画播放时间是1000ms，为了稳妥所以设置时间为1100；
            */
            QTimer::singleShot(1100, this, &StatusManager::restoreDeviceOrientation);
        }
    });
}

void StatusManager::connectKeyboardSignals()
{
    connect(m_keyboardManager, &KeyboardManager::keyboardAvailableChanged,
            this, &StatusManager::keyboardAvailableChanged);
}

void StatusManager::connectDeviceRotateManagerSignals()
{
    connect(m_deviceRotationManager, &DeviceRotationManager::autoRotateEnabledChange,
            this, &StatusManager::autoRotateEnabledChange);
    connect(m_deviceRotationManager, &DeviceRotationManager::orientationChanged,
            this, &StatusManager::orientationChanged);
}

bool StatusManager::localizedAutoRotateEnabled() const
{
    return m_configurationManager->isAutoRotateEnabled();
}

bool StatusManager::localizedTabletMode() const
{
    return m_configurationManager->isTabletMode();
}

bool StatusManager::localizedTabletModeEnabled() const
{
    return m_configurationManager->isTabletModeEnabled();
}

bool StatusManager::localizedAutoSwitchEnabled() const
{
    return m_configurationManager->isAutoSwitchEnabled();
}

QMap<QString, QVariant> StatusManager::localizedLiteMode() const
{
    return m_configurationManager->liteMode();
}

bool StatusManager::localizedClassicalMode() const
{
    return m_configurationManager->isClassicalMode();
}

bool StatusManager::localizedImmersiveMode() const
{
    return m_configurationManager->isImmersiveMode();
}

QString StatusManager::localizedDeviceOrientation() const
{
    return m_configurationManager->getDeviceOrientation();
}

void StatusManager::restoreDeviceMode()
{
    qDebug()<<__func__<<".............";
    m_deviceModeManager->setTabletModeEnabled(localizedTabletModeEnabled());
    m_deviceModeManager->setTabletMode(getDeviceModeStatus());
    m_deviceModeManager->setLiteMode(localizedLiteMode());
    m_deviceModeManager->setClassicalMode(localizedClassicalMode());
    m_deviceModeManager->setImmersiveMode(localizedImmersiveMode());
}

bool StatusManager::getDeviceModeStatus()
{
    if (m_isPhytiumDevice || m_isMaxTabletDevice
            || !isAutoSwitchEnabled()) {
        return localizedTabletMode();
    }
    return m_inputGatherClient->libinputTabletSwitchState() == 1;
}

void StatusManager::restoreAutoSwitchStatus()
{
    setAutoSwitchEnabled(localizedAutoSwitchEnabled());
}

void StatusManager::restoreAutoRotateStatus()
{
    bool autoRotateEnable = isTabletMode() ? localizedAutoRotateEnabled() : false;
    setAutoRotateEnabled(autoRotateEnable, false);
}

void StatusManager::restoreDeviceOrientation()
{
    if (isTabletMode()) {
        auto deviceDirection = isAutoRotateEnabled() ? getDeviceOrientation() : localizedDeviceOrientation();
        setDeviceOrientationPermanent(deviceDirection);
    } else {
        m_deviceRotationManager->setDeviceOrientation("normal");
    }
}

bool StatusManager::isOrientationSensorAvailable() const
{
    return m_deviceRotationManager->isOrientationSensorAvailable();
}

bool StatusManager::isAutoRotateEnabled() const
{
    return m_deviceRotationManager->isAutoRotateEnabled();
}

bool StatusManager::setAutoRotateEnabled(bool autoRotateEnable, bool isPersistence)
{
    if (!isOrientationSensorAvailable()) {
        return false;
    }

    if (isPersistence) {
        m_configurationManager->setAutoRotateEnabled(autoRotateEnable);
    }
    return m_deviceRotationManager->setAutoRotateEnabled(autoRotateEnable);
}

QString StatusManager::getDeviceOrientation() const
{
    return m_deviceRotationManager->getDeviceOrientation();
}

bool StatusManager::setDeviceOrientationPermanent(const QString &orientation)
{
    return m_deviceRotationManager->setDeviceOrientationPermanent(orientation);
}

bool StatusManager::isTabletMode() const
{
    return m_deviceModeManager->isTabletMode();
}

bool StatusManager::isAutoSwitchEnabled() const
{
    return m_deviceModeManager->isAutoSwitchEnabled();
}

bool StatusManager::setAutoSwitchEnabled(bool autoSwitchEnabled)
{
    return m_deviceModeManager->setAutoSwitchEnabled(autoSwitchEnabled);
}

bool StatusManager::setTabletMode(bool tabletMode)
{
    if (!isTabletModeEnabled()) {
        return false;
    }

    m_requestToSwitchModeToTablet = tabletMode;
    m_deviceModeManager->requestToSwitchMode(tabletMode);
    return true;
}

void StatusManager::handleKmreOperationSignal(bool allowModeSwitching)
{
    if (allowModeSwitching) {
        m_deviceModeManager->setTabletMode(m_requestToSwitchModeToTablet);
    }
}

void StatusManager::handleSwitchStateChanged(bool tabletMode)
{
    if (!m_deviceModeManager->isAutoSwitchEnabled()) {
        return;
    }
    setTabletMode(tabletMode);
}

bool StatusManager::isKeyboardAvailable() const
{
    return m_keyboardManager->isKeyboardAvailable();
}

void StatusManager::initInputGatherProxy()
{
    m_inputGatherClient->setEventCallBack(
                [this](Event *e) {
        monitorInputGatherTabletSwitchState(e);
    });
    m_inputGatherClient->startToReceiveEvent();
}

void StatusManager::monitorInputGatherTabletSwitchState(Event* e)
{
    switch (e->type) {
    case LIBINPUT_EVENT_SWITCH_TOGGLE:
        if (e->event.switchEventDate.switchType == LIBINPUT_SWITCH_TABLET_MODE) {
            const bool tabletMode =
                    e->event.switchEventDate.switchState == LIBINPUT_SWITCH_STATE_ON;
            handleSwitchStateChanged(tabletMode);

            if (tabletMode) {
                m_keyboardManager->setKeyboardDisabled();
            } else {
                m_keyboardManager->setKeyboardEnabled();
            }
        }
        break;
    default:
        break;
    }
}

void StatusManager::initKeyboardAvailable()
{
    bool isSpecialDevice = m_isPhytiumDevice || m_isMaxTabletDevice;
    const bool isLibinputTabletState = m_inputGatherClient->libinputTabletSwitchState() != 1;
    const bool keyboardAvailable =  isSpecialDevice ? false : isLibinputTabletState;
    m_keyboardManager->setKeyboardAvailable(keyboardAvailable);
}

void StatusManager::initMachineType()
{
    m_isPhytiumDevice = isPhytunumDevice();
    m_isMaxTabletDevice = isMaxTabletDevice();
}

bool StatusManager::isMaxTabletDevice() const
{
    char* systemCategory = kdk_system_get_systemCategory();
    QString machineType = systemCategory;
    free(systemCategory);
    qDebug() << __func__ << machineType;
    if (machineType == "MaxTablet") {
        return true;
    }
    return false;
}

bool StatusManager::isPhytunumDevice() const
{
    QSettings setting(":/data/conf.ini", QSettings::IniFormat);
    setting.beginGroup("MachineType");//节点开始

    const QString type = getSysVendor() + getProductFamily();
    bool isPhytiumDevice =
            setting.contains(type) && setting.value(type).toString() == "SLATE";

    setting.endGroup();//节点结束
    return isPhytiumDevice;
}

QString StatusManager::getDeviceInfo(const QString &property) const
{
    QString fileName {"/sys/class/dmi/id/" + property};
    QFile file {fileName};
    file.open(QIODevice::ReadOnly);
    QString info = file.readLine().trimmed();
    return info;
}

QString StatusManager::getSysVendor() const
{
    return getDeviceInfo("sys_vendor");
}

QString StatusManager::getProductName() const
{
    return getDeviceInfo("product_name");
}

QString StatusManager::getProductFamily() const
{
    return getDeviceInfo("product_family");
}

bool StatusManager::isOpenkylinSystem() const
{
    bool flag = false;
    const char* systemName = "openKylin";
    char *sdkSystemName = kdk_system_get_systemName();
    if (sdkSystemName) {
        flag = strcmp(sdkSystemName, systemName) == 0;
    }
    free(sdkSystemName);
    sdkSystemName = nullptr;
    return flag;
}

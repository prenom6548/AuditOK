/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "devicestate.h"
#include "loghelper.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>
#include <cstring>
#include <iostream>

namespace
{

const char *upower_service = "org.freedesktop.UPower";
const char *upower_path = "/org/freedesktop/UPower";
const char *upower_path_properties_interface = "org.freedesktop.DBus.Properties";
const char *upower_display_path="/org/freedesktop/UPower/devices/DisplayDevice";
const char *properties_changed_signal = "PropertiesChanged";
const char *on_battery_property = "OnBattery";

const char *schema_id = "org.ukui.power-manager";
const char *ac_key = "powerPolicyAc";
const char *battery_key = "powerPolicyBattery";

const char *status_manager_service = "com.kylin.statusmanager.interface";
const char *status_manager_path = "/";
const char *status_manager_interface = "com.kylin.statusmanager.interface";

const char *power_manager_interface = "ukui.power.manager";

}

PowerManager::PowerManager(const std::shared_ptr<StateManager> &stateManager)
    : m_stateManager(stateManager)
{
    initGSettings();
    initPowerMode();
}

PowerManager::~PowerManager()
{
    PRINT_NOTICE("~PowerManager\n");
    stopPowerWatcher();
}

sched_policy::PowerType PowerManager::lastPowerType() const
{
    return m_lastPowerType;
}

sched_policy::PowerType PowerManager::currentPowerType() const
{
    return m_currentPowerType;
}

sched_policy::PowerMode PowerManager::lastPowerMode() const
{
    return m_lastPowerMode;
}

sched_policy::PowerMode PowerManager::currentPowerMode() const
{
    return m_currentPowerMode;
}

void PowerManager::changePowerMode(Policy policy)
{
    m_gsettings->set(battery_key, (int)policy);
}

Policy PowerManager::getPolicyWithPowerMode(sched_policy::PowerMode mode) 
{
    Policy policy = Policy::Unknown;
    switch(mode)
    {
    case sched_policy::PowerMode::Performance:
        policy = Policy::Performance;
        break;

    case sched_policy::PowerMode::Balance:
        policy = Policy::Balance;
        break;

    case sched_policy::PowerMode::Save:
        policy = Policy::EnergySaving;
        break;
    
    default:
        break;
    }
    return policy;
}

int PowerManager::getCurrPowerLevel()
{
    int battery_percentage = getPowerPercentage();
    return getPowerLevel(battery_percentage);
}

int PowerManager::getPowerLevel(int battery_percentage)
{
    int current_power_level = 0;
    if(battery_percentage == 0) {
        //未获取到信息，返回0
    } else if((battery_percentage > 0) && (battery_percentage <= 20)) {
        current_power_level = 1;
    } else if((battery_percentage > 20) && (battery_percentage <= 50)) {
        current_power_level = 2;
    } else if((battery_percentage > 50) && (battery_percentage <= 80)) {
        current_power_level = 3;
    } else if ((battery_percentage > 80) && (battery_percentage <= 100)) {
        current_power_level = 4;
    }
    return current_power_level;
}

int PowerManager::getPowerPercentage()
{
    QDBusInterface upowerInterface(
        upower_service, upower_display_path,
        upower_path_properties_interface, QDBusConnection::systemBus());
    
    QDBusReply<QVariant> reply = upowerInterface.call("Get", "org.freedesktop.UPower.Device", "Percentage");
    if (!reply.isValid()) {
        //qWarning() << "Get pwoer mode failed, " << reply.error();
        PRINT_NOTICE("Get pwoer mode failed, %s\n", reply.error().message().toStdString().c_str());
        return 100;
    }
    else{
        int batteryPercentage = reply.value().toDouble();
        if (batteryPercentage == 0){
            batteryPercentage = 100;
        }
        return batteryPercentage;
    }
}

void PowerManager::startPowerWatcher()
{
    initPowerMode();
    initConnections();
}

void PowerManager::stopPowerWatcher()
{
    disconnect(this);
}

void PowerManager::onPropertiesChanged(QString name, QMap<QString, QVariant> value, QStringList)
{
    if (value.contains(on_battery_property)) {
        if (value.value(on_battery_property).toBool()) {
            handlePowerTypeChanged(sched_policy::PowerType::Battery);
            handlePowerModeChanged(m_gsettings->get(battery_key).toInt());
        } else {
            handlePowerTypeChanged(sched_policy::PowerType::Ac);
            handlePowerModeChanged(m_gsettings->get(ac_key).toInt());
        }
    }
}

void PowerManager::initGSettings()
{
    if (!QGSettings::isSchemaInstalled(schema_id)) {
        //qWarning() << schema_id << "not installed.";
        PRINT_NOTICE("%s not installed.\n", schema_id);
        return;
    }
    m_gsettings.reset(new QGSettings(schema_id));
}

void PowerManager::initConnections()
{
    connect(m_gsettings.get(), &QGSettings::changed, this, [this](const QString &key) {
        if ((key == ac_key && m_currentPowerType == sched_policy::PowerType::Ac) || (key == battery_key && m_currentPowerType == sched_policy::PowerType::Battery)) {
            handlePowerModeChanged(m_gsettings->get(key).toInt());
        }
    });
    QDBusConnection::systemBus().connect(
        upower_service, upower_path,
        upower_path_properties_interface,
        properties_changed_signal,
        this, SLOT(onPropertiesChanged(QString, QMap<QString, QVariant>, QStringList)));
}

void PowerManager::initPowerMode()
{
    QDBusInterface upowerInterface(
        upower_service, upower_path,
        upower_path_properties_interface, QDBusConnection::systemBus());

    QDBusReply<QDBusVariant> reply = upowerInterface.call("Get", upower_service, on_battery_property);
    if (!reply.isValid()) {
        //qWarning() << "Get pwoer mode failed, " << reply.error();
        PRINT_NOTICE("Get pwoer mode failed, %s\n", reply.error().message().toStdString().c_str());
        return;
    }

    // OnBattery
    if (reply.value().variant().toBool()) {
        //qDebug() << "init pwoer mode: onBattery";
        PRINT_DBG("init pwoer mode: onBattery\n");
        handlePowerTypeChanged(sched_policy::PowerType::Battery);
        if (m_gsettings) {
            handlePowerModeChanged(m_gsettings->get(battery_key).toInt());
        }
    } else {
        //qDebug() << "init pwoer mode: Ac";
        PRINT_DBG("init pwoer mode: Ac\n");
        handlePowerTypeChanged(sched_policy::PowerType::Ac);
        if (m_gsettings) {
            handlePowerModeChanged(m_gsettings->get(ac_key).toInt());
        }
    }
}

void PowerManager::handlePowerTypeChanged(sched_policy::PowerType powerType)
{
    std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::PowerType));
    m_lastPowerType = m_currentPowerType;
    m_currentPowerType = powerType;

    m_stateManager->enableTarget(StateTarget::PowerType);
}

void PowerManager::handlePowerModeChanged(int value)
{
    std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::PowerMode));
    m_lastPowerMode = m_currentPowerMode;
    switch (value) {
    case 0:
        m_currentPowerMode = sched_policy::PowerMode::Performance;
        break;

    case 1:
        m_currentPowerMode = sched_policy::PowerMode::Balance;
        break;

    case 2:
        m_currentPowerMode = sched_policy::PowerMode::Save;
        break;

    default:
        m_currentPowerMode = sched_policy::PowerMode::Unknown;
        //qWarning() << QString("Power mode changed to unkwon mode with the value: %1.").arg(value);
        PRINT_NOTICE("Power mode changed to unkwon mode with the value: %d.\n", value);
        return;
    }
    PRINT_DBG("power mode changed:  %d\n", value);
    m_stateManager->enableTarget(StateTarget::PowerMode);
}

DeviceModeManager::DeviceModeManager(bool softFreezeEnabled, const std::shared_ptr<StateManager> &stateManager)
    : m_softFreezeModeEnabled(softFreezeEnabled)
    , kLoadScreenoffConfigTimerInterval(500)
    , m_stateManager(stateManager)
    , m_deviceModeInfo(new DeviceModeInfo)
    , m_currentDeviceMode(sched_policy::DeviceMode::Unknown)
    , m_realDeviceMode(m_currentDeviceMode)
{
    fetchTabletModeStatus();
    updateDeviceMode();
    initScreenoffConfigTimer();
}

DeviceModeManager::~DeviceModeManager()
{
    stopDeviceModeWatcher();
    m_screenOffConfigTimer.stop();
    QObject::disconnect(m_screenOffConn);
    m_screenOffConn = {};
}

sched_policy::DeviceMode DeviceModeManager::currentDeviceMode() const
{
    return m_currentDeviceMode;
}

sched_policy::DeviceMode DeviceModeManager::realDeviceMode() const
{
    return m_realDeviceMode;
}

sched_policy::DeviceMode DeviceModeManager::lastDeviceMode() const
{
    return m_lastDeviceMode;
}

sched_policy::DPMSMode DeviceModeManager::currDPMSMode() const
{
    return m_currDPMSMode;
}

sched_policy::DPMSMode DeviceModeManager::lastDPMSMode() const
{
    return m_lastDPMSMode;
}

void DeviceModeManager::startDeviceModeWatcher()
{
    updateDeviceMode();
    QDBusConnection::sessionBus().connect(
        status_manager_service, status_manager_path,
        status_manager_interface, "modeChangeSignal",
        this, SLOT(updateTabletMode(bool)));
}

void DeviceModeManager::stopDeviceModeWatcher()
{
    QDBusConnection::sessionBus().disconnect(
        status_manager_service, status_manager_path,
        status_manager_interface, "modeChangeSignal",
        this, SLOT(updateTabletMode(bool)));
}

void DeviceModeManager::startDPMSModeWatcher()
{
    {
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::DPMSMode));
        m_currDPMSMode = m_deviceModeInfo->currentDPMSModeType();
        m_lastDPMSMode = m_currDPMSMode;
        m_stateManager->enableTarget(StateTarget::DPMSMode);
    }
    QDBusConnection::sessionBus().connect(
        "", "", power_manager_interface,
        "TurnOffDisplay", this, SLOT(onDPMSModeChanged(bool)));
}

void DeviceModeManager::stopDPMSModeWatcher()
{
    QDBusConnection::sessionBus().disconnect(
        "", "", power_manager_interface,
        "TurnOffDisplay", this, SLOT(onDPMSModeChanged(bool)));
}

void DeviceModeManager::handleSoftFreezeModeEnabledChanged(bool enabled)
{
    m_softFreezeModeEnabled = enabled;
    updateDeviceMode();
    fetchTabletModeStatus();
}

void DeviceModeManager::updateTabletMode(bool isTabletMode)
{
    m_isTabletMode = isTabletMode;
    updateDeviceMode();
}

void DeviceModeManager::onDPMSModeChanged(bool state)
{
    m_screenOffConfigTimer.start();
}

void DeviceModeManager::updateDeviceMode()
{
    auto tmpDeviceMode = sched_policy::DeviceMode::PC;

    if (m_isTabletMode.value_or(false)) {
        tmpDeviceMode = sched_policy::DeviceMode::Tablet;
    } else if (m_softFreezeModeEnabled) {
        tmpDeviceMode = sched_policy::DeviceMode::SoftFreeze;
    } else {
        tmpDeviceMode = sched_policy::DeviceMode::PC;
    }

    if (tmpDeviceMode != m_currentDeviceMode){
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::DeviceMode));
        m_lastDeviceMode = m_currentDeviceMode;
        m_currentDeviceMode = tmpDeviceMode;
        if (tmpDeviceMode != sched_policy::DeviceMode::SoftFreeze) {
            m_realDeviceMode = m_currentDeviceMode;
        }
        m_stateManager->enableTarget(StateTarget::DeviceMode);
    }
}

void DeviceModeManager::initScreenoffConfigTimer()
{
    m_screenOffConfigTimer.setSingleShot(true);
    m_screenOffConfigTimer.setInterval(kLoadScreenoffConfigTimerInterval);
    m_screenOffConn = QObject::connect(&m_screenOffConfigTimer, &QTimer::timeout, qApp, [this] {
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::DPMSMode));
        auto DPMSMode = m_deviceModeInfo->currentDPMSModeType();
        if (DPMSMode != m_currDPMSMode){
            m_lastDPMSMode = m_currDPMSMode;
            m_currDPMSMode = DPMSMode;
        }
    
        m_stateManager->enableTarget(StateTarget::DPMSMode);
    });
}

void DeviceModeManager::fetchTabletModeStatus()
{
    if (m_isTabletMode.has_value()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(status_manager_service, status_manager_path, status_manager_interface, "get_current_tabletmode");
    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher]() {
        QDBusPendingReply<bool> reply = *watcher;
        if (reply.isError()) {
            PRINT_NOTICE("Failed to get tablet mode: %s\n", reply.error().message().toStdString().c_str());
        } else {
            m_isTabletMode = reply.value();
            updateDeviceMode();
        }
        watcher->deleteLater();
    });
}


void DeviceModeInfo::registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    //qDebug() << "remove name:" << name;
    PRINT_DBG("remove name: %d\n", name);
}

void dpms_handle_done(void *data, struct org_kde_kwin_dpms *dpms)
{
    // qDebug() << "DPMS done";
    PRINT_DBG("DPMS done\n");
}
void dpms_handle_supported(void *data, struct org_kde_kwin_dpms *dpms, uint32_t support)
{
    // qDebug() << "DPMS supported" << support;
    PRINT_DBG("DPMS supported %d\n", support);
}

void dpms_handle_mode(void *data, struct org_kde_kwin_dpms *dpms, uint32_t state)
{
    sched_policy::DPMSMode *currentDPMSMode = static_cast<sched_policy::DPMSMode *>(data);
    switch (state) {
    case ORG_KDE_KWIN_DPMS_MODE_ON:
        // qDebug() << "DPMS mode ON";
        PRINT_DBG("DPMS mode ON\n");
        *currentDPMSMode = sched_policy::DPMSMode::DPMSOn;
        break;
    case ORG_KDE_KWIN_DPMS_MODE_STANDBY:
        //qDebug() << "DPMS mode standby";
        PRINT_DBG("DPMS mode standby\n");
        *currentDPMSMode = sched_policy::DPMSMode::DPMSStandby;
        break;
    case ORG_KDE_KWIN_DPMS_MODE_SUSPEND:
        // qDebug() << "DPMS mode suspend";
        PRINT_DBG("DPMS mode suspend\n");
        *currentDPMSMode = sched_policy::DPMSMode::DPMSSuspend;
        break;
    case ORG_KDE_KWIN_DPMS_MODE_OFF:
        // qDebug() << "DPMS mode off";
        PRINT_DBG("DPMS mode off\n");
        *currentDPMSMode = sched_policy::DPMSMode::DPMSOff;
        break;
    }
}

struct org_kde_kwin_dpms_listener dpms_listener = {
    .supported = dpms_handle_supported,
    .mode = dpms_handle_mode,
    .done = dpms_handle_done,
};
;

void DeviceModeInfo::registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    struct wayland_display *d = (struct wayland_display *)data;
    // //printf("interface: '%s', version: %d, name: %d\n", interface, version, name);
    if (!strcmp(interface, "org_kde_kwin_dpms_manager")) {
        //qDebug() << "interface:" << interface<< "version:" << version << "name:" << name;
        d->dpms_manager = (struct org_kde_kwin_dpms_manager *)wl_registry_bind(registry, name, &org_kde_kwin_dpms_manager_interface, version);
        if (d->dpms_manager && d->default_output && !d->dpms_interface) {
            d->dpms_interface = org_kde_kwin_dpms_manager_get(d->dpms_manager, d->default_output);
            if (d->dpms_interface) {
                org_kde_kwin_dpms_add_listener(d->dpms_interface, &dpms_listener, &d->mode);
            }
        }
    } else if (!strcmp(interface, "wl_output")) {
        //qDebug() << "interface:" << interface<< "version:" << version << "name:" << name;
        d->default_output = (struct wl_output *)wl_registry_bind(registry, name, &wl_output_interface, version);
        if (d->dpms_manager && d->default_output && !d->dpms_interface) {
            d->dpms_interface = org_kde_kwin_dpms_manager_get(d->dpms_manager, d->default_output);
            if (d->dpms_interface) {
                org_kde_kwin_dpms_add_listener(d->dpms_interface, &dpms_listener, &d->mode);
            }
        }
    }
}

void DeviceModeInfo::initRegistryListener()
{
    registry_listener = {
	        .global = registry_handle_global,
	        .global_remove = registry_handle_global_remove,
    };
}

sched_policy::DPMSMode DeviceModeInfo::currentDPMSModeType()
{
    sched_policy::DPMSMode ret;
    QByteArray xdgSessionTypeValue = qgetenv("XDG_SESSION_TYPE");
    if (xdgSessionTypeValue.compare("wayland", Qt::CaseInsensitive) == 0) {
        // qDebug() << "is wayland platform";
        PRINT_DBG("is wayland platform\n");
        struct wayland_display *d = new wayland_display;
        d->display = wl_display_connect(NULL);
        if (!d->display) {
            //  qWarning() << "Failed to connect to wayland display";
            PRINT_NOTICE("Failed to connect to wayland display\n");
            delete d;
            return sched_policy::DPMSMode::Unknown;
        }

        DeviceModeInfo::initRegistryListener();

        d->registry = wl_display_get_registry(d->display);
        if (!d->registry) {
            PRINT_NOTICE("Failed to get registry\n");
            delete d;
            return sched_policy::DPMSMode::Unknown;
        }
        d->dpms_manager = nullptr;
        d->default_output = nullptr;
        d->dpms_interface = nullptr;
        wl_registry_add_listener(d->registry, &registry_listener, d);
        wl_display_dispatch(d->display);
        wl_display_roundtrip(d->display);

        ret = d->mode;
        goto cleanup;

cleanup:
        if (d->dpms_interface) {
            org_kde_kwin_dpms_destroy(d->dpms_interface);
            d->dpms_interface = nullptr;
        }
        if (d->dpms_manager) {
            org_kde_kwin_dpms_manager_destroy(d->dpms_manager);
            d->dpms_manager = nullptr;
        }
        if (d->registry) {
            wl_registry_destroy(d->registry);
            d->registry = nullptr;
        }
        if (d->display) {
            wl_display_disconnect(d->display);
            d->display = nullptr;
        }
        delete d;
    } else {
        // qDebug() << "is X platform";
        PRINT_DBG("is X platform\n");
        char *disp = NULL;
        Display *dpy;
        int dummy;

        dpy = XOpenDisplay(disp);    /*  Open display and check for success */
        if (dpy == NULL) {
            PRINT_NOTICE("can not open: %s\n", XDisplayName(disp));
            m_currentDPMSMode = sched_policy::DPMSMode::Unknown;
            return sched_policy::DPMSMode::Unknown;
        }
        if (DPMSQueryExtension(dpy, &dummy, &dummy)) {
            if (DPMSCapable(dpy)) {
                BOOL onoff;
	            CARD16 state;
                DPMSInfo(dpy, &state, &onoff);
                if (onoff) {
                    switch (state) {
                    case DPMSModeOn:
                        // qDebug() << "Monitor is On";
                        PRINT_DBG("Monitor is On\n");
                        m_currentDPMSMode = sched_policy::DPMSMode::DPMSOn;
                        ret = sched_policy::DPMSMode::DPMSOn;
                        break;
                    case DPMSModeOff:
                        // qDebug() << "Monitor is Off";
                        PRINT_DBG("Monitor is Off\n");
                        m_currentDPMSMode = sched_policy::DPMSMode::DPMSOff;
                        ret = sched_policy::DPMSMode::DPMSOff;
                        break;
                    default:
                        // qWarning() << "DPMS state not care";
                        PRINT_DBG("DPMS state not care\n");
                        m_currentDPMSMode = sched_policy::DPMSMode::Unknown;
                        ret = sched_policy::DPMSMode::Unknown;
                    }
                }
            } else {
                // qWarning() << "Display is not capable of DPMS";
                PRINT_DBG("Display is not capable of DPMS\n");
                ret = sched_policy::DPMSMode::Unknown;
            }
        }

        XCloseDisplay(dpy);
    }
    return ret;
}

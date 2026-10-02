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

#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include "schedpolicy.h"
#include "statemanager.h"
#include "dpms_client.h"

#include <QDBusInterface>
#include <QGSettings/qgsettings.h>
#include <wayland-client.h>
#include <QObject>
#include <memory>
#include <optional>

enum Policy
{
    Performance = 0,
    Balance,
    EnergySaving,
    Unknown
};

class PowerManager : public QObject
{
    Q_OBJECT

public:
    PowerManager(const std::shared_ptr<StateManager> &stateManager);
    ~PowerManager();

    sched_policy::PowerType lastPowerType() const;
    sched_policy::PowerType currentPowerType() const;
    sched_policy::PowerMode lastPowerMode() const;
    sched_policy::PowerMode currentPowerMode() const;
    void changePowerMode(Policy policy);
    Policy getPolicyWithPowerMode(sched_policy::PowerMode mode);

    static int getPowerPercentage();
    static int getCurrPowerLevel();
    static int getPowerLevel(int battery_percentage);

    void startPowerWatcher();
    void stopPowerWatcher();

private Q_SLOTS:
    void onPropertiesChanged(
        QString name, QMap<QString, QVariant> value, QStringList);

private:
    void initGSettings();
    void initConnections();
    void initPowerMode();
    void handlePowerTypeChanged(sched_policy::PowerType powerType);
    void handlePowerModeChanged(int value);

private:
    sched_policy::PowerType m_lastPowerType;
    sched_policy::PowerType m_currentPowerType;
    sched_policy::PowerMode m_lastPowerMode;
    sched_policy::PowerMode m_currentPowerMode;

    std::unique_ptr<QGSettings> m_gsettings;
    std::shared_ptr<StateManager> m_stateManager;
};

class DeviceModeInfo;
class DeviceModeManager : public QObject
{
    Q_OBJECT

public:
    DeviceModeManager(bool softFreezeEnabled, const std::shared_ptr<StateManager> &stateManager);
    ~DeviceModeManager();

    sched_policy::DeviceMode currentDeviceMode() const;
    sched_policy::DeviceMode lastDeviceMode() const;
    sched_policy::DeviceMode realDeviceMode() const;

    sched_policy::DPMSMode currDPMSMode() const;
    sched_policy::DPMSMode lastDPMSMode() const;

    void startDeviceModeWatcher();
    void stopDeviceModeWatcher();

    void startDPMSModeWatcher();
    void stopDPMSModeWatcher();

    void handleSoftFreezeModeEnabledChanged(bool enabled);

private Q_SLOTS:
    void updateTabletMode(bool isTabletMode);
    void onDPMSModeChanged(bool state);

private:
    void fetchTabletModeStatus();
    void updateDeviceMode();
    void initScreenoffConfigTimer();

private:
    sched_policy::DeviceMode m_lastDeviceMode;
    sched_policy::DeviceMode m_realDeviceMode;
    sched_policy::DeviceMode m_currentDeviceMode;
    sched_policy::DPMSMode m_lastDPMSMode;
    sched_policy::DPMSMode m_currDPMSMode;
    std::unique_ptr<QDBusInterface> m_dbusInterface;
    std::shared_ptr<StateManager> m_stateManager;
    std::unique_ptr<DeviceModeInfo> m_deviceModeInfo;
    QTimer m_screenOffConfigTimer;
    QMetaObject::Connection m_screenOffConn;
    const int kLoadScreenoffConfigTimerInterval;
    bool m_softFreezeModeEnabled = false;
    std::optional<bool> m_isTabletMode = std::nullopt;
};

class DeviceModeInfo : public QObject
{
    Q_OBJECT
public:
    sched_policy::DPMSMode currentDPMSModeType();
   
private:
    struct wayland_display {
	    struct wl_display *display;
	    struct wl_registry *registry;
        struct org_kde_kwin_dpms_manager *dpms_manager;
	    struct org_kde_kwin_dpms *dpms_interface;
	    struct wl_output *default_output;
        sched_policy::DPMSMode mode;
    };
    struct wl_registry_listener registry_listener;

    void initRegistryListener();
    // 静态包装函数，用于调用成员函数
    static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name);
    static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);

private:
    sched_policy::DPMSMode m_currentDPMSMode;
};

#endif // DEVICESTATE_H

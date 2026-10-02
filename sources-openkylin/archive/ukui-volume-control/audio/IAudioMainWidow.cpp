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

#include "IAudioMainWidow.h"

#include <thread>

IAudioMainWindow::IAudioMainWindow(std::list<AppManagerItem>& appWidgetList)
    : m_appWidgetList(appWidgetList)
{
}

std::shared_ptr<IAudioMainWindow::VolumeChangedCallback> IAudioMainWindow::addVolumeObserver(const VolumeChangedCallback& callback)
{
    auto spCallback = std::make_shared<VolumeChangedCallback>(std::move(callback));
    m_volumeObservers.push_back(spCallback);
    return spCallback;
}

void IAudioMainWindow::removeVolumeObserver(const std::shared_ptr<VolumeChangedCallback>& callback)
{
    m_volumeObservers.erase(std::remove_if(m_volumeObservers.begin(), m_volumeObservers.end(),
                                     [callback](const std::shared_ptr<VolumeChangedCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_volumeObservers.end());
}

void IAudioMainWindow::setVolume(int type, int idx, int volume)
{
    volumeNotify(type, idx, volume);
}


std::shared_ptr<IAudioMainWindow::BalanceVolumeChangedCallback> IAudioMainWindow::addBalanceVolumeObserver(const BalanceVolumeChangedCallback& callback)
{
    auto spCallback = std::make_shared<BalanceVolumeChangedCallback>(std::move(callback));
    m_balanceVolumeObservers.push_back(spCallback);
    return spCallback;
}

void IAudioMainWindow::removeBalanceVolumeObserver(const std::shared_ptr<BalanceVolumeChangedCallback>& callback)
{
    m_balanceVolumeObservers.erase(std::remove_if(m_balanceVolumeObservers.begin(), m_balanceVolumeObservers.end(),
                                     [callback](const std::shared_ptr<BalanceVolumeChangedCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_balanceVolumeObservers.end());
}

void IAudioMainWindow::setBalance(int type, double volume)
{
    balanceVolumeNotify(type, volume);
}

std::shared_ptr<IAudioMainWindow::MuteChangedCallback> IAudioMainWindow::addMuteObserver(const MuteChangedCallback& callback)
{
    auto spCallback = std::make_shared<MuteChangedCallback>(std::move(callback));
    m_muteObservers.push_back(spCallback);
    return spCallback;
}

void IAudioMainWindow::removeMuteObserver(const std::shared_ptr<MuteChangedCallback>& callback)
{
    m_muteObservers.erase(std::remove_if(m_muteObservers.begin(), m_muteObservers.end(),
                                     [callback](const std::shared_ptr<MuteChangedCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_muteObservers.end());
}

void IAudioMainWindow::setMute(int type, int idx)
{
    muteNotify(type, idx);
}

std::shared_ptr<IAudioMainWindow::DeviceChangedCallback> IAudioMainWindow::addDeviceObserver(const DeviceChangedCallback& callback)
{
    auto spCallback = std::make_shared<DeviceChangedCallback>(std::move(callback));
    m_deviceObservers.push_back(spCallback);
    return spCallback;
}

void IAudioMainWindow::removeDeviceObserver(const std::shared_ptr<DeviceChangedCallback>& callback)
{
    m_deviceObservers.erase(std::remove_if(m_deviceObservers.begin(), m_deviceObservers.end(),
                                     [callback](const std::shared_ptr<DeviceChangedCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_deviceObservers.end());
}

void IAudioMainWindow::setDefaultDevice(int type, int idx, const QString& portName, const QString& cardName)
{
    defaultDeviceNotify(type, idx, portName, cardName);
}

std::shared_ptr<IAudioMainWindow::SettingsChangedCallback> IAudioMainWindow::addSettingsObserver(const SettingsChangedCallback& callback)
{
    auto spCallback = std::make_shared<SettingsChangedCallback>(std::move(callback));
    m_settingsObservers.push_back(spCallback);
    return spCallback;
}

void IAudioMainWindow::removeSettingsObserver(const std::shared_ptr<SettingsChangedCallback>& callback)
{
    m_settingsObservers.erase(std::remove_if(m_settingsObservers.begin(), m_settingsObservers.end(),
                                     [callback](const std::shared_ptr<SettingsChangedCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_settingsObservers.end());
}

void IAudioMainWindow::setSettingsValue(const QString& key, const QVariant& value)
{
    settingsChangedNotify(key, value);
}

std::shared_ptr<IAudioMainWindow::DetailSettingsClickedCallback> IAudioMainWindow::addDetailSettingsObserver(const DetailSettingsClickedCallback& callback)
{
    auto spCallback = std::make_shared<DetailSettingsClickedCallback>(std::move(callback));
    m_detailSettingsObservers.push_back(spCallback);
    return spCallback;
}

void IAudioMainWindow::removeDetailSettingsObserver(const std::shared_ptr<DetailSettingsClickedCallback>& callback)
{
    m_detailSettingsObservers.erase(std::remove_if(m_detailSettingsObservers.begin(), m_detailSettingsObservers.end(),
                                     [callback](const std::shared_ptr<DetailSettingsClickedCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_detailSettingsObservers.end());
}

void IAudioMainWindow::setDetailSettings(int type)
{
    detailSettingsClickNotify(type);
}

std::shared_ptr<IAudioMainWindow::EnableDeviceCallback> IAudioMainWindow::addEnableDeviceObserver(const EnableDeviceCallback& callback)
{
    auto spCallback = std::make_shared<EnableDeviceCallback>(std::move(callback));
    m_enableDeviceObservers.push_back(spCallback);
    return spCallback;
}

void IAudioMainWindow::removeEnableDeviceObserver(const std::shared_ptr<EnableDeviceCallback>& callback)
{
    m_enableDeviceObservers.erase(std::remove_if(m_enableDeviceObservers.begin(), m_enableDeviceObservers.end(),
                                     [callback](const std::shared_ptr<EnableDeviceCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_enableDeviceObservers.end());
}

void IAudioMainWindow::setEnabled(const QString& card, const QString& port, bool enable)
{
    enableDeviceNotify(card, port, enable);
}

void IAudioMainWindow::startMonitor(uint32_t idx)
{
    if (m_pMonitorStream)
        std::thread([idx, this] () {
            m_pMonitorStream->startMonitor(idx);
        }).detach();
}

void IAudioMainWindow::stopMonitor()
{
    if (m_pMonitorStream)
        m_pMonitorStream->stopMonitor();
}

AppManagerMainWidget* IAudioMainWindow::getAppManagerWidget() const
{
    return m_pAppManagerWidget;
}

void IAudioMainWindow::volumeNotify(int type, int idx, int volume)
{
    for (const auto& callback : m_volumeObservers) {
        if (callback) {
            (*callback)(type, idx, volume);
        }
    }
}

void IAudioMainWindow::balanceVolumeNotify(int type, double volume)
{
    for (const auto& callback : m_balanceVolumeObservers) {
        if (callback) {
            (*callback)(type, volume);
        }
    }
}

void IAudioMainWindow::muteNotify(int type, int idx)
{
    for (const auto& callback : m_muteObservers) {
        if (callback) {
            (*callback)(type, idx);
        }
    }
}

void IAudioMainWindow::defaultDeviceNotify(int type, int idx, const QString& portName, const QString& cardName)
{
    for (const auto& callback : m_deviceObservers) {
        if (callback) {
            (*callback)(type, idx, portName, cardName);
        }
    }
}

void IAudioMainWindow::settingsChangedNotify(const QString& key, const QVariant& value)
{
    for (const auto& callback : m_settingsObservers) {
        if (callback) {
            (*callback)(key, value);
        }
    }
}

void IAudioMainWindow::detailSettingsClickNotify(int type)
{
    for (const auto& callback : m_detailSettingsObservers) {
        if (callback) {
            (*callback)(type);
        }
    }
}

void IAudioMainWindow::enableDeviceNotify(const QString& card, const QString& port, bool enable)
{
    for (const auto& callback : m_enableDeviceObservers) {
        if (callback) {
            (*callback)(card, port, enable);
        }
    }
}

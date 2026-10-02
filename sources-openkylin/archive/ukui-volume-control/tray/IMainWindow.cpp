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

#include "IMainWindow.h"

std::shared_ptr<IMainWindow::VolumeChangedCallback> IMainWindow::addVolumeObserver(const VolumeChangedCallback& callback)
{
    auto spCallback = std::make_shared<VolumeChangedCallback>(std::move(callback));
    m_volumeObservers.push_back(spCallback);
    return spCallback;
}

void IMainWindow::removeVolumeObserver(const std::shared_ptr<VolumeChangedCallback>& callback)
{
    m_volumeObservers.erase(std::remove_if(m_volumeObservers.begin(), m_volumeObservers.end(),
                                     [callback](const std::shared_ptr<VolumeChangedCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_volumeObservers.end());
}

void IMainWindow::setVolume(int type, int volume)
{
    volumeNotify(type, volume);
}

std::shared_ptr<IMainWindow::MuteChangedCallback> IMainWindow::addMuteObserver(const MuteChangedCallback& callback)
{
    auto spCallback = std::make_shared<MuteChangedCallback>(std::move(callback));
    m_muteObservers.push_back(spCallback);
    return spCallback;
}

void IMainWindow::removeMuteObserver(const std::shared_ptr<MuteChangedCallback>& callback)
{
    m_muteObservers.erase(std::remove_if(m_muteObservers.begin(), m_muteObservers.end(),
                                     [callback](const std::shared_ptr<MuteChangedCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_muteObservers.end());
}

void IMainWindow::setMute(int type, const QString& name)
{
    muteNotify(type, name);
}

std::shared_ptr<IMainWindow::DeviceChangedCallback> IMainWindow::addDeviceObserver(const DeviceChangedCallback& callback)
{
    auto spCallback = std::make_shared<DeviceChangedCallback>(std::move(callback));
    m_deviceObservers.push_back(spCallback);
    return spCallback;
}

void IMainWindow::removeDeviceObserver(const std::shared_ptr<DeviceChangedCallback>& callback)
{
    m_deviceObservers.erase(std::remove_if(m_deviceObservers.begin(), m_deviceObservers.end(),
                                     [callback](const std::shared_ptr<DeviceChangedCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_deviceObservers.end());
}

void IMainWindow::setDefaultDevice(int type, const QString& portName, const QString& cardName)
{
    defaultDeviceNotify(type, portName, cardName);
}

std::shared_ptr<IMainWindow::TrayIconTriggerCallback> IMainWindow::addTrayIconTriggerObserver(const TrayIconTriggerCallback& callback)
{
    auto spCallback = std::make_shared<TrayIconTriggerCallback>(std::move(callback));
    m_trayIconObservers.push_back(spCallback);
    return spCallback;
}

void IMainWindow::removeTrayIconTriggerObserver(const std::shared_ptr<TrayIconTriggerCallback>& callback)
{
    m_trayIconObservers.erase(std::remove_if(m_trayIconObservers.begin(), m_trayIconObservers.end(),
                                     [callback](const std::shared_ptr<TrayIconTriggerCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_trayIconObservers.end());
}

void IMainWindow::triggerWidget()
{
    trayIconTriggerNotify();
}

std::shared_ptr<IMainWindow::SizeChangedCallback> IMainWindow::addSizeChangedObserver(const SizeChangedCallback& callback)
{
    auto spCallback = std::make_shared<SizeChangedCallback>(std::move(callback));
    m_sizeChangedObservers.push_back(spCallback);
    return spCallback;
}

void IMainWindow::removeSizeChangedObserver(const std::shared_ptr<SizeChangedCallback>& callback)
{
    m_sizeChangedObservers.erase(std::remove_if(m_sizeChangedObservers.begin(), m_sizeChangedObservers.end(),
                                     [callback](const std::shared_ptr<SizeChangedCallback>& existingCallback) {
                                         return existingCallback == callback;
                                     }), m_sizeChangedObservers.end());
}

void IMainWindow::setSize(const QSize& size)
{
    sizeChangedNotify(size);
}

void IMainWindow::volumeNotify(int type, int volume)
{
    for (const auto& callback : m_volumeObservers) {
        if (callback && *callback) {
            (*callback)(type, volume);
        }
    }
}

void IMainWindow::muteNotify(int type, const QString& name)
{
    for (const auto& callback : m_muteObservers) {
        if (callback && *callback) {
            (*callback)(type, name);
        }
    }
}

void IMainWindow::defaultDeviceNotify(int type, const QString& portName, const QString& cardName)
{
    for (const auto& callback : m_deviceObservers) {
        if (callback && *callback) {
            (*callback)(type, portName, cardName);
        }
    }
}

void IMainWindow::trayIconTriggerNotify()
{
    for (const auto& callback : m_trayIconObservers) {
        if (callback && *callback) {
            (*callback)();
        }
    }
}

void IMainWindow::sizeChangedNotify(const QSize& size)
{
    for (const auto& callback : m_sizeChangedObservers) {
        if (callback && *callback) {
            (*callback)(size);
        }
    }
}

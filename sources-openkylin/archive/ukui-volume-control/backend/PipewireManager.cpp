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

#include "PipewireManager.h"

#include <pipewire/pipewire.h>

namespace UkuiAudioFramwork
{
PipewireManager::PipewireManager(int times) : m_reconnectTime(times)
{

}

void PipewireManager::init()
{
    // TODO
}

bool PipewireManager::connect()
{
    // TODO
    return true;
}

// Interface
void PipewireManager::loadModule(const std::string& name, const std::string& param)
{
    // TODO
}

void PipewireManager::unloadModule(const std::string& name, bool restoreMasterDevice)
{
    // TODO
}

// Method
void PipewireManager::setDetailVolume(const NodeType& type, const std::string& name, const std::string& dev, int volume)
{
    // TODO
}

int PipewireManager::getDetailVolume(const NodeType& type, const std::string& name, const std::string& dev) const
{
    // TODO
    return 100;
}

void PipewireManager::setVolume(const NodeType& type, int idx, int volume)
{
    // TODO
}

int PipewireManager::getVolume(const NodeType& type, int idx) const
{
    // TODO
    return 100;
}

void PipewireManager::setDetailMute(const NodeType& type, const std::string& name, const std::string& dev, bool mute)
{
    // TODO
}

bool PipewireManager::getDetailMute(const NodeType& type, const std::string& name, const std::string& dev) const
{
    // TODO
    return false;
}

void PipewireManager::setBalance(const NodeType& type, double v)
{
    // TODO
}

double PipewireManager::getBalance(const NodeType& type) const
{
    // TODO
    return 0.0;
}

void PipewireManager::setMute(const NodeType& type, int idx, bool mute)
{
    // TODO
}

bool PipewireManager::getMute(const NodeType& type, int idx) const
{
    // TODO
    return false;
}

DefaultDeviceInfo PipewireManager::getDefaultDevice(const NodeType& type, int idx) const
{
    // TODO
    return {};
}

void PipewireManager::setDefaultDevice(const NodeType& type, int idx, const std::string& name, const std::string& dev)
{
    // TODO
}

std::vector<std::string> PipewireManager::getDeviceList(const NodeType& type) const
{
    // TODO
    return {};
}

std::list<DefaultDeviceInfo> PipewireManager::getSinkList() const
{
    // TODO
    return {};
}

std::list<DefaultDeviceInfo> PipewireManager::getSourceList() const
{
    // TODO
    return {};
}

std::list<std::shared_ptr<DeviceInfo>> PipewireManager::getAvailablePortList(const NodeType& type) const
{
    // TODO
    return {};
}

std::list<StreamMediaInfo> PipewireManager::getSinkInputList() const
{
    // TODO
    return {};
}

std::list<StreamMediaInfo> PipewireManager::getSourceOutputList() const
{
    // TODO
    return {};
}

std::list<StreamMediaIntegrationInfo> PipewireManager::getStreamMediaList() const
{
    // TODO
    return {};
}

std::list<std::string> PipewireManager::getModuleList()
{
    // TODO
    return {};
}

void PipewireManager::setEnabled(std::shared_ptr<RestoreVolumeJson>, const std::string&, const std::string&, bool)
{
    // TODO
}

bool PipewireManager::isEnabled(const std::string&, const std::string&) const
{
    // TODO
    return true;
}

bool PipewireManager::isLoaded(const ModuleType&) const
{
    // TODO
    return true;
}

bool PipewireManager::isValidDevice(const NodeType&) const
{
    // TODO
    return true;
}

// signal
void PipewireManager::deviceChanged(const NodeType& type, const std::string& portName, const std::string& cardName)
{
    // TODO
}

void PipewireManager::deviceAdjust(const NodeType& type)
{
    // TODO
}

void PipewireManager::portChanged(const NodeType& type, const std::string& name)
{
    // TODO
}

void PipewireManager::cardRemoved(const std::string& name)
{
    // TODO
}

void PipewireManager::addStream(int idx, int value, const std::string& iconName, const std::string& descName)
{
    // TODO
}

void PipewireManager::removeStream(int idx)
{
    // TODO
}

void PipewireManager::addStream(int dire, int idx, int value, bool mute, const std::string& iconName, const std::string& descName)
{
    // TODO
}

void PipewireManager::removeStream(int dire, int idx)
{
    // TODO
}
}

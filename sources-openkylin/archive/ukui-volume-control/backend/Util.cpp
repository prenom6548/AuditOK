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

#include "Util.h"
#include <iostream>
#include <string>
#include <type_traits>
#include <unordered_set>

namespace UkuiAudioFramwork
{

namespace {
template <typename Enum>
std::string unknownEnum(Enum value)
{
    using U = typename std::underlying_type<Enum>::type;
    const auto raw = static_cast<U>(value);
    return std::string("Unknow(") + std::to_string(static_cast<long long>(raw)) + ")";
}
} // namespace

std::string getModuleName(const Module& module)
{
    switch (module) {
        CASE(Module::ControlCenter);
        CASE(Module::Service);
        CASE(Module::Tray);
        default:
            return unknownEnum(module);
    }
}

std::string getBackendName(const BackendType& type)
{
    switch (type) {
        CASE(BackendType::PULSEAUDIO);
        CASE(BackendType::PIPEWIRE);
        CASE(BackendType::ALSA);
        CASE(BackendType::JACK);
        CASE(BackendType::UNKNOWN);
        default:
            return unknownEnum(type);
    }
}

std::string getNodeTypeName(const NodeType& type)
{
    switch (type) {
        CASE(NodeType::INPUT);
        CASE(NodeType::OUTPUT);
        CASE(NodeType::SINK_INPUT);
        CASE(NodeType::SOURCE_OUTPUT);
        default:
            return unknownEnum(type);
    }
}

bool hasKeyword(const std::vector<std::string>& stringList, const std::string& keyword)
{
    for (const auto& str : stringList) {
        if (str.find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
}

DevicePortType getDevicePortType(const std::string& cardName, const std::string& portName)
{
    std::vector<std::string> stringList = {cardName, portName};

    if (hasKeyword(stringList, "multichannel")) {
        return DevicePortType::PORT_TYPE_MULTICHANNEL;
    }
    else if (hasKeyword(stringList, "bluez") ||
        hasKeyword(stringList, "bluetooth")) {
        return DevicePortType::PORT_TYPE_BLUETOOTH;
    }
    else if (hasKeyword(stringList, "linein") ||
        hasKeyword(stringList, "lineout")) {
        return DevicePortType::PORT_TYPE_LINEIO;
    }
    else if (hasKeyword(stringList, "rear-mic") ||
        hasKeyword(stringList, "front-mic") ||
        hasKeyword(stringList, "[In] Mic2") ||
        hasKeyword(stringList, "headphone") ||
        hasKeyword(stringList, "Headphone") ||
        hasKeyword(stringList, "headset")) {
        return DevicePortType::PORT_TYPE_HEADSET;
    }
    else if (hasKeyword(stringList, "usb")) {
        return DevicePortType::PORT_TYPE_USB;
    }
    else if (hasKeyword(stringList, "hdmi") ||
        hasKeyword(stringList, "HDMI")) {
        return DevicePortType::PORT_TYPE_HDMI;
    }
    else if (hasKeyword(stringList, "speaker") ||
        hasKeyword(stringList, "input-mic") ||
        hasKeyword(stringList, "[In] Mic1")) {
        return DevicePortType::PORT_TYPE_BUILTIN;
    }

    return DevicePortType::PORT_TYPE_UNKNOWN;
}

int calaPriorityByName(const std::string& name)
{
    if (name.find("bluez") != std::string::npos) {
        return 2;
    }
    else if (name.find("usb") != std::string::npos) {
        return 1;
    }

    return 0;
}

std::string DeviceInfo::toString() const
{
    return std::string("cardName: ") + cardName + ", cardDesc: " + cardDesc + ", portName: " + portName +
           ", portLabel: " + portLabel + ", priority: " + std::to_string(priority) +
           ", direction: " + std::to_string(direction) + ", available: " + std::to_string(available) +
           ", enabled: " + std::string(enabled ? "true" : "false") + ", type: " + std::to_string(EnumToInt(type));
}

std::string DefaultDeviceInfo::toString() const
{
    return std::string("idx: ") + std::to_string(idx) + ", name: " + name + ", cardName: " + cardName +
           ", activePortName: " + activePortName + ", activePortLabel: " + activePortLabel;
}

std::string StreamMediaInfo::toString() const
{
    return std::string("name: ") + name + ", index: " + std::to_string(index) +
           ", volume: " + std::to_string(volume) + ", mediaRole: " + mediaRole + ", iconName: " + iconName +
           ", binary: " + binary;
}

namespace {
bool isGenericBrowserIcon(const std::string& iconName)
{
    static const std::unordered_set<std::string> genericIcons = {
        "chromium-browser",
        "chromium",
        "google-chrome",
        "chrome",
    };
    return genericIcons.count(iconName) > 0;
}
} // namespace

std::string resolveStreamAppIconName(
    const char* iconName,
    const char* processBinary,
    const char* streamName,
    const std::unordered_map<std::string, std::string>& adapterMap)
{
    std::string appIcon = iconName ? iconName : "application-x-desktop";

    if (processBinary && isGenericBrowserIcon(appIcon)) {
        const std::string binary = processBinary;
        if (!binary.empty() && binary != appIcon) {
            appIcon = binary;
        }
    }

    auto it = adapterMap.find(appIcon);
    if (it != adapterMap.end()) {
        appIcon = it->second;
    }

    return appIcon;
}
}

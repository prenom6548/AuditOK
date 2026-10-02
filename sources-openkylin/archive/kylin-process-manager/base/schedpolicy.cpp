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

#include "schedpolicy.h"
#include <algorithm>

namespace sched_policy
{

DeviceMode deviceModeFromString(const std::string &mode)
{
    static std::map<std::string, DeviceMode> deviceModeMap{
        {"PC", DeviceMode::PC},
        {"Tablet", DeviceMode::Tablet},
        {"SoftFreeze", DeviceMode::SoftFreeze}};

    if (deviceModeMap.find(mode) == deviceModeMap.end()) {
        return DeviceMode::Unknown;
    }

    return deviceModeMap[mode];
}

PowerMode powerModeFromString(const std::string &mode)
{
    static std::map<std::string, PowerMode> powerModeMap{
        {"Balance", PowerMode::Balance},
        {"Save", PowerMode::Save},
        {"Performance", PowerMode::Performance}};

    if (powerModeMap.find(mode) == powerModeMap.end()) {
        return PowerMode::Unknown;
    }
    return powerModeMap[mode];
}

GroupType groupTypeFromString(const std::string &type)
{
    static std::map<std::string, GroupType> groupTypeMap{
        {"Session", GroupType::SessionGroup},
        {"Top", GroupType::TopGroup},
        {"Focus", GroupType::FocusGroup},
        {"Foreground", GroupType::ForegroundGroup},
        {"Background", GroupType::BackgroundGroup},
        {"Service", GroupType::ServiceGroup},
        {"Cached", GroupType::CachedGroup},
        {"Default", GroupType::DefaultGroup},
    };

    if (groupTypeMap.find(type) == groupTypeMap.end()) {
        return GroupType::Unknown;
    }

    return groupTypeMap[type];
}

std::string groupTypeToString(GroupType type)
{
    static std::map<GroupType, std::string> groupTypeMap{
        {GroupType::SessionGroup, "Session"},
        {GroupType::TopGroup, "Top"},
        {GroupType::FocusGroup, "Focus"},
        {GroupType::ForegroundGroup, "Foreground"},
        {GroupType::BackgroundGroup, "Background"},
        {GroupType::CachedGroup, "Cached"},
        {GroupType::DefaultGroup, "Default"},
    };

    if (groupTypeMap.find(type) == groupTypeMap.end()) {
        return std::string();
    }

    return groupTypeMap[type];
}

std::string deviceModeToString(DeviceMode mode)
{
    static std::map<DeviceMode, std::string> deviceModeMap{
        {DeviceMode::PC, "PC"},
        {DeviceMode::Tablet, "Tablet"},
        {DeviceMode::SoftFreeze, "SoftFreeze"}};

    return deviceModeMap[mode];
}

std::string generatePolicyId(const std::string &deviceMode, const std::string &powerMode)
{
    auto toLower = [](const std::string &str) {
        std::string tmp(str);
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        return tmp;
    };

    if (deviceModeFromString(deviceMode) == DeviceMode::Unknown || powerModeFromString(powerMode) == PowerMode::Unknown) {
        return std::string();
    }

    return toLower(deviceMode) + "_" + toLower(powerMode);
}

std::string generatePolicyId(DeviceMode deviceMode, PowerMode powerMode)
{
    static std::map<PowerMode, std::string> powerModeMap{
        {PowerMode::Balance, "Balance"},
        {PowerMode::Save, "Save"},
        {PowerMode::Performance, "Performance"}};

    std::string deviceModeStr = deviceModeToString(deviceMode);

    if (deviceModeStr.empty() || powerModeMap.find(powerMode) == powerModeMap.end()) {
        return std::string();
    }

    return generatePolicyId(deviceModeStr, powerModeMap[powerMode]);
}

sched_policy::GroupType getGroupTypeByNormalAppInfo(sched_policy::AppState appState)
{
    switch (appState) {
    case sched_policy::AppState::Focus:
        return sched_policy::GroupType::FocusGroup;
    case sched_policy::AppState::Foreground:
        return sched_policy::GroupType::ForegroundGroup;
    case sched_policy::AppState::Background:
        return sched_policy::GroupType::BackgroundGroup;
    case sched_policy::AppState::Cached:
        return sched_policy::GroupType::CachedGroup;
    default:
        break;
    }

    return sched_policy::GroupType::DefaultGroup;
}

sched_policy::GroupType getGroupTypeByAppInfo(sched_policy::AppType appType, sched_policy::AppState appState)
{
    switch (appType) {
    case sched_policy::AppType::Normal: {
        return getGroupTypeByNormalAppInfo(appState);
    }
    case sched_policy::AppType::Top:
        return sched_policy::GroupType::TopGroup;
    case sched_policy::AppType::Session: {
        return sched_policy::GroupType::SessionGroup;
    }
    default: {
        return sched_policy::GroupType::DefaultGroup;
    }
    }
    return sched_policy::GroupType::DefaultGroup;
}

} // namespace sched_policy

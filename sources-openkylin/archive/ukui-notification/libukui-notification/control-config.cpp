/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */

#include "control-config.h"

#include <utils.h>

using namespace UkuiNotification;

ControlConfig::ControlConfig(const QString& configName, const QJsonObject& config, QObject* parent)
    : QObject(parent)
    , m_configName(configName)
{
    parseConfig(config);
}

void ControlConfig::parseConfig(const QJsonObject& config)
{
    if (config.contains("mode")) {
        if (int modeValue = config["mode"].toInt(); isValidMode(modeValue)) {
            if (const auto mode = static_cast<ControlMode>(modeValue); m_mode != mode) {
                m_mode = mode;
            }
        } else {
            qWarning() << "Invalid mode value:" << modeValue << "for config:" << m_configName;
        }
    } else {
        qWarning() << "No mode value found in config:" << m_configName;
    }
    if (config.contains("appList")) {
        //比较应用列表是否不同
        auto isListDifferent = [](const QStringList& oldList, const QStringList& newList) {
            if (oldList.size() != newList.size()) {
                return true;
            }
            const QSet<QString> oldSet(oldList.begin(), oldList.end());
            const QSet<QString> newSet(newList.begin(), newList.end());
            return oldSet != newSet;
        };

        //兼容.desktop后缀的应用名称
        QSet<QString> result;
        for (const auto& appDesktopPath : config["appList"].toVariant().toStringList()) {
            if (const auto& appDesktop = Utils::desktopEntry2Name(appDesktopPath); !appDesktop.isEmpty()) {
                result.insert(appDesktop);
            }
        }

        if (const auto appList = result.values(); isListDifferent(m_appList, appList)) {
            m_appList = appList;
        }
    } else {
        qWarning() << "No appList value found in config:" << m_configName;
    }
}
std::optional<bool> ControlConfig::isAppAllowed(const QString& desktopEntry) const
{
    switch (m_mode) {
    case ControlMode::Whitelist:
        return m_appList.contains(desktopEntry);
    case ControlMode::Blacklist:
        if (m_appList.contains(desktopEntry)) {
            return false;
        }
        return std::nullopt;
    default:
        return std::nullopt;
    }
}

bool ControlConfig::isOperationAllowed(const QString& desktopEntry) const
{
    switch (m_mode) {
    case ControlMode::Whitelist:
        return false;
    case ControlMode::Blacklist:
        return !m_appList.contains(desktopEntry);
    default:
        return true;
    }
}

bool ControlConfig::isEnabled() const
{
    return m_mode != ControlMode::None;
}

void ControlConfig::updateConfig(const QJsonObject& config)
{
    parseConfig(config);
}

QStringList ControlConfig::appList() const
{
    if (!isEnabled()) {
        return {};
    }
    return m_appList;
}


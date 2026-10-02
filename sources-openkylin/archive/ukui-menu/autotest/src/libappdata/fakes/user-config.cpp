/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 *
 */

#include "user-config.h"

namespace UkuiMenu {

UserConfig *UserConfig::instance()
{
    static UserConfig config;
    return &config;
}

UserConfig::UserConfig(QObject *parent)
    : QObject(parent)
{
}

bool UserConfig::isFirstStartUp() const
{
    return m_isFirstStartUp;
}

QSet<QString> UserConfig::preInstalledApps() const
{
    return m_preInstalledApps;
}

void UserConfig::addPreInstalledApp(const QString &appid)
{
    m_preInstalledApps.insert(appid);
}

void UserConfig::removePreInstalledApp(const QString &appid)
{
    m_preInstalledApps.remove(appid);
}

bool UserConfig::isPreInstalledApps(const QString &appid) const
{
    return m_preInstalledApps.contains(appid);
}

void UserConfig::initPreInstalledLists(QStringList apps)
{
    m_preInstalledApps = QSet<QString>(apps.begin(), apps.end());
}

void UserConfig::sync()
{
}

void UserConfig::setFirstStartUp(bool firstStartUp)
{
    m_isFirstStartUp = firstStartUp;
}

}

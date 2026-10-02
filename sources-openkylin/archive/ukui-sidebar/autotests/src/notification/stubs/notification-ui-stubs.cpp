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

#include "global-settings.h"
#include "screen-monitor.h"

namespace Sidebar {

GlobalSettings::GlobalSettings(QObject *parent) : QObject(parent)
{
}

GlobalSettings *GlobalSettings::globalInstance(QObject *parent)
{
    static GlobalSettings settings(parent);
    return &settings;
}

QVariant GlobalSettings::getValue(const QString &key) const
{
    return m_values.value(key);
}

void GlobalSettings::setValue(const QString &key, const QVariant &value)
{
    if (m_values.value(key) == value) {
        return;
    }
    m_values.insert(key, value);
    Q_EMIT valueChanged(key);
}

ScreenMonitor::ScreenMonitor(QObject *parent) : QObject(parent)
{
}

ScreenMonitor *ScreenMonitor::getInstance()
{
    static ScreenMonitor monitor;
    return &monitor;
}

bool ScreenMonitor::getScreenLockState() const
{
    return m_screenLockState;
}

bool ScreenMonitor::getSessionToolsState() const
{
    return m_sessionToolsState;
}

void ScreenMonitor::setScreenLockState(bool state)
{
    if (m_screenLockState == state) {
        return;
    }
    m_screenLockState = state;
    Q_EMIT screenLockStateChanged(state);
}

void ScreenMonitor::setSessionToolsState(bool state)
{
    if (m_sessionToolsState == state) {
        return;
    }
    m_sessionToolsState = state;
    Q_EMIT sessionToolsStateChanged(state);
}

} // namespace Sidebar

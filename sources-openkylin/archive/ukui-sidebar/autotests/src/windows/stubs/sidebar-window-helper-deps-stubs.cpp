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
#include "hand-gesture-helper.h"
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

QStringList GlobalSettings::getKeys() const
{
    return m_values.keys();
}

int GlobalSettings::getPanelLength(const QString &screenName) const
{
    return m_panelLengths.value(screenName, 0);
}

void GlobalSettings::setValue(const QString &key, const QVariant &value)
{
    if (m_values.value(key) == value) {
        return;
    }
    m_values.insert(key, value);
    Q_EMIT valueChanged(key);
}

void GlobalSettings::setPanelLength(const QString &screenName, int length)
{
    m_panelLengths.insert(screenName, length);
    Q_EMIT valueChanged(UKUI_PANEL_LENGTH_KEY);
}

void GlobalSettings::reset()
{
    m_values.clear();
    m_panelLengths.clear();
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

QScreen *ScreenMonitor::getPrimaryScreen() const
{
    return m_primaryScreen;
}

QRect ScreenMonitor::getGeometry() const
{
    return m_primaryScreen ? m_primaryScreen->geometry() : QRect();
}

Qt::LayoutDirection ScreenMonitor::getLayoutDirection() const
{
    return m_direction;
}

int ScreenMonitor::getPanelPosition() const
{
    return m_panelPosition;
}

int ScreenMonitor::getPanelSize() const
{
    return m_panelSize;
}

void ScreenMonitor::setScreenLockState(bool state)
{
    if (m_screenLockState == state) {
        return;
    }
    m_screenLockState = state;
    Q_EMIT screenLockStateChanged(state);
}

void ScreenMonitor::setPrimaryScreen(QScreen *screen)
{
    m_primaryScreen = screen;
    Q_EMIT primaryScreenChanged();
    Q_EMIT geometryChanged(getGeometry());
}

void ScreenMonitor::setLayoutDirection(Qt::LayoutDirection direction)
{
    if (m_direction == direction) {
        return;
    }
    m_direction = direction;
    Q_EMIT layoutDirectionChanged();
}

void ScreenMonitor::setPanelPosition(int position)
{
    m_panelPosition = position;
}

void ScreenMonitor::setPanelSize(int size)
{
    m_panelSize = size;
}

void ScreenMonitor::emitPanelPropertyChanged()
{
    Q_EMIT panelPropertyChanged();
}

void ScreenMonitor::reset()
{
    m_screenLockState = false;
    m_primaryScreen = nullptr;
    m_direction = Qt::LeftToRight;
    m_panelPosition = 0;
    m_panelSize = 0;
}

HandGestureHelper::HandGestureHelper(QObject *parent) : QObject(parent)
{
}

HandGestureHelper *HandGestureHelper::getInstance()
{
    static HandGestureHelper helper;
    return &helper;
}

void HandGestureHelper::reset()
{
}

void HandGestureHelper::callNotificationCenter(int posY)
{
    Q_EMIT notificationCenterCalled(posY);
}

void HandGestureHelper::top2BottomRelease(int posX, int posY)
{
    Q_EMIT top2BottomReleased(posX, posY);
}

void HandGestureHelper::callControlCenter(int posX)
{
    Q_EMIT controlCenterCalled(posX);
}

void HandGestureHelper::right2LeftRelease(int posX, int posY)
{
    Q_EMIT right2LeftReleased(posX, posY);
}

} // namespace Sidebar

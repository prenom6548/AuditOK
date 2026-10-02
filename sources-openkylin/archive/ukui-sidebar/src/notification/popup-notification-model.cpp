/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "popup-notification-model.h"
#include "notification-model.h"

#include "global-settings.h"
#include "screen-monitor.h"

using namespace UkuiNotification;

PopupNotificationModel::PopupNotificationModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    m_isTabletMode = Sidebar::GlobalSettings::globalInstance()->getValue(TABLET_MODE).toBool();
    m_screenLockState = Sidebar::ScreenMonitor::getInstance()->getScreenLockState();
    m_sessionToolsState = Sidebar::ScreenMonitor::getInstance()->getSessionToolsState();

    connect(Sidebar::GlobalSettings::globalInstance(), &Sidebar::GlobalSettings::valueChanged, this, [this] (const QString &key) {
        if (key == TABLET_MODE) {
            m_isTabletMode = Sidebar::GlobalSettings::globalInstance()->getValue(key).toBool();
            invalidateFilter();
        }
    });
    connect(Sidebar::ScreenMonitor::getInstance(), &Sidebar::ScreenMonitor::screenLockStateChanged, this, [this] (bool state) {
        if (m_screenLockState != state) {
            m_screenLockState = state;
            invalidateFilter();
        }
    });
    connect(Sidebar::ScreenMonitor::getInstance(), &Sidebar::ScreenMonitor::sessionToolsStateChanged, this, [this] (bool state) {
        if (m_sessionToolsState != state) {
            m_sessionToolsState = state;
            invalidateFilter();
        }
    });
}

bool PopupNotificationModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_isTabletMode || m_sessionToolsState) {
        return false;
    }

    const QModelIndex modelIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if (m_screenLockState) {
        return !modelIndex.data(NotificationItem::IsStored).toBool() && modelIndex.data(NotificationItem::ShowOnLockScreen).toBool();
    }
    return !modelIndex.data(NotificationItem::IsStored).toBool();
}

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

#include "sidebar-main-deps.h"

namespace UkuiNotification {

NotificationCenterWindow::NotificationCenterWindow(QWindow *parent)
    : UkuiQuick::SharedEngineView(parent)
{
}

NotificationGroupModel *NotificationCenterWindow::globalGroupModel()
{
    static NotificationGroupModel model;
    if (!model.sourceModel()) {
        model.setSourceModel(NotificationModel::instance());
    }
    return &model;
}

void NotificationCenterWindow::init()
{
    m_inited = true;
}

void NotificationCenterWindow::activeNotificationCenter(bool active)
{
    m_lastActive = active;
    if (isVisible() == active) {
        return;
    }
    setProperty("_test_visible", active);
    Q_EMIT visibleChanged(active);
}

bool NotificationCenterWindow::lastActive() const
{
    return m_lastActive;
}

bool NotificationCenterWindow::inited() const
{
    return m_inited;
}

bool NotificationCenterWindow::isVisible() const
{
    return property("_test_visible").toBool();
}

} // namespace UkuiNotification

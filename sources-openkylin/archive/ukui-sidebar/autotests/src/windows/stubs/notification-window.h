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

#ifndef AUTOTESTS_WINDOWS_STUB_NOTIFICATION_WINDOW_H
#define AUTOTESTS_WINDOWS_STUB_NOTIFICATION_WINDOW_H

#include "notification-group-model.h"
#include "shared-engine-view.h"

namespace UkuiNotification {

class NotificationCenterWindow : public UkuiQuick::SharedEngineView
{
public:
    explicit NotificationCenterWindow(QWindow *parent = nullptr);

    static NotificationGroupModel *globalGroupModel();

    void init();
    void activeNotificationCenter(bool active);
    bool lastActive() const;
    bool inited() const;
    bool isVisible() const;

private:
    bool m_lastActive = false;
    bool m_inited = false;
};

} // namespace UkuiNotification

#endif

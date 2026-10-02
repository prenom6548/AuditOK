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

#ifndef TEST_NOTIFICATION_GLOBAL_SETTINGS_H
#define TEST_NOTIFICATION_GLOBAL_SETTINGS_H

#include <QObject>

namespace UkuiNotification {

class NotificationGlobalSettings : public QObject
{
    Q_OBJECT
public:
    explicit NotificationGlobalSettings(QObject *parent = nullptr);

    bool receiveNotificationsFromApps() const;
    bool isDND() const;
    bool notifyAlarmWhileDND() const;

    void setReceiveNotificationsFromApps(bool value);
    void setDND(bool value);
    void setNotifyAlarmWhileDND(bool value);
    static NotificationGlobalSettings *lastInstance();

private:
    bool m_receiveNotificationsFromApps {true};
    bool m_isDnd {false};
    bool m_notifyAlarmWhileDnd {false};
};

} // namespace UkuiNotification

#endif

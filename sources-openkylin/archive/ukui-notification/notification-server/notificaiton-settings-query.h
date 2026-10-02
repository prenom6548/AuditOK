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
#ifndef NOTIFICAITONSETTINGSQUERY_H
#define NOTIFICAITONSETTINGSQUERY_H
#include <QObject>
#include <notification-settings/notification-global-settings.h>
#include <notification-settings/single-application-settings.h>

class NotificaitonSettingsQuery : public QObject
{
    Q_OBJECT
public:
    explicit  NotificaitonSettingsQuery(QObject* parent = nullptr);
    ~NotificaitonSettingsQuery() = default;

    bool isAlarmSoundAllowed() const;
    bool isSoundAllowed(const QString& desktopEntry) const;
    void setDND(bool enable);
    bool isDND();
Q_SIGNALS:
    void alarmSoundAllowedChanged(bool);
    void dndChanged(bool);
private Q_SLOTS:
    void checkAlarmSoundAllowed();
private:
    UkuiNotification::NotificationGlobalSettings* m_notificationGlobalSettings{nullptr};
    UkuiNotification::SingleApplicationSettings* m_alarmSingleAppSettings{nullptr};
    bool m_alarmSoundAllowed{false};
};

#endif //NOTIFICAITONSETTINGSQUERY_H

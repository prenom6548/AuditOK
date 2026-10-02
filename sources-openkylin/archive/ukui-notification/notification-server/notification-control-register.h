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

#ifndef NOTIFICATIONCONTROLREGISTER_H
#define NOTIFICATIONCONTROLREGISTER_H

#include <QObject>
class NotificationControlRegister final : public QObject
{
    Q_OBJECT
public:
    enum class RegisterMode {Default, Notice, Interface, Domain};


    static QString registerModeToString(RegisterMode mode) {
        switch (mode) {
        default:
        case RegisterMode::Default:
            return "Default";
        case RegisterMode::Notice:
            return "Notice";
        case RegisterMode::Interface:
            return "Interface";
        case RegisterMode::Domain:
            return "Domain";
        }
    }

    explicit NotificationControlRegister(QObject* parent = nullptr);
    ~NotificationControlRegister() override = default;
    void registerNotificationControl(const QString& desktopEntry, RegisterMode mode = RegisterMode::Notice);
    void unRegisterNotificationControl(const QString& desktopEntry);
    void updateNotificationControl(const QString& desktopEntry, RegisterMode mode);
    void unRegisterNotificationControlByMode(RegisterMode mode) const;
};

#endif // NOTIFICATIONCONTROLREGISTER_H

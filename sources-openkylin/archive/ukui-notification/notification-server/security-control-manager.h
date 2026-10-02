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
#ifndef SECURITYCONTROLMANAGER_H
#define SECURITYCONTROLMANAGER_H

#include <QObject>
#include <memory>
#include <security-control-config.h>

#include "notification-control-register.h"
class SecurityControlManager final : public QObject
{
    Q_OBJECT
public:
    explicit SecurityControlManager(const std::unique_ptr<NotificationControlRegister>&, QObject *parent = nullptr);

    bool isNotificationAllowed(const QString& desktopEntry) const;
    bool isEnabledNotification() const;

private Q_SLOTS:
    void onSecurityControlConfigChanged();
    void onSecurityControlExit();

private:
    void updateSecurityControlApps();
    void unregisterSecurityControlApps() const;

    const std::unique_ptr<NotificationControlRegister>& m_controlRegister;
};

#endif //SECURITYCONTROLMANAGER_H

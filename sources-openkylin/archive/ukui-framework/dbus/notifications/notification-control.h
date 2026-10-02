/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NOTIFICATION_CONTROL_H
#define NOTIFICATION_CONTROL_H

#include <QDBusPendingCall>
#include <QObject>
#include <QString>

class NotificationControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool DND READ GetDND NOTIFY DNDChanged)
public:
    explicit NotificationControl(QObject *parent = nullptr);
    ~NotificationControl() override = default;

    void SetDND(bool enabled);
    bool GetDND() const;

Q_SIGNALS:
    void DNDChanged(bool enabled);

private Q_SLOTS:
    void onNotificationDNDChanged(bool enabled);

private:
    Q_DISABLE_COPY(NotificationControl)
    void connectNotificationSignals();
    void asyncSetDND(bool enabled);
    void refreshDNDState();
    void updateDND(bool enabled);

    static QDBusPendingCall asyncCallNotificationsService(const QString &method, const QVariantList &args = {});

    bool m_isDND = false;
};

#endif // NOTIFICATION_CONTROL_H

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

#ifndef TEST_NOTIFICATION_CLIENT_H
#define TEST_NOTIFICATION_CLIENT_H

#include <QObject>
#include <QList>

#include "popup-notification.h"
#include "notification-close-reason.h"

namespace UkuiNotification {

class NotificationClient : public QObject
{
    Q_OBJECT
public:
    explicit NotificationClient(QObject *parent = nullptr);

    bool registerClient();
    void invokeAction(uint id, const QString &action);
    void closeNotification(uint id, NotificationCloseReason::CloseReason reason);

    static void reset();
    static QList<uint> closedIds();
    static QList<uint> invokedIds();
    static QStringList invokedActions();
    static void setRegisterClientResult(bool value);

Q_SIGNALS:
    void newNotification(const UkuiNotification::PopupNotification &notification);
    void notificationClosed(uint id, UkuiNotification::NotificationCloseReason::CloseReason closeReason);
};

} // namespace UkuiNotification

#endif

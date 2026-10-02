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

#ifndef TEST_NOTIFICATION_HELPER_H
#define TEST_NOTIFICATION_HELPER_H

#include <QObject>
#include <QString>

namespace Notify {

class Message
{
public:
    Message() = default;
    Message(QString id, QString appName, bool defaultActionResult);

    QString getID() const;
    QString getAppName() const;
    bool executeDefaultAction() const;

private:
    QString m_id;
    QString m_appName;
    bool m_defaultActionResult {true};
};

class NotificationHelper : public QObject
{
    Q_OBJECT
public:
    static NotificationHelper *instance();
    void emitNewMessage(Message &message);

Q_SIGNALS:
    void newMessage(Message &message);

private:
    explicit NotificationHelper(QObject *parent = nullptr);
};

} // namespace Notify

#endif

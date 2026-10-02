/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#ifndef SYSTEM_NOTIFICATION_H
#define SYSTEM_NOTIFICATION_H

#include <QObject>

class QDBusInterface;

class SystemNotification : public QObject
{
    Q_OBJECT
public:
    explicit SystemNotification(QObject *parent = nullptr);

    void sendMessage(const QString &text);

    // timeout :
    // -1 : 表示消息通知常驻,不会收纳进通知中心
    //  0 : 表示通知将直接进入侧边栏通知中心，不会弹窗
    // >0 : 表示通知弹窗持续时间
    void sendMessage(const QString &text, const QString &title, int timeout);

    void sendMessage(const QString &text, const QString &title, int timeout, const QStringList &actions);

private:
    QDBusInterface *m_interface;
};

#endif // SYSTEM_NOTIFICATION_H
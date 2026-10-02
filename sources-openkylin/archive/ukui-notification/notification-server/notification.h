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

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <memory>

namespace NotificationServer {

class NotificationPrivate;
class NotificationAction : public QObject
{
    Q_OBJECT
public:
    explicit NotificationAction(QObject* parent = nullptr);
    ~NotificationAction() = default;
    void active();

Q_SIGNALS:
    void actionActived();
};

class Notification : public std::enable_shared_from_this<Notification>
{
public:
    Notification();
    Notification& operator=(const Notification& other);
    ~Notification();

    QString appName() const;
    uint id() const;
    uint replaceId() const;
    QString appIcon() const;
    QString summary() const;
    QString body() const;
    QStringList actions() const;
    QVariantMap hints() const;
    int timeout() const;

    void setAppName(const QString &appName);
    void setId(uint id);
    void setReplaceId(uint replaceId);
    void setAppIcon(const QString &appIcon);
    void setSummary(const QString &summary);
    void setBody(const QString &body);
    void setActions(const QStringList &actions);
    void setHints(const QVariantMap &hints);
    void setTimeout(int timeout);

    NotificationAction* addAction(const QString& actionId, const QString& label);
    NotificationAction* findAction(const QString& actionId);
    void activeAction(const QString& actionId);
    void send();

private:
    NotificationPrivate* d{nullptr};
};

using NotificationPtr = std::shared_ptr<Notification>;

}

#endif // NOTIFICATION_H

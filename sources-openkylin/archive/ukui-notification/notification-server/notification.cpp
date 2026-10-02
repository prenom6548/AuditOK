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

#include <QVariantMap>
#include <QMap>
#include <QDebug>
#include "notification.h"
#include "notification-manager.h"
namespace NotificationServer{

class NotificationPrivate
{
public:
    NotificationPrivate() = default;
    ~NotificationPrivate();

    QString m_appName;
    uint m_id = 0;
    uint m_replaceId = 0;
    QString m_appIcon;
    QString m_summary;
    QString m_body;
    QStringList m_actions;
    QVariantMap m_hints;
    int m_timeout = -1;
    QMap<QString, NotificationAction*> m_notificationActions;
};
}
using namespace NotificationServer;

NotificationPrivate::~NotificationPrivate()
{
    qDeleteAll(m_notificationActions);
    m_notificationActions.clear();
}

NotificationAction::NotificationAction(QObject* parent)
    : QObject(parent)
{

}

void NotificationAction::active()
{
    Q_EMIT actionActived();
}

Notification::Notification()
    : d(new NotificationPrivate)
{

}

Notification::~Notification()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

uint Notification::id() const
{
    return d->m_id;
}

void Notification::setId(uint id)
{
    d->m_id = id;
}

uint Notification::replaceId() const
{
    return d->m_replaceId;
}

void Notification::setReplaceId(uint replaceId)
{
    d->m_replaceId = replaceId;
}

QString Notification::appName() const
{
    return d->m_appName;
}

void Notification::setAppName(const QString &appName)
{
    d->m_appName = appName;
}

QString Notification::appIcon() const
{
    return d->m_appIcon;
}

void Notification::setAppIcon(const QString &appIcon)
{
    d->m_appIcon = appIcon;
}

QString Notification::summary() const
{
    return d->m_summary;
}

void Notification::setSummary(const QString &summary)
{
    d->m_summary = summary;
}

QString Notification::body() const
{
    return d->m_body;
}

void Notification::setBody(const QString &body)
{
    d->m_body = body;
}

QStringList Notification::actions() const
{
    return d->m_actions;
}

void Notification::setActions(const QStringList &actions)
{
    if (actions.count() % 2 != 0) {
        qWarning() << "List of actions must contain an even number of items, tried to set actions to" << actions;
        return;
    }

    for (int i = 0; i < actions.count(); i += 2) {
        const QString &actionId = actions.at(i);
        const QString &label = actions.at(i + 1);
        addAction(actionId, label);
    }
}

QVariantMap Notification::hints() const
{
    return d->m_hints;
}

void Notification::setHints(const QVariantMap &hints)
{
    d->m_hints = hints;
}

int Notification::timeout() const
{
    return d->m_timeout;
}

void Notification::setTimeout(int timeout)
{
    d->m_timeout = timeout;
}

NotificationAction* Notification::addAction(const QString& actionId, const QString &label)
{
    d->m_actions << actionId << label;
    NotificationAction* action = new NotificationAction;
    d->m_notificationActions.insert(actionId, action);
    return action;
}

NotificationAction *Notification::findAction(const QString &actionId)
{
    return d->m_notificationActions[actionId];
}

void Notification::activeAction(const QString &actionId)
{
    auto* action = d->m_notificationActions[actionId];
    if (action) {
        action->active();
    }
}

void Notification::send()
{
    NotificationManager::self().notify(shared_from_this());
}

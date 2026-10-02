/*
 * Copyright © 2017 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_UKUI_REQUEST_H
#define XDG_DESKTOP_PORTAL_UKUI_REQUEST_H

#include <QDBusVirtualObject>
#include <QObject>

class QDBusObjectPath;

class Request : public QDBusVirtualObject
{
    Q_OBJECT
public:
    explicit Request(const QDBusObjectPath &handle, QObject *parent = nullptr, const QString &portalName = QString(), const QVariant &data = QVariant());
    ~Request() override;

    bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override;
    QString introspect(const QString &path) const override;

    template<class T>
    static Request *makeClosableDialogRequest(const QDBusObjectPath &handle, T *dialogAndParent)
    {
        auto request = new Request(handle, dialogAndParent);
        connect(request, &Request::closeRequested, dialogAndParent, &T::reject);
        return request;
    }

Q_SIGNALS:
    void closeRequested();

private:
    const QVariant m_data;
    const QString m_portalName;
};

#endif // REQUEST_H

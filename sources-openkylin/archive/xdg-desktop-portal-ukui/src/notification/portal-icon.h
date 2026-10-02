/*
 * SPDX-FileCopyrightText: 2016 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 * SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>
 */

#ifndef XDG_DESKTOP_PORTAL_UKUI_PORTAL_ICON_H
#define XDG_DESKTOP_PORTAL_UKUI_PORTAL_ICON_H


#include <QDBusArgument>
#include <QDBusMetaType>

struct PortalIcon {
    QString str;
    QDBusVariant data;

    static auto registerDBusType()
    {
        return qDBusRegisterMetaType<PortalIcon>();
    }
};

QDBusArgument &operator<<(QDBusArgument &argument, const PortalIcon &icon);
const QDBusArgument &operator>>(const QDBusArgument &argument, PortalIcon &icon);

Q_DECLARE_METATYPE(PortalIcon)


#endif //XDG_DESKTOP_PORTAL_UKUI_PORTAL_ICON_H

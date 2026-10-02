// Copyright (C) 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef KMRE_WAYLAND_XDG_ACTIVATION_V1
#define KMRE_WAYLAND_XDG_ACTIVATION_V1

#include <QObject>
#include <QByteArray>
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)

#include "qwayland-xdg-activation-v1.h"

#include <QtWaylandClient/qtwaylandclientglobal.h>

namespace QtWaylandClient {
class QWaylandDisplay;
}

namespace kmre {
namespace wayland {

class XdgActivationTokenV1 : public QObject, public QtWayland::xdg_activation_token_v1
{
    Q_OBJECT
public:
    ~XdgActivationTokenV1() override;
    void xdg_activation_token_v1_done(const QString &token) override { Q_EMIT done(token); }

Q_SIGNALS:
    void done(const QString &token);
};

class XdgActivationV1 : public QtWayland::xdg_activation_v1
{
public:
    XdgActivationV1(struct ::wl_registry *registry, uint32_t id, uint32_t availableVersion);
    ~XdgActivationV1() override;

    XdgActivationTokenV1* requestXdgActivationToken(QtWaylandClient::QWaylandDisplay *display,
            struct ::wl_surface *surface,
            uint32_t serial,
            const QString &app_id);
};

}
}

#endif
#endif

#endif

// Copyright (C) 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "XdgActivationV1.h"

#include <QByteArray>
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>

namespace kmre {
namespace wayland {
XdgActivationTokenV1::~XdgActivationTokenV1()
{
    if (isInitialized()) {
        destroy();
    }
}

XdgActivationV1::XdgActivationV1(wl_registry *registry,
        uint32_t id,
        uint32_t availableVersion)
    : QtWayland::xdg_activation_v1(registry, id, qMin(availableVersion, 1u))
{
}

XdgActivationV1::~XdgActivationV1()
{
    if (isInitialized()) {
        destroy();
    }
}

XdgActivationTokenV1* XdgActivationV1::requestXdgActivationToken(QtWaylandClient::QWaylandDisplay *display,
        struct ::wl_surface *surface,
        uint32_t serial,
        const QString &app_id)
{
    auto wl = get_activation_token();
    auto provider = new XdgActivationTokenV1;
    provider->init(wl);

    if (surface) {
        provider->set_surface(surface);
    }

    if (!app_id.isEmpty()) {
        provider->set_app_id(app_id);
    }

    if (serial && display->lastInputDevice()) {
        provider->set_serial(serial, display->lastInputDevice()->wl_seat());
    }

    provider->commit();
    return provider;
}

}
}

#endif
#endif

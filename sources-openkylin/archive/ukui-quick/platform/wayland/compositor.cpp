/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2025 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "compositor.h"
#include <wayland-client-protocol.h>
#include "wayland-pointer_p.h"
#include <qpa/qplatformnativeinterface.h>
#include <QGuiApplication>

namespace UkuiQuick::WaylandClient {
class Q_DECL_HIDDEN Compositor::Private
{
public:
    Private() = default;

    WaylandPointer<wl_compositor, wl_compositor_destroy> compositor;
    EventQueue *queue = nullptr;
};
Compositor::Compositor(QObject* parent): QObject(parent)
    , d(new Private)
{
}

Compositor::~Compositor()
{
    release();
}

Compositor* Compositor::fromApplication(QObject* parent)
{
    QPlatformNativeInterface *native = qGuiApp->platformNativeInterface();
    if (!native) {
        return nullptr;
    }
    wl_compositor *compositor = static_cast<wl_compositor *>(native->nativeResourceForIntegration(QByteArrayLiteral("compositor")));
    if (!compositor) {
        return nullptr;
    }
    auto *c = new Compositor(parent);
    c->d->compositor.setup(compositor, true);
    return c;
}

bool Compositor::isValid() const
{
    return d->compositor.isValid();
}

void Compositor::setup(wl_compositor* compositor)
{
    Q_ASSERT(compositor);
    Q_ASSERT(!d->compositor);
    d->compositor.setup(compositor);
}

void Compositor::release()
{
    d->compositor.release();
}

void Compositor::destroy()
{
    d->compositor.destroy();
}

void Compositor::setEventQueue(EventQueue* queue)
{
    d->queue = queue;
}

EventQueue* Compositor::eventQueue()
{
    return d->queue;
}

wl_surface* Compositor::createSurface()
{
    if (!isValid()) {
        return nullptr;
    }
    return wl_compositor_create_surface(d->compositor);
}

wl_region* Compositor::createRegion()
{
    if (!isValid()) {
        return nullptr;
    }
    return wl_compositor_create_region(d->compositor);
}
}

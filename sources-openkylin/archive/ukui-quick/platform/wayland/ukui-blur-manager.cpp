/*
*  * Copyright (C) 2025, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "ukui-blur-manager.h"

#include <compositor.h>

#include "wayland-integration_p.h"
#include "wayland-pointer_p.h"
#include "wayland-ukui-blur-v1-client-protocol.h"
#include <QDebug>

namespace UkuiQuick::WaylandClient {
class Q_DECL_HIDDEN UkuiBlurManager::Private
{
public:
    Private() = default;

    WaylandPointer<ukui_blur_manager_v1, ukui_blur_manager_v1_destroy> manager;
    EventQueue *queue = nullptr;
};
UkuiBlurManager::UkuiBlurManager(QObject* parent): QObject(parent), d(new Private)
{
}

UkuiBlurManager::~UkuiBlurManager()
{
    release();
}

bool UkuiBlurManager::isValid() const
{
    return d->manager.isValid();
}

void UkuiBlurManager::setup(ukui_blur_manager_v1* manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager);
    d->manager.setup(manager);
}

void UkuiBlurManager::release()
{
    d->manager.release();
}

void UkuiBlurManager::destroy()
{
    d->manager.destroy();
}

void UkuiBlurManager::setEventQueue(EventQueue* queue)
{
    d->queue = queue;
}

EventQueue* UkuiBlurManager::eventQueue()
{
    return d->queue;
}

UkuiBlur* UkuiBlurManager::getBlur(KWayland::Client::Surface *surface, QObject *parent)
{
    if (!isValid() || !surface) {
        return nullptr;
    }
    auto blur = new UkuiBlur(parent);
    auto w = ukui_blur_manager_v1_get_blur(d->manager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    blur->setup(w);
    return blur;
}

class UkuiBlur::Private
{
public:
    WaylandPointer<ukui_blur_surface_v1, ukui_blur_surface_v1_destroy> blur;
};

UkuiBlur::UkuiBlur(QObject* parent): QObject(parent)
    , d(new Private)
{
}

UkuiBlur::~UkuiBlur()
{
    release();
}

void UkuiBlur::setup(ukui_blur_surface_v1* blur)
{
    Q_ASSERT(blur);
    Q_ASSERT(!d->blur);
    d->blur.setup(blur);
}

void UkuiBlur::release()
{
    d->blur.release();
}

void UkuiBlur::destroy()
{
    d->blur.destroy();
}

bool UkuiBlur::isValid() const
{
    return d->blur.isValid();
}

void UkuiBlur::setRegion(QRegion region)
{
    auto wlRegion = WaylandIntegration::self()->waylandCompositor()->createRegion();

    for (const QRect &rect : region) {
        wl_region_add(wlRegion, rect.x(), rect.y(), rect.width(), rect.height());
    }

    ukui_blur_surface_v1_set_region(d->blur, wlRegion);
}

void UkuiBlur::setLevel(uint32_t level)
{
    ukui_blur_surface_v1_set_level(d->blur, level);
}
void UkuiBlur::setOpacity(const qreal opacity) const
{
    if (ukui_blur_surface_v1_get_version(d->blur) >= UKUI_BLUR_SURFACE_V1_SET_OPACITY_SINCE_VERSION) {
        ukui_blur_surface_v1_set_opacity(d->blur, wl_fixed_from_double(opacity));
    } else {
        qWarning() << "UkuiBlur::setOpacity: Blur surface protocol version too low, opacity setting not supported. "
                   << "Current version:" << ukui_blur_surface_v1_get_version(d->blur)
                   << ", required version:" << UKUI_BLUR_SURFACE_V1_SET_OPACITY_SINCE_VERSION;
    }
}

void UkuiBlur::setPreferredType(uint32_t type) const
{
    if (ukui_blur_surface_v1_get_version(d->blur) >= UKUI_BLUR_SURFACE_V1_SET_PREFERRED_TYPE_SINCE_VERSION) {
        ukui_blur_surface_v1_set_preferred_type(d->blur, type);
    } else {
        qWarning() << "UkuiBlur::setPreferredType: Blur surface protocol version too low, preferred type setting not supported. "
                   << "Current version:" << ukui_blur_surface_v1_get_version(d->blur)
                   << ", required version:" << UKUI_BLUR_SURFACE_V1_SET_PREFERRED_TYPE_SINCE_VERSION;
    }
}
}

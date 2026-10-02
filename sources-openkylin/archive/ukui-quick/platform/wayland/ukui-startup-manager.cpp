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

#include "ukui-startup-manager.h"
#include "wayland-pointer_p.h"
#include <QDebug>

#include "wayland-ukui-startup-v1-client-protocol.h"

namespace UkuiQuick::WaylandClient {
class Q_DECL_HIDDEN UkuiStartupManagement::Private
{
public:
    Private(UkuiStartupManagement *q);

    WaylandPointer<ukui_startup_management_v1, ukui_startup_management_v1_destroy> manager;
    EventQueue *queue = nullptr;
    UkuiStartupManagement *q;
};

UkuiStartupManagement::Private::Private(UkuiStartupManagement* q): q(q)
{
}

UkuiStartupManagement::UkuiStartupManagement(QObject* parent): QObject(parent)
                                                         , d(new Private(this))
{
}

UkuiStartupManagement::~UkuiStartupManagement()
{
    release();
}

bool UkuiStartupManagement::isValid() const
{
    return d->manager.isValid();
}

void UkuiStartupManagement::release()
{
    if (!d->manager) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->manager.release();
}

void UkuiStartupManagement::destroy()
{
    if (!d->manager) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->manager.destroy();
}

void UkuiStartupManagement::setup(ukui_startup_management_v1* shell) const
{
    Q_ASSERT(!d->manager);
    Q_ASSERT(shell);
    d->manager.setup(shell);
}

void UkuiStartupManagement::setEventQueue(EventQueue* queue) const
{
    d->queue = queue;
}

EventQueue* UkuiStartupManagement::eventQueue() const
{
    return d->queue;
}

void UkuiStartupManagement::setStartupGeometry(uint32_t pid, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    if (!d->manager) {
        return;
    }
    if (width <= 0 || height <= 0) {
        return;
    }
    ukui_startup_management_v1_set_startup_geometry(d->manager, pid, x, y, width, height);
}
}

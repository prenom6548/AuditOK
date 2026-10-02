/*
 * libkysdk-waylandhelper's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "ukuiblur.h"
#include "ukuiwaylandpointer.h"

class Q_DECL_HIDDEN UkuiBlur::Private
{
public:
    UkuiWaylandPointer<ukui_blur_surface_v1, ukui_blur_surface_v1_destroy> blur;
};

class Q_DECL_HIDDEN UkuiBlurManager::Private
{
public:
    Private(UkuiBlurManager *q);

    UkuiWaylandPointer<ukui_blur_manager_v1, ukui_blur_manager_v1_destroy> blurManager;
    EventQueue *queue = nullptr;

private:
    UkuiBlurManager *q;
};

UkuiBlur::UkuiBlur(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

UkuiBlur::~UkuiBlur()
{
    release();
}

bool UkuiBlur::isValid() const
{
    return d->blur.isValid();
}

void UkuiBlur::release()
{
    d->blur.release();
}

void UkuiBlur::destroy()
{
    d->blur.destroy();
}

void UkuiBlur::setup(ukui_blur_surface_v1 *blur)
{
    Q_ASSERT(blur);
    Q_ASSERT(!d->blur);
    d->blur.setup(blur);
}

void UkuiBlur::setRegion(Region *region)
{
    ukui_blur_surface_v1_set_region(d->blur, *region);
}

void UkuiBlur::setStrength(uint32_t strength)
{
    ukui_blur_surface_v1_set_level(d->blur, strength);
}

UkuiBlur::operator ukui_blur_surface_v1 *()
{
    return d->blur;
}

UkuiBlur::operator ukui_blur_surface_v1 *() const
{
    return d->blur;
}

UkuiBlurManager::UkuiBlurManager(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

UkuiBlurManager::~UkuiBlurManager()
{
    release();
}

bool UkuiBlurManager::isValid() const
{
    if (!d->blurManager)
        return false;
    return d->blurManager.isValid();
}

void UkuiBlurManager::release()
{
    d->blurManager.release();
}

void UkuiBlurManager::destroy()
{
    d->blurManager.destroy();
}

void UkuiBlurManager::setup(ukui_blur_manager_v1 *blur_manager)
{
    d->blurManager.setup(blur_manager);
}

void UkuiBlurManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *UkuiBlurManager::eventQueue()
{
    return d->queue;
}

UkuiBlur *UkuiBlurManager::createBlur(Surface *surface, QObject *parent)
{
    if (!isValid())
        return nullptr;
    UkuiBlur *s = new UkuiBlur(parent);
    auto w = ukui_blur_manager_v1_get_blur(d->blurManager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

void UkuiBlurManager::removeBlur(Surface *surface)
{
    Q_ASSERT(isValid());
    //    ukui_blur_manager_unset(d->blurManager, *surface);
}

UkuiBlurManager::operator ukui_blur_manager_v1 *()
{
    return d->blurManager;
}

UkuiBlurManager::operator ukui_blur_manager_v1 *() const
{
    return d->blurManager;
}

UkuiBlurManager::Private::Private(UkuiBlurManager *q)
    : q(q)
{
}

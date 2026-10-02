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

#ifndef UKUIBLUR_H
#define UKUIBLUR_H

#include "ukui-blur-manager-client-protocol.h"
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/region.h>
#include <KWayland/Client/surface.h>
#include <QObject>

struct ukui_blur_manager_v1;
struct ukui_blur_surface_v1;
using namespace KWayland::Client;

/**
 * @since 2.5.1.0
 */

class UkuiBlur : public QObject
{
    Q_OBJECT
public:
    explicit UkuiBlur(QObject *parent = nullptr);
    ~UkuiBlur();

    bool isValid() const;

    void release();

    void destroy();

    void setup(ukui_blur_surface_v1 *blur);

    void setRegion(Region *region);

    void setStrength(uint32_t strength);

    operator ukui_blur_surface_v1 *();
    operator ukui_blur_surface_v1 *() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

class UkuiBlurManager : public QObject
{
    Q_OBJECT
public:
    explicit UkuiBlurManager(QObject *parent = nullptr);
    ~UkuiBlurManager();

    bool isValid() const;

    void release();

    void destroy();

    void setup(ukui_blur_manager_v1 *blur_manager);

    void setEventQueue(EventQueue *queue);

    EventQueue *eventQueue();

    UkuiBlur *createBlur(Surface *surface, QObject *parent = nullptr);

    void removeBlur(Surface *surface);

    operator ukui_blur_manager_v1 *();

    operator ukui_blur_manager_v1 *() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // UKUIBLUR_H

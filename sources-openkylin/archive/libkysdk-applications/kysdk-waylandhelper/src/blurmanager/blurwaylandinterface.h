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

#ifndef BLURWAYLANDINTERFACE_H
#define BLURWAYLANDINTERFACE_H

#include "blurabstractinterface.h"
#include "ukuiblur.h"
#include <KWayland/Client/compositor.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/output.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <QApplication>
#include <QObject>
#include <QRegion>

/**
 * @since 2.5.1.0
 */

using namespace KWayland::Client;

class BlurWaylandInterface : public BlurAbstractInterface
{
    Q_OBJECT

public:
    explicit BlurWaylandInterface(QObject *parent = nullptr);

    bool setBlurBehindWithStrength(QWindow *window, bool enable = true, const QRegion &region = QRegion(), uint32_t strength = -1) override;

    void handleGlobal(void *data, struct wl_registry *registry,
                      uint32_t name, const char *interface, uint32_t version);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    BlurWaylandInterface *q_ptr;
    ConnectionThread *m_connection = nullptr;
    Compositor *m_compositor = nullptr;
    Registry *m_registry = nullptr;
    UkuiBlur *m_blur = nullptr;
    UkuiBlurManager *m_blur_manager = nullptr;
    QMap<QWindow *, Surface *> m_surfaces;
    QMap<QWindow *, Region *> m_regions;
    QMap<QWindow *, UkuiBlur *> m_blurs;
    static const struct wl_registry_listener s_registryListener;
};

#endif // BLURWAYLANDINTERFACE_H

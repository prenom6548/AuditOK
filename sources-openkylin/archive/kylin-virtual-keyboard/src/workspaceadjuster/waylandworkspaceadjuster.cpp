/*
 * Copyright (c) KylinSoft Co., Ltd. 2025.All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "waylandworkspaceadjuster.h"

#include <QGuiApplication>
#include <QScreen>
#include <QVariant>

#include "../log.h"
#include "ukuiwaylandhelper/ukuiwaylandproperties.h"

WaylandWlcomWorkspaceAdjuster::WaylandWlcomWorkspaceAdjuster() {
    KVKBD_INFO("wayland wlcom workspace adjuster.");
}

void WaylandWlcomWorkspaceAdjuster::raiseInputArea(QWindow *window,
                                                   const QRect &rect) {
    KVKBD_INFO("raise inputarea.");
    if (window == nullptr) {
        KVKBD_WARN("window is null, skip raiseInputArea.");
        return;
    }
    surfaceWindow_ = window;
    setSurfaceWindowProperty(rect, 1);
}

void WaylandWlcomWorkspaceAdjuster::fallInputArea() {
    KVKBD_INFO("fall inputarea.");
    if (surfaceWindow_ == nullptr) {
        KVKBD_WARN("surfaceWindow_ is null, skip fallInputArea.");
        return;
    }
    setSurfaceWindowProperty(QRect(), 0);
}

void WaylandWlcomWorkspaceAdjuster::setSurfaceWindowProperty(
    const QRect &rect, const int32_t &enabled) {
    if (surfaceWindow_ == nullptr) {
        KVKBD_WARN("surfaceWindow_ is null, skip ukui_surface_anchor.");
        return;
    }
    QScreen *screen = surfaceWindow_->screen();
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }

    UkuiWaylandProperty::SurfaceProperty sp;
    sp.width = static_cast<int32_t>(rect.width());
    sp.height = static_cast<int32_t>(rect.height());
    sp.exclusive = static_cast<int32_t>(rect.height());
    sp.area = 1;
    sp.enabled = enabled;

    KVKBD_DEBUG("surface anchor vector: w:{} h:{} anchor:{} area:{} excl:{} "
                "res:{} mT:{} mR:{} mB:{} mL:{} on:{}",
                sp.width, sp.height, sp.anchor, sp.area, sp.exclusive,
                sp.reserved, sp.margin_top, sp.margin_right, sp.margin_bottom,
                sp.margin_left, sp.enabled);

    const QPair<QScreen *, QVector<int32_t>> pair(screen, sp.toVector());
    surfaceWindow_->setProperty(UkuiWaylandProperty::SURFACE_ANCHOR,
                                QVariant::fromValue(pair));
}

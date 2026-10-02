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

#include "blurxcbinterface.h"
#include <QApplication>
#include <QX11Info>

BlurXcbInterface::BlurXcbInterface(QObject *parent)
    : BlurAbstractInterface(parent)
{
}

bool BlurXcbInterface::setBlurBehindWithStrength(QWindow *window, bool enable, const QRegion &region, uint32_t strength)
{
    xcb_connection_t *c = QX11Info::connection();
    if (!c) {
        return false;
    }
    const QByteArray effectName = QByteArrayLiteral("_KDE_NET_WM_BLUR_BEHIND_REGION");
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());
    QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));
    if (!atom) {
        return false;
    }

    if (enable) {
        QVector<uint32_t> data;
        data.reserve(region.rectCount() * 4 + 1);
        for (const QRect &r : region) {
            auto dpr = qApp->devicePixelRatio();
            data << r.x() * dpr << r.y() * dpr << r.width() * dpr << r.height() * dpr;
        }
        data << strength;

        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->winId(), atom->atom, XCB_ATOM_CARDINAL, 32, data.size(), data.constData());
        return true;
    } else {
        xcb_delete_property(c, window->winId(), atom->atom);
        xcb_flush(c);
        return true;
    }
}

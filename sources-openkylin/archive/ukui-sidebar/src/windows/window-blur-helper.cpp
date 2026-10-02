/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "window-blur-helper.h"
#include <xcb/xcb.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#include <QPainterPath>
#include <QApplication>
#include <QDebug>
#include <QPainterPath>

// 窗管特效接口
#include <KWindowEffects>
#include <KWindowSystem>

using namespace Sidebar;
using namespace UkuiQuick;
WindowBlurHelper::WindowBlurHelper(QObject *parent) : QObject(parent)
{
}

void WindowBlurHelper::setWindowProxy(WindowProxy *windowProxy)
{
    m_windowProxy = windowProxy;
}

void WindowBlurHelper::setEnable(bool enable)
{
    if (m_enable == enable) {
        return;
    }

    m_enable = enable;
    updateBlurArea();
}

void WindowBlurHelper::setRadius(double radius)
{
    if (m_radius == radius) {
        return;
    }

    if (radius > 1) {
        m_radius = 1;
    } else {
        m_radius = radius;
    }

    updateBlurArea();
    Q_EMIT radiusChanged();
}

void WindowBlurHelper::setRegion(qreal x, qreal y, qreal w, qreal h, qreal radius)
{
    QPainterPath path;
    path.addRoundedRect(x, y, w, h, radius, radius);
    m_region = QRegion(path.toFillPolygon().toPolygon());

    updateBlurArea();
}

void WindowBlurHelper::enableBlurBehind(WId window, bool enable, const QRegion &region, uint32_t radius)
{
    xcb_connection_t *c = QX11Info::connection();
    if (!c) {
        return;
    }
    const QByteArray effectName = QByteArrayLiteral("_KDE_NET_WM_BLUR_BEHIND_REGION");
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());
    QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));
    if (!atom) {
        return;
    }

    if (enable) {
        QVector<uint32_t> data;
        data.reserve(region.rectCount() * 4 + 1); // 1 means radius
        for (const QRect &r : region) {
            // kwin on X uses device pixels, convert from logical
            auto dpr = qApp->devicePixelRatio();
            data << r.x() * dpr << r.y() * dpr << r.width() * dpr << r.height() * dpr;
        }
        data << radius;

        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window, atom->atom, XCB_ATOM_CARDINAL, 32, data.size(), data.constData());
    } else {
        xcb_delete_property(c, window, atom->atom);
        xcb_flush(c);
    }
}

bool WindowBlurHelper::enable()
{
    return m_enable;
}

double WindowBlurHelper::radius()
{
    return m_radius;
}

bool WindowBlurHelper::updateBlurArea()
{
    if (!m_windowProxy) {
        qWarning() << "WindowBlurHelper::updateBlurArea: windowProxy in not init.";
        return false;
    }

    bool enable = (m_enable && !m_region.isEmpty());
    if (enable) {
        m_windowProxy->setBlurOpacity(m_radius);
        m_windowProxy->setBlurRegion(enable, m_region);
    } else {
        m_windowProxy->setBlurRegion(enable);
        m_windowProxy->setBlurOpacity(0);
    }
    return enable;
}

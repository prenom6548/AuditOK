/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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

#define private public
#include "../../../../src/windows/window-blur-helper.h"
#undef private

using namespace Sidebar;

WindowBlurHelper::WindowBlurHelper(QObject *parent) : QObject(parent)
{
}

void WindowBlurHelper::setWindowProxy(UkuiQuick::WindowProxy *windowProxy)
{
    m_windowProxy = windowProxy;
}

bool WindowBlurHelper::enable()
{
    return m_enable;
}

void WindowBlurHelper::setEnable(bool enable)
{
    if (m_enable == enable) {
        return;
    }
    m_enable = enable;
    updateBlurArea();
}

double WindowBlurHelper::radius()
{
    return m_radius;
}

void WindowBlurHelper::setRadius(double radius)
{
    if (qFuzzyCompare(m_radius, radius)) {
        return;
    }
    m_radius = radius > 1 ? 1 : radius;
    updateBlurArea();
    Q_EMIT radiusChanged();
}

void WindowBlurHelper::setRegion(qreal x, qreal y, qreal w, qreal h, qreal radius)
{
    Q_UNUSED(radius)
    m_region = QRegion(QRect(qRound(x), qRound(y), qRound(w), qRound(h)));
    updateBlurArea();
}

bool WindowBlurHelper::updateBlurArea()
{
    if (!m_windowProxy) {
        return false;
    }

    const bool enabled = m_enable && !m_region.isEmpty();
    m_windowProxy->setBlurOpacity(enabled ? m_radius : 0);
    m_windowProxy->setBlurRegion(enabled, m_region);
    return enabled;
}

void WindowBlurHelper::enableBlurBehind(WId window, bool enable, const QRegion &region, uint32_t radius)
{
    Q_UNUSED(window)
    Q_UNUSED(enable)
    Q_UNUSED(region)
    Q_UNUSED(radius)
}

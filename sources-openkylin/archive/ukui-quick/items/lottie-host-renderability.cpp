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
 */

#include "lottie-host-renderability_p.h"

#include "lottie-attached.h"
#include "lottie-renderer.h"
#include "ukui/dt-theme.h"

#include <QQuickItem>
#include <QQuickWindow>
#include <QtMath>

#include <limits>
#include <utility>

namespace UkuiQuick {

namespace {

bool pixelDimension(qreal logicalDimension, qreal devicePixelRatio, int *result)
{
    const qreal scaledDimension = logicalDimension * devicePixelRatio;
    const qreal maximumRoundableDimension =
        static_cast<qreal>(std::numeric_limits<int>::max()) - 0.5;
    if (!qIsFinite(logicalDimension) || logicalDimension <= 0.0 ||
        !qIsFinite(scaledDimension) || scaledDimension <= 0.0 ||
        scaledDimension > maximumRoundableDimension) {
        return false;
    }
    *result = qMax(1, qRound(scaledDimension));
    return true;
}

bool renderTargetSize(const QSizeF &logicalSize, qreal devicePixelRatio, QSize *result)
{
    if (!qIsFinite(devicePixelRatio) || devicePixelRatio <= 0.0) {
        return false;
    }

    int width = 0;
    int height = 0;
    if (!pixelDimension(logicalSize.width(), devicePixelRatio, &width) ||
        !pixelDimension(logicalSize.height(), devicePixelRatio, &height)) {
        return false;
    }

    const QSize candidate(width, height);
    if (!LottieRenderer::isRenderSizeAllowed(candidate)) {
        return false;
    }
    *result = candidate;
    return true;
}

} // namespace

QString automaticLottieThemeCandidate(bool highlighted)
{
    return highlighted || ThemeGlobalConfig::instance()->isDarkTheme()
        ? QStringLiteral("dark")
        : QStringLiteral("light");
}

LottieHostRenderabilityTracker::LottieHostRenderabilityTracker(
    QQuickItem *host, LottieAttached *controller)
    : m_host(host)
    , m_controller(controller)
{
    Q_ASSERT(host);
    Q_ASSERT(controller);

    QObject::connect(host, &QQuickItem::windowChanged, this,
                     [this](QQuickWindow *) {
        rebuildConnections();
        sync();
    });
    rebuildConnections();
    sync();
}

bool LottieHostRenderabilityTracker::renderSizeValid() const
{
    return m_renderSizeValid;
}

void LottieHostRenderabilityTracker::sync()
{
    syncForWindow(m_host ? m_host->window() : nullptr);
}

void LottieHostRenderabilityTracker::syncForWindow(QQuickWindow *window)
{
    if (!m_host || !m_controller) {
        m_renderSizeValid = false;
        return;
    }

    QSize targetSize;
    m_renderSizeValid = window &&
        renderTargetSize(m_host->size(), window->devicePixelRatio(), &targetSize);
    // 可见性决定隐藏播放策略；尺寸只决定现在能不能栅格化。把两者分开后，
    // 临时的零尺寸或超限尺寸只会暂停渲染，不会被误认为一次隐藏/显示切换。
    const bool effectivelyVisible = window && window->isVisible() &&
        window->visibility() != QWindow::Minimized &&
        !window->windowStates().testFlag(Qt::WindowMinimized) &&
        itemAndAncestorsAreVisible();
    if (!m_renderSizeValid) {
        targetSize = {};
    }
    if (effectivelyVisible) {
        // 恢复时先给出可用尺寸，再结算隐藏时间；这样追到的目标帧可以直接
        // 请求，也不会被紧随其后的尺寸变化清掉帧内余数。
        m_controller->setHostRenderSize(targetSize);
        m_controller->setSceneActive(true);
    } else {
        // 隐藏或 detach 时顺序相反：先保存当前帧已走过的时间，再清掉可能
        // 已经失效的窗口尺寸。
        m_controller->setSceneActive(false);
        m_controller->setHostRenderSize(targetSize);
    }
}

bool LottieHostRenderabilityTracker::itemAndAncestorsAreVisible() const
{
    for (QQuickItem *item = m_host; item; item = item->parentItem()) {
        if (!item->isVisible()) {
            return false;
        }
    }
    return true;
}

void LottieHostRenderabilityTracker::rebuildConnections()
{
    for (const QMetaObject::Connection &connection : std::as_const(m_connections)) {
        QObject::disconnect(connection);
    }
    m_connections.clear();

    for (QQuickItem *item = m_host; item; item = item->parentItem()) {
        m_connections.append(QObject::connect(
            item, &QQuickItem::visibleChanged, this, [this] { sync(); }));
        m_connections.append(QObject::connect(
            item, &QQuickItem::parentChanged, this,
            [this](QQuickItem *) {
                rebuildConnections();
                sync();
            }));
    }

    if (m_host && m_host->window()) {
        QQuickWindow *window = m_host->window();
        m_connections.append(QObject::connect(
            window, &QWindow::visibleChanged, this, [this](bool) { sync(); }));
        m_connections.append(QObject::connect(
            window, &QWindow::visibilityChanged, this,
            [this](QWindow::Visibility) { sync(); }));
        m_connections.append(QObject::connect(
            window, &QWindow::windowStateChanged, this,
            [this](Qt::WindowState) { sync(); }));
    }
}

} // namespace UkuiQuick

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

#ifndef UKUI_QUICK_ITEMS_LOTTIE_HOST_RENDERABILITY_P_H
#define UKUI_QUICK_ITEMS_LOTTIE_HOST_RENDERABILITY_P_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>

class QQuickItem;
class QQuickWindow;

namespace UkuiQuick {

class LottieAttached;

// 普通状态跟随系统背景；高亮状态使用适合品牌色等深色背景的 dark 主题。
// 这里的 light/dark 描述背景兼容性，不是图形本身的颜色。
QString automaticLottieThemeCandidate(bool highlighted);

// Icon 和 LottieItem 对“现在能不能提交一帧”使用完全相同的规则。
// 这个小对象只负责观察尺寸、窗口和祖先可见性；来源、主题以及帧落到
// 哪个 Host，仍由各自的 adapter 处理。
class LottieHostRenderabilityTracker final : public QObject
{
public:
    LottieHostRenderabilityTracker(QQuickItem *host, LottieAttached *controller);

    bool renderSizeValid() const;
    void sync();
    void syncForWindow(QQuickWindow *window);

private:
    bool itemAndAncestorsAreVisible() const;
    void rebuildConnections();

    QPointer<QQuickItem> m_host;
    QPointer<LottieAttached> m_controller;
    QVector<QMetaObject::Connection> m_connections;
    bool m_renderSizeValid = false;
};

} // namespace UkuiQuick

#endif // UKUI_QUICK_ITEMS_LOTTIE_HOST_RENDERABILITY_P_H

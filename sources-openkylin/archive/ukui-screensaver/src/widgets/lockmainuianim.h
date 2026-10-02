/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * 锁屏主界面进/退场动效与 Mockplus 对齐的常量（时长、贝塞尔、位移基数等）。
 */
#ifndef LOCKMAINUIANIM_H
#define LOCKMAINUIANIM_H

#include <QEasingCurve>
#include <QPointF>
#include <QtGlobal>

namespace LockMainUiAnim {

constexpr int kDurationMs = 400;

/** 毛玻璃遮罩进/退场时长（ms），与摹客表「毛玻璃 400」一致 */
constexpr int kFrostedGlassTransitionMs = kDurationMs;

/** 锁屏→登录：动态壁纸首帧缩放时长（ms），表为 200 */
constexpr int kWallpaperEnterScaleDurationMs = 200;
/** 登录→桌面：动态壁纸首帧缩放时长（ms），表为 260 */
constexpr int kWallpaperUnlockScaleDurationMs = 260;

/** 登录→桌面：头像/名称/输入退场放大（设计稿视频约 1.2；登录选项不参与） */
constexpr qreal kAuthExitScaleEnd = 1.2;
constexpr int kAuthExitScaleDurationMs = kDurationMs;

/** 认证区头像/名称/输入：进场错开起播（ms），摹客进场表 */
constexpr int kAuthEnterNameDelayMs = 124;
constexpr int kAuthEnterEditDelayMs = 148;

/** 解锁动画墙钟（ms）：登录→桌面 UI 与毛玻璃均为 400ms 并行 */
constexpr int kUnlockWallClockMs = kDurationMs;

inline QEasingCurve exitEasingCurve()
{
    QEasingCurve curve(QEasingCurve::BezierSpline);
    curve.addCubicBezierSegment(QPointF(0.0, 0.0), QPointF(0.15, 1.0), QPointF(1.0, 1.0));
    return curve;
}

inline QEasingCurve enterEasingCurve()
{
    QEasingCurve curve(QEasingCurve::BezierSpline);
    curve.addCubicBezierSegment(QPointF(0.28, 0.0), QPointF(1.0, 1.0), QPointF(1.0, 1.0));
    return curve;
}

/** 入场：相对目标位置的位移基数（再乘 LockWidget::scale） */
constexpr int kEnterTopOffsetBase = 64;
constexpr int kEnterAuthOffsetBase = 30;
constexpr int kEnterBottomOffsetBase = 40;

/** 退场：时间条上移、底栏下移的基数（再乘 scale） */
constexpr int kExitTopOffsetBase = 64;
constexpr int kExitBottomOffsetBase = 40;

/** 全屏背景解锁过渡（与主 UI 退场同总时长；贝塞尔沿用原 FullBackgroundWidget） */
inline QEasingCurve unlockBackgroundExitEasingCurve()
{
    QEasingCurve curve(QEasingCurve::BezierSpline);
    curve.addCubicBezierSegment(QPointF(0.0, 0.0), QPointF(0.15, 1.0), QPointF(1.0, 1.0));
    return curve;
}

} // namespace LockMainUiAnim

#endif

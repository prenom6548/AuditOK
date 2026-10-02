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
 * 锁屏解锁退场：对认证控件做中心缩放 + 淡出（QGraphicsEffect，非截图叠加层）。
 */
#ifndef AUTHDIALOGEXITEFFECT_H
#define AUTHDIALOGEXITEFFECT_H

#include <QEvent>
#include <QGraphicsEffect>
#include <QObject>
#include <QPixmap>
#include <QPoint>

class AuthDialogExitEffect : public QGraphicsEffect
{
    Q_OBJECT
    Q_PROPERTY(qreal exitOpacity READ exitOpacity WRITE setExitOpacity)
    Q_PROPERTY(qreal exitScale READ exitScale WRITE setExitScale)

public:
    /** 缩放锚点：避免以控件中心缩放时上下行在视觉上互相「挤」进中间 */
    enum class ScaleAnchor {
        Center,
        TopCenter,
        BottomCenter
    };

    explicit AuthDialogExitEffect(QObject *parent = nullptr);
    ~AuthDialogExitEffect() override;

    void setScaleAnchor(ScaleAnchor anchor) { m_scaleAnchor = anchor; }
    ScaleAnchor scaleAnchor() const { return m_scaleAnchor; }

    qreal exitOpacity() const { return m_exitOpacity; }
    void setExitOpacity(qreal opacity);

    qreal exitScale() const { return m_exitScale; }
    void setExitScale(qreal scale);

    QRectF boundingRectFor(const QRectF &sourceRect) const override;

protected:
    void draw(QPainter *painter) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void invalidateSourceCache();
    static QPointF scalePivotFor(const QRect &r, ScaleAnchor anchor);

    qreal m_exitOpacity = 1.0;
    qreal m_exitScale = 1.0;
    ScaleAnchor m_scaleAnchor = ScaleAnchor::Center;

    /** 退场动画期间复用首帧栅格化结果，避免每帧 sourcePixmap 重绘子树（主要卡顿来源） */
    QPixmap m_sourceCache;
    QPoint m_sourceCacheOffset;
    bool m_sourceCacheValid = false;
};

#endif

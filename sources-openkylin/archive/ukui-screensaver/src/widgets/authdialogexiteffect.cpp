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
 */

#include "authdialogexiteffect.h"

#include <QEvent>
#include <QPainter>
#include <QWidget>

AuthDialogExitEffect::AuthDialogExitEffect(QObject *parent)
    : QGraphicsEffect(parent)
{
    if (auto *pw = qobject_cast<QWidget *>(parent)) {
        pw->installEventFilter(this);
    }
}

AuthDialogExitEffect::~AuthDialogExitEffect()
{
    if (auto *pw = qobject_cast<QWidget *>(parent())) {
        pw->removeEventFilter(this);
    }
}

QPointF AuthDialogExitEffect::scalePivotFor(const QRect &r, ScaleAnchor anchor)
{
    switch (anchor) {
    case ScaleAnchor::TopCenter:
        return QPointF(r.center().x(), r.top());
    case ScaleAnchor::BottomCenter:
        return QPointF(r.center().x(), r.bottom());
    case ScaleAnchor::Center:
    default:
        return r.center();
    }
}

void AuthDialogExitEffect::invalidateSourceCache()
{
    m_sourceCacheValid = false;
    m_sourceCache = QPixmap();
}

bool AuthDialogExitEffect::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    switch (event->type()) {
    case QEvent::Resize:
    case QEvent::LayoutRequest:
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
        invalidateSourceCache();
        break;
    default:
        break;
    }
    return false;
}

void AuthDialogExitEffect::setExitOpacity(qreal opacity)
{
    opacity = qBound(0.0, opacity, 1.0);
    if (qFuzzyCompare(m_exitOpacity, opacity)) {
        return;
    }
    m_exitOpacity = opacity;
    updateBoundingRect();
    update();
}

void AuthDialogExitEffect::setExitScale(qreal scale)
{
    scale = qMax(0.01, scale);
    if (qFuzzyCompare(m_exitScale, scale)) {
        return;
    }
    m_exitScale = scale;
    updateBoundingRect();
    update();
}

QRectF AuthDialogExitEffect::boundingRectFor(const QRectF &sourceRect) const
{
    if (qFuzzyCompare(m_exitScale, 1.0)) {
        return sourceRect;
    }
    const QRect r = sourceRect.toRect();
    const QPointF p = scalePivotFor(r, m_scaleAnchor);
    QRectF out = sourceRect;
    const QPointF corners[4] = {
        sourceRect.topLeft(), sourceRect.topRight(), sourceRect.bottomLeft(), sourceRect.bottomRight()};
    for (int i = 0; i < 4; ++i) {
        const QPointF m = p + (corners[i] - p) * m_exitScale;
        out = out.united(QRectF(m, QSizeF(1, 1)));
    }
    return out.united(sourceRect);
}

void AuthDialogExitEffect::draw(QPainter *painter)
{
    if (m_exitOpacity <= 0.0) {
        return;
    }

    QWidget *w = qobject_cast<QWidget *>(parent());
    if (!w) {
        return;
    }

    if (!m_sourceCacheValid) {
        m_sourceCacheOffset = QPoint();
        m_sourceCache = sourcePixmap(
            Qt::LogicalCoordinates,
            &m_sourceCacheOffset,
            QGraphicsEffect::PadToEffectiveBoundingRect);
        if (!m_sourceCache.isNull()) {
            m_sourceCacheValid = true;
        }
    }
    if (m_sourceCache.isNull()) {
        return;
    }

    painter->save();
    /* 与优化前一致：缩放时使用平滑采样，避免观感变化 */
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setOpacity(m_exitOpacity);

    const QRect r = w->rect();
    const QPointF pivot = scalePivotFor(r, m_scaleAnchor);

    painter->translate(pivot);
    painter->scale(m_exitScale, m_exitScale);
    painter->translate(-pivot);
    painter->drawPixmap(m_sourceCacheOffset, m_sourceCache);
    painter->restore();
}

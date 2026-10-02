/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
 
#include "oobetabbar.h"
#include <QDebug>
#include <QStyleOptionTab>
#include <QPainterPath>
#include <QRectF>
#include <QPainter>

OobeTabBar::OobeTabBar(QWidget *parent)
    : QTabBar(parent)
{
    setExpanding(true);
    setMovable(false);
    setTabsClosable(false);
    setDocumentMode(true);
    setStyleSheet("");
}

QPainterPath OobeTabBar::roundedRectPath(const QRectF &r, qreal tl, qreal tr, qreal br, qreal bl)
{
    QPainterPath p;
    qreal x = r.x(), y = r.y(), w = r.width(), h = r.height();
    qreal right = x + w, bottom = y + h;
    tl = qMin(tl, qMin(w/2, h/2));
    tr = qMin(tr, qMin(w/2, h/2));
    br = qMin(br, qMin(w/2, h/2));
    bl = qMin(bl, qMin(w/2, h/2));

    p.moveTo(x + tl, y);
    p.lineTo(right - tr, y);
    if (tr > 0) p.quadTo(right, y, right, y + tr);
    p.lineTo(right, bottom - br);
    if (br > 0) p.quadTo(right, bottom, right - br, bottom);
    p.lineTo(x + bl, bottom);
    if (bl > 0) p.quadTo(x, bottom, x, bottom - bl);
    p.lineTo(x, y + tl);
    if (tl > 0) p.quadTo(x, y, x + tl, y);
    p.closeSubpath();
    return p;
}

void OobeTabBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int n = count();
    const int radius = m_cornerRadius;

    QFontMetrics fm(font());

    for (int i = 0; i < n; ++i) {
        QRect r = tabRect(i);
        QRectF rf = QRectF(r).adjusted(1, 1, -1, -2);

        bool sel = (currentIndex() == i);
        QColor bg = sel ? QColor("#0078d7") : QColor("#555555");
        QColor text = QColor("#ffffff");

        qreal tl = 0, tr = 0, br = 0, bl = 0;
        if (i == 0) { tl = bl = radius; }
        if (i == n-1) { tr = br = radius; }

        QPainterPath path = roundedRectPath(rf, tl, tr, br, bl);
        painter.fillPath(path, bg);

        // draw text
        painter.setPen(text);
        QString txt = tabText(i);
        painter.drawText(rf, Qt::AlignCenter, txt);
    }
}

QSize OobeTabBar::tabSizeHint(int index) const
{
    Q_UNUSED(index);
    int n = count();
    if (n <= 0)
        return QTabBar::tabSizeHint(index);

    // Distribute the available width evenly among tabs.
    int totalW = width();
    int w = totalW / n;

    // Use the default height from base implementation to keep style-consistent height.
    QSize base = QTabBar::tabSizeHint(index);
    return QSize(w, base.height());
}
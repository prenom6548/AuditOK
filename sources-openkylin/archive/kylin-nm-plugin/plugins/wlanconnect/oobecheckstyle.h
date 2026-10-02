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

#ifndef OOBECHECKSTYLE_H
#define OOBECHECKSTYLE_H

#include <QProxyStyle>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include <QRect>
#include <QRectF>
#include <QPixmap>
#include <QPainterPath>
#include <QColor>
#include <QPen>
#include <QSize>
#include <QPoint>

class OobeCheckStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit OobeCheckStyle(QStyle *base = nullptr) : QProxyStyle(base) {}

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt,
                       QPainter *p, const QWidget *w) const override
    {
        if (pe == PE_IndicatorCheckBox) {
            if (w && w->property("oobe").toBool()) {
                const QStyleOption *option = opt;
                QRect r = option->rect;

                bool checked = (option->state & QStyle::State_On);

                p->save();
                p->setRenderHint(QPainter::Antialiasing, true);

                // draw rounded rect background
                QColor fill = checked ? QColor("#3790FA") : QColor(255,255,255,30);
                QColor border = checked ? QColor("#3790FA") : QColor(128,128,128,200); // 修改未选中时的边框颜色，使其更清晰
                p->setBrush(fill);
                p->setPen(QPen(border));
                QRectF rr = QRectF(r).adjusted(0.5, 0.5, -0.5, -0.5);
                p->drawRoundedRect(rr, 3.0, 3.0);

                // draw white check image when checked
                if (checked) {
                    QPixmap px(QLatin1String("://res/icon/checkmark-white.png"));
                    if (!px.isNull()) {
                        QSize ps = px.size();
                        QPoint center = r.center() - QPoint(ps.width()/2, ps.height()/2);
                        p->drawPixmap(center, px);
                    } else {
                        // fallback: simple white tick
                        QPen pen(Qt::white);
                        pen.setWidthF(2.0);
                        pen.setCapStyle(Qt::RoundCap);
                        pen.setJoinStyle(Qt::RoundJoin);
                        p->setPen(pen);
                        QPainterPath path;
                        path.moveTo(r.left()+3, r.center().y());
                        path.lineTo(r.center().x()-1, r.bottom()-4);
                        path.lineTo(r.right()-3, r.top()+4);
                        p->drawPath(path);
                    }
                }

                p->restore();
                return;
            }
        }
        QProxyStyle::drawPrimitive(pe, opt, p, w);
    }
};

#endif // OOBECHECKSTYLE_H

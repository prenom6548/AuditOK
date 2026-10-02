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

#ifndef OOBETABBAR_H
#define OOBETABBAR_H

#include <QTabBar>
#include <QPainter>

class OobeTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit OobeTabBar(QWidget *parent = nullptr);
    ~OobeTabBar() override = default;

    void setCornerRadius(int r) { m_cornerRadius = r; update(); }
    int cornerRadius() const { return m_cornerRadius; }

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize tabSizeHint(int index) const override;

private:
    int m_cornerRadius = 6;
    static QPainterPath roundedRectPath(const QRectF &r, qreal tl, qreal tr, qreal br, qreal bl);
};

#endif // OOBETABBAR_H
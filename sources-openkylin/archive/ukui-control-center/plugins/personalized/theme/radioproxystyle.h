/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#ifndef RADIOPROXYSTYLE_H
#define RADIOPROXYSTYLE_H

#include <QProxyStyle>

#include <QColor>
#include <QMenu>
#include <QPushButton>

class RadioProxystyle : public QProxyStyle
{
    Q_OBJECT
public:
    RadioProxystyle(const QColor &color, QStyle *style = nullptr);


    int pixelMetric(QStyle::PixelMetric metric,
                                  const QStyleOption *option,
                                  const QWidget *widget) const;

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget) const;

    void drawPrimitive(QStyle::PrimitiveElement element,
                       const QStyleOption *option, QPainter *painter,
                       const QWidget *widget) const;

    QRect subElementRect(SubElement element,
                     const QStyleOption *option,
                     const QWidget *widget) const;

    QSize sizeFromContents(ContentsType ct, const QStyleOption *option,
                                     const QSize &size, const QWidget *widget) const;


private:
    ~RadioProxystyle();

private:
    QColor mColor;
};

#endif // RADIOPROXYSTYLE_H

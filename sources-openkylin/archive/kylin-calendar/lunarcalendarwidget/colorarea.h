/*
* Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#ifndef COLORAREA_H
#define COLORAREA_H

#include <QWidget>
#include <QColor>
#include <QDebug>
#include <QPainter>
#include <QGSettings>
class ColorArea : public QWidget {
    Q_OBJECT

public:
    explicit ColorArea(QWidget *parent = nullptr);
     ~ColorArea() Q_DECL_OVERRIDE;
    void setcolor(const QColor &color);
       QColor selectBgColor;
//       QGSettings *StyleSetting =  nullptr;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor this_Color;
    QColor color1 ;
};

#endif // COLORAREA_H

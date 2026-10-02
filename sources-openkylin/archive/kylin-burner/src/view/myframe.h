/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
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


#ifndef MYFRAME_H
#define MYFRAME_H

#include "klineframe.h"

#include <QObject>
#include <QFrame>
#include <QPainter>
#include <QPalette>
#include <QColor>
using namespace kdk;

class MyFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MyFrame(QWidget *parent = nullptr);
    kdk::KHLineFrame *exframe;
protected:
    void paintEvent(QPaintEvent *event);

};

#endif // MYFRAME_H

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


/*
 * 可点击标签类，在QLabel的基础上增加鼠标左键点击信号
 */
#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include <QEvent>
#include <QMouseEvent>

class ClickLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickLabel(QWidget *parent = nullptr);
    ~ClickLabel();

protected:
    void mousePressEvent(QMouseEvent * event);

Q_SIGNALS:
    void clicked();

};

#endif // CLICKLABEL_H

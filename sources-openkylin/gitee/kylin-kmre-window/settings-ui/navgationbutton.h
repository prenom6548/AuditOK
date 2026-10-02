/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NAVGATIONBUTTON_H
#define NAVGATIONBUTTON_H

#include <QPushButton>

class NavgationButton : public QPushButton
{
    Q_OBJECT

public:
    enum FringePosition {
        Right = 0,
        Left,
        Top,
        Bottom,
        Invalid,
    };
    Q_ENUM(FringePosition)
    explicit NavgationButton(FringePosition position = FringePosition::Right, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    FringePosition m_position;
};

#endif // NAVGATIONBUTTON_H

/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Lu Hongxu <lhongxu@outlook.com>
 *
 */

#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QPushButton>
#include <QProxyStyle>

/**
 * \brief a clone of "controls/icon-container.h" in peony
*/

class IconButton : public QPushButton
{
    Q_OBJECT
public:
    explicit IconButton(QWidget *parent = nullptr);
    ~IconButton();

protected:
    void mouseMoveEvent(QMouseEvent *e) { Q_UNUSED(e) }
    void mousePressEvent(QMouseEvent *e) { Q_UNUSED(e) }
    void paintEvent(QPaintEvent *e);

private:
    QStyle *m_style;
};


class IconButtonStyle : public QProxyStyle
{
    friend class IconButton;
    explicit IconButtonStyle();
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
};

#endif // ICONBUTTON_H

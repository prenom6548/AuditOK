/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#ifndef SYSTEMBUTTON_H
#define SYSTEMBUTTON_H

#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>

class SystemButton : public QPushButton
{
    Q_OBJECT
public:
    enum ButtonStatus{NORMAL, ENTER, PRESS};

    explicit SystemButton(QWidget *parent = 0);
    void loadPixmap(const QString &normalPic, const QString &hoverPic, const QString &pressPic);
    void setOffset(int x_offset, int y_offset);

protected:
    void enterEvent(QEvent *) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:
    QPixmap m_normalPixmap;
    QPixmap m_hoverPixmap;
    QPixmap m_pressPixmap;

    ButtonStatus status;
    bool mouse_press; //按钮左键是否按下
    int btn_width;
    int btn_height;
    int m_xOffSet;
    int m_yOffSet;
};

#endif // SYSTEMBUTTON_H

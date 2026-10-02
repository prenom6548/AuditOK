/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SYSTEMBUTTON_H
#define SYSTEMBUTTON_H

#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
namespace KylinRubbishClear
{

class SystemButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SystemButton(QWidget *parent = 0);
    void loadPixmap(QString pic_name);



protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);

public Q_SLOTS:

private:
    QPixmap pixmap;
    enum ButtonStatus { NORMAL, ENTER, PRESS };
    ButtonStatus status;
    bool m_mousePressed; //按钮左键是否按下
    int btn_width;
    int btn_height;
};

} // namespace KylinRubbishClear


#endif // SYSTEMBUTTON_H

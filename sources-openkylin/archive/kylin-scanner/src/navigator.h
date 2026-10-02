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
#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QDebug>
#include <QMouseEvent>

class Navigator : public QWidget
{
    Q_OBJECT
public:
    explicit Navigator(QWidget *parent = nullptr);

private:
    QLabel *m_bottomImage = nullptr;
    bool m_isPress = false;
    QPoint m_startPos = QPoint(-1, -1);
    QPoint m_endPos = QPoint(-1, -1);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

public:
    void showNavigation(QPixmap pix);
    void getHighLightRegion(QPoint startPos, QPoint endPos); //拿起始点和终点



Q_SIGNALS:
    void naviChange();
    void posChange(QPoint pos);
};

#endif // NAVIGATOR_H

/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This file is part of Kylin-Screenshot.
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
#include "movelabel.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include "ukuiwindowhelper/ukuiwindowhelper.h"
#include <QPainterPath>
MoveLabel::MoveLabel(QWidget *parent) : QLabel(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
    {
    }
    else
    {
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
        windowHelper->setBlurEffect(QRegion(), 0, true);
    }

    QPalette p = palette();
    m_backColor = p.color(QPalette::Window);
    m_backColor.setAlpha(125);
}

bool MoveLabel::getFirstMove()
{
    return isFirstMove;
}

void MoveLabel::setFirstMove(bool firstMove)
{
    isFirstMove = firstMove;
}

void MoveLabel::paintEvent(QPaintEvent *event)
{
#ifndef UNUI
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);
    painter.fillPath(path, m_backColor);
#endif
    QWidget::paintEvent(event);
}

void MoveLabel::mousePressEvent(QMouseEvent *e)
{
#ifndef UNUI
    m_dragStart = e->globalPos();
    m_offsetPos = m_dragStart-this->mapToGlobal(QPoint(0,0));
#endif
}

void MoveLabel::mouseMoveEvent(QMouseEvent *e)
{
#ifndef UNUI
    if(e->buttons() & Qt::LeftButton){
        QDesktopWidget* desktop = QApplication::desktop();
        QRect desktopRect = desktop->availableGeometry();
        //临时方案，四周不可拖动区域大小尚且不清楚
        QRect windowRect(desktopRect.x()+20,desktopRect.y()+40,desktopRect.width()-40,desktopRect.height()-40);
        //不可拖动区域拖动趋势判定，如果拖向不可拖动区域会被阻止不作任何处理
        QPoint point(e->globalPos() - m_dragStart);
        bool flag = false;
        //靠左左拖
        if(point.x()<=0&&this->x()<=windowRect.x()){
            flag = true;
        }
        //靠上上拖
        if(point.y()<=0&&this->y()<=windowRect.y()){
            flag = true;
        }
        //靠右右拖
        if(point.x()>=0&&this->x()+width()>=windowRect.x()+windowRect.width()){
            flag = true;
        }
        //靠下下拖
        if(point.y()>=0&&this->y()+height()>=windowRect.y()+windowRect.height()){
            flag = true;
        }
        if(!flag){
            isFirstMove = true;
            QPoint posMove = e->globalPos()-m_offsetPos;
            int xx = posMove.x()-20;
            int yy = posMove.y()-60;
            if(m_buttonHandler){
//                m_buttonHandler->setIsMoved(isFirstMove);
                m_buttonHandler->updatePosition(QRect(xx,yy,this->width(),this->height()));
                vectorButtons = m_buttonHandler->getButtons();
                this->move(vectorButtons.first()->pos().x() - 15,
                           vectorButtons.first()->pos().y() - 3);
            }
        }
    }
#endif
}


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
#include "navigator.h"
#include "globalsignal.h"
Navigator::Navigator(QWidget *parent) : QWidget(parent)
{
    this->resize(QSize(130, 133));
    this->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.4);}");
    m_bottomImage = new QLabel(this);
    m_bottomImage->setAlignment(Qt::AlignCenter);
    m_bottomImage->resize(this->width(), this->height());
    m_bottomImage->move(0, 0);
    m_bottomImage->setMouseTracking(true);
    this->setMouseTracking(true);
    //此处绑定信号和槽
    connect(this, &Navigator::posChange, g_user_signal, &GlobalUserSignal::posChange);
}
//显示导航器
void Navigator::showNavigation(QPixmap pix)
{
    if (pix.isNull()) {
        this->hide();
        return;
    }
    m_bottomImage->setPixmap(pix);
    if (this->isHidden()) {
        this->show();
        Q_EMIT naviChange();
    }
}
//发送鼠标移动坐标
void Navigator::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    QPoint currpos = this->mapFromGlobal(QCursor().pos());

    if (m_isPress) {
        Q_EMIT posChange(currpos);
        this->setCursor(Qt::ClosedHandCursor);
    } else {
        if ((currpos.x() >= m_startPos.x() && currpos.x() <= m_endPos.x())
            && (currpos.y() >= m_startPos.y() && currpos.y() <= m_endPos.y())) {
            this->setCursor(Qt::ClosedHandCursor);
        } else {
            this->unsetCursor();
        }
    }

    QWidget::mouseMoveEvent(event);
}
//发送鼠标按下坐标
void Navigator::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint currpos = this->mapFromGlobal(QCursor().pos());
        Q_EMIT posChange(currpos);
    }
    m_isPress = true;
    this->setCursor(Qt::ClosedHandCursor);
    QWidget::mousePressEvent(event);
}

void Navigator::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPress = false;
    QPoint currpos = this->mapFromGlobal(QCursor().pos());
    if ((currpos.x() >= m_startPos.x() && currpos.x() <= m_endPos.x())
        && (currpos.y() >= m_startPos.y() && currpos.y() <= m_endPos.y())) {
        this->setCursor(Qt::ClosedHandCursor);
    } else {
        this->unsetCursor();
    }
    //判断在区域内，鼠标不变，否则，鼠标变化为
    QWidget::mouseReleaseEvent(event);
}

void Navigator::getHighLightRegion(QPoint startPos, QPoint endPos)
{
    m_startPos = startPos;
    m_endPos = endPos;
}

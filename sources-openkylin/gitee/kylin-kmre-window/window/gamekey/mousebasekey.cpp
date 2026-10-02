/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
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

#include "mousebasekey.h"
#include "joystickmanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <QDebug>

MouseBaseKey::MouseBaseKey(KmreWindow* window,int type) 
    : JoystickBaseKey(window, type)
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    mWidthRatio = DEFAULT_MOUSE_KEY_WIDTH  / static_cast<double>(initialWidth);
    mHeightRatio = DEFAULT_MOUSE_KEY_HEIGHT / static_cast<double>(initialHeight);

    this->resize(mWidthRatio*displayWidth,mHeightRatio*displayHeight);
}

void MouseBaseKey::enableEdit(bool enable){
    m_enableEdit = enable;
}

void MouseBaseKey::updateSize(bool update_panel){
    Q_UNUSED(update_panel);
    int displayWidth, displayHeight;
    mMainWindow->getJoystickManager()->getMainWidgetDisplaySize(displayWidth, displayHeight);

    this->resize(mWidthRatio * displayWidth, mHeightRatio * displayHeight);
    mCloseBtn->move(this->width() - mCloseBtn->width(), 0);
    mBgBtn->setGeometry(0,mCloseBtn->height(),this->width()-mCloseBtn->width(),this->height()-mCloseBtn->height());
}

void MouseBaseKey::getTranslateCenterPos(QPoint pos){
    event_coordinate_x = pos.x()+mBgBtn->width()/2;
    event_coordinate_y = pos.y()+mCloseBtn->height()+mBgBtn->height()/2;
}

bool MouseBaseKey::eventFilter(QObject *obj, QEvent *event)
{
    //syslog(LOG_DEBUG, "[SingleKey] eventFilter, event type: %d", event->type());
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        m_lastPos =QPoint(mouseEvent->pos().x(),mouseEvent->pos().y()+20) ;    //这个加20很关键，否则坐标跳变，这里得到的是小控件的相对坐标
        //qDebug() << "MouseBaseKey::eventFilter MouseButtonPress" << m_lastPos.x() << m_lastPos.y();
        m_mousePressed = true;
    }
    else if (event->type() == QEvent::MouseMove && m_enableEdit && m_mousePressed) {
        QRect validRect;
        validRect = mMainWindow->getJoystickManager()->getGameKeyValidRect();
        //计算新的移动后的位置
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        //qDebug() << "MouseBaseKey::eventFilter MouseMove" << mouseEvent->pos().x() << mouseEvent->pos().y();
        QPoint movePoint;
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            movePoint = mouseEvent->pos() - m_lastPos + QPoint(this->x(), this->y());
        } else {
            movePoint = mouseEvent->globalPos() - m_lastPos;
            //qDebug() << "MouseBaseKey::eventFilter MouseMove globalPos()" << mouseEvent->globalPos().x() << mouseEvent->globalPos().y();
        }
        //设置可移动的X和Y的范围
        bool moveX = (movePoint.x() > validRect.x()) && (movePoint.x() < validRect.right() - this->width());
        bool moveY = (movePoint.y() > validRect.y()) && (movePoint.y() < validRect.bottom() - this->height());

        if (moveX && moveY) {//在X和Y的允许范围内移动
            this->move(movePoint);
        }else if (moveX) {//在X的允许范围内移动
            this->move(movePoint.x(), this->pos().y());
        }else if (moveY) {//在Y的允许范围内移动
            this->move(this->pos().x(), movePoint.y());
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
        m_mousePressed = false;
    }

    return QWidget::eventFilter(obj, event);
}


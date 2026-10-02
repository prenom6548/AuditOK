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

#include "joystickrightrock.h"
#include "joystickmanager.h"
#include "displaymanager/displaymanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <QDebug>

//右摇杆不可以调整大小
JoystickRightRock::JoystickRightRock(KmreWindow* window)
    : JoystickRockBaseKey(window, JOYRIGHT)
{
    mJoystickDeleteBtn = new QPushButton(this);
    mJoystickDeleteBtn->setFixedSize(DEFAULT_DELETE_KEY, DEFAULT_DELETE_KEY);
    mJoystickDeleteBtn->setStyleSheet("QPushButton{background:transparent;border-image:url(:/new/res/close_normal.svg);}"
                              "QPushButton:hover{background:transparent;border-image:url(:/new/res/close_hover.svg);}");
    mJoystickDeleteBtn->move(this->width()-mJoystickDeleteBtn->width(),0);
    connect(mJoystickDeleteBtn,&QPushButton::clicked,this,[this](){
        emit deleteGameKey(JOYRIGHT,0);
    });

    mJoystickBtn = new QPushButton(this);
    mJoystickBtn->resize(DEFAULT_JOYSTICK_ROCKER_ICON_SIZE,DEFAULT_JOYSTICK_ROCKER_ICON_SIZE);
    mJoystickBtn->setStyleSheet("QPushButton{background:transparent; border-image: url(:/new/res/joystick_right_hover.svg);}"
                                "QPushButton:focus{outline: none;}");
    mJoystickBtn->move(0,DEFAULT_JOYSTICK_ROCKER_MARGIN);
    mJoystickBtn->setMouseTracking(true);
    mJoystickBtn->installEventFilter(this);

    mJoystickFinishBtn = new QPushButton(this);
    mJoystickFinishBtn->resize(DEFAULT_JOYSTICK_ROCKER_FINISH_SIZE,DEFAULT_JOYSTICK_ROCKER_FINISH_SIZE);
    mJoystickFinishBtn->setStyleSheet("QPushButton{background:transparent; border-image: url(:/new/res/joystick_right_normal.svg);}"
                                      "QPushButton:focus{outline: none;}");
    mJoystickFinishBtn->setVisible(false);



    this->installEventFilter(this);

}
JoystickRightRock::~JoystickRightRock(){

}

void JoystickRightRock::getOperateHW(double &w,double &h){ //获取宽高的数据
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    w = displayWidth*mRealOperWidthRatio*4;
    h = displayHeight*mRealOPerHeightRatio*4;
}

bool JoystickRightRock::eventFilter(QObject *obj, QEvent *event){
    if((event->type() == QEvent::MouseMove) && m_enableEdit && m_mousePressed){
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        QRect validRect = mMainWindow->getJoystickManager()->getGameKeyValidRect();
        //计算新的移动后的位置
        QPoint movePoint;
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            QPoint point = mouseEvent->pos();
            movePoint = point - m_lastPos + QPoint(this->x(), this->y());
        } else {
            movePoint = mouseEvent->globalPos() - m_lastPos;
        }
        //设置可移动的X和Y的范围
        bool moveX = (movePoint.x() > validRect.x()) && (movePoint.x() < validRect.right() - this->width());
        bool moveY = (movePoint.y() > validRect.y()) && (movePoint.y() < validRect.bottom() - this->height());

        if (moveX && moveY) {//在X和Y的允许范围内移动
            this->move(movePoint);
        }
        else if (moveX) {//在X的允许范围内移动
            this->move(movePoint.x(), this->pos().y());
        }
        else if (moveY) {//在Y的允许范围内移动
            this->move(this->pos().x(), movePoint.y());
        }

    }else if (event->type() == QEvent::MouseButtonPress && m_enableEdit) {
        //记住当前控件坐标和宽高以及鼠标按下的坐标
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        if(mouseEvent->button() == Qt::RightButton){
            return true;
        }
        if(obj == this){
            m_lastPos = mouseEvent->pos();
        }else if(obj == mJoystickBtn){
            m_lastPos = mJoystickBtn->mapToParent(mouseEvent->pos());
        }
        m_mousePressed = true;
    }else if(event->type() == QEvent::MouseButtonRelease && m_enableEdit){
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::RightButton){
            return true;
        }
        m_mousePressed = false;
    }else if((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease ||
              event->type() == QEvent::MouseButtonDblClick)&& !m_enableEdit){
         sendEventToApp(event);
         event->accept();
         return true;
     }
    return QWidget::eventFilter(obj, event);
}

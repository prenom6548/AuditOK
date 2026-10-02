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

#include "joysticksinglebasekey.h"
#include "joystickmanager.h"
#include "displaymanager/android_display/displaywidget.h"
#include "displaymanager/displaymanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <QDebug>
#include <sys/syslog.h>

JoystickSingleBaseKey::JoystickSingleBaseKey(KmreWindow* window, int type)
    :JoystickBaseKey(window, type)
    ,mComboFreq(1)
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);
    if(type == COMBOKEY){
        mWidthRatio = DEFAULT_COMBO_KEY_WIDTH / static_cast<double>(initialWidth);
        mHeightRatio = DEFAULT_COMBO_KEY_HEIGHT / static_cast<double>(initialHeight);
    }else if(type == FIRE){
        mWidthRatio = DEFAULT_MOUSE_KEY_WIDTH / static_cast<double>(initialWidth);
        mHeightRatio = DEFAULT_MOUSE_KEY_HEIGHT / static_cast<double>(initialHeight);
    }else{
        mWidthRatio = DEFAULT_SINGLE_KEY_WIDTH / static_cast<double>(initialWidth);
        mHeightRatio = DEFAULT_SINGLE_KEY_HEIGHT / static_cast<double>(initialHeight);
    }
    this->resize(mWidthRatio * displayWidth, mHeightRatio * displayHeight);
}
JoystickSingleBaseKey::~JoystickSingleBaseKey(){

}

void JoystickSingleBaseKey::enableEdit(bool enable){
    m_enableEdit = enable;
}
void JoystickSingleBaseKey::updateSize(bool update_panel){

}

void JoystickSingleBaseKey::getTranslateCenterPos(QPoint pos){
    event_coordinate_x = pos.x()+this->width()/2;
    event_coordinate_y = pos.y()+this->height()/2;
}

void JoystickSingleBaseKey::setKeyString(QString character){
    if(mKeyType == FIRE){
        mKeyString = character;
    }
    if(mInputEdit){
        setShowKeyString(mInputEdit,character);
    }
}

bool JoystickSingleBaseKey::isLineEditHasFocus(){
    if(mInputEdit){
        return mInputEdit->hasFocus();
    }
    return false;
}

void JoystickSingleBaseKey::setIndex(int index){
    this->mIndex = index;
}
void JoystickSingleBaseKey::getMouseClickPos(QObject *obj,QPoint pos){
    if(mKeyType == SINGLEKEY){
        if(obj == this){
            m_lastPos = pos;
        }else if(obj == mInputEdit){
            m_lastPos = QPoint(pos.x(),pos.y()+DEFAULT_DELETE_KEY);
        }
    }else if(mKeyType == COMBOKEY){
        if(obj == this){
            m_lastPos = pos;
        }else if(obj == mInputEdit){
            int offset = (this->width()-mInputEdit->width())/2;
            m_lastPos = QPoint(pos.x()+offset,pos.y()+DEFAULT_DELETE_KEY);
        }
    }else if(mKeyType ==FIRE){
        if(obj == this){
            m_lastPos = pos;
        }else if(obj == mBgBtn){
            m_lastPos = QPoint(pos.x(),pos.y()+DEFAULT_DELETE_KEY);
        }
    }else{
        m_lastPos = pos;
    }
}
bool JoystickSingleBaseKey::eventFilter(QObject *obj, QEvent *event){
    if (event->type() == QEvent::MouseButtonPress && m_enableEdit) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::RightButton){
            if(m_enableEdit && !isJoystick){
                emit mouseRightPress(mouseEvent);
            }
            return true;
        }
        m_mousePressed = true;
        if(obj == this){
            m_lastPos = mouseEvent->pos();
        }else{
            QWidget* widget = static_cast<QWidget*>(obj);
            m_lastPos = widget->mapToParent(mouseEvent->pos());
        }
    }else if (event->type() == QEvent::MouseMove && m_enableEdit && m_mousePressed) {
        mIsBeEdit = true;
        QRect validRect;
        validRect = mMainWindow->getJoystickManager()->getGameKeyValidRect();
        //计算新的移动后的位置
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint movePoint;
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            movePoint = mouseEvent->pos() - m_lastPos + QPoint(this->x(), this->y());
        } else {
            movePoint = mouseEvent->globalPos() - m_lastPos;
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
    }else if (event->type() == QEvent::MouseButtonRelease && m_enableEdit) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::RightButton){
            return true;
        }
        m_mousePressed = false;
    }else if(event->type() == QEvent::KeyPress && !isJoystick){
        if(m_enableEdit){
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if(!keyEvent->isAutoRepeat()){
                emit keyboardPress(keyEvent);
            }
            return true;
        }
        else{
            if (!SessionSettings::getInstance().windowUsePlatformWayland()) {
                mMainWindow->getJoystickManager()->sendEventToMainDisplay(event);
                event->accept();
            }
        }

    }else if(event->type() == QEvent::KeyRelease && !isJoystick){
        if(m_enableEdit ){
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if(!keyEvent->isAutoRepeat()){
                emit keyboardRelease(keyEvent);
            }
            return true;
        }
        else{
            if (!SessionSettings::getInstance().windowUsePlatformWayland()) {
                mMainWindow->getJoystickManager()->sendEventToMainDisplay(event);
                event->accept();
            }
        }

    }else if((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease ||
             event->type() == QEvent::MouseButtonDblClick)&& !m_enableEdit){
        sendEventToApp(event);
        event->accept();
        return true;
    }

    return QWidget::eventFilter(obj, event);
}



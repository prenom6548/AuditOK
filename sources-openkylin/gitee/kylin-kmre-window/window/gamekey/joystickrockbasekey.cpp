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

#include "joystickrockbasekey.h"
#include "joystickmanager.h"
#include "displaymanager/displaymanager.h"
#include "kmrewindow.h"

#include <QDebug>

JoystickRockBaseKey::JoystickRockBaseKey(KmreWindow* window,int type)
    :JoystickBaseKey(window, type)
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    mWidthRatio = DEFAULT_JOYSTICK_ROCKER_SIZE / static_cast<double>(initialWidth);
    mHeightRatio = DEFAULT_JOYSTICK_ROCKER_SIZE / static_cast<double>(initialHeight);

    mRealOperWidthRatio = DEFAULT_JOYSTICK_ROCKER_ICON_SIZE / static_cast<double>(initialWidth);
    mRealOPerHeightRatio = DEFAULT_JOYSTICK_ROCKER_ICON_SIZE / static_cast<double>(initialHeight);

    mFinishWidthRatio = DEFAULT_JOYSTICK_ROCKER_FINISH_SIZE / static_cast<double>(initialWidth);
    mFInishHeightRatio = DEFAULT_JOYSTICK_ROCKER_FINISH_SIZE / static_cast<double>(initialHeight);

    resize(mWidthRatio * displayWidth, mHeightRatio * displayHeight);
}

void JoystickRockBaseKey::enableEdit(bool enable){
    m_enableEdit = enable;
    mJoystickDeleteBtn->setVisible(enable);
    if(enable){
        updateSize();
        updatePos();
    }else{
        QRect rect = this->geometry();
        this->resize(mJoystickFinishBtn->width(),mJoystickFinishBtn->height());
        int x = rect.x()+(mJoystickBtn->width()-this->width())/2;
        int y = rect.y()+DEFAULT_JOYSTICK_ROCKER_MARGIN+(mJoystickBtn->height()-this->height())/2;

        int displayWidth, displayHeight;
        mMainWindow->getJoystickManager()->getMainWidgetDisplaySize(displayWidth, displayHeight);
        mLittleType_x = (mJoystickBtn->width()-this->width())/static_cast<double>(2*displayWidth);
        mLittleType_y = (DEFAULT_JOYSTICK_ROCKER_MARGIN*2+(mJoystickBtn->height()-this->height()))/static_cast<double>(2*displayHeight);

        this->move(x,y);
    }
    mJoystickBtn->setVisible(enable);
    mJoystickFinishBtn->setVisible(!enable);
}
void JoystickRockBaseKey::updateSize(bool update_panel){
    int displayWidth, displayHeight;
    mMainWindow->getJoystickManager()->getMainWidgetDisplaySize(displayWidth, displayHeight);

    if(mMainWindow->getJoystickManager()->isJoystickInEditing()){
        if (update_panel) {
            this->resize(mWidthRatio * displayWidth, mHeightRatio * displayHeight);
        }
        int icon_size = std::min(this->width(), this->height());
        mJoystickDeleteBtn->move(this->width()-mJoystickDeleteBtn->width(),0);
        mJoystickBtn->resize(icon_size-DEFAULT_JOYSTICK_ROCKER_MARGIN,icon_size-DEFAULT_JOYSTICK_ROCKER_MARGIN);
        mJoystickBtn->move(0,this->height()-mJoystickBtn->height());
        mJoystickFinishBtn->resize(mFinishWidthRatio*displayWidth,mFInishHeightRatio*displayHeight);
    }else{   //这个地方需要优化一下
        if (update_panel) {
            this->resize(mFinishWidthRatio * displayWidth, mFInishHeightRatio * displayHeight);
        }
    }
}

void JoystickRockBaseKey::updatePos(){
    int displayWidth, displayHeight;
    mMainWindow->getJoystickManager()->getMainWidgetDisplaySize(displayWidth, displayHeight);
    int xpos,ypos;
    if(mMainWindow->getJoystickManager()->isJoystickInEditing()){
        xpos = displayWidth * geometry_x;  //相当于是转换出添加按键时的   按键位置，相对widget的
        ypos = displayHeight * geometry_y;
    }else{
        xpos = displayWidth * (geometry_x+mLittleType_x);
        ypos = displayHeight *(geometry_y+mLittleType_y);
    }
    QPoint pos = mMainWindow->getJoystickManager()->getGlobalPos(QPoint(xpos, ypos));  //获取相对整个屏幕的坐标
    QPoint newpos = checkPosOutsideApp(pos);
    //move(pos);
    //checkPosOutsideScreen(pos);
    getTranslateCenterPos(newpos);
}

void JoystickRockBaseKey::getTranslateCenterPos(QPoint pos){
    if(mMainWindow->getJoystickManager()->isJoystickInEditing()){
        event_coordinate_x = pos.x()+mJoystickBtn->width()/2;
        event_coordinate_y = pos.y()+this->height()-mJoystickBtn->height()+mJoystickBtn->height()/2;
    }else{
        event_coordinate_x = pos.x()+mJoystickFinishBtn->width()/2;
        event_coordinate_y = pos.y()+mJoystickFinishBtn->height()/2;
    }
}

//void JoystickRockBaseKey::getJoystickRectWH(int &h,int &y){ //获取宽高的数据
//    int displayWidth, displayHeight, initialWidth, initialHeight;
//    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

//    h = displayWidth*mIconWidthRatio;
//    y = displayHeight*mIconHeightRatio;
//}





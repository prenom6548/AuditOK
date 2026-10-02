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

#include "joystickbasekey.h"
#include "displaymanager/displaymanager.h"
#include "joystickmanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <QTime>
#include <QtGlobal>
#include <QDebug>



JoystickBaseKey::JoystickBaseKey(KmreWindow *window,int type)
    : QWidget(window)
    , mMainWindow(window)
    , mKeyType(type)
{
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        setWindowFlags(Qt::FramelessWindowHint);
    } else {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    }
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        opacityEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(opacityEffect);
    }
}

JoystickBaseKey::~JoystickBaseKey(){
    if (opacityEffect) {
        delete opacityEffect;
    }
    setParent(nullptr);//将该子对象从父对象中移除，防止手动提前删除该子对象后，父对象析构时会再次析构该子对象
}

void JoystickBaseKey::showKey(bool show){
    if ((!m_outsideScreen) && show) {
        this->show();
    }else {
        this->hide();
    }
}
void JoystickBaseKey::setShowKeyString(QLineEdit* edit,QString str){
    mKeyString = str;
    mIsBeEdit = true;
    QString showStr;
    if(str.length()>3){
        showStr = str.left(2).append("…");
        QFont font;
        int size = this->font().pointSize();
        font.setPointSize(size-2);
        edit->setFont(font);
        mFontSizeChange = true;
    }else{
        if(mFontSizeChange){
            QFont font;
            int size = this->font().pointSize();
            font.setPointSize(size+2);
            edit->setFont(font);
            mFontSizeChange = false;
        }
        showStr = str;
    }
    edit->clear();
    edit->setText(showStr);
}

void JoystickBaseKey::setOpacity(double opacity){
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        opacityEffect->setOpacity(opacity);
    } else {
        setWindowOpacity(opacity);
    }
}
void JoystickBaseKey::setGeometryPos(double x,double y){//控件的起始坐标
    geometry_x = x;
    geometry_y = y;
}
void JoystickBaseKey::getGeometryPos(double &x,double &y){
    x = geometry_x;
    y = geometry_y;
}
void JoystickBaseKey::getRealEventPos(double &x,double &y){//获取真实的有效坐标点
    x = event_coordinate_x;
    y = event_coordinate_y;
}
void JoystickBaseKey::setGeometryHWRatio(double w,double h){
    mWidthRatio = w;
    mHeightRatio = h;
}
void JoystickBaseKey::getGeometruHWRatio(double &w,double &h){
    w = mWidthRatio;
    h = mHeightRatio;
}
void JoystickBaseKey::setOperateHWRatio(double w,double h){
    mRealOperWidthRatio = w;
    mRealOPerHeightRatio = h;
}
void JoystickBaseKey::getOperateHW(double &w,double &h){
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    w = displayWidth*mRealOperWidthRatio;
    h = displayHeight*mRealOPerHeightRatio;
}
void JoystickBaseKey::updatePos(){
    int displayWidth, displayHeight;
    mMainWindow->getJoystickManager()->getMainWidgetDisplaySize(displayWidth, displayHeight);

    int xpos = displayWidth * geometry_x;  //相当于是转换出添加按键时的   按键位置，相对widget的
    int ypos = displayHeight * geometry_y;
    QPoint pos = mMainWindow->getJoystickManager()->getGlobalPos(QPoint(xpos, ypos));  //获取相对整个屏幕的坐标
    QPoint newpos = checkPosOutsideApp(pos);
    //move(pos);
    checkPosOutsideScreen(pos);
    getTranslateCenterPos(newpos);
}

void JoystickBaseKey::checkPosOutsideScreen(QPoint pos)
{
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        return;
    }

    int x = pos.x();
    int y = pos.y();
    QRect allScreenSize, availableSize;
    mMainWindow->getDisplayManager()->getScreenInfo(allScreenSize, availableSize);
    if ((x < 0) || ((x + width()) > allScreenSize.width()) ||
        (y < 0) || ((y + height()) > allScreenSize.height())) {
        m_outsideScreen = true;
    }
    else {
        m_outsideScreen = false;
    }
}
void JoystickBaseKey::sendEventToApp(QEvent *event){
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    QPoint globalPos = mouseEvent->globalPos();
    int mainWidget_x, mainWidget_y, margin_width, titlebar_height;
    mMainWindow->getJoystickManager()->_getMainWidgetInfo(mainWidget_x, mainWidget_y, margin_width, titlebar_height);
    QPoint pos = QPoint(globalPos.x() - mainWidget_x - margin_width,
                        globalPos.y() - mainWidget_y - titlebar_height);
    if ((pos.x() >= 0) && (pos.y() >= 0)) {
        mouseEvent->setLocalPos(pos);
        mMainWindow->getJoystickManager()->sendEventToMainDisplay(mouseEvent);
    }
}
QPoint JoystickBaseKey::checkPosOutsideApp(QPoint pos){
    int x = pos.x();
    int y = pos.y();
    QRect validRect;
    validRect = mMainWindow->getJoystickManager()->getGameKeyValidRect();
    if (x<validRect.x() || x > (validRect.right()-this->width())){
        x = getRandom(validRect.x(),validRect.width());
    }
    if(y<validRect.y() || y >(validRect.bottom()-this->height())){
        y = getRandom(validRect.y(),validRect.height());
    }
    move(x,y);
    return QPoint(x,y);
}

int JoystickBaseKey::getRandom(int init,int range){
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    int num = qrand()%(range);
    return init+num;
}


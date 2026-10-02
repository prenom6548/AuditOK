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

#include "shademask.h"
#include "joystickmanager.h"
#include "displaymanager/displaymanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

ShadeMask::ShadeMask(KmreWindow* window) 
    : QWidget(window)
    , mMainWindow(window)
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
    setFocusPolicy(Qt::NoFocus);
    mlabel = new QLabel(this);
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        mlabel->setStyleSheet("QLabel{background:transparent;border-image:url(:/new/res/crosshair_bg.svg);}");
    } else {
        mlabel->setBackgroundRole(QPalette::Window);
        mlabel->setAutoFillBackground(true);
    }
    setOpacity(0.5);    //设置窗口的透明度，0-1
    setAttribute(Qt::WA_ShowWithoutActivating,true);
    int x,y,width,height;
    mMainWindow->getJoystickManager()->getBenchMarkPos(x,y,width,height);
    this->setGeometry(x,y,width,height);
    mlabel->resize(this->width(),this->height());
    //this->setWindowModality(Qt::ApplicationModal);  //这个模态设置以后，弹出的widget就无法移动了
    //this->setProperty("useStyleWindowManager", false);   //这个属性相当于设置窗口为模态
    mlabel->installEventFilter(this);
    installEventFilter(this);
}

ShadeMask::~ShadeMask(){
    if (opacityEffect) {
        delete opacityEffect;
    }
}

void ShadeMask::setOpacity(double opacity){
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        opacityEffect->setOpacity(opacity);
    } else {
        setWindowOpacity(opacity);
    }
}
void ShadeMask::updatePos(){
    int x,y,width,height;
    mMainWindow->getJoystickManager()->getBenchMarkPos(x,y,width,height);
    move(x,y);
    checkPosOutsideScreen(QPoint(x,y));
}
void ShadeMask::showShadeWidget(bool show){
    if ((!m_outsideScreen) && show) {
        this->show();
    }
    else {
        this->hide();
    }
}
void ShadeMask::checkPosOutsideScreen(QPoint pos){
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
bool ShadeMask::eventFilter(QObject *obj, QEvent *event){
    if (!SessionSettings::getInstance().windowUsePlatformWayland()) {
        if(obj == mlabel){
            this->lower();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

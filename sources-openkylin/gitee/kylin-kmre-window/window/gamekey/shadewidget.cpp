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

#include "shadewidget.h"
#include "joystickmanager.h"
#include "joysticksinglekey.h"
#include "displaymanager/displaymanager.h"
#include "preferences.h"
#include "common.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <QDebug>
#include <QPalette>
#include <sys/syslog.h>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QGridLayout>
#include <QToolButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>

const int DEFAULT_TOP_PANEL_WIDTH = 450;
const int DEFAULT_TOP_PANEL_HEIGHT = 80;

ShadeWidget::ShadeWidget(KmreWindow* window) 
    : QWidget(window)
    , mMainWindow(window)
{

    /*---------------方法一-----------------*/
    //this->setWindowModality(Qt::ApplicationModal);    //设置窗口为模态，但是这里不管用
//    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowTitleHint);   //无边框，无标题栏
//    this->setPalette(Qt::white);
//    this->setWindowOpacity(0.5);    //设置窗口的透明度，0-1
//    this->setProperty("useStyleWindowManager", false);   //这个属性相当于设置窗口为模态
    /*-----------------------------------------*/

    /********************方法二************************/
    //setAttribute(Qt::WA_NoMousePropagation);  //鼠标事件不往下穿透
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::Tool);   //无边框，无标题栏
    this->setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::NoFocus);
    mlabel = new QLabel(this);
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        mlabel->setStyleSheet("QLabel{background:transparent;border-image:url(:/new/res/crosshair_bg.svg);}");
    } else {
        mlabel->setBackgroundRole(QPalette::Window);
        mlabel->setAutoFillBackground(true);
    }

    setWindowOpacity(0.3);    //设置窗口的透明度，0-1
    /**************************************************/

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

void ShadeWidget::initShadeWidget(){

}

void ShadeWidget::updatePos(){
    int x,y,width,height;
    mMainWindow->getJoystickManager()->getBenchMarkPos(x,y,width,height);
    move(x,y);
    checkPosOutsideScreen(QPoint(x,y));
}
void ShadeWidget::showShadeWidget(bool show){
    if ((!m_outsideScreen) && show) {
        this->show();
    }
    else {
        this->hide();
    }
}
void ShadeWidget::checkPosOutsideScreen(QPoint pos)
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
bool ShadeWidget::eventFilter(QObject *obj, QEvent *event){
    if(obj == mlabel){
        this->lower();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

ShadeWidget::~ShadeWidget()
{

}

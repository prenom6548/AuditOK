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

#include "joystickleftrock.h"
#include "joystickmanager.h"
#include "displaymanager/displaymanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <QDebug>

//左摇杆可以调整大小
JoystickLeftRock::JoystickLeftRock(KmreWindow* window)
    :JoystickRockBaseKey(window, JOYRIGHT)
{
    mJoystickDeleteBtn = new QPushButton(this);
    mJoystickDeleteBtn->setFixedSize(DEFAULT_DELETE_KEY, DEFAULT_DELETE_KEY);
    mJoystickDeleteBtn->setStyleSheet("QPushButton{background:transparent;border-image:url(:/new/res/close_normal.svg);}"
                              "QPushButton:hover{background:transparent;border-image:url(:/new/res/close_hover.svg);}");
    mJoystickDeleteBtn->move(this->width()-mJoystickDeleteBtn->width(),0);
    connect(mJoystickDeleteBtn,&QPushButton::clicked,this,[this](){
        emit deleteGameKey(JOYLEFT,0);
    });

    mJoystickBtn = new QPushButton(this);
    mJoystickBtn->resize(DEFAULT_JOYSTICK_ROCKER_ICON_SIZE,DEFAULT_JOYSTICK_ROCKER_ICON_SIZE);
    mJoystickBtn->setStyleSheet("QPushButton{background:transparent; border-image: url(:/new/res/joystick_left_hover.svg);}"
                                "QPushButton:focus{outline: none;}");
    mJoystickBtn->move(0,DEFAULT_JOYSTICK_ROCKER_MARGIN);
    mJoystickBtn->setMouseTracking(true);
    mJoystickBtn->installEventFilter(this);

    mJoystickFinishBtn = new QPushButton(this);
    mJoystickFinishBtn->resize(DEFAULT_JOYSTICK_ROCKER_FINISH_SIZE,DEFAULT_JOYSTICK_ROCKER_FINISH_SIZE);
    mJoystickFinishBtn->setStyleSheet("QPushButton{background:transparent; border-image: url(:/new/res/joystick_left_normal.svg);}"
                                      "QPushButton:focus{outline: none;}");
    mJoystickFinishBtn->setVisible(false);

    this->installEventFilter(this);
    resetValues();
}

JoystickLeftRock::~JoystickLeftRock(){

}

void JoystickLeftRock::calculateTracingArea(){
    //重新计算八个描点的区域,描点区域的作用还有就是计算鼠标坐标是否在某一个区域内
    int width = this->width();
    int height = this->height();

    //左侧描点区域
    m_rectLeft = QRect(0, DEFAULT_JOYSTICK_MARGIN, DEFAULT_JOYSTICK_MARGIN, height - (DEFAULT_JOYSTICK_MARGIN * 2));
    //上侧描点区域
    m_rectTop = QRect(DEFAULT_JOYSTICK_MARGIN, 0, width - (DEFAULT_JOYSTICK_MARGIN * 2), DEFAULT_JOYSTICK_MARGIN);
    //右侧描点区域
    m_rectRight = QRect(width - DEFAULT_JOYSTICK_MARGIN, DEFAULT_JOYSTICK_MARGIN, DEFAULT_JOYSTICK_MARGIN, height - (DEFAULT_JOYSTICK_MARGIN * 2));
    //下侧描点区域
    m_rectBottom = QRect(DEFAULT_JOYSTICK_MARGIN, height - DEFAULT_JOYSTICK_MARGIN, width - (DEFAULT_JOYSTICK_MARGIN * 2), DEFAULT_JOYSTICK_MARGIN);
    //左上角描点区域
    m_rectLeftTop = QRect(0, 0, DEFAULT_JOYSTICK_MARGIN, DEFAULT_JOYSTICK_MARGIN);
    //右上角描点区域
    m_rectRightTop = QRect(width - DEFAULT_JOYSTICK_MARGIN, 0, DEFAULT_JOYSTICK_MARGIN, DEFAULT_JOYSTICK_MARGIN);
    //左下角描点区域
    m_rectLeftBottom = QRect(0, height - DEFAULT_JOYSTICK_MARGIN, DEFAULT_JOYSTICK_MARGIN, DEFAULT_JOYSTICK_MARGIN);
    //右下角描点区域
    m_rectRightBottom = QRect(width - DEFAULT_JOYSTICK_MARGIN, height - DEFAULT_JOYSTICK_MARGIN, DEFAULT_JOYSTICK_MARGIN, DEFAULT_JOYSTICK_MARGIN);
}

void JoystickLeftRock::setMouseStyle(const QPoint &point){
    if (m_rectLeft.contains(point)) {
        this->setCursor(Qt::SizeHorCursor);
    }
    else if (m_rectRight.contains(point)) {
        this->setCursor(Qt::SizeHorCursor);
    }
    else if (m_rectTop.contains(point)) {
        this->setCursor(Qt::SizeVerCursor);
    }
    else if (m_rectBottom.contains(point)) {
        this->setCursor(Qt::SizeVerCursor);
    }
    // else if (m_rectLeftTop.contains(point)) {
    //     this->setCursor(Qt::SizeFDiagCursor);
    // }
    // else if (m_rectRightTop.contains(point)) {
    //     this->setCursor(Qt::SizeBDiagCursor);
    // }
    // else if (m_rectLeftBottom.contains(point)) {
    //     this->setCursor(Qt::SizeBDiagCursor);
    // }
    // else if (m_rectRightBottom.contains(point)) {
    //     this->setCursor(Qt::SizeFDiagCursor);
    // }
    else {
        this->setCursor(Qt::ArrowCursor);
    }
}

void JoystickLeftRock::resetSizeAndPos(int offsetX, int offsetY){
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);
    double min_width_scale_ratio = DEFAULT_JOYSTICK_ROCKER_SIZE / static_cast<double>(initialWidth);
    double min_height_scale_ratio = DEFAULT_JOYSTICK_ROCKER_SIZE / static_cast<double>(initialHeight);

    int min_width = min_width_scale_ratio * displayWidth;
    int min_height = min_height_scale_ratio * displayHeight;
    QRect validRect = mMainWindow->getJoystickManager()->getGameKeyValidRect();

    if (m_pressedLeft) {
        int new_size = this->width() - offsetX;
        if ((min_width <= new_size) &&
            (this->x() + offsetX > validRect.x()) &&
            (this->y() + offsetX > validRect.y())) {
            this->setGeometry(this->x() + offsetX, this->y() + offsetX, new_size, new_size);
        }
    }
    else if (m_pressedRight) {
        int new_size = rectW + offsetX;
        if ((min_width <= new_size) &&
            (rectX + new_size < validRect.right()) &&
            (rectY + new_size < validRect.bottom())) {
            this->setGeometry(rectX, rectY, new_size, new_size);
        }
    }
    else if (m_pressedTop) {
        int new_size = this->height() - offsetY;
        if ((min_height <= new_size) &&
            (this->x() + offsetY > validRect.x()) &&
            (this->y() + offsetY > validRect.y())) {
            this->setGeometry(this->x() + offsetY, this->y() + offsetY, new_size, new_size);
        }
    }
    else if (m_pressedBottom) {
        int new_size = rectH + offsetY;
        if ((min_height <= new_size) &&
            (rectX + new_size < validRect.right()) &&
            (rectY + new_size < validRect.bottom())) {
            this->setGeometry(rectX, rectY, new_size, new_size);
        }
    }
    else if (m_pressedLeftTop) {
        // don't support
    }
    else if (m_pressedRightTop) {
        // don't support
    }
    else if (m_pressedLeftBottom) {
        // don't support
    }
    else if (m_pressedRightBottom) {
        // don't support
    }

    mRealOperWidthRatio = (this->width()-DEFAULT_JOYSTICK_ROCKER_MARGIN) / static_cast<double>(initialWidth);
    mRealOPerHeightRatio = (this->height()-DEFAULT_JOYSTICK_ROCKER_MARGIN) / static_cast<double>(initialHeight);

    updateSize(false);
}

//判断按下的鼠标的区域位置
void JoystickLeftRock::getPressedAreaPosition(const QPoint &point)
{
    if (m_rectLeft.contains(point)) {
        m_pressedLeft = true;
    }
    else if (m_rectRight.contains(point)) {
        m_pressedRight = true;
    }
    else if (m_rectTop.contains(point)) {
        m_pressedTop = true;
    }
    else if (m_rectBottom.contains(point)) {
        m_pressedBottom = true;
    }
    else if (m_rectLeftTop.contains(point)) {
        m_pressedLeftTop = true;
    }
    else if (m_rectRightTop.contains(point)) {
        m_pressedRightTop = true;
    }
    else if (m_rectLeftBottom.contains(point)) {
        m_pressedLeftBottom = true;
    }
    else if (m_rectRightBottom.contains(point)) {
        m_pressedRightBottom = true;
    }
    else {
        m_mousePressed = true;
    }
}

void JoystickLeftRock::resetValues()
{
    m_mousePressed = false;
    m_pressedLeft = false;
    m_pressedRight = false;
    m_pressedTop = false;
    m_pressedBottom = false;
    m_pressedLeftTop = false;
    m_pressedRightTop = false;
    m_pressedLeftBottom = false;
    m_pressedRightBottom = false;
}

bool JoystickLeftRock::eventFilter(QObject *obj, QEvent *event){
    if(event->type() == QEvent::Resize){
        this->calculateTracingArea();
//        mJoystickBtn->move(0,this->height()-mJoystickBtn->height());
//        mJoystickDeleteBtn->move(this->width() - mJoystickDeleteBtn->width(), 0);
    }else if((event->type() == QEvent::MouseMove) && m_enableEdit){

        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        QPoint point = mouseEvent->pos();
        this->setMouseStyle(point);

        //根据当前鼠标位置,计算XY轴移动了多少
        int offsetX = point.x() - m_lastPos.x();
        int offsetY = point.y() - m_lastPos.y();

        //根据按下处的位置判断是否是移动控件还是拉伸控件
        if (m_mousePressed) {
            QRect validRect = mMainWindow->getJoystickManager()->getGameKeyValidRect();
            //计算新的移动后的位置
            QPoint movePoint;
            if (SessionSettings::getInstance().windowUsePlatformWayland()) {
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
        }else{
            this->resetSizeAndPos(offsetX, offsetY);
        }
    }else if (event->type() == QEvent::MouseButtonPress && m_enableEdit) {
        //记住当前控件坐标和宽高以及鼠标按下的坐标
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        if(mouseEvent->button() == Qt::RightButton){
            return true;
        }
        rectX = this->x();
        rectY = this->y();
        rectW = this->width();
        rectH = this->height();
        if(obj == this){
            m_lastPos = mouseEvent->pos();
        }else if(obj == mJoystickBtn){
            m_lastPos = mJoystickBtn->mapToParent(mouseEvent->pos());
        }
        //判断按下的手柄的区域位置
        this->getPressedAreaPosition(m_lastPos);
    }else if(event->type() == QEvent::MouseButtonRelease && m_enableEdit){
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        if(mouseEvent->button() == Qt::RightButton){
            return true;
        }
        this->resetValues();
        this->setCursor(Qt::ArrowCursor);
    }else if((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease ||
              event->type() == QEvent::MouseButtonDblClick)&& !m_enableEdit){
         sendEventToApp(event);
         event->accept();
         return true;
     }
    return QWidget::eventFilter(obj, event);
}

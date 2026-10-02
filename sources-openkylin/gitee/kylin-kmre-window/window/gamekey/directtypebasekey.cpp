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

#include "directtypebasekey.h"
#include "joystickmanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <QDebug>

DirectTypeBaseKey::DirectTypeBaseKey(KmreWindow* window,int type)
    : JoystickBaseKey(window, type)
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    mWidthRatio = DEFAULT_DIRECTION_SIZE / static_cast<double>(initialWidth);
    mHeightRatio = DEFAULT_DIRECTION_SIZE / static_cast<double>(initialHeight);

    resize(mWidthRatio * displayWidth, mHeightRatio * displayHeight);

    mBgBtn = new QPushButton(this);
    mBgBtn->setFlat(true);
    mBgBtn->resize(DEFAULT_DIRECTION_SIZE-DEFAULT_DELETE_KEY/2, DEFAULT_DIRECTION_SIZE-DEFAULT_DELETE_KEY/2);
    mBgBtn->move(0,DEFAULT_DELETE_KEY/2);
    mBgBtn->setMouseTracking(true);
    mBgBtn->installEventFilter(this);

    mEditTop = new QLineEdit("",this);
    mEditTop->setAlignment(Qt::AlignCenter);
    mEditTop->setContextMenuPolicy(Qt::NoContextMenu);
    mEditTop->setAttribute(Qt::WA_InputMethodEnabled,false);
    mEditTop->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/mEditBg.svg);;color:rgba(255,255,255,1);}");
    mEditTop->setFixedSize(DEFAULT_LABEL_AWDS_SIZE, DEFAULT_LABEL_AWDS_SIZE);
    mEditTop->setMouseTracking(true);
    mEditTop->installEventFilter(this);

    mEditLeft = new QLineEdit("",this);
    mEditLeft->setAlignment(Qt::AlignCenter);
    mEditLeft->setContextMenuPolicy(Qt::NoContextMenu);
    mEditLeft->setAttribute(Qt::WA_InputMethodEnabled,false);
    mEditLeft->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/mEditBg.svg);;color:rgba(255,255,255,1);}");
    mEditLeft->setFixedSize(DEFAULT_LABEL_AWDS_SIZE, DEFAULT_LABEL_AWDS_SIZE);
    mEditLeft->setMouseTracking(true);
    mEditLeft->installEventFilter(this);

    mEditRight = new QLineEdit("",this);
    mEditRight->setAlignment(Qt::AlignCenter);
    mEditRight->setContextMenuPolicy(Qt::NoContextMenu);
    mEditRight->setAttribute(Qt::WA_InputMethodEnabled,false);
    mEditRight->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/mEditBg.svg);;color:rgba(255,255,255,1);}");
    mEditRight->setFixedSize(DEFAULT_LABEL_AWDS_SIZE, DEFAULT_LABEL_AWDS_SIZE);
    mEditRight->installEventFilter(this);

    mEditBottom = new QLineEdit("",this);
    mEditBottom->setAlignment(Qt::AlignCenter);
    mEditBottom->setContextMenuPolicy(Qt::NoContextMenu);
    mEditBottom->setAttribute(Qt::WA_InputMethodEnabled,false);
    mEditBottom->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/mEditBg.svg);;color:rgba(255,255,255,1);}");
    mEditBottom->setFixedSize(DEFAULT_LABEL_AWDS_SIZE, DEFAULT_LABEL_AWDS_SIZE);
    mEditBottom->installEventFilter(this);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(mEditTop, 0, 1, 1, 1, Qt::AlignCenter);
    layout->addWidget(mEditBottom, 2, 1, 1, 1, Qt::AlignCenter);
    layout->addWidget(mEditLeft, 1, 0, 1, 1, Qt::AlignCenter);
    layout->addWidget(mEditRight, 1, 2, 1, 1, Qt::AlignCenter);
    layout->setMargin(5);
    layout->setSpacing(5);
    mBgBtn->setLayout(layout);

    mCloseBtn = new QPushButton(this);
    mCloseBtn->setFixedSize(DEFAULT_DELETE_KEY, DEFAULT_DELETE_KEY);
    mCloseBtn->setStyleSheet("QPushButton{background:transparent;border-image:url(:/new/res/close_normal.svg);}"
                             "QPushButton:hover{background:transparent;border-image:url(:/new/res/close_hover.svg);}");
    mCloseBtn->move(this->width() - mCloseBtn->width(), 0);

    this->installEventFilter(this);
    resetValues();
}

void DirectTypeBaseKey::updateSize(bool update_panel){
    int displayWidth, displayHeight;
    mMainWindow->getJoystickManager()->getMainWidgetDisplaySize(displayWidth, displayHeight);
    if (update_panel) {
        this->resize(mWidthRatio * displayWidth, mHeightRatio * displayHeight);
    }
    int icon_size = std::min(this->width(), this->height());  //保证了背景一定比控件小
    mBgBtn->resize(icon_size-DEFAULT_DELETE_KEY/2, icon_size-DEFAULT_DELETE_KEY/2);
    mBgBtn->move(0,DEFAULT_DELETE_KEY/2);
    mCloseBtn->move(this->width() - mCloseBtn->width(), 0);

    double mEditWidthRatio = this->width() / static_cast<double>(DEFAULT_DIRECTION_SIZE);
    double mEditHeightRatio = this->height() / static_cast<double>(DEFAULT_DIRECTION_SIZE);
    int editWidth = DEFAULT_LABEL_AWDS_SIZE * mEditWidthRatio;
    int editHeigth = DEFAULT_LABEL_AWDS_SIZE * mEditHeightRatio;

    QFont ft;
    int fontSize = MIN_LABEL_AWDS_TEXT_SIZE + (MAX_LABEL_AWDS_TEXT_SIZE - MIN_LABEL_AWDS_TEXT_SIZE) * (mEditWidthRatio - 1.0) / 3.0;
    ft.setPointSize(fontSize);
    mEditTop->setFixedSize(editWidth, editHeigth);
    mEditTop->setFont(ft);
    mEditLeft->setFixedSize(editWidth, editHeigth);
    mEditLeft->setFont(ft);
    mEditRight->setFixedSize(editWidth, editHeigth);
    mEditRight->setFont(ft);
    mEditBottom->setFixedSize(editWidth, editHeigth);
    mEditBottom->setFont(ft);
}


void DirectTypeBaseKey::getOperateHW(double &w,double &h){
    w = mBgBtn->width();
    h = mBgBtn->height();
}

bool DirectTypeBaseKey::isHavingFocus(){
    return mEditTop->hasFocus() ||mEditRight->hasFocus() || mEditBottom->hasFocus() ||mEditLeft->hasFocus();
}

void DirectTypeBaseKey::setKeyString(QString str){
    if(str.isEmpty()){
        return;
    }
    mIsBeEdit = true;
    if(mEditTop && mEditTop->hasFocus()){
        mEditTop->setText(str);
    }else if(mEditLeft && mEditLeft->hasFocus()){
        mEditLeft->setText(str);
    }else if(mEditRight && mEditRight->hasFocus()){
        mEditRight->setText(str);
    }else if(mEditBottom && mEditBottom->hasFocus()){
        mEditBottom->setText(str);
    }
}

void DirectTypeBaseKey::setDefaultKeyString(QString strTop,QString strRight,QString strBottom,QString strLeft){
    if(!strTop.isEmpty() && mEditTop){
        mEditTop->setText(strTop);
    }
    if(!strRight.isEmpty() && mEditRight){
        mEditRight->setText(strRight);
    }
    if(!strBottom.isEmpty() && mEditBottom){
        mEditBottom->setText(strBottom);
    }
    if(!strLeft.isEmpty() && mEditLeft){
        mEditLeft->setText(strLeft);
    }
}
bool DirectTypeBaseKey::isHadLineEditNotSetValue(){
    if(mEditTop->text().isEmpty() || mEditRight->text().isEmpty()|| mEditBottom->text().isEmpty()|| mEditLeft->text().isEmpty()){
        return true;
    }
    return false;
}
void DirectTypeBaseKey::getDirectKeyString(QString &strTop,QString &strRight,QString &strBottom,QString &strLeft){
    strTop = mEditTop->text();
    strRight = mEditRight->text();
    strBottom = mEditBottom->text();
    strLeft = mEditLeft->text();
}

void DirectTypeBaseKey::getTranslateCenterPos(QPoint pos){
    event_coordinate_x = pos.x()+mBgBtn->width()/2;
    event_coordinate_y = pos.y()+this->height()-mBgBtn->height()/2;
}
void DirectTypeBaseKey::resetValues()
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

//计算八个描点的区域
void DirectTypeBaseKey::calculateTracingArea()
{
    //重新计算八个描点的区域,描点区域的作用还有就是计算鼠标坐标是否在某一个区域内
    int width = this->width();
    int height = this->height();

    //左侧描点区域
    m_rectLeft = QRect(0, DEFAULT_MARGIN, DEFAULT_MARGIN, height - (DEFAULT_MARGIN * 2));
    //上侧描点区域
    m_rectTop = QRect(DEFAULT_MARGIN, 0, width - (DEFAULT_MARGIN * 2), DEFAULT_MARGIN);
    //右侧描点区域
    m_rectRight = QRect(width - DEFAULT_MARGIN, DEFAULT_MARGIN, DEFAULT_MARGIN, height - (DEFAULT_MARGIN * 2));
    //下侧描点区域
    m_rectBottom = QRect(DEFAULT_MARGIN, height - DEFAULT_MARGIN, width - (DEFAULT_MARGIN * 2), DEFAULT_MARGIN);
    //左上角描点区域
    m_rectLeftTop = QRect(0, 0, DEFAULT_MARGIN, DEFAULT_MARGIN);
    //右上角描点区域
    m_rectRightTop = QRect(width - DEFAULT_MARGIN, 0, DEFAULT_MARGIN, DEFAULT_MARGIN);
    //左下角描点区域
    m_rectLeftBottom = QRect(0, height - DEFAULT_MARGIN, DEFAULT_MARGIN, DEFAULT_MARGIN);
    //右下角描点区域
    m_rectRightBottom = QRect(width - DEFAULT_MARGIN, height - DEFAULT_MARGIN, DEFAULT_MARGIN, DEFAULT_MARGIN);
}

//设置鼠标样式
void DirectTypeBaseKey::setMouseStyle(const QPoint &point)
{
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

//鼠标拖动事件的函数
void DirectTypeBaseKey::resetSizeAndPos(int offsetX, int offsetY)
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);
    double min_width_scale_ratio = DEFAULT_DIRECTION_SIZE / static_cast<double>(initialWidth);
    double min_height_scale_ratio = DEFAULT_DIRECTION_SIZE / static_cast<double>(initialHeight);

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

    updateSize(false);   //因为上边的setGeometry已经改变大小了，所以这里是false

}

//判断按下的手柄的区域位置
void DirectTypeBaseKey::getPressedAreaPosition(const QPoint &point)
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
void DirectTypeBaseKey::getMouseClickPos(QObject *obj,QPoint pos){
    if(obj == this){
        m_lastPos = pos;
    }else if(obj == mBgBtn){
        m_lastPos = QPoint(pos.x(),pos.y()+DEFAULT_DELETE_KEY/2);
    }else{
        QLineEdit *edit = static_cast<QLineEdit*>(obj);
        pos = edit->mapToParent(pos);
        m_lastPos = QPoint(pos.x(),pos.y()+DEFAULT_DELETE_KEY/2);
    }
}
bool DirectTypeBaseKey::eventFilter(QObject *obj, QEvent *event){
    //syslog(LOG_DEBUG, "[SteelingWheel] eventFilter (type = %d)", event->type());
    if (event->type() == QEvent::Resize) {   //按键弹出和拖动调整大小时都会触发，但是为啥没移动label？
        this->calculateTracingArea();
//        m_bgBtn->move(0,DEFAULT_DELETE_KEY/2 );
//        m_closeBtn->move(this->width() - m_closeBtn->width(),0);
    }
    else if ((event->type() == QEvent::MouseMove) && m_enableEdit) {
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        QPoint point = mouseEvent->pos();
        this->setMouseStyle(point);
        mIsBeEdit=true;
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
            }else if (moveX) {//在X的允许范围内移动
                this->move(movePoint.x(), this->pos().y());
            }else if (moveY) {//在Y的允许范围内移动
                this->move(this->pos().x(), movePoint.y());
            }
        }
        this->resetSizeAndPos(offsetX, offsetY);
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
        //m_lastPos = mouseEvent->pos();
        getMouseClickPos(obj,mouseEvent->pos());
        //判断按下的手柄的区域位置
        this->getPressedAreaPosition(m_lastPos);   //判断鼠标是在哪个位置被按下的
    }else if (event->type() == QEvent::MouseButtonRelease && m_enableEdit) {
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        if(mouseEvent->button() == Qt::RightButton){
            return true;
        }
        this->resetValues();
        this->setCursor(Qt::ArrowCursor);
    }else if(event->type() == QEvent::KeyPress){
            if(m_enableEdit){
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                if(!keyEvent->isAutoRepeat()){
                    emit keyboardPress(keyEvent);
                }
            }
            else{
                if (!SessionSettings::getInstance().windowUsePlatformWayland()) {
                    mMainWindow->getJoystickManager()->sendEventToMainDisplay(event);
                    event->accept();
                }
            }
            return true;
        }else if(event->type() == QEvent::KeyRelease){
            if(m_enableEdit){
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                if(!keyEvent->isAutoRepeat()){
                    emit keyboardRelease(keyEvent);
                }
            }
            else{
                if (!SessionSettings::getInstance().windowUsePlatformWayland()) {
                    mMainWindow->getJoystickManager()->sendEventToMainDisplay(event);
                    event->accept();
                }
            }
            return true;
        }else if((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease ||
                  event->type() == QEvent::MouseButtonDblClick)&& !m_enableEdit){
             sendEventToApp(event);
             event->accept();
             return true;
         }

    return QWidget::eventFilter(obj, event);
}

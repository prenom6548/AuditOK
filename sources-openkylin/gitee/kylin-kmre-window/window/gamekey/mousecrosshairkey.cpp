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

#include "mousecrosshairkey.h"
#include "joystickmanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

MouseCrossHairKey::MouseCrossHairKey(KmreWindow* window) 
    : JoystickBaseKey(window, CROSSHAIR)
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    mWidthRatio = DEFAULT_CROSS_HAIR_WIDTH  / static_cast<double>(initialWidth);
    mHeightRatio = DEFAULT_CROSS_HAIR_HEIGHT / static_cast<double>(initialHeight);

    this->resize(mWidthRatio*displayWidth,mHeightRatio*displayHeight);
    setMouseTracking(false);
    //setWindowFlag(Qt::X11BypassWindowManagerHint);

    mBgLabel = new QLabel(this);
    mBgLabel->resize(this->width()-DEFAULT_CROSS_EDGE_MARGIN*2,this->height()-DEFAULT_CROSS_EDGE_MARGIN*2);
    mBgLabel->move(DEFAULT_CROSS_EDGE_MARGIN,DEFAULT_CROSS_EDGE_MARGIN);
    mBgLabel->setStyleSheet("QLabel{background:transparent;border-image:url(:/new/res/crosshair_bg.svg);}");
    mBgLabel->setMouseTracking(true);
    mBgLabel->installEventFilter(this);

    mContainWidget = new QWidget(this);
    mContainWidget->setFixedSize(DEFAULT_CROSS_CORE_SIZE,DEFAULT_CROSS_CORE_SIZE);
    mContainWidget->installEventFilter(this);
    mContainWidget->move((this->width()-DEFAULT_CROSS_CORE_SIZE)/2,(this->height()-DEFAULT_CROSS_CORE_SIZE)/2);

    mCloseBtn = new QPushButton(this);
    mCloseBtn->setFixedSize(DEFAULT_DELETE_KEY, DEFAULT_DELETE_KEY);
    mCloseBtn->setStyleSheet("QPushButton{background:transparent;border-image:url(:/new/res/close_normal.svg);}"
                              "QPushButton:hover{background:transparent;border-image:url(:/new/res/close_hover.svg);}");
    //mCloseBtn->move(this->width() - mCloseBtn->width(), 0);
    connect(mCloseBtn, &QPushButton::clicked, this, [this] {
        emit deleteGameKey(CROSSHAIR, 0);
    });

    mCheckBox = new QCheckBox(this);
    QFont font;
    font.setPointSize(8);
    mCheckBox->setFont(font);
    mCheckBox->setText(tr("PointReset"));
    mCheckBox->setFixedSize(DEFAULT_CROSS_CHECKBOX_WIDTH,DEFAULT_CROSS_CHECKBOX_HEIGHT);

    mBgBtn = new QPushButton(this);
    mBgBtn->setFixedSize(DEFAULT_CROSS_ICON_SIZE,DEFAULT_CROSS_ICON_SIZE);
    mBgBtn->setStyleSheet("QPushButton{background:transparent;border-image:url(:/new/res/cross_hair_hover.svg);}");
    mBgBtn->setMouseTracking(true);
    mBgBtn->installEventFilter(this);

    QHBoxLayout* mHlayout = new QHBoxLayout;
    mHlayout->addStretch(1);
    mHlayout->addWidget(mCheckBox);
    mHlayout->addWidget(mCloseBtn);

    mEdit = new QLineEdit(this);
    mEdit->setFixedSize(DEFAULT_CROSS_EDIT_SIZE,DEFAULT_CROSS_EDIT_SIZE);
    mEdit->setFocusPolicy(Qt::ClickFocus);
    mEdit->setAlignment(Qt::AlignCenter);
    mEdit->setContextMenuPolicy(Qt::NoContextMenu);
    mEdit->setAttribute(Qt::WA_InputMethodEnabled,false);
    //border-image:url(:/new/res/single_key_normal.svg);color:rgba(255,255,255,1);
    mEdit->setStyleSheet("QLineEdit{background:transparent;border-image:url(:/new/res/single_key_hover.svg);color:rgba(255,255,255,1);}");
    mEdit->setMouseTracking(true);
    mEdit->installEventFilter(this);

    QHBoxLayout* mEditLayout = new QHBoxLayout;
    mEditLayout->addStretch();
    mEditLayout->addWidget(mEdit);
    mEditLayout->addStretch();
    mBgBtn->setLayout(mEditLayout);

    mSensiWidget = new SensitivityWidget(this);
    mSensiWidget->setFixedSize(DEFAULT_CROSS_SENS_WIDTH,DEFAULT_CROSS_SENS_HEIGHT);
    connect(mSensiWidget,SIGNAL(currentSliderValue(int)),this,SLOT(updateComboFreq(int)));

    QVBoxLayout* mContainLayout = new QVBoxLayout();
    mContainLayout->setMargin(0);
    mContainLayout->setSpacing(2);
    //mContainLayout->addStretch();
    mContainLayout->addLayout(mHlayout);
    mContainLayout->addWidget(mBgBtn);
    mContainLayout->addWidget(mSensiWidget);
   // mContainLayout->addStretch();
    mContainLayout->setAlignment(mHlayout,Qt::AlignHCenter);
    mContainLayout->setAlignment(mBgBtn,Qt::AlignHCenter);
    mContainLayout->setAlignment(mSensiWidget,Qt::AlignHCenter);
    mContainWidget->setLayout(mContainLayout);


    //this->installEventFilter(this);
}

void MouseCrossHairKey::enableEdit(bool enable)
{
    //qDebug( )<< "MouseCrossHairKey::enableEdit"<<enable;
    this->setVisible(enable);
}
bool MouseCrossHairKey::isPointReset(){
    return mIsPointReset;
}
void MouseCrossHairKey::setPointReset(bool isReset){
    mIsPointReset = isReset;
}

int MouseCrossHairKey::getCheckBoxStatus(){
    return mCheckBox->checkState();
}
void MouseCrossHairKey::setCheckBoxStatus(int status){
    mCheckBox->setCheckState(status);
}
void MouseCrossHairKey::updateComboFreq(int freq){
    this->mComboFreq = freq;
}
void MouseCrossHairKey::setComboFreq(int freq){
    mSensiWidget->setSliderLabelDefault(freq);
}
void MouseCrossHairKey::setKeyString(QString character){
    if(mEdit){
        setShowKeyString(mEdit,character);
    }
}

void MouseCrossHairKey::updateSize(bool update_panel){

    int displayWidth, displayHeight;
    mMainWindow->getJoystickManager()->getMainWidgetDisplaySize(displayWidth, displayHeight);
    if(update_panel){
        this->resize(mWidthRatio * displayWidth, mHeightRatio * displayHeight);
    }
    mBgLabel->resize(this->width()-DEFAULT_CROSS_EDGE_MARGIN*2,this->height()-DEFAULT_CROSS_EDGE_MARGIN*2);
    mBgLabel->move(DEFAULT_CROSS_EDGE_MARGIN,DEFAULT_CROSS_EDGE_MARGIN);
    mContainWidget->move((this->width()-DEFAULT_CROSS_CORE_SIZE)/2,(this->height()-DEFAULT_CROSS_CORE_SIZE)/2);
    //mCloseBtn->move(this->width() - mCloseBtn->width(), 0);
}

void MouseCrossHairKey::getTranslateCenterPos(QPoint pos){
    event_coordinate_x = pos.x()+this->width()/2;
    event_coordinate_y = pos.y()+this->height()/2;
}

void MouseCrossHairKey::getOperateHW(double &w,double &h){
//    int displayWidth, displayHeight, initialWidth, initialHeight;
//    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    w = this->width();
    h = this->height();
}

void MouseCrossHairKey::resetValues(){
    //m_mousePressed = false;
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
void MouseCrossHairKey::calculateTracingArea()
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
void MouseCrossHairKey::setMouseStyle(const QPoint &point)
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
void MouseCrossHairKey::resetSizeAndPos(int offsetX, int offsetY)
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getJoystickManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);
    double min_width_scale_ratio = DEFAULT_CROSS_HAIR_WIDTH / static_cast<double>(initialWidth);
    double min_height_scale_ratio = DEFAULT_CROSS_HAIR_WIDTH / static_cast<double>(initialHeight);

    int min_width = min_width_scale_ratio * displayWidth;
    int min_height = min_height_scale_ratio * displayHeight;
    QRect validRect = mMainWindow->getJoystickManager()->getGameKeyValidRect();

    if (m_pressedLeft) {
        int new_size = this->width() - offsetX*2;

        if ((min_width <= new_size) &&
                (this->x() + offsetX > validRect.x())&&
                (this->x()+this->width()<validRect.right())) {
            this->setGeometry(this->x() + offsetX,rectY, new_size, rectH);
        }
    }else if (m_pressedRight) {
        int new_size = rectW + offsetX;
        if ((min_width <= new_size) &&
            (rectX + new_size < validRect.right()) &&
            (this->x()>validRect.x())) {
            this->setGeometry(rectX - offsetX/2, rectY, new_size,rectH);
        }
    }else if (m_pressedTop) {
        int new_size = this->height() - offsetY*2;
        if ((min_height <= new_size) &&
            (this->y()+this->height()<validRect.bottom()) &&
            (this->y() + offsetY > validRect.y())) {
            this->setGeometry(rectX, this->y() + offsetY, rectW, new_size);
        }
    }else if (m_pressedBottom) {
        int new_size = rectH + offsetY;
        if ((min_height <= new_size) &&
            (this->y()>validRect.y()) &&
            (rectY + new_size < validRect.bottom())) {
            this->setGeometry(rectX, rectY-offsetY/2, rectW, new_size);
        }
    }else if (m_pressedLeftTop) {
        // don't support
    }else if (m_pressedRightTop) {
        // don't support
    }else if (m_pressedLeftBottom) {
        // don't support
    }else if (m_pressedRightBottom) {
        // don't support
    }
    updateSize(false);   //因为上边的setGeometry已经改变大小了，所以这里是false

}

//判断按下的手柄的区域位置
void MouseCrossHairKey::getPressedAreaPosition(const QPoint &point)
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
//    else {
//        m_mousePressed = true;
//    }
}

bool MouseCrossHairKey::eventFilter(QObject *obj, QEvent *event)
{
    //syslog(LOG_DEBUG, "[SteelingWheel] eventFilter (type = %d)", event->type());
    if (event->type() == QEvent::Resize) {   //按键弹出和拖动调整大小时都会触发，但是为啥没移动label？
        this->calculateTracingArea();
//        m_bgBtn->move(0,DEFAULT_DELETE_KEY/2 );
//        mCloseBtn->move(this->width() - mCloseBtn->width(),0);
    }
    else if ((event->type() == QEvent::MouseMove) && m_enableEdit && obj!=this) {
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
            return true;
        }else{
            this->resetSizeAndPos(offsetX, offsetY);
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonPress) {
        //记住当前控件坐标和宽高以及鼠标按下的坐标
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        if(mouseEvent->button() == Qt::RightButton){
            return true;
        }
        if(obj == mBgBtn){
            m_lastPos = mouseEvent->globalPos()-this->pos();
            m_mousePressed = true;
            return true;
        }else if(obj == mBgLabel){
            m_lastPos = mouseEvent->pos();
            rectX = this->x();
            rectY = this->y();
            rectW = this->width();
            rectH = this->height();
            //判断按下的手柄的区域位置
            this->getPressedAreaPosition(m_lastPos);   //判断鼠标是在哪个位置被按下的
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease){
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        if(mouseEvent->button() == Qt::RightButton){
            return true;
        }
        if(obj == mBgLabel){
            this->resetValues();
        }else if(obj == mBgBtn){
            m_mousePressed = false;
        }
        this->setCursor(Qt::ArrowCursor);
    }
    else if ((!m_enableEdit) && ((event->type() == QEvent::KeyPress) || (event->type() == QEvent::KeyRelease))) {
        if (!SessionSettings::getInstance().windowUsePlatformWayland()) {
            //syslog(LOG_DEBUG, "[SteelingWheel] Send key event (type = %d) to main display", event->type());
            mMainWindow->getJoystickManager()->sendEventToMainDisplay(event);
            event->accept();
            return true;
        }
    }
    else if(event->type() == QEvent::KeyPress){
            if(m_enableEdit){
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                if(!keyEvent->isAutoRepeat()){
                    emit keyboardPress(keyEvent);
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
            return true;
        }

    return QWidget::eventFilter(obj, event);
}

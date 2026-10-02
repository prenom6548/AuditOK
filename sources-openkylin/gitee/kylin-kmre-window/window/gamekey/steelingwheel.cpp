/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
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

#include "steelingwheel.h"
#include "gamekeymanager.h"
#include "displaymanager/displaymanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <sys/syslog.h>
#include <QStyleOption>
#include <QHBoxLayout>
#include <QPainter>

#define DEFAULT_MARGIN  8
#define DEFAULT_LABEL_AWDS_SIZE 20
#define MIN_LABEL_AWDS_TEXT_SIZE 12
#define MAX_LABEL_AWDS_TEXT_SIZE 26

SteelingWheel::SteelingWheel(KmreWindow* window)
    : BaseKey(window)
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getDisplayManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);
    //syslog(LOG_DEBUG, "[SteelingWheel][%s] displayWidth = %d, displayHeight = %d, initialWidth = %d, initialHeight = %d", 
    //        __func__, displayWidth, displayHeight, initialWidth, initialHeight);

    m_width_scale_ratio = DEFAULT_STEELING_WHEEL_SIZE / static_cast<double>(initialWidth);
    m_height_scale_ratio = DEFAULT_STEELING_WHEEL_SIZE / static_cast<double>(initialHeight);
    resize(m_width_scale_ratio * displayWidth, m_height_scale_ratio * displayHeight);

    m_labelA = new QLabel("A", this);
    m_labelA->setAlignment(Qt::AlignCenter);
    m_labelA->setStyleSheet("QLabel{background:transparent;border:none;color:rgba(255,255,255,1);}");
    m_labelA->setFixedSize(DEFAULT_LABEL_AWDS_SIZE, DEFAULT_LABEL_AWDS_SIZE);

    m_labelW = new QLabel("W",this);
    m_labelW->setAlignment(Qt::AlignCenter);
    m_labelW->setStyleSheet("QLabel{background:transparent;border:none;color:rgba(255,255,255,1);}");
    m_labelW->setFixedSize(DEFAULT_LABEL_AWDS_SIZE, DEFAULT_LABEL_AWDS_SIZE);

    m_labelD = new QLabel("D",this);
    m_labelD->setAlignment(Qt::AlignCenter);
    m_labelD->setStyleSheet("QLabel{background:transparent;border:none;color:rgba(255,255,255,1);}");
    m_labelD->setFixedSize(DEFAULT_LABEL_AWDS_SIZE, DEFAULT_LABEL_AWDS_SIZE);

    m_labelS = new QLabel("S",this);
    m_labelS->setAlignment(Qt::AlignCenter);
    m_labelS->setStyleSheet("QLabel{background:transparent;border:none;color:rgba(255,255,255,1);}");
    m_labelS->setFixedSize(DEFAULT_LABEL_AWDS_SIZE, DEFAULT_LABEL_AWDS_SIZE);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(m_labelW, 0, 1, 1, 1, Qt::AlignTop);
    layout->addWidget(m_labelS, 2, 1, 1, 1, Qt::AlignBottom);
    layout->addWidget(m_labelA, 1, 0, 1, 1, Qt::AlignLeft);
    layout->addWidget(m_labelD, 1, 2, 1, 1, Qt::AlignRight);
    layout->setMargin(5);
    layout->setSpacing(5);
    this->setLayout(layout);

    m_bgBtn = new QPushButton(this);
    m_bgBtn->setFlat(true);
    m_bgBtn->resize(DEFAULT_STEELING_WHEEL_SIZE, DEFAULT_STEELING_WHEEL_SIZE);
    m_bgBtn->setStyleSheet("QPushButton{background:transparent; border-image: url(:/res/key_walk_hover.png);}"
                           "QPushButton:focus{outline: none;}");
    m_bgBtn->setMouseTracking(true);
    m_bgBtn->installEventFilter(this);

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFixedSize(20, 20);
    m_closeBtn->setStyleSheet("QPushButton{background:transparent;background-image:url(:/res/key_close_normal.png);}"
                              "QPushButton:hover{background:transparent;background-image:url(:/res/key_close_hover.png);}");
    m_closeBtn->move(this->width() - m_closeBtn->width(), 0);
    connect(m_closeBtn, &QPushButton::clicked, this, [this] {
        emit deleteGameSteelingWheel();
    });

    m_overlayBtn = new OverlayButton(mMainWindow, this);
    m_overlayBtn->resize(this->width(), this->height());
    m_overlayBtn->setVisible(false);

    this->installEventFilter(this);
    resetValues();
}

SteelingWheel::~SteelingWheel()
{
    delete m_labelA;
    delete m_labelW;
    delete m_labelD;
    delete m_labelS;
    delete m_bgBtn;
#ifndef KYLIN_V10
    delete m_closeBtn;// delete m_closeBtn will cause crash on V10 platform, why ?
#endif
}

void SteelingWheel::enableEdit(bool enable)
{
    m_bgBtn->setStyleSheet(QString("QPushButton{background:transparent; border-image: url(:/res/%1);}"
                            "QPushButton:focus{outline: none;}")
                            .arg(enable ? "key_walk_hover.png" : "key_walk_normal.png"));
    m_closeBtn->setVisible(enable);
    m_enableEdit = enable;
    m_overlayBtn->setVisible(!enable);// 覆盖 m_bgBtn，防止按键被鼠标拖动
    this->setCursor(Qt::ArrowCursor);
}

void SteelingWheel::updatePos()
{
    DisplayManager *displayManager = mMainWindow->getDisplayManager();
    int xpos = displayManager->getDisplayWidth() * event_coordinate_x;
    int ypos = displayManager->getDisplayHeight() * event_coordinate_y;
    QPoint coordnate = mMainWindow->getGameKeyManager()->getGlobalPos(QPoint(xpos, ypos));
    move(coordnate);
    checkPosOutsideScreen(coordnate);
}

void SteelingWheel::updateSize(bool update_panel)
{
    QSize displaySize = mMainWindow->getDisplayManager()->getMainWidgetDisplaySize();
    if (update_panel) {
        this->resize(m_width_scale_ratio * displaySize.width(), m_height_scale_ratio * displaySize.height());
    }

    int icon_size = std::min(this->width(), this->height());
    m_bgBtn->setFixedSize(icon_size, icon_size);
    m_bgBtn->move((this->width() - m_bgBtn->width()) / 2, (this->height() - m_bgBtn->height()) / 2);
    m_closeBtn->move(this->width() - m_closeBtn->width(), 0);

    double width_scale_ratio = this->width() / static_cast<double>(DEFAULT_STEELING_WHEEL_SIZE);
    double height_scale_ratio = this->height() / static_cast<double>(DEFAULT_STEELING_WHEEL_SIZE);
    int width = DEFAULT_LABEL_AWDS_SIZE * width_scale_ratio;
    int height = DEFAULT_LABEL_AWDS_SIZE * height_scale_ratio;

    QFont ft;
    int font_size = MIN_LABEL_AWDS_TEXT_SIZE + (MAX_LABEL_AWDS_TEXT_SIZE - MIN_LABEL_AWDS_TEXT_SIZE) * (width_scale_ratio - 1.0) / 3.0;
    ft.setPointSize(font_size);
    m_labelA->setFixedSize(width, height);
    m_labelA->setFont(ft);
    m_labelW->setFixedSize(width, height);
    m_labelW->setFont(ft);
    m_labelD->setFixedSize(width, height);
    m_labelD->setFont(ft);
    m_labelS->setFixedSize(width, height);
    m_labelS->setFont(ft);

    m_overlayBtn->setFixedSize(icon_size, icon_size);
}

void SteelingWheel::setCoordinateData(double x, double y, double width, double height)
{
    event_coordinate_x = x;
    event_coordinate_y = y;
    m_width_scale_ratio = width;
    m_height_scale_ratio = height;
}

void SteelingWheel::resetValues()
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
void SteelingWheel::calculateTracingArea()
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
void SteelingWheel::setMouseStyle(const QPoint &point)
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

void SteelingWheel::resetSizeAndPos(int offsetX, int offsetY)
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getDisplayManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);
    double min_width_scale_ratio = DEFAULT_STEELING_WHEEL_SIZE / static_cast<double>(initialWidth);
    double min_height_scale_ratio = DEFAULT_STEELING_WHEEL_SIZE / static_cast<double>(initialHeight);

    int min_width = min_width_scale_ratio * displayWidth;
    int min_height = min_height_scale_ratio * displayHeight;
    QRect validRect = mMainWindow->getGameKeyManager()->getGameKeyValidRect();

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

    updateSize(false);
    
}

//判断按下的手柄的区域位置
void SteelingWheel::getPressedAreaPosition(const QPoint &point)
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

bool SteelingWheel::eventFilter(QObject *obj, QEvent *event)
{
    //syslog(LOG_DEBUG, "[SteelingWheel] eventFilter (type = %d)", event->type());
    if (event->type() == QEvent::Resize) {
        this->calculateTracingArea();
        m_bgBtn->move((this->width() - m_bgBtn->width()) / 2, (this->height() - m_bgBtn->height()) / 2);
        m_closeBtn->move(this->width() - m_closeBtn->width(), 0);
    }
    else if ((event->type() == QEvent::MouseMove) && m_enableEdit) {
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        QPoint point = mouseEvent->pos();
        this->setMouseStyle(point);

        //根据当前鼠标位置,计算XY轴移动了多少
        int offsetX = point.x() - m_lastPos.x();
        int offsetY = point.y() - m_lastPos.y();

        //根据按下处的位置判断是否是移动控件还是拉伸控件
        if (m_mousePressed) {
            QRect validRect = mMainWindow->getGameKeyManager()->getGameKeyValidRect();
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
        }

        this->resetSizeAndPos(offsetX, offsetY);
    }else if (event->type() == QEvent::MouseButtonPress) {
        //记住当前控件坐标和宽高以及鼠标按下的坐标
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        rectX = this->x();
        rectY = this->y();
        rectW = this->width();
        rectH = this->height();
        m_lastPos = mouseEvent->pos();

        //判断按下的手柄的区域位置
        this->getPressedAreaPosition(m_lastPos);
    }else if (event->type() == QEvent::MouseButtonRelease) {
        this->resetValues();
        this->setCursor(Qt::ArrowCursor);
    }

    return QWidget::eventFilter(obj, event);
}


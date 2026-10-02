/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "sliderwidget.h"
#include "UI/base/widgetstyle.h"
#include "UIControl/player/player.h"
#include <ukuistylehelper/ukuistylehelper.h>

#include <QPainter>
#include <QColor>
#include <QRect>
#include <QTimer>
#include <ukuistylehelper/ukuistylehelper.h>
#include <ukuiwindowhelper/ukuiwindowhelper.h>
SliderWidget::SliderWidget(QWidget *parent) : QDialog(parent)
{
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
    windowHelper->removeTitleBar();
    windowHelper->setSkipTaskBar(true);

    this->installEventFilter(this);

    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    this->setProperty("useStyleWindowManager", false);

    m_radius = 8;

    initUi();

    // 当dialog弹出后，通过定时器检查是否切换了焦点
    m_focusCheckTimer = new QTimer(this);
    connect(m_focusCheckTimer, &QTimer::timeout, this, [this]() {
        if (!this->hasFocus() && !this->isActiveWindow()) {
            qDebug() << "Window lost focus in Wayland, hiding.";
            this->hide();
            g_user_signal->historyButtonIsShow = false;
            m_focusCheckTimer->stop();
        }
    });
}


void SliderWidget::initUi()
{
    setFixedSize(30,90);

    vSlider = new QSlider(this);
    vSlider->setAccessibleName("kylin-music_QSlider_vSlider");
    vSlider->installEventFilter(this);
    vSlider->setOrientation(Qt::Vertical);
    vSlider->setRange(0, 100);
    vSlider->setValue(playController::getInstance().getVolume());

    HLayout = new QHBoxLayout;
    HLayout->addWidget(vSlider);
    HLayout->setContentsMargins(0, 10, 0, 10);
    this->setLayout(HLayout);

    initColor();
}


/**
 * @brief SliderWidget::eventFilter 鼠标滑块点击
 * @param obj
 * @param event
 * @return
 */
bool SliderWidget::eventFilter(QObject *obj, QEvent *event)
{
   if(obj == vSlider) {
        if (event->type()==QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (mouseEvent->button() == Qt::LeftButton) {
               int dur = vSlider->maximum() - vSlider->minimum();
               int pos = vSlider->minimum() + (dur * (vSlider->height() - ((double)mouseEvent->y())) / vSlider->height());

               if(pos != vSlider->sliderPosition()) {
                    vSlider->setValue(pos);
                }
            }
        }

        if (event->type() == QEvent::Show) {
            m_focusCheckTimer->start(500);
        }

        if (event->type() == QEvent::Hide) {
            m_focusCheckTimer->stop();
        }
    }

    return QObject::eventFilter(obj,event);
}
#ifndef WHITE_TEST
bool SliderWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);

    if(eventType != "xcb_generic_event_t") {
        return false;
    }

    xcb_generic_event_t *event = (xcb_generic_event_t*)message;

    switch (event->response_type & ~0x80) {
        case XCB_FOCUS_OUT:
            QRect rect(volunmPosX, volunmPosY, volunmPosWidth, volunmPosHeight);
            if(rect.contains(QCursor::pos(), false)) {
                return 0;
            } else {
                this->hide();
                break;
            }
    }

    return false;
}

void SliderWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿;
    QRect rect = this->rect();
    painter.setBrush(QBrush(m_color));
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(rect, m_radius, m_radius);

    QWidget::paintEvent(event);
}
#endif
void SliderWidget::changeVolumePos(int posX, int posY, int width, int height)
{
    volunmPosX = posX;
    volunmPosY = posY;
    volunmPosWidth = width;
    volunmPosHeight = height;
}

void SliderWidget::initColor()
{
    QGraphicsDropShadowEffect * effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0);//设置阴影距离
    if (WidgetStyle::themeColor == 1) {
        m_color = QColor(50, 50, 50, 240);
        effect->setColor(QColor(0,0,0,10));//设置阴影颜色
    } else {
        m_color = QColor(239, 240, 240, 240);
        effect->setColor(QColor(0,0,0,90));//设置阴影颜色
    }
    effect->setBlurRadius(0);//设置阴影圆角
    this->setGraphicsEffect(effect);
}

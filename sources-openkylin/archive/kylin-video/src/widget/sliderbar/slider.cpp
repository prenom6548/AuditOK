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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "slider.h"

#include <QDebug>
#include <QTimer>
#include <QMouseEvent>

Slider::Slider(QWidget *parent) : QWidget(parent)
{
    m_canHide = true;

    m_widgetSlider = new QWidget(this);
    m_widgetSlider->setAccessibleName("kylin-video_QWidget_m_widgetSlider");
    m_widgetSlider->resize(size());
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_widgetSlider->setObjectName("slider");
    m_widgetSlider->setStyleSheet("#slider{background-color:#FFFFFF;border-radius:6px;}");
}

Slider::~Slider()
{
    if (m_widgetSlider) {
        delete m_widgetSlider;
        m_widgetSlider = nullptr;
    }
}

void Slider::setHide()
{
    if (m_canHide) {
        // 不延迟有冲突会一直 hide show，不知道为啥
        QTimer::singleShot(100, [this](){
            if (m_canHide) {
#ifndef WHITE_TEST
                hide();
#endif
            }
        });
    }
}

/*
当鼠标按压的时候，此时间段内的位置信息不通过seek发送给mpv
鼠标释放的时候，通过seek发送新的位置信息
 */
void Slider::updatePos(bool needSeek)
{
    int correct_l = m_mouse_x - width() / 2;
    // 点击位置矫正，如果鼠标点击位置不在中心的话要移到中心
    if (correct_l != 0) {
        if (correct_l + x() < m_min_x) {
            move(m_min_x, y());
        }
        else if (correct_l + x() > m_max_x) {
            move(m_max_x, y());
        }
        else {
            move(x() + correct_l, y());
        }
    }

    // 矫正之后去更新进度条的位置
    if (needSeek) {
        emit posXUpdate(x());
    } else {
        emit posXUpdateWithoutSeek(x());
    }
}

void Slider::enterEvent(QEvent *e)
{
    m_canHide = false;
}

void Slider::leaveEvent(QEvent *e)
{
    m_canHide = true;
    hide();
}

void Slider::resizeEvent(QResizeEvent *e)
{
    m_widgetSlider->resize(size());
}

void Slider::mouseMoveEvent(QMouseEvent *e)
{
        m_mouse_x = e->x();
        updatePos();
}

void Slider::mousePressEvent(QMouseEvent *e)
{
    m_canHide = false;

    // 滑块被点击的时候进度条不能自动更新位置，只有被释放的时候才能去更新位置
    emit lockProgress(true);
}

void Slider::mouseReleaseEvent(QMouseEvent *e)
{
    // 鼠标释放的时候进度条可以自动更新位置
    emit lockProgress(false);
    m_mouse_x = e->x();
    updatePos(true);

}

FixedSlider::FixedSlider(QWidget *parent) :
    QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_widgetSlider = new QWidget(this);
    m_widgetSlider->setAccessibleName("kylin-video_QWidget_m_widgetSlider");
    setFixedSize(6, 10);
    m_widgetSlider->setFixedSize(6, 10);
    m_widgetSlider->setStyleSheet("background-color:#11A8FF;"
                  "border-radius:3px;"
                  "border:1px solid #FFFFFF;");
}

FixedSlider::~FixedSlider()
{
    if (m_widgetSlider) {
        delete m_widgetSlider;
        m_widgetSlider = nullptr;
    }
}

void FixedSlider::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        emit clicked();
    }
}

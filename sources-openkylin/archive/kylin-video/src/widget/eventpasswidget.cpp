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
#include "eventpasswidget.h"

#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QMouseEvent>
#include <QTouchEvent>

#include "global/globalsignal.h"
#include "global/global.h"

#include <ukui-log4qt.h>

using namespace Global;

EventPassWidget::EventPassWidget(QWidget *parent) :
    QWidget(parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    setMouseTracking(true);

    m_dbClicked = false;
    m_mouseUsed = true;

    initGlobalSig();

    m_checkMouseTimer = new QTimer;
    m_checkMouseTimer->setObjectName("kylin-video_QTimer_m_checkMouseTimer");
    m_checkMouseTimer->setInterval(2000);

    //添加定时器对光标位置进行判断，若位置发生改变则调用光标移动的逻辑
    m_mouseTrackTimer = new QTimer;
    connect(m_mouseTrackTimer,&QTimer::timeout,[&](){
#ifndef WHITE_TEST
        QPoint current = mapFromGlobal(QCursor::pos());
        if(current != m_posLast){
            m_posLast = current;
            onMove();
        }
#endif
    });
    m_mouseTrackTimer->start(100);
    connect(m_checkMouseTimer, &QTimer::timeout, [&](){
        if (!m_hasVideo)
            return;
        if (m_isMouseEnter) {
            if (m_mouseUsed) {
                KyInfo() << "set cursor blank cursor";
                this->setCursor(QCursor(Qt::BlankCursor));
            }
        }
        g_user_signal->hideBar(true);

        m_checkMouseTimer->stop();
    });

    m_lMouseClickTimer = new QTimer;
    m_lMouseClickTimer->setObjectName("kylin-video_QTimer_m_lMouseClickTimer");
    m_lMouseClickTimer->setInterval(300);
    connect(m_lMouseClickTimer, &QTimer::timeout, [&](){
        if (m_dbClicked) {
            m_lMouseClickTimer->stop();
            return;
        }
        emit mousePressed();
        m_lMouseClickTimer->stop();
    });
}

EventPassWidget::~EventPassWidget()
{

}

void EventPassWidget::setMouseUsed(bool used)
{
    m_mouseUsed = used;
    if (!m_mouseUsed) {
        setCursor(QCursor(Qt::ArrowCursor));
        m_checkMouseTimer->stop();
    }
}

void EventPassWidget::playStateChange()
{
    if(g_playstate < 0) {
        m_checkMouseTimer->stop();
    }
    else {
        m_checkMouseTimer->start();
    }
}

void EventPassWidget::videIdChange(int vid)
{
    if(vid < 0)
        m_hasVideo = false;
    else
        m_hasVideo = true;
}

void EventPassWidget::initGlobalSig()
{
    connect(g_core_signal, &GlobalCoreSignal::sigStateChange, this, &EventPassWidget::playStateChange);
    connect(g_core_signal, &GlobalCoreSignal::sigVideoIdChange, this, &EventPassWidget::videIdChange);
}

void EventPassWidget::onMove()
{

    if (m_isMousePressed) {
        m_posCurrent = this->pos();

        int xc = abs(m_posCurrent.x() - m_posLast.x());
        int yc = abs(m_posCurrent.y() - m_posLast.y());
        if (xc < yc) {
            m_volumeChange = yc * 200 / height();
            //符合音量调节
            if (m_volumeChange > 0) {
                if (m_posCurrent.y() - m_posLast.y() > 2) {
                    g_user_signal->setVolumeDown(m_volumeChange);
                    m_posLast = m_posCurrent;
                }
                else {
                    g_user_signal->setVolumeUp(m_volumeChange);
                    m_posLast = m_posCurrent;
                }
            }
        }
    }

    m_isMouseEnter = true;
    m_checkMouseTimer->stop();
    setCursor(QCursor(Qt::ArrowCursor));
    g_user_signal->hideBar(false);
    if(g_playstate > 0)
        m_checkMouseTimer->start();

}

void EventPassWidget::mousePressEvent(QMouseEvent *e)
{
    m_mousePosPressed = e->pos();
    m_posStart = e->pos();
    m_posLast = e->pos();

    // 右键点击触发显示菜单
    if (e->button() == Qt::LeftButton) {
        m_isMousePressed = true;
        m_timePressStart = QDateTime::currentMSecsSinceEpoch();
    }
    return QWidget::mousePressEvent(e);
}

#ifndef WHITE_TEST
void EventPassWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_isMousePressed = false;
    // 左键离开触发暂停，如果左键按得时间很长就不触发了，不然会和触摸屏长按冲突
    if (e->button() == Qt::LeftButton && (QDateTime::currentMSecsSinceEpoch() - m_timePressStart) < 500) {
        if (!m_mouseUsed)
            return QWidget::mouseReleaseEvent(e);
        if (e->pos() == m_mousePosPressed) {
            m_lMouseClickTimer->start();
        }
    }
    return QWidget::mouseReleaseEvent(e);
}

void EventPassWidget::mouseMoveEvent(QMouseEvent *e)
{
    onMove();
    e->accept();
}

void EventPassWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (!m_mouseUsed)
        return QWidget::mouseDoubleClickEvent(e);
    if(e->button() == Qt::LeftButton) {
        g_user_signal->fullScreen();
        m_dbClicked = true;
        QTimer::singleShot(1000, [this]{m_dbClicked = false;});
        m_lMouseClickTimer->stop();
    }
    return QWidget::mouseDoubleClickEvent(e);
}

void EventPassWidget::wheelEvent(QWheelEvent *e)
{
    // 鼠标滚轮和触控板双指滑动调节音量
    int wheel_distance = e->delta();
    if (abs(wheel_distance) / 120 == 1) {
        if (wheel_distance > 0)
            g_user_signal->setVolumeUp(5);
        else
            g_user_signal->setVolumeDown(5);
    }
    return QWidget::wheelEvent(e);
}

void EventPassWidget::enterEvent(QEvent *e)
{
    m_isMouseEnter = true;
    e->accept();
}

void EventPassWidget::leaveEvent(QEvent *e)
{
    m_isMouseEnter = false;
    m_isMousePressed = false;
    if(g_playstate > 0)
        m_checkMouseTimer->start();

    return QWidget::leaveEvent(e);
}
#endif
bool EventPassWidget::event(QEvent *e)
{
//    if (e->type() == QEvent::ContextMenu) {
//        setCursor(Qt::ArrowCursor);
//        m_isMouseEnter = false;
//        g_user_signal->showRightMenu();
//        return QWidget::event(e);
//    }

    return QWidget::event(e);
}

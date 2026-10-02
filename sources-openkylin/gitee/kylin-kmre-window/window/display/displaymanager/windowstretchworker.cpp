/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#include "windowstretchworker.h"
#include "kmrewindow.h"
#include "common.h"
#include "displaymanager.h"

#include <sys/syslog.h>
#include <QApplication>
#include <QStatusBar>
#include <QEvent>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QGSettings>
#include <QDebug>

WindowStretchWorker::WindowStretchWorker(KmreWindow *window)
    : mMainWindow(window)
    , m_enabled(true)
    , m_windowStretchReady(false)
{
    mMainWindow->installEventFilter(this);
    mMainWindow->setMouseTracking(true);
}

WindowStretchWorker::~WindowStretchWorker()
{
    
}

void WindowStretchWorker::init()
{
    if (mMainWindow->isVisible()) {
        m_oldSize = mMainWindow->size();
        m_minSize = mMainWindow->minimumSizeHint();
        m_aspectRatio = 1.0 * m_oldSize.width() / m_oldSize.height();
    }
}

void WindowStretchWorker::setEnabled(bool b)
{
    m_enabled = b;
}

bool WindowStretchWorker::updateOldSizeAndAspectRatio()
{
    QSize currSize = mMainWindow->size(); // 在窗口刚启动后，这个值偶尔会获取错误
    if ((currSize.width() <= 0) || (currSize.height() <= 0)) {
        return false;
    }

    m_oldSize = currSize;
    m_aspectRatio = 1.0 * currSize.width() / currSize.height();

    return true;
}

// 计算和设置窗口尺寸
void WindowStretchWorker::resizeWindowSize(QPoint point, bool updateDisplay)
{
    QPoint offsetPoint = point - m_oldPoint;
    int newWidth, newHeight;
    QPoint newPos = {0,0};//从左边调整窗口大小时还需要移动窗口
    bool keepAspect = !mMainWindow->getDisplayManager()->isDDSSupported();
    QSize maxSize = mMainWindow->getDisplayManager()->getAvailableScreenSize().size();

    if (keepAspect) {
        if ((maxSize.width() / m_aspectRatio) > maxSize.height()) {
            maxSize.setWidth(maxSize.height() * m_aspectRatio);
        }
        else if ((maxSize.height() * m_aspectRatio) > maxSize.width()) {
            maxSize.setHeight(maxSize.width() / m_aspectRatio);
        }
    }

    if ((m_minSize.width() <= 0) || (m_minSize.height() < 0)) {
        syslog(LOG_ERR, "[WindowStretchWorker][%s] Invalid window min size!", __func__);
        return;
    }
    if ((maxSize.width() < m_minSize.width()) || (maxSize.height() < m_minSize.height())) {
        syslog(LOG_ERR, "[WindowStretchWorker][%s] Invalid window max size!", __func__);
        return;
    }

    switch (m_cornerEdge) {
    case WindowStretchWorker::CE_Left: {
        newWidth = m_oldSize.width() - offsetPoint.x();
        newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
        newHeight = keepAspect ? (newWidth / m_aspectRatio) : m_oldSize.height();
        newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());

        newPos.setY(m_oldPos.y());
        newPos.setX(m_oldPos.x() - (newWidth - m_oldSize.width()));
    }
    break;
    case WindowStretchWorker::CE_Top: {
        newHeight = m_oldSize.height() - offsetPoint.y();
        newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
        newWidth = keepAspect ? (newHeight * m_aspectRatio) : m_oldSize.width();
        newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());

        newPos.setX(m_oldPos.x());
        newPos.setY(m_oldPos.y() - (newHeight - m_oldSize.height()));
    }
    break;
    case WindowStretchWorker::CE_TopLeft:
    {
        newWidth = m_oldSize.width() - offsetPoint.x();
        newHeight = m_oldSize.height() - offsetPoint.y();
        if (keepAspect) {
            if (newWidth > newHeight * m_aspectRatio) {
                newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
                newWidth = std::clamp(static_cast<int>(newHeight * m_aspectRatio), m_minSize.width(), maxSize.width());
            }
            else {
                newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
                newHeight = std::clamp(static_cast<int>(newWidth / m_aspectRatio), m_minSize.height(), maxSize.height());
            }
        }
        else {
            newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
            newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
        }

        newPos.setX(m_oldPos.x() - (newWidth - m_oldSize.width()));
        newPos.setY(m_oldPos.y() - (newHeight - m_oldSize.height()));
    }
    break;
    case WindowStretchWorker::CE_TopRight:
    {
        newWidth = m_oldSize.width() + offsetPoint.x();
        newHeight = m_oldSize.height() - offsetPoint.y();
        if (keepAspect) {
            if (newWidth > newHeight * m_aspectRatio) {
                newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
                newWidth = std::clamp(static_cast<int>(newHeight * m_aspectRatio), m_minSize.width(), maxSize.width());
            }
            else {
                newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
                newHeight = std::clamp(static_cast<int>(newWidth / m_aspectRatio), m_minSize.height(), maxSize.height());
            }
        }
        else {
            newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
            newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
        }

        newPos.setX(m_oldPos.x());
        newPos.setY(m_oldPos.y() - (newHeight - m_oldSize.height()));
    }
    break;
    case WindowStretchWorker::CE_Right: {
        newWidth = m_oldSize.width() + offsetPoint.x();
        newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
        newHeight = keepAspect ? (newWidth / m_aspectRatio) : m_oldSize.height();
        newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
    }
    break;
    case WindowStretchWorker::CE_Bottom: {
        newHeight = m_oldSize.height() + offsetPoint.y();
        newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
        newWidth = keepAspect ? (newHeight * m_aspectRatio) : m_oldSize.width();
        newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
    }
    break;
    case WindowStretchWorker::CE_BottomLeft:
    {
        newWidth = m_oldSize.width() - offsetPoint.x();
        newHeight = m_oldSize.height() + offsetPoint.y();
        if (keepAspect) {
            if (newWidth > newHeight * m_aspectRatio) {
                newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
                newWidth = std::clamp(static_cast<int>(newHeight * m_aspectRatio), m_minSize.width(), maxSize.width());
            }
            else {
                newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
                newHeight = std::clamp(static_cast<int>(newWidth / m_aspectRatio), m_minSize.height(), maxSize.height());
            }
        }
        else {
            newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
            newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
        }
        newPos.setY(m_oldPos.y());
        newPos.setX(m_oldPos.x() - (newWidth - m_oldSize.width()));
    }
    break;
    case WindowStretchWorker::CE_BottomRight:
    {
        newWidth = m_oldSize.width() + offsetPoint.x();
        newHeight = m_oldSize.height() + offsetPoint.y();
        if (keepAspect) {
            if (newWidth > newHeight * m_aspectRatio) {
                newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
                newWidth = std::clamp(static_cast<int>(newHeight * m_aspectRatio), m_minSize.width(), maxSize.width());
            }
            else {
                newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
                newHeight = std::clamp(static_cast<int>(newWidth / m_aspectRatio), m_minSize.height(), maxSize.height());
            }
        }
        else {
            newWidth = std::clamp(newWidth, m_minSize.width(), maxSize.width());
            newHeight = std::clamp(newHeight, m_minSize.height(), maxSize.height());
        }
    }
    break;
    default:
        return;
    }

    mMainWindow->getDisplayManager()->resetWindowSize(QSize(newWidth, newHeight), updateDisplay);
    if (newPos != QPoint(0,0)) {//当从左边调整窗口大小时还需要移动窗口
        mMainWindow->move(newPos);
    }

    m_windowSizeChanged = true;
}

// 设置各个区域的鼠标样式
void WindowStretchWorker::setMouseDirection(QWidget *w, const QPoint &pos)
{
    // 在 左、上、右、下、左上、右上、右下、左下 共8个地方进行拖拽和鼠标样式更换
    // 横向调整大小
    bool enableHorizontal = pos.x() > m_oldSize.width() - MOUSE_MARGINS || pos.x() < MOUSE_MARGINS;
    // 纵向调整大小
    bool enableVertical = pos.y() > m_oldSize.height() - MOUSE_MARGINS || pos.y() < MOUSE_MARGINS;//bool enableVertical = me->pos().y() > m_oldSize.height() - MOUSE_MARGINS;

    if (enableHorizontal && enableVertical) {
        if (pos.x() < MOUSE_MARGINS) {
            if (pos.y() < MOUSE_MARGINS) {
                w->setCursor(Qt::SizeFDiagCursor);
                m_cornerEdge = CE_TopLeft;
            }
            else {
                w->setCursor(Qt::SizeBDiagCursor);
                m_cornerEdge = CE_BottomLeft;
            }
        }
        else {
            if (pos.y() < MOUSE_MARGINS) {
                w->setCursor(Qt::SizeBDiagCursor);
                m_cornerEdge = CE_TopRight;
            }
            else {
                w->setCursor(Qt::SizeFDiagCursor);
                m_cornerEdge = CE_BottomRight;
            }
        }
    }
    else if (enableHorizontal) {
        w->setCursor(Qt::SizeHorCursor);
        if (pos.x() < MOUSE_MARGINS) {
            m_cornerEdge = CE_Left;
        }
        else {
            m_cornerEdge = CE_Right;
        }
    }
    else if (enableVertical) {
        w->setCursor(Qt::SizeVerCursor);
        if (pos.y() < MOUSE_MARGINS) {
            m_cornerEdge = CE_Top;
        }
        else {
            m_cornerEdge = CE_Bottom;
        }
    }
    else {
        m_cornerEdge = CE_Unknow;
        w->unsetCursor();
//        qApp->restoreOverrideCursor();
//        w->setCursor(Qt::ArrowCursor);
    }

    m_windowStretchReady = (enableHorizontal || enableVertical);

}

// 界面拉伸时重置相关参数
void WindowStretchWorker::updateGeoAndRatio(const QSize& minSz)
{
    updateOldSizeAndAspectRatio();
    syslog(LOG_DEBUG, "[%s] min size: w = %d, h = %d", __func__, minSz.width(), minSz.height());
    m_minSize = minSz;//w->minimumSizeHint();
}

bool WindowStretchWorker::eventFilter(QObject *watched, QEvent *event)
{
    //bool isKeyTab = false;
    if (mMainWindow != watched) {
        return false;
    }

    if (!m_enabled) {
        return false;
    }
    //syslog(LOG_DEBUG, "[WindowStretchWorker][%s] event type = %d", __func__, event->type());
    switch (static_cast<int>(event->type())) {
    case QEvent::MouseButtonPress:
        if (m_cornerEdge != CE_Unknow) {
            mMainWindow->grabMouse();
            if (updateOldSizeAndAspectRatio()) {
                m_oldPos = mMainWindow->frameGeometry().topLeft();
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                m_oldPoint = me->globalPos();
                m_enableResizing = true;
            }
        }
        break;
    case QEvent::MouseButtonRelease:
    {
        // 防止界面拉伸后，鼠标在界面上移动导致画面内容可以上下移动
        //DisplayManager::getDisplayManager()->focusWindow();
        if(m_enableResizing) {
            if (m_windowSizeChanged && mMainWindow->getDisplayManager()->isDDSSupported()) {
                QMouseEvent *me = static_cast<QMouseEvent*>(event);
                this->resizeWindowSize(me->globalPos(), true);
            }
            m_enableResizing = false;
            m_windowSizeChanged = false;
        }

        updateOldSizeAndAspectRatio();
        mMainWindow->releaseMouse();
    }
    break;
    case QEvent::MouseMove://For QWidget
    {
        //捕获鼠标之后收到的消息，在这里调整窗口大小
        if (m_enableResizing) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            bool updateDisplay = mMainWindow->getDisplayManager()->isDDSSupported() ? false : true;
            this->resizeWindowSize(me->globalPos(), updateDisplay);
        }
        else {
            // 重新获取 'm_oldSize' 的值并计算 'm_aspectRatio'，防止在窗口刚启动后，这2个值在函数 'updateGeoAndRatio' 里被错误设置
            if (updateOldSizeAndAspectRatio()) {
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                QPoint pos = me->globalPos() - mMainWindow->pos();
                this->setMouseDirection(mMainWindow, pos);
            }
        }
    }
    break;
    case QEvent::Enter:
        mMainWindow->setCursor(QCursor(Qt::ArrowCursor));
        break;
    case QEvent::Leave:
        mMainWindow->unsetCursor();
        break;
    default:
        QApplication::overrideCursor();//QApplication::setOverrideCursor(Qt::CrossCursor);
        break;
    }

    return false;
}

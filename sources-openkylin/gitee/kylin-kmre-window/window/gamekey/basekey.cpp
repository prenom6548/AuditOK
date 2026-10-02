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

#include "basekey.h"
#include <sys/syslog.h>
#include "displaymanager/displaymanager.h"
#include "gamekeymanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

OverlayButton::OverlayButton(KmreWindow* window, QWidget *parent) 
    : QPushButton(parent)
    , mMainWindow(window)
{
    setFlat(true);
    setStyleSheet("QPushButton{background:transparent;border:none;}"
                    "QPushButton:hover{background:transparent;border-image:url(:/res/empty.svg);}"
                    "QPushButton:focus{outline: none;}");
    installEventFilter(this);
}

bool OverlayButton::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || 
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint globalPos = mouseEvent->globalPos();
        int mainWidget_x, mainWidget_y, margin_width, titlebar_height;
        mMainWindow->getDisplayManager()->getMainWidgetInfo(mainWidget_x, mainWidget_y, margin_width, titlebar_height);
        QPoint pos = QPoint(globalPos.x() - mainWidget_x - margin_width, 
                            globalPos.y() - mainWidget_y - titlebar_height);
        //syslog(LOG_DEBUG, "[OverlayButton] pos: x = %d, y = %d", pos.x(), pos.y());
        if ((pos.x() >= 0) && (pos.y() >= 0)) {
            //syslog(LOG_DEBUG, "[OverlayButton] Send mouse event to main display");
#if defined(KYLIN_V10)
            QMouseEvent* me = new QMouseEvent(event->type(), pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            mGameKeyManager->sendEventToMainDisplay(me);
#else
            mouseEvent->setLocalPos(pos);
            mMainWindow->getGameKeyManager()->sendEventToMainDisplay(mouseEvent);
#endif
            event->accept();
            return true;
        }
    }
    else if (((event->type() == QEvent::KeyPress) || (event->type() == QEvent::KeyRelease))) {
        //syslog(LOG_DEBUG, "[OverlayButton] Send key event (type = %d) to main display", event->type());
        mMainWindow->getGameKeyManager()->sendEventToMainDisplay(event);
        event->accept();
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

BaseKey::BaseKey(KmreWindow* window) 
    : QWidget(window)
    , mMainWindow(window)
    , m_mousePressed(false)
    , m_enableEdit(true)
    , m_lastPos(QPoint(0, 0))
    , m_outsideScreen(false)
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

}

BaseKey::~BaseKey()
{
    if (opacityEffect) {
        delete opacityEffect;
    }
}

void BaseKey::showKey(bool show)
{
    if ((!m_outsideScreen) && show) {
        this->show();
    }
    else {
        this->hide();
    }
}

void BaseKey::setOpacity(double opacity)
{
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        opacityEffect->setOpacity(opacity);
    } else {
        setWindowOpacity(opacity);
    }
}

void BaseKey::checkPosOutsideScreen(QPoint pos)
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


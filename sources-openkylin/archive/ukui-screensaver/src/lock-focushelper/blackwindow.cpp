/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "blackwindow.h"
#include <QColor>
#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QX11Info>
#include <QPaintEvent>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <xcb/xcb.h>

BlackWindow::BlackWindow(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
                   | Qt::X11BypassWindowManagerHint);

    setAttribute(Qt::WA_TranslucentBackground);

    setCursor(Qt::BlankCursor);
    //qApp->installNativeEventFilter(this);
    //installEventFilter(this);
}

void BlackWindow::paintEvent(QPaintEvent *event)
{
    for(auto screen : QGuiApplication::screens())
    {
        QPainter painter(this);
        QColor cor = "#000000";
        painter.setBrush(cor);
        painter.drawRect(screen->geometry());

    }
    return QWidget::paintEvent(event);
}

void BlackWindow::laterActivate()
{
    raise();
}

bool BlackWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::WindowDeactivate){
         QTimer::singleShot(50,this,SLOT(laterActivate()));
    }

    return false;
}

bool BlackWindow::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    return false;
}

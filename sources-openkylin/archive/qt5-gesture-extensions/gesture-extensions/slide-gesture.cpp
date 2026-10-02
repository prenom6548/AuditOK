/*
Copyright (c) KylinSoft  Co., Ltd. 2021.All rights reserved.s
License: GPL-2+
 This package is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 .
 This package is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 .
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>
 .
 On Debian systems, the complete text of the GNU General
 Public License version 2 can be found in "/usr/share/common-licenses/GPL-2".
*/

#include "slide-gesture.h"

#include <QApplication>
#include <QDebug>
#include <QWidget>
#include <QScroller>
#include <QSettings>
#include <QMouseEvent>
#include <QAbstractItemView>
#include <QTimer>
#include <QScrollBar>

#define LONG_PRESS_TIME 300
#define PROPERTY_NOT_USE_GESTURE "notUseSlideGesture"

//#define DEBUG_MODE

namespace UkuiGestureExtension {

SlideGesture::SlideGesture(QObject *parent) : QObject(parent)
{
    // we handle the fake mouse event, not real touch event
    qApp->setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);

    m_touch_slide_timer = new QTimer(this);
    connect(m_touch_slide_timer, &QTimer::timeout, this, [this](){
       m_touch_slide_timer->stop();
    });

    //! init os release
    QSettings osinfo("/etc/os-release", QSettings::IniFormat);
    if (osinfo.contains(QString("PROJECT_CODENAME"))) {
        m_project_codename = osinfo.value(QString("PROJECT_CODENAME")).toString();
    }
}

void SlideGesture::registerWidget(QWidget *widget)
{
    if (!isWidgetUseExtension(widget)) {
        qDebug() << __FILE__ << __FUNCTION__ << widget << "do not use this slide extension";
        return;
    }
    if (!widget->inherits("QAbstractScrollArea"))
        return;

    auto* viewport = static_cast<QAbstractScrollArea*>(widget)->viewport();
    QScroller::grabGesture(viewport, QScroller::TouchGesture);

#ifndef __sw_64__
    QScroller* scroller = QScroller::scroller(viewport);
    QScrollerProperties properties = scroller->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::MaximumClickThroughVelocity, 0);
    scroller->setScrollerProperties(properties);
#else
    qDebug() << "Current is SW architecture, don't set scroller property to prevent the crash";
#endif

    if (widget->inherits("QAbstractItemView")) {
        QAbstractItemView* listView = static_cast<QAbstractItemView *>(widget);
        listView->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
        listView->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
    }

#ifdef DEBUG_MODE
    qDebug() << __FILE__ << __FUNCTION__ << " register successful";
#endif
}

void SlideGesture::unregisterWidget(QWidget *widget)
{
    if (!widget) {
        qDebug() << __FILE__ << __FUNCTION__ << " widget is empty";
        return;
    }

    QScroller::ungrabGesture(widget);
#ifdef DEBUG_MODE
    qDebug() << __FILE__ << __FUNCTION__ << " unregister successful";
#endif
}

bool SlideGesture::isWidgetUseExtension(QWidget* widget)
{
    if (widget->property(PROPERTY_NOT_USE_GESTURE).toBool())
        return false;
    return true;
}

} // namespace

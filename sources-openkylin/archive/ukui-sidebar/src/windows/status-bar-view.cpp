/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 * Author: hxf <hewenfei@kylinos.cn>
 *
 */

#include <QWidget>
#include "status-bar-view.h"
#include "global-settings.h"
#include "sidebar-window-helper.h"
#include "hand-gesture-helper.h"

StatusBarView::StatusBarView(QWindow *parent) : SharedEngineView(parent)
{
    setColor("transparent");
    setResizeMode(SharedEngineView::SizeRootObjectToView);

    rootContext()->setContextProperty("statusBarView", this);
    rootContext()->setContextProperty("handGestureHelper", Sidebar::HandGestureHelper::getInstance());

    setFlags(Qt::FramelessWindowHint);
    m_windowProxy = new UkuiQuick::WindowProxy(this);
    m_windowProxy->setWindowType(UkuiQuick::WindowType::Switcher);

    updateGeometry();
    connect(Sidebar::SidebarWindowHelper::instance(), &Sidebar::SidebarWindowHelper::geometryChanged,
            this, &StatusBarView::updateGeometry);

    setVisible(isTabletModel());
    connect(Sidebar::GlobalSettings::globalInstance(), &Sidebar::GlobalSettings::valueChanged, this, [this] (const QString &key) {
        if (key == TABLET_MODE) {
            setVisible(isTabletModel());
            Q_EMIT isTabletModelChanged();
        }
    });
}

bool StatusBarView::isTabletModel() const
{
    return Sidebar::GlobalSettings::globalInstance()->getValue(TABLET_MODE).toBool();
}

void StatusBarView::updateGeometry()
{
    QRect geometry = Sidebar::SidebarWindowHelper::instance()->getWindowGeometry(Sidebar::SidebarWindowType::StatusBar);
    if (geometry.isEmpty()) return;
    setGeometry(geometry);
    m_windowProxy->setPosition(geometry.topLeft());
}

bool StatusBarView::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::TouchBegin: {
            auto *touchEvent = static_cast<QTouchEvent*>(event);

            if (targetWindow(touchEvent) == this) {
                m_isPressed = true;
                return true;
            }

            break;
        }
        case QEvent::TouchUpdate: {
            auto *touchEvent = static_cast<QTouchEvent*>(event);
            if (targetWindow(touchEvent) == this && m_isPressed) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                int touchY = touchEvent->touchPoints().first().pos().y();
#else
                int touchY = touchEvent->points().first().position().y();
#endif
                Sidebar::HandGestureHelper::getInstance()->callNotificationCenter(touchY);
                return true;
            }

            break;
        }
        case QEvent::TouchEnd: {
            auto *touchEvent = static_cast<QTouchEvent*>(event);
            if (targetWindow(touchEvent) == this) {
                m_isPressed = false;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                int touchX = touchEvent->touchPoints().first().pos().x();
                int touchY = touchEvent->touchPoints().first().pos().y();
#else
                int touchX = touchEvent->points().first().position().x();
                int touchY = touchEvent->points().first().position().y();
#endif
                Sidebar::HandGestureHelper::getInstance()->top2BottomRelease(touchX, touchY);
                return true;
            }

            break;
        }
        case QEvent::MouseButtonPress: {
            auto *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_isPressed = true;
                return true;
            }

            break;
        }
        case QEvent::MouseButtonRelease: {
            auto *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_isPressed = false;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                Sidebar::HandGestureHelper::getInstance()->top2BottomRelease(mouseEvent->x(), mouseEvent->y());
#else
                Sidebar::HandGestureHelper::getInstance()->top2BottomRelease(
                    mouseEvent->position().x(), mouseEvent->position().y());
#endif
                return true;
            }

            break;
        }
        case QEvent::MouseMove: {
            if (m_isPressed) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                Sidebar::HandGestureHelper::getInstance()->callNotificationCenter(static_cast<QMouseEvent*>(event)->y());
#else
                Sidebar::HandGestureHelper::getInstance()->callNotificationCenter(dynamic_cast<QMouseEvent*>(event)->position().y());


#endif
                return true;
            }

            break;
        }
        case QEvent::Expose: {
            break;
        }
        case QEvent::Move: {
            updateGeometry();
            return true;
        }
        default:
            break;
    }

    return QQuickWindow::event(event);
}

QWindow* StatusBarView::targetWindow(QTouchEvent* touchEvent)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return touchEvent->window();
#else
    if (const QWidget *targetWidget = qobject_cast<QWidget*>(touchEvent->target())) {
        return targetWidget->windowHandle();
    }
    return nullptr;
#endif
}

void StatusBarView::init()
{
    setSource(QUrl("qrc:/qml/StatusBar.qml"));
}

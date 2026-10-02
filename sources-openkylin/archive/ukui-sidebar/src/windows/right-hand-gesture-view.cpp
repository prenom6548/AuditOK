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

#include "right-hand-gesture-view.h"

#include <QWidget>

#include "sidebar-window-helper.h"
#include "hand-gesture-helper.h"
#include "global-settings.h"
#include "settings.h"

namespace Sidebar {

RightHandGestureView::RightHandGestureView(QWindow *parent) : SharedEngineView(parent)
{
    setColor("transparent");
    setResizeMode(SharedEngineView::SizeRootObjectToView);

    setFlags(Qt::WindowDoesNotAcceptFocus | Qt::FramelessWindowHint);
    m_windowProxy = new UkuiQuick::WindowProxy(this);

    m_windowProxy->setWindowType(UkuiQuick::Settings::instance()->platformName() == "wayland" ?
                                     UkuiQuick::WindowType::Switcher : UkuiQuick::WindowType::SystemWindow);

    updateGeometry();
    connect(SidebarWindowHelper::instance(), &SidebarWindowHelper::geometryChanged, this, &RightHandGestureView::updateGeometry);

    connect(GlobalSettings::globalInstance(), &GlobalSettings::valueChanged, this, [this] (const QString &key) {
        if (key == TABLET_MODE && m_isPressed) {
            m_isPressed = false;
        }
    });
}

void RightHandGestureView::init()
{
    setSource(QUrl("qrc:/qml/Right2LeftSwipe.qml"));
}

void RightHandGestureView::updateGeometry()
{
    QRect geometry = windowGeometry();
    if (geometry.isEmpty()) return;
    setGeometry(geometry);
    m_windowProxy->setPosition(geometry.topLeft());

    bool mirrored = SidebarWindowHelper::instance()->getLayoutDirection() == Qt::LayoutDirection::RightToLeft;
    int panelPos = SidebarWindowHelper::instance()->getPanelPos();
    bool shouldHide = mirrored ? (panelPos == 2) : (panelPos == 3);

    if (shouldHide) {
        hide();
    } else if (!isVisible()) {
        show();
    }
}

bool RightHandGestureView::event(QEvent *event)
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
                int touchX = touchEvent->touchPoints().first().pos().x();
#else
                const int touchX = touchEvent->points().first().position().x();
#endif
                callControlCenter(touchX);
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
                const int touchX = touchEvent->points().first().position().x();
                const int touchY = touchEvent->points().first().position().y();
#endif
                callControlCenterEnd(touchX, touchY);
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
                callControlCenterEnd(mouseEvent->x(), mouseEvent->y());
#else
                callControlCenterEnd(mouseEvent->position().x(), mouseEvent->position().y());
#endif
                return true;
            }

            break;
        }
        case QEvent::MouseMove: {
            if (m_isPressed) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                callControlCenter(dynamic_cast<QMouseEvent*>(event)->x());
#else
                callControlCenter(dynamic_cast<QMouseEvent*>(event)->position().x());
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

void RightHandGestureView::callControlCenter(int x)
{
    int right;
    if (SidebarWindowHelper::instance()->getLayoutDirection() == Qt::LayoutDirection::RightToLeft) {
        if (x < 12) {
            return;
        }

        right = x - 12;
    } else {
        if (x > -12) {
            return;
        }

        right = windowGeometry().x() + x + 12;
    }
    HandGestureHelper::getInstance()->callControlCenter(right);
}

void RightHandGestureView::callControlCenterEnd(int x, int y)
{
    int posX = (SidebarWindowHelper::instance()->getLayoutDirection() == Qt::LayoutDirection::RightToLeft)
            ? x - 12
            : windowGeometry().x() + x + 12;
    int posY = windowGeometry().y() + y + 12;
    HandGestureHelper::getInstance()->right2LeftRelease(posX, posY);
}

QWindow* RightHandGestureView::targetWindow(QTouchEvent* touchEvent)
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

QRect RightHandGestureView::windowGeometry()
{
    return SidebarWindowHelper::instance()->getWindowGeometry(SidebarWindowType::RightHandGesture);
}

} // Sidebar

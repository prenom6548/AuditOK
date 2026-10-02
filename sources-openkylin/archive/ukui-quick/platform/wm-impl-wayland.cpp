/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "wm-impl-wayland.h"

#include <KWayland/Client/surface.h>
#include <QPlatformSurfaceEvent>
#include <QDebug>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <kx11extras.h>
#else
#include <KWindowSystem>
#endif
#include <QGuiApplication>
#include <window-helper-common.h>

#include "window-helper.h"

namespace UkuiQuick {

WMImplWayland::WMImplWayland(QWindow *window) : WMInterface(window)
{
    m_pos = window->position();
    m_useUkuiShellIntegration = WindowProxy::useUkuiShellIntegration(window);
    if(!m_useUkuiShellIntegration) {
        window->installEventFilter(this);
        initSurface();
    }
}

QPoint WMImplWayland::position() const
{
    return m_pos;
}

void WMImplWayland::setPosition(const QPoint &point)
{
    m_pos = point;
    window()->setPosition(m_pos);
    if (m_shellSurface && !m_useUkuiShellIntegration) {
        m_shellSurface->setPosition(m_pos);
    }
}

void WMImplWayland::setWindowType(WindowType::Type type)
{
    if (m_useUkuiShellIntegration) {
        window()->setProperty("ukui_surface_role_v1", ukui_surface_roleMap.value(type));
        return;
    }
    if (!m_shellSurface) {
        return;
    }

    using namespace WaylandClient;
    switch (type) {
        default:
        case WindowType::Normal:
            m_shellSurface->setRole(UkuiShellSurface::Role::Normal);
            break;
        case WindowType::Desktop:
            m_shellSurface->setRole(UkuiShellSurface::Role::Desktop);
            break;
        case WindowType::Dock:
        case WindowType::Panel:
            m_shellSurface->setRole(UkuiShellSurface::Role::Panel);
            break;
        case WindowType::SystemWindow:
            m_shellSurface->setRole(UkuiShellSurface::Role::SystemWindow);
            break;
        case WindowType::Notification:
            m_shellSurface->setRole(UkuiShellSurface::Role::Notification);
            break;
        case WindowType::CriticalNotification:
            m_shellSurface->setRole(UkuiShellSurface::Role::CriticalNotification);
            break;
        case WindowType::ScreenLockNotification:
            m_shellSurface->setRole(UkuiShellSurface::Role::ScreenLockNotification);
            break;
        case WindowType::OnScreenDisplay:
            m_shellSurface->setRole(UkuiShellSurface::Role::OnScreenDisplay);
            break;
        case WindowType::Dialog:
            window()->setFlags(window()->flags() | Qt::Dialog);
        case WindowType::Menu:
        case WindowType::ToolTip:
            m_shellSurface->setRole(UkuiShellSurface::Role::ToolTip);
            break;
        case WindowType::PopupMenu:
            window()->setFlags(window()->flags() | Qt::Popup);
            m_shellSurface->setRole(UkuiShellSurface::Role::ToolTip);
            break;
        case WindowType::AppletPopup:
            m_shellSurface->setRole(UkuiShellSurface::Role::AppletPop);
            break;
    }

    // thank kde.
    if (type == WindowType::OnScreenDisplay) {
        window()->setFlags((window()->flags() & ~Qt::Dialog) | Qt::Window);
    }

    bool onAllDesktop = (type == WindowType::Desktop
                         || type == WindowType::Dock || type == WindowType::Panel
                         || type == WindowType::SystemWindow
                         || type == WindowType::Notification
                         || type == WindowType::CriticalNotification
                         || type == WindowType::OnScreenDisplay);

    // TODO: wlcom ?
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    KX11Extras::setOnAllDesktops(window()->winId(), onAllDesktop);
#else
    KWindowSystem::setOnAllDesktops(window()->winId(), onAllDesktop);
#endif
}

void WMImplWayland::setSkipTaskBar(bool skip)
{
    if(m_useUkuiShellIntegration) {
        window()->setProperty("ukui_surface_skip_taskbar",skip);
    } else if (m_shellSurface) {
        m_shellSurface->setSkipTaskbar(skip);
    }
}

void WMImplWayland::setSkipSwitcher(bool skip)
{
    if(m_useUkuiShellIntegration) {
        window()->setProperty("ukui_surface_skip_switcher",skip);
    } else if (m_shellSurface) {
        m_shellSurface->setSkipSwitcher(skip);
    }
}

void WMImplWayland::setRemoveTitleBar(bool remove)
{
    if (window()->flags().testFlag(Qt::FramelessWindowHint)) {
        return;
    }
    if(m_useUkuiShellIntegration) {
        window()->setProperty("ukui_surface_no_titlebar", remove);
    } else if (m_shellSurface) {
        m_shellSurface->setRemoveTitleBar(remove);
    }
}

void WMImplWayland::setPanelAutoHide(bool autoHide)
{
    if(m_useUkuiShellIntegration) {
        window()->setProperty("ukui_surface_panel_auto_hide", autoHide);
    } else if (m_shellSurface) {
        m_shellSurface->setPanelAutoHide(autoHide);
    }
}

void WMImplWayland::setPanelTakesFocus(bool takesFocus)
{
    if(m_useUkuiShellIntegration) {
        window()->setProperty("ukui_surface_panel_takes_focus", takesFocus);
    } else if (m_shellSurface) {
        m_shellSurface->setPanelTakesFocus(takesFocus);
    }
}

QScreen* WMImplWayland::currentScreen()
{
    auto shell = WaylandIntegration::self()->waylandUkuiShell();
    if (shell) {
        shell->updateCurrentOutput();
        WaylandIntegration::self()->sync();
        if (shell->isCurrentOutputReady()) {
            QString name = shell->outputName();
            for (auto screen: qGuiApp->screens()) {
                if (screen->name() == name) {
                    return screen;
                }
            }
        }
    }
    return qGuiApp->primaryScreen();
}

void WMImplWayland::setDecorationComponents(Decoration::Components components)
{
    if (m_useUkuiShellIntegration) {
        window()->setProperty("ukui_surface_decoration_components", static_cast<uint>(components));
    } else if (m_shellSurface) {
        m_shellSurface->setDecorationComponents(components);
    }
}

bool WMImplWayland::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == window()) {
        switch (event->type()) {
            case QEvent::PlatformSurface: {
                const auto surfaceEvent = static_cast<QPlatformSurfaceEvent *>(event);
                if (surfaceEvent->surfaceEventType() != QPlatformSurfaceEvent::SurfaceCreated) {
                    destroySurface();
                }
                break;
            }
            case QEvent::Expose:
                if (!m_shellSurface) {
                    initSurface();

                    // 初始化位置
                    if (m_shellSurface) {
                        m_shellSurface->setPosition(m_pos);
                    }
                }
                break;
            case QEvent::Hide:
                destroySurface();
                break;
            default:
                break;
        }
    }

    return QObject::eventFilter(watched, event);
}

void WMImplWayland::initSurface()
{
    auto shell = WaylandIntegration::self()->waylandUkuiShell();
    if (!shell) {
        return;
    }

    KWayland::Client::Surface *surface = KWayland::Client::Surface::fromWindow(window());
    if (!surface) {
        return;
    }

    m_shellSurface.reset(shell->createSurface(surface));
}

inline void WMImplWayland::destroySurface()
{
    m_shellSurface.reset();
}

} // UkuiQuick

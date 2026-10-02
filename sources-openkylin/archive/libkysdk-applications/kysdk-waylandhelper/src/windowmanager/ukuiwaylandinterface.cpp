/*
 * libkysdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "ukuiwaylandinterface.h"

#include "ukui-shell-client-protocol.h"
#include "ukui-window-management-client-protocol.h"
#include "wayland-ext-idle-notify-v1-client-protocol.h"
#include "wayland-xdg-activation-v1-client-protocol.h"
#include <QApplication>
#include <QDebug>

static const u_int32_t ukui_window_management_surpported_version = 1;
static const u_int32_t ukui_shell_surpported_version = 2;
static const u_int32_t wayland_xdg_activation_surpported_version = 1;
static const u_int32_t wayland_ext_idle_notify_surpported_version = 1;

void handle_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version)
{
    auto waylandInterface = reinterpret_cast<UkuiWaylandInterface *>(data);
    Q_ASSERT(wl_registry == *waylandInterface->registry());
    waylandInterface->handleGlobal(data, wl_registry, name, interface, version);
}
void handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name)
{
}

const struct wl_registry_listener UkuiWaylandInterface::s_registryListener = {handle_global, handle_global_remove};

UkuiWaylandInterface::UkuiWaylandInterface(QObject *parent)
    : AbstractInterface(parent)
{
    m_connection = KWayland::Client::ConnectionThread::fromApplication(qApp);
    m_registry = new Registry(this);
    m_registry->create(m_connection->display());

    connect(m_registry, &KWayland::Client::Registry::shellAnnounced, this, [=]() {
        const auto interface = m_registry->interface(KWayland::Client::Registry::Interface::Shell);
        if (interface.name != 0) {
            m_shell = m_registry->createShell(interface.name, interface.version, this);
        }
    });

    wl_registry *registry = *m_registry;
    wl_registry_add_listener(registry, &s_registryListener, this);
    wl_display_dispatch(m_connection->display());
    wl_display_roundtrip(m_connection->display());

    m_registry->setup();
    //    m_connection->roundtrip();
}

UkuiWaylandInterface::~UkuiWaylandInterface()
{
}

WindowInfo UkuiWaylandInterface::requestInfo(WindowId wid)
{
    WindowInfo windowInfo;

    auto w = windowFor(wid);

    if (w) {
        windowInfo.setIsValid(true);
        windowInfo.setWid(wid);
        windowInfo.setIsActive(w->isActive());
        windowInfo.setIsMinimized(w->isMinimized());
        windowInfo.setIsMaxVert(w->isMaximized());
        windowInfo.setIsMaxHoriz(w->isMaximized());
        windowInfo.setIsFullscreen(w->isFullscreen());
        windowInfo.setIsShaded(w->isShaded());
        windowInfo.setIsOnAllDesktops(w->isOnAllDesktops());
        windowInfo.setIsOnAllActivities(true);
        windowInfo.setHasSkipTaskbar(w->skipTaskbar());
        windowInfo.setHasSkipSwitcher(w->skipSwitcher());
        windowInfo.setIsKeepAbove(w->isKeepAbove());
        //! Window Abilities
        windowInfo.setIsClosable(w->isCloseable());
        windowInfo.setIsFullScreenable(w->isFullscreenable());
        windowInfo.setIsMaximizable(w->isMaximizeable());
        windowInfo.setIsMinimizable(w->isMinimizeable());
        windowInfo.setIsMovable(w->isMovable());
        windowInfo.setIsResizable(w->isResizable());
        windowInfo.setIsShadeable(w->isShadeable());
        windowInfo.setIsVirtualDesktopsChangeable(w->isVirtualDesktopChangeable());
    } else {
        windowInfo.setIsValid(false);
    }

    return windowInfo;
}

void UkuiWaylandInterface::requestActivate(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestActivate();
        m_connection->roundtrip();
        emit windowChanged(w->uuid());
    }
}

void UkuiWaylandInterface::requestClose(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestClose();
        m_connection->roundtrip();
    }
}

void UkuiWaylandInterface::requestToggleKeepAbove(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestToggleKeepAbove();
        m_connection->roundtrip();
        emit windowChanged(w->uuid());
    }
}

void UkuiWaylandInterface::requestToggleMinimized(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestToggleMinimized();
        m_connection->roundtrip();
    }
}

void UkuiWaylandInterface::requestToggleMaximized(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestToggleMaximized();
        m_connection->roundtrip();
    }
}

QIcon UkuiWaylandInterface::iconFor(WindowId wid)
{
    auto window = windowFor(wid);

    if (window) {
        return window->icon();
    }

    return QIcon();
}

QString UkuiWaylandInterface::titleFor(WindowId wid)
{
    auto window = windowFor(wid);

    if (window) {
        return window->title();
    }
    return QString();
}

QString UkuiWaylandInterface::windowGroupFor(WindowId wid)
{
    auto window = windowFor(wid);
    if (window) {
        m_connection->roundtrip();
        return window->appId();
    } else
        return QString();
}

quint32 UkuiWaylandInterface::pid(WindowId wid)
{
    quint32 pid = 0;
    auto window = windowFor(wid);
    if (window) {
        m_connection->roundtrip();
        return window->pid();
    } else
        return pid;
}

void UkuiWaylandInterface::showCurrentDesktop()
{
    if (m_ukuiWindowManager) {
        m_ukuiWindowManager->showDesktop();
        m_connection->roundtrip();
    }
}

void UkuiWaylandInterface::hideCurrentDesktop()
{
    if (m_ukuiWindowManager) {
        m_ukuiWindowManager->hideDesktop();
        m_connection->roundtrip();
    }
}

bool UkuiWaylandInterface::windowCanBeDragged(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        WindowInfo winfo = requestInfo(wid);
        return (winfo.isValid()
                && w->isMovable()
                && !winfo.isMinimized()
                && inCurrentDesktopActivity(winfo)
                && !winfo.isPlasmaDesktop());
    }

    return false;
}

bool UkuiWaylandInterface::windowCanBeMaximized(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        WindowInfo winfo = requestInfo(wid);
        return (winfo.isValid()
                && w->isMaximizeable()
                && !winfo.isMinimized()
                && inCurrentDesktopActivity(winfo)
                && !winfo.isPlasmaDesktop());
    }

    return false;
}

WindowId UkuiWaylandInterface::activeWindow()
{
    if (!m_ukuiWindowManager) {
        return 0;
    }
    m_connection->roundtrip();
    auto wid = m_ukuiWindowManager->activeWindow();
    return wid ? QVariant(wid->uuid()) : "";
}
bool UkuiWaylandInterface::removeHeaderBar(QWindow *window)
{
    if (!window)
        return false;
    if (!m_ukuiShell)
        return false;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return false;

    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return false;
    ukuiShellSurface->setSurfaceProperty(UkuiShellSurface::SurfaceProperty::NoTitleBar, 1);

    return true;
}

bool UkuiWaylandInterface::setWindowRadius(QWindow *window, int radius)
{
    if (!window || radius < 0)
        return false;

    if (!m_ukuiShell)
        return false;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return false;

    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return false;
    ukuiShellSurface->setSurfaceProperty(UkuiShellSurface::SurfaceProperty::WindowRadius, radius);
    return true;
}

void UkuiWaylandInterface::activateWindow(QWindow *window1, QWindow *window2)
{
    if (!window1 || !window2)
        return;

    if (!m_seat)
        return;
    if (!m_xdgActivation)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window1);
    if (!surface)
        return;
    if (!m_surfaces.contains(window1)) {
        m_surfaces.insert(window1, surface);
    }
    XdgActivationToken *token = m_xdgActivation->requestXdgActivationToken(m_seat, surface, 0, "");
    connect(token, &XdgActivationToken::done, this, [=](const QString &str) {
        auto surface2 = KWayland::Client::Surface::fromWindow(window2);
        if (!surface2)
            return;
        if (!m_surfaces.contains(window2)) {
            m_surfaces.insert(window2, surface2);
        }
        m_xdgActivation->activateWindow(str, surface2);
    });
}

void UkuiWaylandInterface::setIdleInterval(int msec)
{
    if (!m_notifier || !m_seat)
        return;
    ExtIdleNotification *notification = m_notifier->requestExtIdleNotification(m_seat, msec);
    if (!notification)
        return;
    connect(notification, &ExtIdleNotification::idled, this, [=]() {
        Q_EMIT idled();
    });
    connect(notification, &ExtIdleNotification::resumed, this, [=]() {
        Q_EMIT resumed();
    });
}

QRect UkuiWaylandInterface::windowGeometry(const WindowId &windowId)
{
    UkuiWindow *w = windowFor(windowId);
    if (w)
        return w->geometry();
    else
        return QRect(0, 0, 0, 0);
}

void UkuiWaylandInterface::setPanelAutoHide(QWindow *window, bool autoHide)
{
    if (!window)
        return;
#ifdef USE_UKUI_SHELL_PLUGIN
    window->setProperty("ukui_surface_panel_auto_hide", autoHide);
#elif
    if (!m_ukuiShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;


    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return;

    ukuiShellSurface->setPanelAutoHide(autoHide);

#endif
}

void UkuiWaylandInterface::setGrabKeyboard(QWindow *window, bool autoHide)
{
    Q_UNUSED(autoHide)
    if (!window)
        return;
    if (!m_ukuiShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;

    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return;
    ukuiShellSurface->setGrabKeyboard(m_seat);
}

void UkuiWaylandInterface::setWindowLayer(QWindow *window, WindowLayer layer)
{
    if (!window)
        return;
    if (!m_ukuiShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;

    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return;
    switch (layer) {
    case WindowLayer::Desktop:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::Desktop);
        break;
    case WindowLayer::Panel:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::Panel);
        break;
    case WindowLayer::OnScreenDisplay:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::OnScreenDisplay);
        break;
    case WindowLayer::Notification:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::Notification);
        break;
    case WindowLayer::ToolTip:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::ToolTip);
        break;
    case WindowLayer::CriticalNotification:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::CriticalNotification);
        break;
    case WindowLayer::AppletPop:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::AppletPop);
        break;
    case WindowLayer::ScreenLock:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::ScreenLock);
        break;
    case WindowLayer::Watermark:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::Watermark);
        break;
    case WindowLayer::SystemWindow:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::SystemWindow);
        break;
    case WindowLayer::InputPanel:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::InputPanel);
        break;
    case WindowLayer::Logout:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::Logout);
        break;
    case WindowLayer::ScreenLockNotification:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::ScreenLockNotification);
        break;
    case WindowLayer::Switcher:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::Switcher);
        break;
    case WindowLayer::Authentication:
        ukuiShellSurface->setRole(UkuiShellSurface::Role::Authentication);
        break;
    default: {
        qWarning() << "It is not a surpported window layer by ukui protocols! id:" << (int)layer;
        ukuiShellSurface->setRole(UkuiShellSurface::Role::Normal);
        break;
    }
    }
}

WindowLayer UkuiWaylandInterface::windowLayer(QWindow *window)
{
    if (!window)
        return WindowLayer::Normal;
    if (!m_ukuiShell)
        return WindowLayer::Normal;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return WindowLayer::Normal;

    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return WindowLayer::Normal;

    WindowLayer layer;
    switch (ukuiShellSurface->role()) {
    case UkuiShellSurface::Role::Normal:
        layer = WindowLayer::Normal;
        break;
    case UkuiShellSurface::Role::Desktop:
        layer = WindowLayer::Desktop;
        break;
    case UkuiShellSurface::Role::Panel:
        layer = WindowLayer::Panel;
        break;
    case UkuiShellSurface::Role::OnScreenDisplay:
        layer = WindowLayer::OnScreenDisplay;
        break;
    case UkuiShellSurface::Role::Notification:
        layer = WindowLayer::Notification;
        break;
    case UkuiShellSurface::Role::ToolTip:
        layer = WindowLayer::ToolTip;
        break;
    case UkuiShellSurface::Role::CriticalNotification:
        layer = WindowLayer::CriticalNotification;
        break;
    case UkuiShellSurface::Role::AppletPop:
        layer = WindowLayer::AppletPop;
        break;
    case UkuiShellSurface::Role::ScreenLock:
        layer = WindowLayer::ScreenLock;
        break;
    case UkuiShellSurface::Role::Watermark:
        layer = WindowLayer::Watermark;
        break;
    case UkuiShellSurface::Role::SystemWindow:
        layer = WindowLayer::SystemWindow;
        break;
    case UkuiShellSurface::Role::InputPanel:
        layer = WindowLayer::InputPanel;
        break;
    case UkuiShellSurface::Role::Logout:
        layer = WindowLayer::Logout;
        break;
    case UkuiShellSurface::Role::ScreenLockNotification:
        layer = WindowLayer::ScreenLockNotification;
        break;
    case UkuiShellSurface::Role::Switcher:
        layer = WindowLayer::Switcher;
        break;
    default:
        layer = WindowLayer::Normal;
        break;
    }
    return layer;
}

void UkuiWaylandInterface::setHighlight(const WindowId &wid, bool highlight)
{
    auto w = windowFor(wid);

    if (w) {
        highlight ? w->setHighlight() : w->unsetHightlight();
        m_connection->roundtrip();
    }
}

bool UkuiWaylandInterface::istHighlight(const WindowId &wid)
{
    auto w = windowFor(wid);

    if (w) {
        return w->isHighlight();
    } else
        return false;
}

void UkuiWaylandInterface::setOpenUnderCursor(QWindow *window)
{
    if (!window)
        return;
    if (!m_ukuiShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;

    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return;

    ukuiShellSurface->setOpenUnderCursor();
}

void UkuiWaylandInterface::setOpenUnderCursor(QWindow *window, int x, int y)
{
    if (!window)
        return;
    if (!m_ukuiShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;

    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return;

    ukuiShellSurface->setOpenUnderCursor(x, y);
}

void UkuiWaylandInterface::setIconName(QWindow *window, const QString &iconName)
{
    if (!window || !m_ukuiShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;

    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return;

    ukuiShellSurface->setIconName(iconName);
}

QString UkuiWaylandInterface::currentSeatName()
{
    if (!m_ukuiShell)
        return QString();
    m_ukuiShell->updateCurrentOutput();
    return m_ukuiShell->seatName();
}

QString UkuiWaylandInterface::currentOutputName()
{
    if (!m_ukuiShell)
        return QString();
    m_ukuiShell->updateCurrentOutput();
    return m_ukuiShell->outputName();
}

QList<OutputInfo *> UkuiWaylandInterface::outputsInfo()
{
    //Obsolete
    QList<OutputInfo *> list;
    return list;
}

QList<WindowId> UkuiWaylandInterface::getWindowIdByPid(quint32 pid)
{
    QList<WindowId> list;
    qDebug() << m_ukuiWindowManager->windows().count();
    for (auto w : m_ukuiWindowManager->windows()) {
        if (w->pid() == pid)
            list.append(w->uuid());
    }
    return list;
}

QList<WindowId> UkuiWaylandInterface::getWindowIdByTtile(const QString &title)
{
    QList<WindowId> list;
    for (auto w : m_ukuiWindowManager->windows()) {
        if (w->title() == title)
            list.append(w->uuid());
    }
    return list;
}

void UkuiWaylandInterface::setGeometry(QWindow *window, const QRect &rect)
{
    if (!window)
        return;
#ifdef USE_UKUI_SHELL_PLUGIN
    window->setGeometry(rect);
#elif
    if (!m_ukuiShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;

    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return;
    ukuiShellSurface->setPosition(rect.topLeft());
    window->resize(rect.size());
#endif
}

void UkuiWaylandInterface::setSkipTaskBar(QWindow *window, bool skip)
{
    if (!window)
        return;
#ifdef USE_UKUI_SHELL_PLUGIN
    window->setProperty("ukui_surface_skip_taskbar", skip);
#elif
    if (!m_ukuiShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return;
    ukuiShellSurface->setSkipTaskbar(skip);
#endif
}

void UkuiWaylandInterface::setSkipSwitcher(QWindow *window, bool skip)
{
    if (!window)
        return;
#ifdef USE_UKUI_SHELL_PLUGIN
    window->setProperty("ukui_surface_skip_switcher", skip);
#elif
    if (!m_ukuiShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return;

    ukuiShellSurface->setSkipSwitcher(skip);
#endif
}

bool UkuiWaylandInterface::skipTaskBar(const WindowId &wid)
{
    auto window = windowFor(wid);

    return window ? window->skipTaskbar() : false;
}

bool UkuiWaylandInterface::skipSwitcher(const WindowId &wid)
{
    auto window = windowFor(wid);

    return window ? window->skipSwitcher() : false;
}

bool UkuiWaylandInterface::isShowingDesktop()
{
    bool flag = false;
    if (m_ukuiWindowManager) {
        flag = m_ukuiWindowManager->isShowingDesktop();
    }
    return flag;
}

void UkuiWaylandInterface::setOnAllDesktops(const WindowId &wid)
{
    auto w = windowFor(wid);

    if (w && m_desktops.count() > 1) {
        if (w->isOnAllDesktops()) {
            w->requestEnterVirtualDesktop(m_currentDesktop);
        }
    }
}

NET::WindowType UkuiWaylandInterface::windowType(WindowId wid)
{
    return NET::WindowType::Normal;
}

void UkuiWaylandInterface::setPanelTakefocus(QWindow *window, bool flag)
{
    if (!window)
        return;
#ifdef USE_UKUI_SHELL_PLUGIN
    window->setProperty("ukui_surface_panel_takes_focus", flag);
#elif
    if (!m_ukuiShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;

    auto ukuiShellSurface = m_ukuiShell->getUkuiShellSurfaceFromWindow(window, window);
    if (!ukuiShellSurface)
        return;
    ukuiShellSurface->setPanelTakesFocus(flag);
#endif
}

void UkuiWaylandInterface::demandAttention(const WindowId &wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestDemandAttention();
        m_connection->roundtrip();
    }
}

bool UkuiWaylandInterface::eventFilter(QObject *obj, QEvent *ev)
{
    auto window = qobject_cast<QWindow *>(obj);
    if (window && ev->type() == QEvent::Hide) {
        if (m_ukuiShellSurfaces.contains(window)) {
            auto ukuiShellSurface = m_ukuiShellSurfaces.value(window);
            if (ukuiShellSurface) {
                ukuiShellSurface->release();
                ukuiShellSurface->destroy();
            }
            m_ukuiShellSurfaces.remove(window);
        }
        if (m_surfaces.contains(window)) {
            auto surface = m_surfaces.value(window);
            if (surface) {
                surface->release();
                surface->destroy();
            }
            m_surfaces.remove(window);
        }
    }
    return QObject::eventFilter(obj, ev);
}

bool UkuiWaylandInterface::ukuiProtocolReady()
{
    return m_ukuiShell && m_ukuiWindowManager;
}

Registry *UkuiWaylandInterface::registry()
{
    return m_registry;
}

UkuiWindow *UkuiWaylandInterface::windowFor(WindowId wid)
{
    auto it = std::find_if(m_ukuiWindowManager->windows().constBegin(), m_ukuiWindowManager->windows().constEnd(), [&wid](UkuiWindow *w) noexcept {
        return w->isValid() && w->uuid() == wid;
    });

    if (it == m_ukuiWindowManager->windows().constEnd()) {
        return nullptr;
    }

    return *it;
}

void UkuiWaylandInterface::handleGlobal(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    if (strcmp(interface, ext_idle_notifier_v1_interface.name) == 0) {
        u_int32_t supportedVersion = qMin(wayland_ext_idle_notify_surpported_version, version);
        ext_idle_notifier_v1 *notifier = (ext_idle_notifier_v1 *)wl_registry_bind(registry, name, &ext_idle_notifier_v1_interface, supportedVersion);
        if (notifier) {
            m_notifier = new ExtIdleNotifier(this);
            m_notifier->setup(notifier);
        }
    }
    if (strcmp(interface, wl_seat_interface.name) == 0) {
        m_seat = (wl_seat *)wl_registry_bind(registry, name, &wl_seat_interface, version);
    }
    if (strcmp(interface, xdg_activation_v1_interface.name) == 0) {
        u_int32_t supportedVersion = qMin(wayland_xdg_activation_surpported_version, version);
        xdg_activation_v1 *xdg_activation = (xdg_activation_v1 *)wl_registry_bind(registry, name, &xdg_activation_v1_interface, supportedVersion);
        if (xdg_activation) {
            m_xdgActivation = new XdgActivation(this);
            m_xdgActivation->setup(xdg_activation);
        }
    }
    if (strcmp(interface, ukui_shell_v1_interface.name) == 0) {
        u_int32_t supportedVersion = qMin(ukui_shell_surpported_version, version);
        ukui_shell_v1 *shell = (ukui_shell_v1 *)wl_registry_bind(registry, name, &ukui_shell_v1_interface, supportedVersion);
        if (shell) {
            m_ukuiShell = new UkuiShell(this);
            m_ukuiShell->setup(shell);
        }
    }
    if (strcmp(interface, ukui_window_management_interface.name) == 0) {
        u_int32_t supportedVersion = qMin(ukui_window_management_surpported_version, version);
        ukui_window_management *wm =
            (ukui_window_management *)wl_registry_bind(registry, name, &ukui_window_management_interface, supportedVersion);
        if (wm) {
            m_ukuiWindowManager = new UkuiWindowManagement(this);
            m_ukuiWindowManager->setup(wm);

            connect(m_ukuiWindowManager, &UkuiWindowManagement::showingDesktopChanged, this, &AbstractInterface::isShowingDesktopChanged);
            connect(m_ukuiWindowManager, &UkuiWindowManagement::windowCreated, this, &UkuiWaylandInterface::ukuiWindowCreatedProxy);
            connect(
                m_ukuiWindowManager, &UkuiWindowManagement::activeWindowChanged, this, [&]() noexcept {
                UkuiWindow *w = m_ukuiWindowManager->activeWindow();
                if (w) {
                    emit activeWindowChanged(w ? w->uuid() : "");
                }
            },
                Qt::QueuedConnection);
            connect(m_ukuiWindowManager, &UkuiWindowManagement::windowCreated, this,
                    [this](UkuiWindow *window) {
                if (!m_windows.contains(window->uuid())) {
                    m_windows.insert(window->uuid(), this->requestInfo(window->uuid()));
                }
                emit windowAdded(window->uuid());
            });
        }
    }
}

void UkuiWaylandInterface::handleGlobalRemove(void *data, wl_registry *registry, uint32_t name)
{
}

void UkuiWaylandInterface::ukuiWindowCreatedProxy(UkuiWindow *w)
{
    if (!w->isValid())
        return;

    if ((w->appId() == "ukui-panel"))
        return;

    trackUkuiWindow(w);
}

void UkuiWaylandInterface::trackUkuiWindow(UkuiWindow *w)
{
    if (!w || w->appId() == "ukui-panel")
        return;

    connect(w, &UkuiWindow::titleChanged, this, [=]() {
        emit titleChanged(w->uuid());
    });
    connect(w, &UkuiWindow::fullscreenChanged, this, [=]() {
        emit fullscreenChanged(w->uuid());
    });
    connect(w, &UkuiWindow::iconChanged, this, [=]() {
        emit iconChanged(w->uuid());
    });
    connect(w, &UkuiWindow::activeChanged, this, [=]() {
        emit activeChanged(w->uuid());
    });
    connect(w, &UkuiWindow::keepAboveChanged, this, [=]() {
        emit keepAboveChanged(w->uuid());
    });
    connect(w, &UkuiWindow::minimizedChanged, this, [=]() {
        emit minimizedChanged(w->uuid());
    });
    connect(w, &UkuiWindow::maximizedChanged, this, [=]() {
        emit maximizedChanged(w->uuid());
    });
    connect(w, &UkuiWindow::onAllDesktopsChanged, this, [=]() {
        emit onAllDesktopsChanged(w->uuid());
    });
    connect(w, &UkuiWindow::demandsAttentionChanged, this, [=]() {
        emit demandsAttentionChanged(w->uuid());
    });
    connect(w, &UkuiWindow::skipTaskbarChanged, this, [=]() {
        emit skipTaskbarChanged(w->uuid());
    });
    connect(w, &UkuiWindow::skipSwitcherChanged, this, [=]() {
        emit skipSwitcherChanged(w->uuid());
    });
    connect(w, &UkuiWindow::geometryChanged, this, [=]() {
        emit geometryChanged(w->uuid());
    });
    connect(w, &UkuiWindow::unmapped, this, [=]() {
        emit windowRemoved(w->uuid());
        untrackUkuiWindow(w);
    });
}

void UkuiWaylandInterface::untrackUkuiWindow(UkuiWindow *w)
{
    disconnect(w, &UkuiWindow::titleChanged, 0, 0);
    disconnect(w, &UkuiWindow::iconChanged, 0, 0);
    disconnect(w, &UkuiWindow::activeChanged, 0, 0);
    disconnect(w, &UkuiWindow::keepAboveChanged, 0, 0);
    disconnect(w, &UkuiWindow::keepBelowChanged, 0, 0);
    disconnect(w, &UkuiWindow::minimizedChanged, 0, 0);
    disconnect(w, &UkuiWindow::maximizedChanged, 0, 0);
    disconnect(w, &UkuiWindow::onAllDesktopsChanged, 0, 0);
    disconnect(w, &UkuiWindow::demandsAttentionChanged, 0, 0);
    disconnect(w, &UkuiWindow::skipTaskbarChanged, 0, 0);
    disconnect(w, &UkuiWindow::skipSwitcherChanged, 0, 0);
    disconnect(w, &UkuiWindow::geometryChanged, 0, 0);
}

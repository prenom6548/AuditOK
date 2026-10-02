/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "window-helper.h"

#include <QGuiApplication>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#include <kx11extras.h>
#else
#include <QtX11Extras/QX11Info>
#include <KWindowSystem>
#endif
#include <QTimer>
#include <QDebug>
#include <QPlatformSurfaceEvent>
#include <qpa/qplatformnativeinterface.h>

#include <private/qwaylandwindow_p.h>

#include "xatom-helper.h"
#include "ukui-shell-v1.h"
//kde
#include <KWayland/Client/surface.h>
#include <KWindowEffects>
#include <QPair>
#include <netwm_def.h>

#include "wayland-integration_p.h"
#include "window-helper-common.h"
#include "ukui-blur-manager.h"

namespace UkuiQuick {

// ====== WindowProxyPrivate ====== //
class WindowProxyPrivate
{
public:
    bool setupWaylandIntegration();

    const QVector<int> buildBlurArgs() const;
    void removeHeaderBar(bool remove = true);
    void skipTaskBarOrSwitcher();
    void setWindowTypeX11();
    void setWindowTypeWayland();

    void updateGeometry();
    void updatePoint();
    void setWindowType();
    void setAutoHideState();

    void exec();
    void setDecorationComponents();
    void setBlurBehind();
    void setWindowStates();
    void setRestoreGeometry();
    // 代理操作
    bool m_isWayland = false;
    bool m_panelAutoHide = false;
    bool m_removeTitleBar = false;

    // 窗口类型
    WindowType::Type m_type = WindowType::Normal;
    WindowProxy::Operations m_operations;

    // old winId
    WId        m_winId {0};
    // 被代理的窗口
    QWindow   *m_window{nullptr};
    QSize      m_size;
    QPoint     m_point;
    QRect      m_restoreGeometry;

    QPointer<Surface> m_surface;
    QPointer<WaylandClient::UkuiShellSurface> m_shellSurface;
    QPointer<WaylandClient::UkuiBlurManager> m_blurManager;
    QPointer<WaylandClient::UkuiBlur> m_blur;

    int  m_slideOffset = -1;
    WindowProxy::SlideFromEdge m_slideFromEdge = WindowProxy::NoEdge;

    bool m_useUkuiShellIntegration = false;
    UkuiQuick::Decoration::Components m_decorationComponents;
    QRegion m_blurRegion;
    uint32_t m_blurLevel {12};
    qreal m_blurOpacity {1.0};
    WindowProxy::BlurType m_blurType {WindowProxy::BlurRealtime};
    bool m_enableBlur = false;

    QtWaylandClient::QWaylandWindow *m_waylandWindow{nullptr};
    QPair<WindowState::States, bool> m_states;
};

void WindowProxyPrivate::setRestoreGeometry()
{
    if (!m_isWayland) {
        qWarning() << "setRestoreGeometry can be only set on wayland!";
        return;
    }

    if (m_useUkuiShellIntegration) {
        m_window->setProperty("ukui_surface_restore_geometry", m_restoreGeometry);
    } else if (m_shellSurface) {
        m_shellSurface->setRestoreGeometry(m_restoreGeometry);
    } else {
        qWarning() << "Failed to set restore geometry: requires both ukui_shell integration and valid shell surface";
    }
}

bool WindowProxyPrivate::setupWaylandIntegration()
{
    if (!m_shellSurface) {
        WaylandClient::UkuiShell *interface = WaylandIntegration::self()->waylandUkuiShell();
        if (!interface) {
            return false;
        }

        KWayland::Client::Surface *s = KWayland::Client::Surface::fromWindow(m_window);
        if (!s) {
            return false;
        }
        m_surface = s;
        m_shellSurface = interface->createSurface(s, m_window);
    }

    if (!m_blurManager) {
        m_blurManager = WaylandIntegration::self()->waylandBlurManager();
    }
    return m_shellSurface != nullptr && m_blurManager != nullptr;
}

const QVector<int> WindowProxyPrivate::buildBlurArgs() const
{
    if (!m_enableBlur) {
        return {-1, -1, static_cast<int>(m_blurType)};
    }

    return {static_cast<int>(m_blurLevel), wl_fixed_from_double(m_blurOpacity), static_cast<int>(m_blurType)};
}

void WindowProxyPrivate::removeHeaderBar(bool remove)
{
    // 无边框窗口不需要去除标题栏
    if (m_window->flags().testFlag(Qt::FramelessWindowHint)) {
        qDebug() << "frameless window, no need to remove header bar" << m_window;
        return;
    }

    if(m_useUkuiShellIntegration) {
        m_window->setProperty("ukui_surface_no_titlebar", remove);
        return;
    }
    if (m_isWayland) {
        if (m_shellSurface) {
            m_shellSurface->setRemoveTitleBar(remove);
        }
    } else {
        //x下暂不能取消
        if (remove && m_winId != m_window->winId()) {
            m_winId = m_window->winId();

            MotifWmHints hints;
            hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
            hints.functions = MWM_FUNC_ALL;
            hints.decorations = MWM_DECOR_BORDER;
            XAtomHelper::getInstance()->setWindowMotifHint(static_cast<int>(m_winId), hints);
        }
    }
}

void WindowProxyPrivate::setWindowTypeX11()
{
    NET::WindowType t;
    switch (m_type) {
        default:
            t = NET::Unknown;
            break;
        case WindowType::Normal:
            t = NET::Normal;
            break;
        case WindowType::Dock:
        case WindowType::Panel:
            t = NET::Dock;
            break;
        case WindowType::Desktop:
            t = NET::Desktop;
            break;
        case WindowType::Menu:
            t = NET::Menu;
            break;
        case WindowType::Dialog:
            t = NET::Dialog;
            m_window->setFlags(m_window->flags() | Qt::Dialog);
            break;
        case WindowType::PopupMenu:
            t = NET::PopupMenu;
            m_window->setFlags(m_window->flags() | Qt::Popup);
            break;
        case WindowType::ToolTip:
            t = NET::Tooltip;
            m_window->setFlags(m_window->flags() | Qt::ToolTip);
            break;
        case WindowType::Notification:
            t = NET::Notification;
            break;
        case WindowType::CriticalNotification:
            t = NET::CriticalNotification;
            break;
        case WindowType::SystemWindow:
            t = NET::AppletPopup;
            break;
        case WindowType::OnScreenDisplay:
            t = NET::OnScreenDisplay;
            break;
        case WindowType::AppletPopup:
            t = NET::AppletPopup;
            break;
        case WindowType::ScreenLockNotification:
            t = NET::CriticalNotification;
            break;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    KX11Extras::setType(m_window->winId(), t);
#else
    KWindowSystem::setType(m_window->winId(), t);
#endif

    bool onAllDesktop = false;
    if (m_type == WindowType::Dock || m_type == WindowType::Notification || m_type == WindowType::OnScreenDisplay || m_type == WindowType::CriticalNotification) {
        onAllDesktop = true;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    KX11Extras::setOnAllDesktops(m_window->winId(), onAllDesktop);
#else
    KWindowSystem::setOnAllDesktops(m_window->winId(), onAllDesktop);
#endif
}

void WindowProxyPrivate::setWindowTypeWayland()
{
    if (m_useUkuiShellIntegration) {
        m_window->setProperty("ukui_surface_role_v1", ukui_surface_roleMap.value(m_type));
        return;
    }
    if (!m_shellSurface) {
        return;
    }

    using namespace WaylandClient;
    switch (m_type) {
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
        case WindowType::Menu:
        case WindowType::Dialog:
            m_window->setFlags(m_window->flags() | Qt::Dialog);
        case WindowType::ToolTip:
            m_shellSurface->setRole(UkuiShellSurface::Role::ToolTip);
            break;
        case WindowType::PopupMenu:
            m_window->setFlags(m_window->flags() | Qt::Popup);
            break;
        case WindowType::AppletPopup:
            m_shellSurface->setRole(UkuiShellSurface::Role::AppletPop);
            break;
        case WindowType::Switcher:
            m_shellSurface->setRole(UkuiShellSurface::Role::Switcher);
            break;
    }
}

void WindowProxyPrivate::updateGeometry()
{
    if (m_size.isValid()) {
        m_window->resize(m_size);
    }
    updatePoint();
}

void WindowProxyPrivate::updatePoint()
{
    m_window->setPosition(m_point);
    if (m_shellSurface && !m_useUkuiShellIntegration) {
        m_shellSurface->setPosition(m_point);
    }
}

void WindowProxyPrivate::setWindowType()
{
    if (m_isWayland) {
        setWindowTypeWayland();
    } else {
        setWindowTypeX11();
    }

    // thank kde.
    if (m_type == WindowType::OnScreenDisplay) {
        m_window->setFlags((m_window->flags() & ~Qt::Dialog) | Qt::Window);
    }
}

void WindowProxyPrivate::skipTaskBarOrSwitcher()
{
    if(m_useUkuiShellIntegration) {
        m_window->setProperty("ukui_surface_skip_taskbar", m_operations.testFlag(WindowProxy::SkipTaskBar));
        m_window->setProperty("ukui_surface_skip_switcher", m_operations.testFlag(WindowProxy::SkipSwitcher));
        return;
    }
    if (m_isWayland) {
        if (m_shellSurface) {
            m_shellSurface->setSkipTaskbar(m_operations.testFlag(WindowProxy::SkipTaskBar));
            m_shellSurface->setSkipSwitcher(m_operations.testFlag(WindowProxy::SkipSwitcher));
        }
    } else {
        NET::States states;
        if (m_operations.testFlag(WindowProxy::SkipTaskBar)) {
            states |= NET::SkipTaskbar;
        }
        if (m_operations.testFlag(WindowProxy::SkipSwitcher)) {
            states |= (NET::SkipPager | NET::SkipSwitcher);
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        KX11Extras::setState(m_window->winId(), states);
#else
        KWindowSystem::setState(m_window->winId(), states);
#endif
    }
}

void WindowProxyPrivate::setAutoHideState()
{
    if(m_useUkuiShellIntegration) {
        m_window->setProperty("ukui_surface_panel_auto_hide", m_panelAutoHide);
    } else if (m_shellSurface) {
        m_shellSurface->setPanelAutoHide(m_panelAutoHide);
    }
}

void WindowProxyPrivate::exec()
{
    if (m_isWayland) {
        if (!setupWaylandIntegration()) {
            return;
        }
    }

    setWindowType();
    setAutoHideState();
    removeHeaderBar(m_operations.testFlag(WindowProxy::RemoveTitleBar));
    skipTaskBarOrSwitcher();
    updateGeometry();
    setDecorationComponents();
    setBlurBehind();
    WindowProxy::slideWindow(m_window, m_slideFromEdge, m_slideOffset);
    setWindowStates();
}

void WindowProxyPrivate::setDecorationComponents()
{
    if (m_shellSurface) {
        m_shellSurface->setDecorationComponents(m_decorationComponents);
    }
}

void WindowProxyPrivate::setBlurBehind()
{
    if (!m_enableBlur) {
        if (m_blur) {
            delete m_blur;
            m_blur = nullptr;
        }
        return;
    }
    if (m_blurManager) {
        if (!m_blur) {
            m_blur = m_blurManager->getBlur(KWayland::Client::Surface::fromWindow(m_window), m_window);
        }
        if (m_blur) {
            m_blur->setRegion(m_blurRegion);
            m_blur->setLevel(m_blurLevel);
            m_blur->setOpacity(m_blurOpacity);
            m_blur->setPreferredType(static_cast<uint32_t>(m_blurType));
            if (m_surface) {
                m_surface->commit();
            }
        }
    }
}

void WindowProxyPrivate::setWindowStates()
{
    if (m_shellSurface) {
        m_shellSurface->setStates(m_states.first, m_states.second);
    }
}

// ====== WindowProxy ====== //
WindowProxy::WindowProxy(QWindow *window, WindowProxy::Operations operations)
    : QObject(window),
    d(new WindowProxyPrivate)
{
    Q_ASSERT(window);

    qRegisterMetaType<UkuiQuick::WindowType::Type>();
    d->m_window = window;
    d->m_operations = operations;
    d->m_isWayland = WindowProxy::isWayland();
    d->m_useUkuiShellIntegration = useUkuiShellIntegration(d->m_window);

    for (auto child: window->children()) {
        auto* proxy = qobject_cast<WindowProxy *>(child);
        if (proxy && proxy != this) {
            qWarning() << "WindowProxy::WindowProxy has been created for window" << window
                    << ", you should use WindowProxy::fromWindow to get the instance.";
            return;
        }
    }

    if(d->m_useUkuiShellIntegration) {
        qDebug() << "UkuiQuick::WindowProxy using ukui_shell" << d->m_window <<  d->m_operations;
        //在wlcom中,此属性和FramelessWindowHint flag冲突,如果直接讲此属性设为false,那么再设置FramelessWindowHint将无效.
        if(d->m_operations.testFlag(WindowProxy::RemoveTitleBar)) {
            d->removeHeaderBar(true);
        }
        d->skipTaskBarOrSwitcher();
    } else {
        d->m_window->installEventFilter(this);
    }
}

WindowProxy::~WindowProxy()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

bool WindowProxy::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->m_window) {
        switch (event->type()) {
            case QEvent::PlatformSurface: {
                if (isWayland()) {
                    const QPlatformSurfaceEvent *pSEvent = static_cast<QPlatformSurfaceEvent *>(event);
                    if (pSEvent->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated && !d->m_waylandWindow) {
                        d->m_waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(d->m_window->handle());
                        d->exec();
                        connect(d->m_waylandWindow, &QtWaylandClient::QWaylandWindow::wlSurfaceCreated, this, [this]() {
                            d->exec();
                        });
                        connect(d->m_waylandWindow, &QtWaylandClient::QWaylandWindow::wlSurfaceDestroyed, this, [this]() {
                            delete d->m_shellSurface;
                            d->m_shellSurface = nullptr;
                            delete d->m_blur;
                            d->m_blur = nullptr;
                        });
                    }
                }
            }
            case QEvent::Show:
            case QEvent::Expose: {
                if(!isWayland())
                    d->exec();
            }
            default:
                break;
        }
    }

    return QObject::eventFilter(watched, event);
}

bool WindowProxy::isWayland()
{
    return QGuiApplication::platformName().startsWith(QStringLiteral("wayland"));
}

// ==APIS==
void WindowProxy::setRemoveTitleBar(bool remove)
{
    if (remove) {
        d->m_operations |= RemoveTitleBar;
    } else {
        d->m_operations &= ~RemoveTitleBar;
    }
    if(d->m_useUkuiShellIntegration) {
        d->removeHeaderBar(remove);
    }
}

void WindowProxy::setSkipTaskBar(bool skip)
{
    if (skip) {
        d->m_operations |= SkipTaskBar;
    } else {
        d->m_operations &= ~SkipTaskBar;
    }
    if(d->m_useUkuiShellIntegration) {
        d->skipTaskBarOrSwitcher();
    }
}

void WindowProxy::setSkipSwitcher(bool skip)
{
    if (skip) {
        d->m_operations |= SkipSwitcher;
    } else {
        d->m_operations &= ~SkipSwitcher;
    }
    if(d->m_useUkuiShellIntegration) {
        d->skipTaskBarOrSwitcher();
    }
}

void WindowProxy::setPosition(const QPoint &point)
{
    d->m_point = point;
    d->updatePoint();
}

void WindowProxy::setGeometry(const QRect &rect)
{
    d->m_size = rect.size();
    d->m_point = rect.topLeft();
    if(d->m_useUkuiShellIntegration) {
        d->m_window->setGeometry(rect);
    } else {
        d->updateGeometry();
    }
}

void WindowProxy::setWindowType(WindowType::Type type)
{
    d->m_type = type;
    d->setWindowType();
}

void WindowProxy::setPanelAutoHide(bool autoHide)
{
    d->m_panelAutoHide = autoHide;
    d->setAutoHideState();
}

void WindowProxy::slideWindow(QWindow *window, WindowProxy::SlideFromEdge fromEdge, int offset)
{
    if(useUkuiShellIntegration(window)) {
        window->setProperty("ukui_surface_slide", QVariant::fromValue(QPair(ukui_surface_slideEdge.value(fromEdge), offset)));
        return;
    }
    KWindowEffects::SlideFromLocation fromLocation;
    switch (fromEdge) {
        case NoEdge:
            fromLocation = KWindowEffects::SlideFromLocation::NoEdge;
            break;
        case TopEdge:
            fromLocation = KWindowEffects::SlideFromLocation::TopEdge;
            break;
        case RightEdge:
            fromLocation = KWindowEffects::SlideFromLocation::RightEdge;
            break;
        case BottomEdge:
            fromLocation = KWindowEffects::SlideFromLocation::BottomEdge;
            break;
        case LeftEdge:
            fromLocation = KWindowEffects::SlideFromLocation::LeftEdge;
            break;
    }

    KWindowEffects::slideWindow(window, fromLocation, offset);
}

void WindowProxy::slideWindow(WindowProxy::SlideFromEdge fromEdge, int offset)
{
    if(d->m_useUkuiShellIntegration) {
        d->m_window->setProperty("ukui_surface_slide", QVariant::fromValue(QPair(ukui_surface_slideEdge.value(fromEdge), offset)));
        return;
    }
    d->m_slideFromEdge = fromEdge;
    d->m_slideOffset = offset;
    WindowProxy::slideWindow(d->m_window, fromEdge, offset);
}

void WindowProxy::setBlurRegion(QWindow *window, bool enable, const QRegion &region)
{
    KWindowEffects::enableBlurBehind(window, enable, region);
}

void WindowProxy::setBlurRegion(bool enable, const QRegion &region)
{
    d->m_enableBlur = enable;
    d->m_blurRegion = region;
    if (isWayland()) {
        if(d->m_useUkuiShellIntegration) {
            d->m_window->setProperty("ukui_surface_blur_v1", QVariant::fromValue(QPair<QRegion, QVector<int>>(
                d->m_blurRegion, d->buildBlurArgs())));
            return;
        }
        d->setBlurBehind();
    } else {
        KWindowEffects::enableBlurBehind(d->m_window, enable, region);
    }
}

void WindowProxy::setBlurLevel(uint32_t level)
{
    if (d->m_blurLevel == level) {
        return;
    }
    d->m_blurLevel = level;
    if (isWayland()) {
        if(d->m_useUkuiShellIntegration) {
            d->m_window->setProperty("ukui_surface_blur_v1", QVariant::fromValue(QPair<QRegion, QVector<int>>(
                d->m_blurRegion, d->buildBlurArgs())));
            return;
        }
        d->setBlurBehind();
    } else {
        qWarning() << "Blur level can be only set on wayland!";
    }
}

/*!
    /brief 设置毛玻璃透明度
    \a opacity 毛玻璃透明度，范围 0.0 - 1.0
 */
void WindowProxy::setBlurOpacity(const qreal opacity)
{
    if (d->m_blurOpacity == opacity) {
        return;
    }
    d->m_blurOpacity = opacity;
    if (isWayland()) {
        if(d->m_useUkuiShellIntegration) {
            d->m_window->setProperty("ukui_surface_blur_v1", QVariant::fromValue(QPair<QRegion, QVector<int>>(
                d->m_blurRegion, d->buildBlurArgs())));
            return;
        }
        d->setBlurBehind();
    } else {
        qWarning() << "Blur opacity can be only set on wayland!";
    }
}

void WindowProxy::setBlurType(WindowProxy::BlurType type)
{
    d->m_blurType = type;
    if (isWayland()) {
        if(d->m_useUkuiShellIntegration) {
            d->m_window->setProperty("ukui_surface_blur_v1", QVariant::fromValue(QPair<QRegion, QVector<int>>(d->m_blurRegion, d->buildBlurArgs())));
            return;
        }
        d->setBlurBehind();
    } else {
        qWarning() << "Blur type can be only set on wayland!";
    }
}

QScreen* WindowProxy::currentScreen()
{
    if(isWayland()) {
        auto shell = WaylandIntegration::self()->waylandUkuiShell();
        if (shell) {
            shell->updateCurrentOutput();
            WaylandIntegration::self()->sync();
            if(shell->isCurrentOutputReady()) {
                QString name = shell->outputName();
                for (auto screen: qGuiApp->screens()) {
                    if (screen->name() == name) {
                        return screen;
                    }
                }
            }
        }
    } else {
        QScreen * screen =  qGuiApp->screenAt(QCursor::pos());
        if(screen) {
            return screen;
        }
    }
    return qGuiApp->primaryScreen();
}

bool WindowProxy::useUkuiShellIntegration(QWindow *window)
{
    return QGuiApplication::platformName().startsWith(QStringLiteral("wayland")) &&
        QGuiApplication::platformNativeInterface()->nativeResourceForWindow("ukui_shell", window);
}

QPoint WindowProxy::position() const
{
    return d->m_point;
}

void WindowProxy::setDecorationComponents(Decoration::Components components)
{
    if (!isWayland()) {
        qWarning() << "Decoration components can be only set on wayland!";
        return;
    }
    if(d->m_useUkuiShellIntegration) {
        d->m_window->setProperty("ukui_surface_decoration_components", static_cast<uint32_t>(components));
    } else {
        d->m_decorationComponents = components;
        d->setDecorationComponents();
    }
}

void WindowProxy::setWindowStates(WindowState::States states, bool enable)
{
    if (isWayland()) {
        if(d->m_useUkuiShellIntegration) {
            QPair<uint32_t, uint32_t> pair(states, enable? states : WindowState::States());
            qDebug() << "===setWindowState===" << pair;
            d->m_window->setProperty("ukui_surface_state", QVariant::fromValue(pair));
        } else {
            d->m_states = QPair<WindowState::States, bool>(states, enable);
            d->setWindowStates();
        }
    } else {
        const WindowState::States otherStates = states & (~(WindowState::KeepAbove | WindowState::KeepBelow));
        if (otherStates != 0) {
            qWarning() << "Unsupported window states in X11 environment:" << static_cast<uint>(otherStates);
        }

        const WId winId = d->m_window->winId();
        if (states.testFlag(WindowState::KeepAbove)) {
            if (enable) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                if (enable) {
                    KX11Extras::setState(winId, NET::KeepAbove);
                } else {
                    KX11Extras::clearState(winId, NET::KeepAbove);
                }
#else
                if (enable) {
                    KWindowSystem::setState(winId, NET::KeepAbove);
                } else {
                    KWindowSystem::clearState(winId, NET::KeepAbove);
                }
#endif
            }
        }
        if (states.testFlag(WindowState::KeepBelow)) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (enable) {
                KX11Extras::setState(winId, NET::KeepBelow);
            } else {
                KX11Extras::clearState(winId, NET::KeepBelow);
            }
#else
            if (enable) {
                KWindowSystem::setState(winId, NET::KeepBelow);
            } else {
                KWindowSystem::clearState(winId, NET::KeepBelow);
            }
#endif
        }
    }
}

WindowProxy* WindowProxy::fromWindow(QWindow* window)
{
    for (auto child : window->children()) {
        if (auto *proxy = qobject_cast<WindowProxy *>(child)) {
            return proxy;
        }
    }
    return new WindowProxy(window, Operations());
}

QRect WindowProxy::restoreGeometry() const
{
    return d->m_restoreGeometry;
}

void WindowProxy::setRestoreGeometry(const QRect& restoreGeomerty)
{
    d->m_restoreGeometry = restoreGeomerty;
    d->setRestoreGeometry();
}

// ======== WindowProxy2 =========== //
class WindowProxy2::Private
{
public:
    WMInterface *m_wm {nullptr};

    bool m_needResetProp = true;
    bool m_panelAutoHide = false;
    bool m_panelTakeFocus = false;
    WindowProxy::Operations m_operations;
    // 窗口类型
    WindowType::Type m_type = WindowType::Normal;
    QPoint m_pos;
    KWindowEffects::SlideFromLocation m_fromLocation;
    int m_offset;
    UkuiQuick::Decoration::Components m_decorationComponents = UkuiQuick::Decoration::Border | UkuiQuick::Decoration::Shadow | UkuiQuick::Decoration::RoundCorner;

    void setupProp();
};

void WindowProxy2::Private::setupProp()
{
    m_wm->setWindowType(m_type);

    m_wm->setSkipTaskBar(m_operations.testFlag(WindowProxy::SkipTaskBar));
    m_wm->setSkipSwitcher(m_operations.testFlag(WindowProxy::SkipSwitcher));
    m_wm->setRemoveTitleBar(m_operations.testFlag(WindowProxy::RemoveTitleBar));

    m_wm->setPanelTakesFocus(m_panelTakeFocus);
    m_wm->setPanelAutoHide(m_panelAutoHide);
    m_wm->setDecorationComponents(m_decorationComponents);
}

WindowProxy2::WindowProxy2(QWindow *window, WindowProxy::Operations operations)
    : WMInterface(window), d(new WindowProxy2::Private)
{
    if(!WindowProxy::useUkuiShellIntegration(window)) {
        window->installEventFilter(this);
    }
    d->m_operations = operations;
    d->m_wm = WManager::getWM(window);
    d->m_wm->setSkipTaskBar(d->m_operations.testFlag(WindowProxy::SkipTaskBar));
    d->m_wm->setSkipSwitcher(d->m_operations.testFlag(WindowProxy::SkipSwitcher));
    d->m_wm->setRemoveTitleBar(d->m_operations.testFlag(WindowProxy::RemoveTitleBar));
}

WindowProxy2::~WindowProxy2()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

bool WindowProxy2::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == window()) {
        switch (event->type()) {
            case QEvent::Expose:
                if (!d->m_needResetProp || !window()->isVisible()) {
                    break;
                }
                d->m_needResetProp = false;
            case QEvent::Show:
                d->setupProp();
                KWindowEffects::slideWindow(window(), d->m_fromLocation, d->m_offset);
                break;
            case QEvent::Hide:
                d->m_needResetProp = true;
                break;
            default:
                break;
        }
    }

    return QObject::eventFilter(watched, event);
}

QPoint WindowProxy2::position() const
{
    return d->m_wm->position();
}

void WindowProxy2::setPosition(const QPoint &point)
{
    d->m_wm->setPosition(point);
}

void WindowProxy2::setWindowType(WindowType::Type type)
{
    if (d->m_type == type) {
        return;
    }

    d->m_type = type;
    d->m_wm->setWindowType(type);
}

void WindowProxy2::setSkipTaskBar(bool skip)
{
    if (skip) {
        d->m_operations |= WindowProxy::SkipTaskBar;
    } else {
        d->m_operations &= ~WindowProxy::SkipTaskBar;
    }

    d->m_wm->setSkipTaskBar( d->m_operations.testFlag(WindowProxy::SkipTaskBar));
}

void WindowProxy2::setSkipSwitcher(bool skip)
{
    if (skip) {
        d->m_operations |= WindowProxy::SkipSwitcher;
    } else {
        d->m_operations &= ~WindowProxy::SkipSwitcher;
    }

    d->m_wm->setSkipSwitcher( d->m_operations.testFlag(WindowProxy::SkipSwitcher));
}

void WindowProxy2::setRemoveTitleBar(bool remove)
{
    if (remove) {
        d->m_operations |= WindowProxy::RemoveTitleBar;
    } else {
        d->m_operations &= ~WindowProxy::RemoveTitleBar;
    }

    d->m_wm->setRemoveTitleBar( d->m_operations.testFlag(WindowProxy::RemoveTitleBar));
}

void WindowProxy2::setPanelTakesFocus(bool takesFocus)
{
    if (d->m_panelTakeFocus == takesFocus) {
        return;
    }

    d->m_panelTakeFocus = takesFocus;
    d->m_wm->setPanelTakesFocus(takesFocus);
}

void WindowProxy2::setPanelAutoHide(bool autoHide)
{
    if (d->m_panelAutoHide == autoHide) {
        return;
    }

    d->m_panelAutoHide = autoHide;
    d->m_wm->setPanelTakesFocus(autoHide);
}

void WindowProxy2::setSlideWindowArgs(WindowProxy::SlideFromEdge fromEdge, int offset)
{
    if (WindowProxy::useUkuiShellIntegration(window())) {
        window()->setProperty("ukui_surface_slide", QVariant::fromValue(QPair(ukui_surface_slideEdge.value(fromEdge), offset)));
        return;
    }
    KWindowEffects::SlideFromLocation fromLocation;
    switch (fromEdge) {
        case WindowProxy::NoEdge:
            fromLocation = KWindowEffects::SlideFromLocation::NoEdge;
            break;
        case WindowProxy::TopEdge:
            fromLocation = KWindowEffects::SlideFromLocation::TopEdge;
            break;
        case WindowProxy::RightEdge:
            fromLocation = KWindowEffects::SlideFromLocation::RightEdge;
            break;
        case WindowProxy::BottomEdge:
            fromLocation = KWindowEffects::SlideFromLocation::BottomEdge;
            break;
        case WindowProxy::LeftEdge:
            fromLocation = KWindowEffects::SlideFromLocation::LeftEdge;
            break;
    }
    d->m_fromLocation = fromLocation;
    d->m_offset = offset;
    KWindowEffects::slideWindow(window(), d->m_fromLocation, d->m_offset);
}

QScreen* WindowProxy2::currentScreen()
{
    return d->m_wm->currentScreen();
}

void WindowProxy2::setDecorationComponents(Decoration::Components components)
{
    if (d->m_decorationComponents == components) {
        return;
    }
    d->m_decorationComponents = components;
    d->m_wm->setDecorationComponents(components);
}
} // UkuiQuick

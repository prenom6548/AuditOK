// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "ukuishellsurface.h"
#include "logging.h"
#include "xdgdecorationv1.h"
#include "xdgdialogv1.h"

#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include <QtGui/private/qwindow_p.h>

#include <QEvent>

namespace UKUI
{

UkuiShellSurface::UkuiShellSurface(UkuiShell *shell, ::xdg_surface *surface, ::ukui_surface *ukuiSurface, QtWaylandClient::QWaylandWindow *window)
    : QWaylandShellSurface(window)
    , xdg_surface(surface)
    , ukui_surface(ukuiSurface)
    , m_shell(shell)
    , m_window(window)
{
    auto *display = window->display();
    Qt::WindowType type = window->window()->type();
    auto *transientParent = window->transientParent();

    if (type == Qt::ToolTip && transientParent) {
        setPopup(transientParent);
    } else if (type == Qt::Popup && transientParent && display->lastInputDevice()) {
        setGrabPopup(transientParent, display->lastInputDevice(), display->lastInputSerial());
    } else {
        setToplevel();
    }
    setSizeHints();

    initWindowByProperties();
    m_window->window()->installEventFilter(this);
}

UkuiShellSurface::~UkuiShellSurface()
{
    if (m_toplevel) {
        delete m_toplevel;
        m_toplevel = nullptr;
    }
    if (m_popup) {
        delete m_popup;
        m_popup = nullptr;
    }
    xdg_surface::destroy();
    ukui_surface::destroy();
}

UkuiShell *UkuiShellSurface::shell()
{
    return m_shell;
}

Toplevel *UkuiShellSurface::toplevel()
{
    return m_toplevel;
}

Popup *UkuiShellSurface::popup()
{
    return m_popup;
}

bool UkuiShellSurface::resize(QtWaylandClient::QWaylandInputDevice *inputDevice, Qt::Edges edges)
{
    if (!m_toplevel || !m_toplevel->isInitialized())
        return false;

    auto resizeEdges = Toplevel::convertToResizeEdges(edges);
    m_toplevel->resize(inputDevice->wl_seat(), inputDevice->serial(), resizeEdges);
    return true;
}

bool UkuiShellSurface::move(QtWaylandClient::QWaylandInputDevice *inputDevice)
{
    if (m_toplevel && m_toplevel->isInitialized()) {
        m_toplevel->move(inputDevice->wl_seat(), inputDevice->serial());
        return true;
    }
    return false;
}

bool UkuiShellSurface::showWindowMenu(QtWaylandClient::QWaylandInputDevice *seat)
{
    if (m_toplevel && m_toplevel->isInitialized()) {
        QPoint position = seat->pointerSurfacePosition().toPoint();
        m_toplevel->show_window_menu(seat->wl_seat(), seat->serial(), position.x(), position.y());
        return true;
    }
    return false;
}

void UkuiShellSurface::setTitle(const QString &title)
{
    if (m_toplevel)
        m_toplevel->set_title(title);
}

void UkuiShellSurface::setAppId(const QString &appId)
{
    if (m_toplevel)
        m_toplevel->set_app_id(appId);
}

void UkuiShellSurface::setWindowFlags(Qt::WindowFlags flags)
{
    if (m_toplevel)
        m_toplevel->requestWindowFlags(flags);
}

bool UkuiShellSurface::isExposed() const
{
    return m_configured || m_pendingConfigureSerial;
}

bool UkuiShellSurface::handleExpose(const QRegion &region)
{
    if (!isExposed() && !region.isEmpty()) {
        m_exposeRegion = region;
        return true;
    }
    return false;
}

void UkuiShellSurface::applyConfigure()
{
    // It is a redundant ack_configure, so skipped.
    if (m_pendingConfigureSerial == m_appliedConfigureSerial)
        return;

    if (m_toplevel)
        m_toplevel->applyConfigure();
    m_appliedConfigureSerial = m_pendingConfigureSerial;

    m_configured = true;
    ack_configure(m_appliedConfigureSerial);
}

bool UkuiShellSurface::wantsDecorations() const
{
    return m_toplevel && m_toplevel->wantsDecorations();
}

void UkuiShellSurface::propagateSizeHints()
{
    setSizeHints();

    if (m_toplevel && m_window)
        m_window->commit();
}

void UkuiShellSurface::setWindowGeometry(const QRect &rect)
{
    set_window_geometry(rect.x(), rect.y(), rect.width(), rect.height());
}

void UkuiShellSurface::setSizeHints()
{
    if (m_toplevel && m_window) {
        const int minWidth = qMax(0, m_window->windowMinimumSize().width());
        const int minHeight = qMax(0, m_window->windowMinimumSize().height());
        m_toplevel->set_min_size(minWidth, minHeight);

        int maxWidth = qMax(minWidth, m_window->windowMaximumSize().width());
        if (maxWidth == QWINDOWSIZE_MAX)
            maxWidth = 0;
        int maxHeight = qMax(minHeight, m_window->windowMaximumSize().height());
        if (maxHeight == QWINDOWSIZE_MAX)
            maxHeight = 0;
        m_toplevel->set_max_size(maxWidth, maxHeight);
    }
}

void UkuiShellSurface::raise()
{
}

void UkuiShellSurface::lower()
{
}

void UkuiShellSurface::setContentOrientationMask(Qt::ScreenOrientations orientation)
{
}

void UkuiShellSurface::sendProperty(const QString &name, const QVariant &value)
{
}

void UkuiShellSurface::setSkipTaskbar(bool skip)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_skip_taskbar(skip);
    else
        qWarning(UKUI_SHELL) << "ukui_surface is not initialized, ignoring skip taskbar requested.";
}

void UkuiShellSurface::setSkipSwitcher(bool skip)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_skip_switcher(skip);
    else
        qWarning(UKUI_SHELL) << "ukui_surface is not initialized, ignoring skip switcher requested.";
}

void UkuiShellSurface::setNoTitlebar(bool noTitlebar)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_property(property_no_titlebar, noTitlebar);
    else
        qCDebug(UKUI_SHELL) << "ukui_surface is not initialized, ignoring remove titlebar requested.";
}

void UkuiShellSurface::setTheme(int theme)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_property(property_theme, theme);
    else
        qCDebug(UKUI_SHELL) << "ukui_surface is not initialized, ignoring set theme requested.";
}

void UkuiShellSurface::setWindowRadius(int radius)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_property(property_window_radius, radius);
    else
        qCDebug(UKUI_SHELL) << "ukui_surface is not initialized, ignoring set window radius requested.";
}

void UkuiShellSurface::setBorderWidth(int width)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_property(property_border_width, width);
    else
        qCDebug(UKUI_SHELL) << "ukui_surface is not initialized, ignoring set border width requested.";
}

void UkuiShellSurface::setBorderColor(QColor color)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_property(property_border_color, color.rgba());
    else
        qCDebug(UKUI_SHELL) << "ukui_surface is not initialized, ignoring set border color requested.";
}

void UkuiShellSurface::setShadowRadius(int radius)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_property(property_shadow_radius, radius);
    else
        qCDebug(UKUI_SHELL) << "ukui_surface is not initialized, ignoring set shadow radius requested.";
}

void UkuiShellSurface::setShadowOffset(int offset)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_property(property_shadow_offset, offset);
    else
        qCDebug(UKUI_SHELL) << "ukui_surface is not initialized, ignoring set shadow offset requested.";
}

void UkuiShellSurface::setShadowColor(QColor color)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_property(property_shadow_color, color.rgba());
    else
        qCDebug(UKUI_SHELL) << "ukui_surface is not initialized, ignoring set shadow color requested.";
}

void UkuiShellSurface::setWindowRole(int role)
{
    if (ukui_surface::isInitialized()) {
        setSkipTaskbar(role != role_normal);
        setSkipSwitcher(role != role_normal);
        ukui_surface::set_role(role);
    } else {
        qCDebug(UKUI_SHELL) << "ukui_surface is not initialized, ignoring set window role requested.";
    }
}

void UkuiShellSurface::setWindowIcon(const QString &icon_name)
{
    if (ukui_surface::isInitialized())
        ukui_surface::set_icon(icon_name);
    else
        qCDebug(UKUI_SHELL) << "ukui_surface is not initialized, ignoring set window icon requested.";
}

void UkuiShellSurface::requestWindowStates(Qt::WindowStates states)
{
    if (m_toplevel)
        m_toplevel->requestWindowStates(states);
    else
        qCDebug(UKUI_SHELL) << "Ignoring window states requested by non-toplevel zxdg_surface_v6.";
}

void UkuiShellSurface::setToplevel()
{
    Q_ASSERT(!m_toplevel && !m_popup);
    m_toplevel = new Toplevel(this);
}

void UkuiShellSurface::setPopup(QtWaylandClient::QWaylandWindow *parent)
{
    Q_ASSERT(!m_toplevel && !m_popup);

    auto positioner = new xdg_positioner(m_shell->create_positioner());
    // set_popup expects a position relative to the parent
    QPoint transientPos = m_window->geometry().topLeft(); // this is absolute
    transientPos -= parent->geometry().topLeft();
    if (parent->decoration()) {
        transientPos.setX(transientPos.x() + parent->decoration()->margins().left());
        transientPos.setY(transientPos.y() + parent->decoration()->margins().top());
    }
    positioner->set_anchor_rect(transientPos.x(), transientPos.y(), 1, 1);
    positioner->set_anchor(xdg_positioner::anchor_top_left);
    positioner->set_gravity(xdg_positioner::gravity_bottom_right);
    positioner->set_size(m_window->geometry().width(), m_window->geometry().height());
    m_popup = new Popup(this, parent, positioner);
    positioner->destroy();

    delete positioner;
}

void UkuiShellSurface::setGrabPopup(QtWaylandClient::QWaylandWindow *parent, QtWaylandClient::QWaylandInputDevice *device, int serial)
{
    auto parentUkuiShellSurface = static_cast<UkuiShellSurface *>(parent->shellSurface());
    auto *top = m_shell->topmostGrabbingPopup();

    if (top && top->ukuiShellSurface() != parentUkuiShellSurface) {
        qCWarning(UKUI_SHELL) << "setGrabPopup called with a parent," << parentUkuiShellSurface
                              << "which does not match the current topmost grabbing popup,"
                              << top->ukuiShellSurface() << "According to the xdg-shell protocol, this"
                              << "is not allowed. The wayland QPA plugin is currently handling"
                              << "it by setting the parent to the topmost grabbing popup."
                              << "Note, however, that this may cause positioning errors and"
                              << "popups closing unxpectedly because xdg-shell mandate that child"
                              << "popups close before parents";
        parent = top->ukuiShellSurface()->m_window;
    }
    setPopup(parent);
    m_popup->grab(device, serial);

    // Synthesize Qt enter/leave events for popup
    if (!parent)
        return;
    QWindow *current = QGuiApplication::topLevelAt(QCursor::pos());
    QWindow *leave = parent->window();
    if (current != leave)
        return;

    QWindowSystemInterface::handleLeaveEvent(leave);

    QWindow *enter = nullptr;
    if (m_popup && m_popup->ukuiShellSurface() && m_popup->ukuiShellSurface()->window())
        enter = m_popup->ukuiShellSurface()->window()->window();

    if (enter)
        QWindowSystemInterface::handleEnterEvent(enter, enter->mapFromGlobal(QCursor::pos()), QCursor::pos());
}

void UkuiShellSurface::xdg_surface_configure(uint32_t serial)
{
    m_pendingConfigureSerial = serial;
    if (!m_configured) {
        // We have to do the initial applyConfigure() immediately, since that is the expose.
        applyConfigure();
        m_exposeRegion = QRegion(QRect(QPoint(), m_window->geometry().size()));
    } else {
        // Later configures are probably resizes, so we have to queue them up for a time when we
        // are not painting to the window.
        m_window->applyConfigureWhenPossible();
    }

    if (!m_exposeRegion.isEmpty()) {
        m_window->handleExpose(m_exposeRegion);
        m_exposeRegion = QRegion();
    }
}

void UkuiShellSurface::ukui_surface_position(int32_t x, int32_t y)
{
    QWindowSystemInterface::handleGeometryChange(m_window->window(), QRect(x, y, m_window->geometry().width(), m_window->geometry().height()));
}

QMap<QString, UkuiShellSurface::role> roleMap = {
    {"normal", UkuiShellSurface::role_normal},
    {"desktop", UkuiShellSurface::role_desktop},
    {"dock", UkuiShellSurface::role_panel},
    {"panel", UkuiShellSurface::role_panel},
    {"onscreendisplay", UkuiShellSurface::role_onscreendisplay},
    {"notification", UkuiShellSurface::role_notification},
    {"menu", UkuiShellSurface::role_tooltip},
    {"popupmenu", UkuiShellSurface::role_tooltip},
    {"tooltip", UkuiShellSurface::role_tooltip},
    {"criticalnotification", UkuiShellSurface::role_criticalnotification},
    {"appletpopup", UkuiShellSurface::role_appletpopup},
    {"screenlock", UkuiShellSurface::role_screenlock},
    {"watermark", UkuiShellSurface::role_watermark},
    {"systemwindow", UkuiShellSurface::role_systemwindow},
    {"inputpanel", UkuiShellSurface::role_inputpanel},
    {"logout", UkuiShellSurface::role_logout},
    {"screenlocknotification", UkuiShellSurface::role_screenlocknotification},
    {"switcher", UkuiShellSurface::role_switcher}};

void UkuiShellSurface::slotWindowPropertyChanged(const char *name)
{
    QWindow *window = m_window->window();
    const QString key = QString(name).replace(QRegExp("[^a-zA-Z]"), "").toLower();
    if (key == "notitlebar") {
        setNoTitlebar(window->property(name).toBool());
    } else if (key == "theme") {
        setTheme(window->property(name).toInt());
    } else if (key == "windowradius") {
        setWindowRadius(window->property(name).toInt());
    } else if (key == "borderwidth") {
        setBorderWidth(window->property(name).toInt());
    } else if (key == "bordercolor") {
        setBorderColor(window->property(name).value<QColor>());
    } else if (key == "shadowradius") {
        setShadowRadius(window->property(name).toInt());
    } else if (key == "shadowoffset") {
        setShadowOffset(window->property(name).toInt());
    } else if (key == "shadowcolor") {
        setShadowColor(window->property(name).value<QColor>());
    } else if (key.contains("role")) {
        const QString role_str = window->property(name).toString().toLower();
        if (roleMap.contains(role_str)) {
            setWindowRole(roleMap[role_str]);
        } else {
            setWindowRole(UkuiShellSurface::role_normal);
            qCWarning(UKUI_SHELL) << "Unknown role property:" << role_str << ", set role to normal";
        }
    } else if (key == "icon") {
        setWindowIcon(window->property(name).toString());
    } else if (key == "panelautohide") {
        set_panel_auto_hide(window->property(name).toBool());
    } else if (key == "grabkeyboard") {
        grab_keyboard(m_window->display()->lastInputDevice()->object());
    } else if (key == "openundercursor") {
        const auto point = window->property(name).toPoint();
        open_under_cursor(point.x(), point.y());
    } else if (key == "position") {
        const auto point = window->property(name).toPoint();
        set_position(point.x(), point.y());
    } else if (key == "skiptaskbar") {
        setSkipTaskbar(window->property(name).toBool());
    } else if (key == "skipswitcher") {
        setSkipSwitcher(window->property(name).toBool());
    } else if (key == "paneltakefocus") {
        set_panel_takes_focus(window->property(name).toBool());
    }
}

void UkuiShellSurface::initWindowByProperties()
{
    QWindow *window = m_window->window();
    const QList<QByteArray> properties_list = window->dynamicPropertyNames();
    for (const QByteArray &property : properties_list) {
        slotWindowPropertyChanged(property.constData());
    }
}

bool UkuiShellSurface::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_window->window()) {
        if (event->type() == QEvent::DynamicPropertyChange) {
            QDynamicPropertyChangeEvent *propEvent = static_cast<QDynamicPropertyChangeEvent *>(event);
            slotWindowPropertyChanged(propEvent->propertyName().constData());
        }
    }
    return false;
}

Toplevel::Toplevel(UkuiShellSurface *ukuiShellSurface)
    : xdg_toplevel(ukuiShellSurface->get_toplevel())
    , m_ukuiShellSurface(ukuiShellSurface)
{
    QWindow *window = ukuiShellSurface->window()->window();
    if (auto *decorationManager = ukuiShellSurface->shell()->decorationManager()) {
        if (!(window->flags() & Qt::FramelessWindowHint)) {
            m_decoration = decorationManager->createToplevelDecoration(object());
            if (window->flags() & Qt::WindowTransparentForInput && (QT_VERSION_MAJOR < 6 || (QT_VERSION_MAJOR == 6 && QT_VERSION_MINOR < 6))) {
                qWarning() << "Seems like you are trying to make a window transparent for input,"
                              "but it is not supported on server side decorated windows. Please"
                              "set window flags to Qt::FramelessWindowHint to make it work.";
            }
        } else if ((window->flags() & Qt::WindowTransparentForInput) && (QT_VERSION_MAJOR < 6 || (QT_VERSION_MAJOR == 6 && QT_VERSION_MINOR < 6))) {
            auto display = ukuiShellSurface->window()->display();
            auto input_region = display->createRegion(window->mask());
            m_ukuiShellSurface->window()->waylandSurface()->set_input_region(input_region);
            wl_region_destroy(input_region);
        }
    }
    requestWindowStates(window->windowStates());
    requestWindowFlags(window->flags());
    if (auto transientParent = m_ukuiShellSurface->window()->transientParent()) {
        if (auto parentSurface = qobject_cast<UkuiShellSurface *>(transientParent->shellSurface())) {
            set_parent(parentSurface->toplevel()->object());
            if (window->modality() != Qt::NonModal && m_ukuiShellSurface->shell()->xdgDialogWm()) {
                m_xdgDialog.reset(m_ukuiShellSurface->shell()->xdgDialogWm()->getDialog(object()));
                m_xdgDialog->set_modal();

                m_ukuiShellSurface->setSkipTaskbar(true);
                m_ukuiShellSurface->setSkipSwitcher(true);
            }
        }
    }
}

Toplevel::~Toplevel()
{
    // The protocol spec requires that the decoration object is deleted before xdg_toplevel.
    delete m_decoration;
    m_decoration = nullptr;

    if (isInitialized())
        destroy();
}

void Toplevel::applyConfigure()
{
    if (!(m_applied.states & (Qt::WindowMaximized | Qt::WindowFullScreen)))
        m_normalSize = m_ukuiShellSurface->window()->windowFrameGeometry().size();

    if ((m_pending.states & Qt::WindowActive) && !(m_applied.states & Qt::WindowActive)
        && !m_ukuiShellSurface->window()->display()->isKeyboardAvailable())
        m_ukuiShellSurface->window()->display()->handleWindowActivated(m_ukuiShellSurface->window());

    if (!(m_pending.states & Qt::WindowActive) && (m_applied.states & Qt::WindowActive)
        && !m_ukuiShellSurface->window()->display()->isKeyboardAvailable())
        m_ukuiShellSurface->window()->display()->handleWindowDeactivated(m_ukuiShellSurface->window());

    m_ukuiShellSurface->window()->handleWindowStatesChanged(m_pending.states);

    if (m_pending.size.isEmpty()) {
        // An empty size in the configure means it's up to the client to choose the size
        bool normalPending = !(m_pending.states & (Qt::WindowMaximized | Qt::WindowFullScreen));
        if (normalPending && !m_normalSize.isEmpty())
            m_ukuiShellSurface->window()->resizeFromApplyConfigure(m_normalSize);
    } else {
        m_ukuiShellSurface->window()->resizeFromApplyConfigure(m_pending.size);
    }

    m_applied = m_pending;
    qCDebug(UKUI_SHELL) << "Applied pending xdg_toplevel configure event:" << m_applied.size << m_applied.states;
}

bool Toplevel::wantsDecorations()
{
    if (m_decoration && (m_decoration->pending() == XdgToplevelDecorationV1::mode_server_side || !m_decoration->isConfigured()))
        return false;

    return !(m_pending.states & Qt::WindowFullScreen);
}

void Toplevel::xdg_toplevel_configure(int32_t width, int32_t height, wl_array *states)
{
    m_pending.size = QSize(width, height);

    auto *xdgStates = static_cast<uint32_t *>(states->data);
    size_t numStates = states->size / sizeof(uint32_t);

    m_pending.states = Qt::WindowNoState;

    for (size_t i = 0; i < numStates; i++) {
        switch (xdgStates[i]) {
        case XDG_TOPLEVEL_STATE_ACTIVATED:
            m_pending.states |= Qt::WindowActive;
            break;
        case XDG_TOPLEVEL_STATE_MAXIMIZED:
            m_pending.states |= Qt::WindowMaximized;
            break;
        case XDG_TOPLEVEL_STATE_FULLSCREEN:
            m_pending.states |= Qt::WindowFullScreen;
            break;
        default:
            break;
        }
    }
    qCDebug(UKUI_SHELL) << "Received xdg_toplevel.configure with" << m_pending.size
                        << "and" << m_pending.states;
}

void Toplevel::xdg_toplevel_close()
{
    m_ukuiShellSurface->window()->window()->close();
}

void Toplevel::requestWindowFlags(Qt::WindowFlags flags)
{
    if (m_decoration) {
        if (flags & Qt::FramelessWindowHint) {
            delete m_decoration;
            m_decoration = nullptr;
        } else {
            m_decoration->unsetMode();
        }
    }
}

void Toplevel::requestWindowStates(Qt::WindowStates states)
{
    // Re-send what's different from the applied state
    Qt::WindowStates changedStates = m_applied.states ^ states;

    if (changedStates & Qt::WindowMaximized) {
        if (states & Qt::WindowMaximized)
            set_maximized();
        else
            unset_maximized();
    }

    if (changedStates & Qt::WindowFullScreen) {
        if (states & Qt::WindowFullScreen) {
            auto screen = m_ukuiShellSurface->window()->waylandScreen();
            if (screen) {
                set_fullscreen(screen->output());
            }
        } else
            unset_fullscreen();
    }

    // Minimized state is not reported by the protocol, so always send it
    if (states & Qt::WindowMinimized) {
        set_minimized();
        m_ukuiShellSurface->window()->handleWindowStatesChanged(states & ~Qt::WindowMinimized);
    }
}

xdg_toplevel::resize_edge Toplevel::convertToResizeEdges(Qt::Edges edges)
{
    return static_cast<enum resize_edge>(
        ((edges & Qt::TopEdge) ? resize_edge_top : 0)
        | ((edges & Qt::BottomEdge) ? resize_edge_bottom : 0)
        | ((edges & Qt::LeftEdge) ? resize_edge_left : 0)
        | ((edges & Qt::RightEdge) ? resize_edge_right : 0));
}

Popup::Popup(UkuiShellSurface *ukuiShellSurface, QtWaylandClient::QWaylandWindow *parent, xdg_positioner *positioner)
    : m_ukuiShellSurface(ukuiShellSurface)
    , m_parentUkuiShellSurface(qobject_cast<UkuiShellSurface *>(parent->shellSurface()))
    , m_parent(parent)
{
    init(ukuiShellSurface->get_popup(m_parentUkuiShellSurface ? m_parentUkuiShellSurface->xdg_surface::object() : nullptr, positioner->object()));
    if (m_parent) {
        m_parent->addChildPopup(m_ukuiShellSurface->window());
    }
}

Popup::~Popup()
{
    if (isInitialized())
        destroy();

    if (m_parent) {
        m_parent->removeChildPopup(m_ukuiShellSurface->window());
    }

    if (m_grabbing) {
        auto *shell = m_ukuiShellSurface->shell();
        Q_ASSERT(shell->topmostGrabbingPopup() == this);
        shell->setTopmostGrabbingPopup(m_parentUkuiShellSurface ? m_parentUkuiShellSurface->popup() : nullptr);
        m_grabbing = false;

        // Synthesize Qt enter/leave events for popup
        QWindow *leave = nullptr;
        if (m_ukuiShellSurface && m_ukuiShellSurface->window())
            leave = m_ukuiShellSurface->window()->window();
        QWindowSystemInterface::handleLeaveEvent(leave);

        if (QWindow *enter = QGuiApplication::topLevelAt(QCursor::pos()))
            QWindowSystemInterface::handleEnterEvent(enter, enter->mapFromGlobal(QCursor::pos()), QCursor::pos());
    }
}

UkuiShellSurface *Popup::ukuiShellSurface()
{
    return m_ukuiShellSurface;
}

void Popup::grab(QtWaylandClient::QWaylandInputDevice *seat, uint serial)
{
    m_ukuiShellSurface->shell()->setTopmostGrabbingPopup(this);
    xdg_popup::grab(seat->wl_seat(), serial);
    m_grabbing = true;
}

void Popup::xdg_popup_popup_done()
{
    m_ukuiShellSurface->window()->window()->close();
}

} // namespace UKUI

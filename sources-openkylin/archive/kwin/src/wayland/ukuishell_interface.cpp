#include "ukuishell_interface.h"

#include "abstract_output.h"
#include "cursor.h"
#include "main.h"
#include "platform.h"

#include "display.h"
#include "qwayland-server-ukui-shell.h"
#include "seat_interface.h"
#include "wayland_server.h"

namespace KWaylandServer
{

class UkuiShellInterfacePrivate : public QtWaylandServer::ukui_shell
{
public:
    UkuiShellInterfacePrivate(UkuiShellInterface *q, Display *display);

    Resource *resourceForUkuiSurface(UkuiSurfaceInterface *surface) const;

    static UkuiShellInterfacePrivate *get(UkuiShellInterface *shell);

    Display *display;

protected:
    void ukui_shell_create_surface(Resource *resource, uint32_t id, struct ::wl_resource *surfaceResource) override;
    void ukui_shell_get_current_output(Resource *resource) override;

private:
    UkuiShellInterface *q;
};

class UkuiSurfaceInterfacePrivate : public QtWaylandServer::ukui_surface
{
public:
    UkuiSurfaceInterfacePrivate(UkuiSurfaceInterface *q, SurfaceInterface *surface, wl_resource *resource);

    UkuiSurfaceInterface *q;
    UkuiShellInterface *shell;
    QPointer<SurfaceInterface> surface;
    QPoint m_globalPos;
    bool m_positionSet = false;
    bool m_skipTaskbar = false;
    bool m_skipSwitcher = false;
    bool m_noTitlebar = false;
    bool m_openUnderCursorRequested = false;
    bool m_panelTakesFocus = false;
    bool m_panelHide = false;
    int m_windowRadius = -1;
    int m_borderWidth;
    int m_borderColor;
    int m_shadowRadius = -1;
    int m_shadowOffset;
    int m_shadowColor;
    UkuiSurfaceInterface::Role m_role = UkuiSurfaceInterface::Role::Normal;
    QString m_iconName;

    static UkuiSurfaceInterfacePrivate *get(UkuiSurfaceInterface *surface);

protected:
    void ukui_surface_destroy_resource(Resource *resource) override;
    void ukui_surface_destroy(Resource *resource) override;
    void ukui_surface_set_output(Resource *resource, struct ::wl_resource *output) override;
    void ukui_surface_set_position(Resource *resource, int32_t x, int32_t y) override;
    void ukui_surface_set_skip_taskbar(Resource *resource, uint32_t skip) override;
    void ukui_surface_set_skip_switcher(Resource *resource, uint32_t skip) override;
    void ukui_surface_set_property(Resource *resource, uint32_t property, uint32_t value) override;
    void ukui_surface_set_role(Resource *resource, uint32_t role) override;
    void ukui_surface_set_panel_auto_hide(Resource *resource, uint32_t hide) override;
    void ukui_surface_open_under_cursor(Resource *resource, int32_t x, int32_t y) override;
    void ukui_surface_set_panel_takes_focus(Resource *resource, uint32_t takes_focus) override;
    void ukui_surface_grab_keyboard(Resource *resource, struct ::wl_resource *seat) override;
    void ukui_surface_set_icon(Resource *resource, const QString &icon_name) override;
    void ukui_surface_activate(Resource *resource) override;
};

static const int s_version = 4;
static QList<UkuiSurfaceInterface *> s_surfaces;

/*********************************
 * UkuiShellInterface
 *********************************/
UkuiShellInterface::UkuiShellInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new UkuiShellInterfacePrivate(this, display))
{
}

UkuiShellInterface::~UkuiShellInterface() = default;

Display *UkuiShellInterface::display() const
{
    return d->display;
}

/*********************************
 * UkuiShellInterfacePrivate
 *********************************/
UkuiShellInterfacePrivate::UkuiShellInterfacePrivate(UkuiShellInterface *_q, Display *display)
    : QtWaylandServer::ukui_shell(*display, s_version)
    , q(_q)
{
}

UkuiShellInterfacePrivate *UkuiShellInterfacePrivate::get(UkuiShellInterface *shell)
{
    return shell->d.data();
}

static wl_client *clientFromUkuiSurface(UkuiSurfaceInterface *surface)
{
    return UkuiSurfaceInterfacePrivate::get(surface)->resource()->client();
}

UkuiShellInterfacePrivate::Resource *UkuiShellInterfacePrivate::resourceForUkuiSurface(UkuiSurfaceInterface *surface) const
{
    return resourceMap().value(clientFromUkuiSurface(surface));
}

void UkuiShellInterfacePrivate::ukui_shell_create_surface(QtWaylandServer::ukui_shell::Resource *resource,
                                                          uint32_t id,
                                                          struct ::wl_resource *surfaceResource)
{
    SurfaceInterface *surface = SurfaceInterface::get(surfaceResource);
    if (!surface) {
        wl_resource_post_error(resource->handle, 0, "Invalid  surface");
        return;
    }

    // if (UkuiSurfaceInterface::get(surface)) {
    // wl_resource_post_error(resource->handle, 0, "ukui_surface already exists");
    //     return;
    // }

    wl_resource *ukuiSurfaceResource = wl_resource_create(resource->client(), &ukui_surface_interface, resource->version(), id);

    auto ukuiSurface = new UkuiSurfaceInterface(surface, ukuiSurfaceResource);
    s_surfaces.append(ukuiSurface);

    QObject::connect(ukuiSurface, &QObject::destroyed, [ukuiSurface]() {
        s_surfaces.removeOne(ukuiSurface);
    });

    Q_EMIT q->surfaceCreated(ukuiSurface);
}

void UkuiShellInterfacePrivate::ukui_shell_get_current_output(Resource *resource)
{
    // TODO：get the ouput under the cursor
    QPoint pos = KWin::Cursors::self()->currentCursor()->pos();
    KWin::AbstractOutput *output = KWin::kwinApp()->platform()->outputAt(pos);
    if (!output)
        output = KWin::kwinApp()->platform()->primaryOutput();
    Q_ASSERT(output);

    send_current_output(resource->handle, output->name().toUtf8().constData(), KWin::waylandServer()->seat()->name());
    send_done(resource->handle);
}

/*********************************
 * UkuiSurfaceInterface
 *********************************/
UkuiSurfaceInterface::UkuiSurfaceInterface(SurfaceInterface *surface, wl_resource *resource)
    : d(new UkuiSurfaceInterfacePrivate(this, surface, resource))
{
}

UkuiSurfaceInterface::~UkuiSurfaceInterface() = default;

SurfaceInterface *UkuiSurfaceInterface::surface() const
{
    return d->surface;
}

QPoint UkuiSurfaceInterface::position() const
{
    return d->m_globalPos;
}

UkuiSurfaceInterface::Role UkuiSurfaceInterface::role() const
{
    return d->m_role;
}

bool UkuiSurfaceInterface::isPositionSet() const
{
    return d->m_positionSet;
}

bool UkuiSurfaceInterface::skipTaskbar() const
{
    return d->m_skipTaskbar;
}

bool UkuiSurfaceInterface::skipSwitcher() const
{
    return d->m_skipSwitcher;
}

bool UkuiSurfaceInterface::wantsOpenUnderCursor() const
{
    return d->m_openUnderCursorRequested;
}

bool UkuiSurfaceInterface::noTitlebar() const
{
    return d->m_noTitlebar;
}

int UkuiSurfaceInterface::windowRadius() const
{
    return d->m_windowRadius;
}

int UkuiSurfaceInterface::shadowRadius() const
{
    return d->m_shadowRadius;
}

bool UkuiSurfaceInterface::panelTakesFocus() const
{
    return d->m_panelTakesFocus;
}

bool UkuiSurfaceInterface::panelHide() const
{
    return d->m_panelHide;
}

QString UkuiSurfaceInterface::iconName() const
{
    return d->m_iconName;
}

void UkuiSurfaceInterface::sendPosition(const QPoint &position)
{
    if (d->m_globalPos == position) {
        return;
    }

    d->m_globalPos = position;
    d->send_position(position.x(), position.y());
}

UkuiSurfaceInterface *UkuiSurfaceInterface::get(SurfaceInterface *surface)
{
    for (UkuiSurfaceInterface *shellSurface : qAsConst(s_surfaces)) {
        if (shellSurface->surface() == surface) {
            return shellSurface;
        }
    }
    return nullptr;
}

/*********************************
 * UkuiSurfaceInterfacePrivate
 *********************************/
UkuiSurfaceInterfacePrivate::UkuiSurfaceInterfacePrivate(UkuiSurfaceInterface *_q, SurfaceInterface *surface, wl_resource *resource)
    : QtWaylandServer::ukui_surface(resource)
    , q(_q)
    , surface(surface)
{
}

UkuiSurfaceInterfacePrivate *UkuiSurfaceInterfacePrivate::get(UkuiSurfaceInterface *surface)
{
    return surface->d.data();
}

void UkuiSurfaceInterfacePrivate::ukui_surface_destroy_resource(Resource *resource)
{
    delete q;
}

void UkuiSurfaceInterfacePrivate::ukui_surface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void UkuiSurfaceInterfacePrivate::ukui_surface_set_output(Resource *resource, struct ::wl_resource *output)
{
    Q_EMIT q->outputChanged(KWaylandServer::OutputInterface::get(output));
}

void UkuiSurfaceInterfacePrivate::ukui_surface_set_position(Resource *resource, int32_t x, int32_t y)
{

    QPoint globalPos(x, y);
    m_positionSet = true;
    m_globalPos = globalPos;
    Q_EMIT q->positionChanged();
}

void UkuiSurfaceInterfacePrivate::ukui_surface_set_skip_taskbar(Resource *resource, uint32_t skip)
{
    m_skipTaskbar = (bool)skip;
    Q_EMIT q->skipTaskbarChanged();
}

void UkuiSurfaceInterfacePrivate::ukui_surface_set_skip_switcher(Resource *resource, uint32_t skip)
{

    m_skipSwitcher = (bool)skip;
    Q_EMIT q->skipSwitcherChanged();
}

void UkuiSurfaceInterfacePrivate::ukui_surface_set_property(Resource *resource, uint32_t property, uint32_t value)
{
    switch (property) {
    case property_no_titlebar:
        m_noTitlebar = value == 1 ? true : false;
        Q_EMIT q->noTitlebarChanged();
        break;
    case property_theme:
        break;
    case property_window_radius:
        m_windowRadius = value;
        Q_EMIT q->windowRadiusChanged();
        break;
    case property_border_width:
        break;
    case property_border_color:
        break;
    case property_shadow_radius:
        m_shadowRadius = value;
        Q_EMIT q->shadowRadiusChanged();
        break;
    case property_shadow_offset:
        break;
    case property_shadow_color:
        break;
    default:
        break;
    }
}

void UkuiSurfaceInterfacePrivate::ukui_surface_set_role(Resource *resource, uint32_t role)
{
    UkuiSurfaceInterface::Role r = UkuiSurfaceInterface::Role::Normal;
    switch (role) {
    case role_desktop:
        r = UkuiSurfaceInterface::Role::Desktop;
        break;
    case role_panel:
        r = UkuiSurfaceInterface::Role::Panel;
        break;
    case role_onscreendisplay:
        r = UkuiSurfaceInterface::Role::OnScreenDisplay;
        break;
    case role_notification:
        r = UkuiSurfaceInterface::Role::Notification;
        break;
    case role_tooltip:
        r = UkuiSurfaceInterface::Role::ToolTip;
        break;
    case role_criticalnotification:
        r = UkuiSurfaceInterface::Role::CriticalNotification;
        break;
    case role_appletpopup:
        r = UkuiSurfaceInterface::Role::AppletPopup;
        break;
    case role_screenlock:
        r = UkuiSurfaceInterface::Role::ScreenLock;
        break;
    case role_watermark:
        r = UkuiSurfaceInterface::Role::Watermark;
        break;
    case role_systemwindow:
        r = UkuiSurfaceInterface::Role::SystemWindow;
        break;
    case role_inputpanel:
        r = UkuiSurfaceInterface::Role::InputPanel;
        break;
    case role_logout:
        r = UkuiSurfaceInterface::Role::Logout;
        break;
    case role_screenlocknotification:
        r = UkuiSurfaceInterface::Role::ScreenLockNotification;
        break;
    case role_normal:
    default:
        r = UkuiSurfaceInterface::Role::Normal;
        break;
    }
    if (r == m_role) {
        return;
    }
    m_role = r;
    Q_EMIT q->roleChanged();
}
void UkuiSurfaceInterfacePrivate::ukui_surface_set_panel_auto_hide(Resource *resource, uint32_t hide)
{
    if (m_panelHide == hide) {
        return;
    }
    m_panelHide = hide;
    Q_EMIT q->panelHideChanged();
}

void UkuiSurfaceInterfacePrivate::ukui_surface_open_under_cursor(Resource *resource, int32_t x, int32_t y)
{
    if (surface && surface->buffer()) {
        wl_resource_post_error(resource->handle, -1, "open_under_cursor: surface has a buffer");
    }
    m_openUnderCursorRequested = true;
    Q_EMIT q->openUnderCursorRequested();
}

void UkuiSurfaceInterfacePrivate::ukui_surface_set_panel_takes_focus(Resource *resource, uint32_t takes_focus)
{
    if (m_panelTakesFocus == takes_focus) {
        return;
    }
    m_panelTakesFocus = takes_focus;
    Q_EMIT q->panelTakesFocusChanged();
}

void UkuiSurfaceInterfacePrivate::ukui_surface_grab_keyboard(Resource *resource, struct ::wl_resource *seat)
{
    // It is used by the switcher to grab the keyboard in wlcom, we don't need it
}

void UkuiSurfaceInterfacePrivate::ukui_surface_set_icon(Resource *resource, const QString &icon_name)
{
    if (m_iconName == icon_name) {
        return;
    }
    m_iconName = icon_name;
    Q_EMIT q->iconNameChanged();
}

void UkuiSurfaceInterfacePrivate::ukui_surface_activate(Resource *resource)
{
    Q_EMIT q->activatedRequested();
}

} // namespace KWaylandServer
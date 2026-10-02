/*
 * libkysdk-waylandhelper's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#include "ukuishell.h"
#include "ukuiwaylandpointer.h"

class Q_DECL_HIDDEN UkuiShell::Private
{
public:
    Private(UkuiShell *q);

    UkuiWaylandPointer<ukui_shell, ukui_shell_destroy> shell;
    EventQueue *queue = nullptr;
    UkuiShell *q;
    QString m_seatName;
    QString m_outputName;
    bool m_currentOutputReady = false;
    void init_listener();

    static struct ukui_shell_listener s_listener;
    static void currentOutputCallback(void *data, struct ukui_shell *ukui_shell, const char *output_name, const char *seat_name);
    static void doneCallback(void *data, struct ukui_shell *ukui_shell);
};

ukui_shell_listener UkuiShell::Private::s_listener = {
    .current_output = currentOutputCallback,
    .done = doneCallback};

class Q_DECL_HIDDEN UkuiShellSurface::Private
{
public:
    Private(UkuiShellSurface *q);
    ~Private();
    void setup(ukui_surface *surface);

    UkuiWaylandPointer<ukui_surface, ukui_surface_destroy> surface;
    QSize size;
    QPointer<Surface> parentSurface;
    UkuiShellSurface::Role role;
    UkuiShellSurface::SurfaceProperty property;

    static UkuiShellSurface *get(Surface *surface);

private:
    UkuiShellSurface *q;
    static QVector<Private *> s_surfaces;
};

QVector<UkuiShellSurface::Private *> UkuiShellSurface::Private::s_surfaces;

UkuiShell::UkuiShell(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

UkuiShell::~UkuiShell()
{
    release();
}

void UkuiShell::destroy()
{
    if (!d->shell) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->shell.destroy();
}

void UkuiShell::release()
{
    if (!d->shell) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->shell.release();
}

void UkuiShell::setup(ukui_shell *shell)
{
    Q_ASSERT(!d->shell);
    Q_ASSERT(shell);
    d->shell.setup(shell);
    d->init_listener();
}

void UkuiShell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *UkuiShell::eventQueue()
{
    return d->queue;
}

QString UkuiShell::seatName()
{
    if (d->m_currentOutputReady)
        return d->m_seatName;
    else
        return QString();
}

QString UkuiShell::outputName()
{
    if (d->m_currentOutputReady)
        return d->m_outputName;
    else
        return QString();
}

bool UkuiShell::isCurrentOutputReady()
{
    return d->m_currentOutputReady;
}

UkuiShellSurface *UkuiShell::createSurface(wl_surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto kwS = Surface::get(surface);
    if (kwS) {
        if (auto s = UkuiShellSurface::Private::get(kwS)) {
            return s;
        }
    }
    UkuiShellSurface *s = new UkuiShellSurface(parent);
    connect(this, &UkuiShell::interfaceAboutToBeReleased, s, &UkuiShellSurface::release);
    connect(this, &UkuiShell::interfaceAboutToBeDestroyed, s, &UkuiShellSurface::destroy);
    auto w = ukui_shell_create_surface(d->shell, surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    s->d->parentSurface = QPointer<Surface>(kwS);
    return s;
}

UkuiShellSurface *UkuiShell::createSurface(Surface *surface, QObject *parent)
{
    return createSurface(*surface, parent);
}

void UkuiShell::updateCurrentOutput()
{
    ukui_shell_get_current_output(d->shell);
}

bool UkuiShell::isValid() const
{
    return d->shell.isValid();
}

UkuiShell::operator ukui_shell *()
{
    return d->shell;
}

UkuiShell::operator ukui_shell *() const
{
    return d->shell;
}

UkuiShellSurface::Private::Private(UkuiShellSurface *q)
    : role(UkuiShellSurface::Role::Normal)
    , q(q)
{
    s_surfaces << this;
}

UkuiShellSurface::Private::~Private()
{
    s_surfaces.removeAll(this);
}

UkuiShellSurface *UkuiShellSurface::Private::get(Surface *surface)
{
    if (!surface) {
        return nullptr;
    }
    for (auto it = s_surfaces.constBegin(); it != s_surfaces.constEnd(); ++it) {
        if ((*it)->parentSurface == surface) {
            return (*it)->q;
        }
    }
    return nullptr;
}

void UkuiShellSurface::Private::setup(ukui_surface *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!surface);
    surface.setup(s);
}

UkuiShellSurface::UkuiShellSurface(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

UkuiShellSurface::~UkuiShellSurface()
{
    release();
}

void UkuiShellSurface::release()
{
    d->surface.release();
}

void UkuiShellSurface::destroy()
{
    d->surface.destroy();
}

void UkuiShellSurface::setup(ukui_surface *surface)
{
    d->setup(surface);
}

UkuiShellSurface *UkuiShellSurface::get(Surface *surface)
{
    if (auto s = UkuiShellSurface::Private::get(surface)) {
        return s;
    }

    return nullptr;
}

bool UkuiShellSurface::isValid() const
{
    return d->surface.isValid();
}

UkuiShellSurface::operator ukui_surface *()
{
    return d->surface;
}

UkuiShellSurface::operator ukui_surface *() const
{
    return d->surface;
}

void UkuiShellSurface::setPosition(const QPoint &point)
{
    Q_ASSERT(isValid());
    ukui_surface_set_position(d->surface, point.x(), point.y());
}

void UkuiShellSurface::setRole(UkuiShellSurface::Role role)
{
    Q_ASSERT(isValid());
    uint32_t wlRole = UKUI_SURFACE_ROLE_NORMAL;
    switch (role) {
    case Role::Normal:
        wlRole = UKUI_SURFACE_ROLE_NORMAL;
        break;
    case Role::Desktop:
        wlRole = UKUI_SURFACE_ROLE_DESKTOP;
        break;
    case Role::Panel:
        wlRole = UKUI_SURFACE_ROLE_PANEL;
        break;
    case Role::OnScreenDisplay:
        wlRole = UKUI_SURFACE_ROLE_ONSCREENDISPLAY;
        break;
    case Role::Notification:
        wlRole = UKUI_SURFACE_ROLE_NOTIFICATION;
        break;
    case Role::ToolTip:
        wlRole = UKUI_SURFACE_ROLE_TOOLTIP;
        break;
    case Role::CriticalNotification:
        wlRole = UKUI_SURFACE_ROLE_CRITICALNOTIFICATION;
        break;
    case Role::AppletPop:
        wlRole = UKUI_SURFACE_ROLE_APPLETPOPUP;
        break;
    case Role::ScreenLock:
        wlRole = UKUI_SURFACE_ROLE_SCREENLOCK;
        break;
    case Role::Watermark:
        wlRole = UKUI_SURFACE_ROLE_WATERMARK;
        break;
    case Role::SystemWindow:
        wlRole = UKUI_SURFACE_ROLE_SYSTEMWINDOW;
        break;
    case Role::InputPanel:
        wlRole = UKUI_SURFACE_ROLE_INPUTPANEL;
        break;
    case Role::Logout:
        wlRole = UKUI_SURFACE_ROLE_LOGOUT;
        break;
    case Role::ScreenLockNotification:
        wlRole = UKUI_SURFACE_ROLE_SCREENLOCKNOTIFICATION;
        break;
    case Role::Switcher:
        wlRole = UKUI_SURFACE_ROLE_SWITCHER;
        break;
    case Role::Authentication:
        wlRole = UKUI_SURFACE_ROLE_AUTHENTICATION;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    ukui_surface_set_role(d->surface, wlRole);
    d->role = role;
}

UkuiShellSurface::Role UkuiShellSurface::role() const
{
    return d->role;
}

void UkuiShellSurface::setSurfaceProperty(SurfaceProperty property, uint32_t value)
{
    uint32_t wlPropery = -1;
    switch (property) {
    case SurfaceProperty::NoTitleBar:
        wlPropery = UKUI_SURFACE_PROPERTY_NO_TITLEBAR;
        break;
    case SurfaceProperty::Theme:
        wlPropery = UKUI_SURFACE_PROPERTY_THEME;
        break;
    case SurfaceProperty::WindowRadius:
        wlPropery = UKUI_SURFACE_PROPERTY_WINDOW_RADIUS;
        break;
    case SurfaceProperty::BorderWidth:
        wlPropery = UKUI_SURFACE_PROPERTY_BORDER_WIDTH;
        break;
    case SurfaceProperty::BorderColor:
        wlPropery = UKUI_SURFACE_PROPERTY_BORDER_COLOR;
        break;
    case SurfaceProperty::ShadowRadius:
        wlPropery = UKUI_SURFACE_PROPERTY_SHADOW_RADIUS;
        break;
    case SurfaceProperty::ShadowOffset:
        wlPropery = UKUI_SURFACE_PROPERTY_SHADOW_OFFSET;
        break;
    case SurfaceProperty::ShadowColor:
        wlPropery = UKUI_SURFACE_PROPERTY_SHADOW_COLOR;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    ukui_surface_set_property(d->surface, wlPropery, value);
    d->property = property;
}

UkuiShellSurface::SurfaceProperty UkuiShellSurface::surfaceProperty() const
{
    return d->property;
}

void UkuiShellSurface::setSkipTaskbar(bool skip)
{
    ukui_surface_set_skip_taskbar(d->surface, skip);
}

void UkuiShellSurface::setSkipSwitcher(bool skip)
{
    ukui_surface_set_skip_switcher(d->surface, skip);
}

void UkuiShellSurface::setPanelTakesFocus(bool takesFocus)
{
    ukui_surface_set_panel_takes_focus(d->surface, takesFocus);
}

void UkuiShellSurface::setPanelAutoHide(bool autoHide)
{
    ukui_surface_set_panel_auto_hide(d->surface, autoHide);
}

void UkuiShellSurface::setGrabKeyboard(wl_seat *seat)
{
    ukui_surface_grab_keyboard(d->surface, seat);
}

void UkuiShellSurface::setOpenUnderCursor()
{
    ukui_surface_open_under_cursor(d->surface, 0, 0);
}

void UkuiShellSurface::setOpenUnderCursor(int x, int y)
{
    ukui_surface_open_under_cursor(d->surface, x, y);
}

void UkuiShellSurface::setIconName(const QString &iconName)
{
    if (iconName.isEmpty()) {
        ukui_surface_set_icon(d->surface, NULL);
    } else {
        ukui_surface_set_icon(d->surface, iconName.toStdString().c_str());
    }
}

UkuiShell::Private::Private(UkuiShell *q)
    : q(q)
{
}

void UkuiShell::Private::init_listener()
{
    ukui_shell_add_listener(shell, &s_listener, this);
}

void UkuiShell::Private::currentOutputCallback(void *data, ukui_shell *ukui_shell, const char *output_name, const char *seat_name)
{
    auto ukuiShell = reinterpret_cast<UkuiShell::Private *>(data);
    if (ukuiShell->shell != ukui_shell)
        return;
    ukuiShell->m_outputName = QString::fromUtf8(output_name);
    ukuiShell->m_seatName = QString::fromUtf8(seat_name);
}

void UkuiShell::Private::doneCallback(void *data, ukui_shell *ukui_shell)
{
    auto ukuiShell = reinterpret_cast<UkuiShell::Private *>(data);
    if (ukuiShell->shell != ukui_shell)
        return;
    ukuiShell->m_currentOutputReady = true;
    Q_EMIT ukuiShell->q->currentOutputReady();
}

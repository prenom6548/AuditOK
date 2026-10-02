/*
    SPDX-FileCopyrightText: 2024 Teng Wang <wangteng@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "kwin_export.h"

#include <QObject>
#include <QSharedDataPointer>

#include "surface_interface.h"

struct wl_resource;

namespace KWaylandServer
{
class Display;
class SurfaceInterface;
class UkuiShellInterfacePrivate;
class UkuiShellInterface;
class UkuiSurfaceInterfacePrivate;
class UkuiSurfaceInterface;

/**
 * The UkuiShellInterface class represents an extension for destrop-style user interfaces.
 *
 * The UkuiShellInterface class provides a way for a client to extend a regular Wayland surface
 * with functionality required to construct user interface elements, e.g. toplevel windows or
 * menus.
 *
 * UkuiShellInterface corresponds to the WaylandInterface \c ukui_shell.
 */
class KWIN_EXPORT UkuiShellInterface : public QObject
{
    Q_OBJECT

public:
    explicit UkuiShellInterface(Display *display, QObject *parent);
    /**
     * Destructs the UkuiShellInterface object.
     */
    ~UkuiShellInterface() override;

    /**
     * Returns the wayland display of the UkuiShellInterface.
     */
    Display *display() const;

Q_SIGNALS:
    /**
     * This signal is emitted when a new UkuiSurfaceInterface object is created.
     */
    void surfaceCreated(UkuiSurfaceInterface *surface);

private:
    QScopedPointer<UkuiShellInterfacePrivate> d;
    friend class UkuiShellInterfacePrivate;
};

/**
 * The UkuiSurfaceInterface class provides a base set of functionality required to construct
 * user interface elements.
 *
 * UkuiSurfaceInterface corresponds to the Wayland interface \c ukui_surface.
 */
class KWIN_EXPORT UkuiSurfaceInterface : public QObject
{
    Q_OBJECT

public:
    explicit UkuiSurfaceInterface(SurfaceInterface *surface, wl_resource *resource);
    /**
     * Destructs the UkuiSurfaceInterface object.
     */
    ~UkuiSurfaceInterface() override;
    /**
     * Returns the SurfaceInterface assigned to this UkuiSurfaceInterface.
     */
    SurfaceInterface *surface() const;
    /**
     * @returns the requested position in global coordinates.
     */
    QPoint position() const;
    /**
     * @returns Whether a global position has been requested.
     */
    bool isPositionSet() const;
    /**
     * @returns Whether the surface has requested to be opened under the cursor.
     */
    bool wantsOpenUnderCursor() const;
    /**
     * Describes possible roles this UkuiSurfaceInterface can have.
     * The role can be used by the server to e.g. change the stacking order accordingly.
     */
    enum class Role {
        Normal, // A normal Surface
        Desktop, // The Surface represents a desktop, normally stacked below all other surfaces
        Panel, // The Surface represents a panel (dock), normally stacked above normal surfaces
        OnScreenDisplay, // The Surface represents an on screen display, like a volume changed notification
        Notification, // The Surface represents a notification
        ToolTip, // The Surface represents a tooltip
        CriticalNotification, // The Surface represents a critical notification, like battery is running out
        AppletPopup, // The Surface used for applets
        ScreenLock,
        Watermark,
        SystemWindow,
        InputPanel,
        Logout,
        ScreenLockNotification
    };

    /**
     * @returns The requested role, default value is @c Role::Normal.
     */
    Role role() const;
    /**
     * @returns true if this window doesn't want to be listed
     * in the taskbar
     */
    bool skipTaskbar() const;
    /**
     * @returns true if this window doesn't want to be listed
     * in a window switcher
     */
    bool skipSwitcher() const;

    bool noTitlebar() const;

    int windowRadius() const;

    int shadowRadius() const;

    /**
     * Whether a UkuiSurfaceInterface wants to have focus.
     *
     * By default some Ukui roles do not get focus, but the UkuiSurfaceInterface can
     * request that it wants to have focus. The compositor can use this information to
     * pass focus to the surface.
     */
    bool panelTakesFocus() const;

    bool panelHide() const;

    QString iconName() const;

    void sendPosition(const QPoint &position);

    static UkuiSurfaceInterface *get(SurfaceInterface *surface);

Q_SIGNALS:
    void aboutToBeDestroyed();
    void outputChanged(KWaylandServer::OutputInterface *output);
    void positionChanged();
    void skipTaskbarChanged();
    void skipSwitcherChanged();
    void openUnderCursorRequested();
    void noTitlebarChanged();
    void windowRadiusChanged();
    void shadowRadiusChanged();
    void propertyChanged(uint32_t property, uint32_t value);
    void roleChanged();
    void openUnderCursorChanged();
    void panelTakesFocusChanged();
    void panelHideChanged();
    void iconNameChanged();
    void activatedRequested();

private:
    QScopedPointer<UkuiSurfaceInterfacePrivate> d;
    friend class UkuiSurfaceInterfacePrivate;
};

} // namespace KWaylandServer

Q_DECLARE_METATYPE(KWaylandServer::UkuiSurfaceInterface::Role)
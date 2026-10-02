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

#ifndef UKUISHELL_H
#define UKUISHELL_H

#include "ukui-shell-client-protocol.h"
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/surface.h>
#include <QObject>
#include <QPointer>
#include <QSize>

/**
 * @since 2.5.1.0
 */

struct wl_surface;
struct ukui_surface;
struct wl_seat;

class UkuiShellSurface;

using namespace KWayland::Client;

class UkuiShell : public QObject
{
    Q_OBJECT
public:
    explicit UkuiShell(QObject *parent = nullptr);
    ~UkuiShell() override;

    bool isValid() const;

    void release();

    void destroy();

    void setup(ukui_shell *shell);

    void setEventQueue(EventQueue *queue);

    EventQueue *eventQueue();

    QString seatName();

    QString outputName();

    bool isCurrentOutputReady();

    UkuiShellSurface *createSurface(wl_surface *surface, QObject *parent = nullptr);

    UkuiShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    void updateCurrentOutput();

    operator ukui_shell *();
    operator ukui_shell *() const;

Q_SIGNALS:

    void interfaceAboutToBeReleased();

    void interfaceAboutToBeDestroyed();

    void removed();

    void currentOutputReady();

private:
    class Private;
    QScopedPointer<Private> d;
};

class UkuiShellSurface : public QObject
{
    Q_OBJECT
public:
    explicit UkuiShellSurface(QObject *parent);
    ~UkuiShellSurface() override;

    void release();

    void destroy();

    void setup(ukui_surface *surface);

    static UkuiShellSurface *get(Surface *surf);

    bool isValid() const;

    operator ukui_surface *();
    operator ukui_surface *() const;

    enum class Role {
        Normal,
        Desktop,
        Panel,
        OnScreenDisplay,
        Notification,
        ToolTip,
        CriticalNotification,
        AppletPop,
        ScreenLock,
        Watermark,
        SystemWindow,
        InputPanel,
        Logout,
        ScreenLockNotification,
        Switcher,
        Authentication,
    };

    enum class SurfaceProperty {
        NoTitleBar,
        Theme,
        WindowRadius,
        BorderWidth,
        BorderColor,
        ShadowRadius,
        ShadowOffset,
        ShadowColor
    };

    void setRole(Role role);

    Role role() const;

    void setSurfaceProperty(SurfaceProperty property, uint32_t value);

    SurfaceProperty surfaceProperty() const;

    void setPosition(const QPoint &point);

    void setSkipTaskbar(bool skip);

    void setSkipSwitcher(bool skip);

    void setPanelTakesFocus(bool takesFocus);

    void setPanelAutoHide(bool autoHide);

    void setGrabKeyboard(wl_seat *seat);

    void setOpenUnderCursor();

    void setOpenUnderCursor(int x, int y);

    void setIconName(const QString &iconName);

Q_SIGNALS:

    void autoHidePanelHidden();

    void autoHidePanelShown();

private:
    friend class UkuiShell;
    class Private;
    QScopedPointer<Private> d;
};

#endif // UKUISHELL_H

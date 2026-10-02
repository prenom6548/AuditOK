/*
*  * Copyright (C) 2025, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef UKUI_SHELL_V1_H
#define UKUI_SHELL_V1_H

#include "wayland-ukui-shell-client-protocol.h"
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/surface.h>
#include <QObject>
#include <QPointer>
#include "window-type.h"

struct wl_surface;
struct ukui_surface;
struct wl_seat;

using namespace KWayland::Client;

namespace UkuiQuick::WaylandClient {
class UkuiShellSurface;
class UkuiShell : public QObject
{
    Q_OBJECT
public:
    explicit UkuiShell(QObject *parent = nullptr);
    ~UkuiShell() override;

    bool isValid() const;

    void release();

    void destroy();

    void setup(ukui_shell_v1 *shell);

    void setEventQueue(EventQueue *queue);

    EventQueue *eventQueue();

    QString seatName();

    QString outputName();

    bool isCurrentOutputReady();

    UkuiShellSurface *createSurface(wl_surface *surface, QObject *parent = nullptr);

    UkuiShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    void updateCurrentOutput();

    operator ukui_shell_v1 *();
    operator ukui_shell_v1 *() const;

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
    explicit UkuiShellSurface(ukui_decoration_v1 *decoration, QObject *parent);
    ~UkuiShellSurface() override;

    void release();

    void destroy();

    void setup(ukui_surface_v1 *surface);

    static UkuiShellSurface *get(Surface *surf);

    bool isValid() const;

    operator ukui_surface_v1 *();
    operator ukui_surface_v1 *() const;

    enum Role {
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

    void setRole(Role role);

    Role role() const;

    void setPosition(const QPoint &point);

    void setSkipTaskbar(bool skip);

    void setSkipSwitcher(bool skip);

    void setPanelAutoHide(bool autoHide);

    void setGrabKeyboard(wl_seat *seat);

    void setOpenUnderCursor();

    void setOpenUnderCursor(int x, int y);

    void setIconName(const QString &iconName);

    void setStates(WindowState::States states, bool enable);

    void setPanelTakesFocus(bool takesFocus);

    void setRemoveTitleBar(bool remove);

    void setDecorationComponents(Decoration::Components components);

    void setRestoreGeometry(const QRect& restoreGeometry);

    Q_SIGNALS:

    void autoHidePanelHidden();

    void autoHidePanelShown();

private:
    friend class UkuiShell;
    class Private;
    QScopedPointer<Private> d;
};
}

#endif // UKUI_SHELL_V1_H

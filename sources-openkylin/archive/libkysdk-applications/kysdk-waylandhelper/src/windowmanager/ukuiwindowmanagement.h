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

#ifndef UKUIWINDOWMANAGEMENT_H
#define UKUIWINDOWMANAGEMENT_H

#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/output.h>
#include <KWayland/Client/surface.h>
#include <QIcon>
#include <QObject>
#include <QSize>
#include <QVector>

struct ukui_activation;
struct ukui_activation_feedback;
struct ukui_window;
struct ukui_window_management;

class UkuiWindow;

using namespace KWayland::Client;

/**
 * @since 2.5.1.0
 */

class UkuiWindowManagement : public QObject
{
    Q_OBJECT
public:
    explicit UkuiWindowManagement(QObject *parent = nullptr);
    ~UkuiWindowManagement() override;

    bool isValid() const;

    void release();

    void destroy();

    void setup(ukui_window_management *wm);

    void setEventQueue(EventQueue *queue);

    EventQueue *eventQueue();

    operator ukui_window_management *();
    operator ukui_window_management *() const;

    bool isShowingDesktop() const;

    void setShowingDesktop(bool show);

    void showDesktop();

    void hideDesktop();

    QList<UkuiWindow *> windows() const;

    UkuiWindow *activeWindow() const;

    QVector<quint32> stackingOrder() const;

    QVector<QByteArray> stackingOrderUuids() const;

Q_SIGNALS:

    void interfaceAboutToBeReleased();

    void interfaceAboutToBeDestroyed();

    void showingDesktopChanged(bool);

    void windowCreated(UkuiWindow *window);

    void activeWindowChanged();

    void removed();

    void stackingOrderChanged();

    void stackingOrderUuidsChanged();

public:
    class Private;

private:
    QScopedPointer<Private> d;
};

class UkuiWindow : public QObject
{
    Q_OBJECT
public:
    ~UkuiWindow() override;

    void release();

    void destroy();

    bool isValid() const;

    operator ukui_window *();
    operator ukui_window *() const;

    QString title() const;

    QString appId() const;

    quint32 virtualDesktop() const;

    bool isActive() const;

    bool isFullscreen() const;

    bool isKeepAbove() const;

    bool isKeepBelow() const;

    bool isMinimized() const;

    bool isMaximized() const;

    bool isOnAllDesktops() const;

    bool isDemandingAttention() const;

    bool isCloseable() const;

    bool isMaximizeable() const;

    bool isMinimizeable() const;

    bool isFullscreenable() const;

    bool skipTaskbar() const;

    bool skipSwitcher() const;

    QIcon icon() const;

    bool isShadeable() const;

    bool isShaded() const;

    bool isMovable() const;

    bool isResizable() const;

    bool isVirtualDesktopChangeable() const;

    quint32 pid() const;

    void requestActivate();

    void requestClose();

    void requestMove();

    void requestResize();

    void requestVirtualDesktop(quint32 desktop);

    void requestToggleKeepAbove();

    void requestDemandAttention();

    void requestToggleKeepBelow();

    void requestToggleMinimized();

    void requestToggleMaximized();

    void setStartupGeometry(Surface *surface, const QRect &geometry);

    void setMinimizedGeometry(Surface *panel, const QRect &geom);

    void unsetMinimizedGeometry(Surface *panel);

    void requestToggleShaded();

    quint32 internalId() const;

    QByteArray uuid() const;

    QPointer<UkuiWindow> parentWindow() const;

    QRect geometry() const;

    void requestEnterVirtualDesktop(const QString &id);

    void requestEnterNewVirtualDesktop();

    void requestLeaveVirtualDesktop(const QString &id);

    QStringList ukuiVirtualDesktops() const;

    void requestEnterActivity(const QString &id);

    void requestLeaveActivity(const QString &id);

    QStringList ukuiActivities() const;

    QString applicationMenuServiceName() const;

    QString applicationMenuObjectPath() const;

    void sendToOutput(Output *output) const;

    void setHighlight();

    void unsetHightlight();

    bool isHighlight();

Q_SIGNALS:

    void titleChanged();

    void appIdChanged();

    void virtualDesktopChanged();

    void activeChanged();

    void fullscreenChanged();

    void keepAboveChanged();

    void keepBelowChanged();

    void minimizedChanged();

    void maximizedChanged();

    void onAllDesktopsChanged();

    void demandsAttentionChanged();

    void closeableChanged();

    void minimizeableChanged();

    void maximizeableChanged();

    void fullscreenableChanged();

    void skipTaskbarChanged();

    void skipSwitcherChanged();

    void iconChanged();

    void shadeableChanged();

    void shadedChanged();

    void movableChanged();

    void resizableChanged();

    void modalityChanged();

    void acceptFocusChanged();

    void virtualDesktopChangeableChanged();

    void unmapped();

    void parentWindowChanged();

    void geometryChanged();

    void ukuiVirtualDesktopEntered(const QString &id);

    void ukuiVirtualDesktopLeft(const QString &id);

    void ukuiActivityEntered(const QString &id);

    void ukuiActivityLeft(const QString &id);

    void applicationMenuChanged();

private:
    friend class UkuiWindowManagement;
    explicit UkuiWindow(UkuiWindowManagement *parent, ukui_window *activation, quint32 internalId, const char *uuid);
    class Private;
    QScopedPointer<Private> d;
};

Q_DECLARE_METATYPE(UkuiWindow *)

#endif // UKUIWINDOWMANAGEMENT_H

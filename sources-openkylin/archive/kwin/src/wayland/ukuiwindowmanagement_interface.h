/*
    SPDX-FileCopyrightText: 2024 Teng Wang <wangteng@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

#include <QObject>

#include "kwin_export.h"

class QSize;

namespace KWaylandServer
{
class Display;
class OutputInterface;
class UkuiWindowActivationFeedbackInterfacePrivate;
class UkuiWindowActivationInterfacePrivate;
class UkuiWindowInterface;
class UkuiWindowInterfacePrivate;
class UkuiWindowManagementInterfacePrivate;
class PlasmaVirtualDesktopManagementInterface;
class SurfaceInterface;

class KWIN_EXPORT UkuiWindowActivationInterface
{
public:
    ~UkuiWindowActivationInterface();

    void sendAppId(const QString &id);

private:
    friend class UkuiWindowActivationFeedbackInterface;
    explicit UkuiWindowActivationInterface();

    QScopedPointer<UkuiWindowActivationInterfacePrivate> d;
};

class KWIN_EXPORT UkuiWindowActivationFeedbackInterface : public QObject
{
    Q_OBJECT

public:
    explicit UkuiWindowActivationFeedbackInterface(Display *display, QObject *parent = nullptr);
    ~UkuiWindowActivationFeedbackInterface() override;

    /**
     * Notify about a new application with @p app_id being started
     *
     * @returns an instance of @class UkuiWindowActivationInterface to
     * be destroyed as the activation process ends.
     */
    UkuiWindowActivationInterface *createActivation(const QString &app_id);

private:
    QScopedPointer<UkuiWindowActivationFeedbackInterfacePrivate> d;
};

class KWIN_EXPORT UkuiWindowManagementInterface : public QObject
{
    Q_OBJECT

public:
    explicit UkuiWindowManagementInterface(Display *display, QObject *parent = nullptr);
    ~UkuiWindowManagementInterface() override;
    enum class ShowingDesktopState {
        Disabled,
        Enabled,
    };
    void setShowingDesktopState(ShowingDesktopState state);

    UkuiWindowInterface *createWindow(QObject *parent, const QUuid &uuid);
    QList<UkuiWindowInterface *> windows() const;

    void setVirtualDesktopManagementInterface(PlasmaVirtualDesktopManagementInterface *manager);
    PlasmaVirtualDesktopManagementInterface *virtualDesktopManagementInterface() const;

    /**
     * Associate stacking order to this window management
     */
    void setStackingOrder(const QVector<quint32> &stackingOrder);

    void setStackingOrderUuids(const QVector<QString> &stackingOrderUuids);

Q_SIGNALS:
    void requestChangeShowingDesktop(ShowingDesktopState requestedState);

private:
    QScopedPointer<UkuiWindowManagementInterfacePrivate> d;
};

class KWIN_EXPORT UkuiWindowInterface : public QObject
{
    Q_OBJECT
public:
    ~UkuiWindowInterface() override;

    void setTitle(const QString &title);
    void setAppId(const QString &appId);
    void setPid(quint32 pid);
    void setActive(bool set);
    void setMinimized(bool set);
    void setMaximized(bool set);
    void setFullscreen(bool set);
    void setKeepAbove(bool set);
    void setKeepBelow(bool set);
    void setOnAllDesktops(bool set);
    void setDemandsAttention(bool set);
    void setCloseable(bool set);
    void setMinimizeable(bool set);
    void setMaximizeable(bool set);
    void setFullscreenable(bool set);
    void setSkipTaskbar(bool skip);
    void setSkipSwitcher(bool skip);
    void setShadeable(bool set);
    void setShaded(bool set);
    void setMovable(bool set);
    void setResizable(bool set);
    /**
     * FIXME: still relevant with new desktops?
     */
    void setVirtualDesktopChangeable(bool set);

    /**
     * This method removes the Window and the Client is supposed to release the resource
     * bound for this Window.
     *
     * No more events should be sent afterwards.
     */
    void unmap();

    /**
     * @returns Geometries of the taskbar/desktop entries, indicized by the
     *          surface of the panels/desktops
     */
    QHash<SurfaceInterface *, QRect> startupGeometries() const;

    /**
     * @returns Geometries of the taskbar entries, indicized by the
     *          surface of the panels
     */
    QHash<SurfaceInterface *, QRect> minimizedGeometries() const;

    /**
     * Sets this UkuiWindowInterface as a transient window to @p parentWindow.
     * If @p parentWindow is @c nullptr, the UkuiWindowInterface is a toplevel
     * window and does not have a parent window.
     */
    void setParentWindow(UkuiWindowInterface *parentWindow);

    /**
     * Sets the window @p geometry of this UkuiWindow.
     *
     * @param geometry The geometry in absolute coordinates
     */
    void setGeometry(const QRect &geometry);

    /**
     * Set the icon of the UkuiWindowInterface.
     *
     * In case the icon has a themed name, only the name is sent to the client.
     * Otherwise the client is only informed that there is an icon and the client
     * can request the icon in an asynchronous way by passing a file descriptor
     * into which the icon will be serialized.
     *
     * @param icon The new icon
     */
    void setIcon(const QIcon &icon);

    /**
     * Adds a new desktop to this window: a window can be on
     * an arbitrary subset of virtual desktops.
     * If it's on none it will be considered on all desktops.
     */
    void addVirtualDesktop(const QString &id);

    /**
     * Removes a visrtual desktop from a window
     */
    void removeVirtualDesktop(const QString &id);

    /**
     * The ids of all the desktops currently associated with this window.
     * When a desktop is deleted it will be automatically removed from this list
     */
    QStringList virtualDesktops() const;

    /**
     * Adds an activity to this window: a window can be on
     * an arbitrary subset of activities.
     * If it's on none it will be considered on all activities.
     */
    void addUkuiActivity(const QString &id);

    /**
     * Removes an activity from a window
     */
    void removeUkuiActivity(const QString &id);

    /**
     * The ids of all the activities currently associated with this window.
     * When an activity is deleted it will be automatically removed from this list
     */
    QStringList ukuiActivities() const;

    /**
     * Set the application menu D-BUS service name and object path for the window.
     */
    void setApplicationMenuPaths(const QString &serviceName, const QString &objectPath);

    /**
     * Return the window internal id
     */
    quint32 internalId() const;

    /**
     * @return a unique string that identifies this window
     */
    QString uuid() const;

Q_SIGNALS:
    void closeRequested();
    void moveRequested();
    void resizeRequested();
    void activeRequested(bool set);
    void minimizedRequested(bool set);
    void maximizedRequested(bool set);
    void fullscreenRequested(bool set);
    void keepAboveRequested(bool set);
    void keepBelowRequested(bool set);
    void onAllDesktopsRequested(bool set);
    void demandsAttentionRequested(bool set);
    void closeableRequested(bool set);
    void minimizeableRequested(bool set);
    void maximizeableRequested(bool set);
    void fullscreenableRequested(bool set);
    void shadeableRequested(bool set);
    void shadedRequested(bool set);
    void movableRequested(bool set);
    void resizableRequested(bool set);
    void virtualDesktopChangeableRequested(bool set);
    void acceptFocusRequested(bool set);
    void skipTaskbarRequested(bool set);
    void skipSwitcherRequested(bool set);
    void modalityRequested(bool set);
    QRect startupGeometriesChanged();
    QRect minimizedGeometriesChanged();

    /**
     * Emitted when the client wishes this window to enter in a new virtual desktop.
     * The server will decide whether to consent this request
     */
    void enterUkuiVirtualDesktopRequested(const QString &desktop);

    /**
     * Emitted when the client wishes this window to enter in
     * a new virtual desktop to be created for it.
     * The server will decide whether to consent this request
     */
    void enterNewUkuiVirtualDesktopRequested();

    /**
     * Emitted when the client wishes to remove this window from a virtual desktop.
     * The server will decide whether to consent this request
     */
    void leaveUkuiVirtualDesktopRequested(const QString &desktop);

    /**
     * Emitted when the client wishes this window to enter an activity.
     * The server will decide whether to consent this request
     */
    void enterUkuiActivityRequested(const QString &activity);

    /**
     * Emitted when the client wishes to remove this window from an activity.
     * The server will decide whether to consent this request
     */
    void leaveUkuiActivityRequested(const QString &activity);

    /**
     * Requests sending the window to @p output
     */
    void sendToOutput(KWaylandServer::OutputInterface *output);

private:
    friend class UkuiWindowManagementInterface;
    friend class UkuiWindowInterfacePrivate;
    friend class UkuiWindowManagementInterfacePrivate;
    explicit UkuiWindowInterface(UkuiWindowManagementInterface *wm, QObject *parent);

    QScopedPointer<UkuiWindowInterfacePrivate> d;
};

}
/*
    SPDX-FileCopyrightText: 2024 Teng Wang <wangteng@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "ukuiwindowmanagement_interface.h"
#include "display.h"
#include "plasmavirtualdesktop_interface.h"
#include "surface_interface.h"

#include <qwayland-server-ukui-window-management.h>

#include <QFile>
#include <QHash>
#include <QIcon>
#include <QList>
#include <QRect>
#include <QUuid>
#include <QVector>
#include <QtConcurrentRun>

namespace KWaylandServer
{
static const quint32 s_version = 1;
static const quint32 s_activationVersion = 1;

class UkuiWindowManagementInterfacePrivate : public QtWaylandServer::ukui_window_management
{
public:
    UkuiWindowManagementInterfacePrivate(UkuiWindowManagementInterface *_q, Display *display);
    void sendShowingDesktopState();
    void sendShowingDesktopState(wl_resource *resource);
    void sendStackingOrderChanged();
    void sendStackingOrderChanged(wl_resource *resource);
    void sendStackingOrderUuidsChanged();
    void sendStackingOrderUuidsChanged(wl_resource *resource);

    UkuiWindowManagementInterface::ShowingDesktopState state = UkuiWindowManagementInterface::ShowingDesktopState::Disabled;
    QList<UkuiWindowInterface *> windows;
    QPointer<PlasmaVirtualDesktopManagementInterface> virtualDesktopManagementInterface = nullptr;
    quint32 windowIdCounter = 0;
    QVector<quint32> stackingOrder;
    QVector<QString> stackingOrderUuids;
    UkuiWindowManagementInterface *q;

protected:
    void ukui_window_management_bind_resource(Resource *resource) override;
    void ukui_window_management_destroy_resource(Resource *resource) override;
    void ukui_window_management_show_desktop(Resource *resource, uint32_t state) override;
    void ukui_window_management_create_window(Resource *resource, uint32_t id, const QString &internal_window_uuid) override;
};

class UkuiWindowInterfacePrivate : public QtWaylandServer::ukui_window
{
public:
    UkuiWindowInterfacePrivate(UkuiWindowManagementInterface *wm, UkuiWindowInterface *q);
    ~UkuiWindowInterfacePrivate();

    void setTitle(const QString &title);
    void setAppId(const QString &appId);
    void setPid(quint32 pid);
    void setThemedIconName(const QString &iconName);
    void setIcon(const QIcon &icon);
    void unmap();
    void setState(ukui_window_state flag, bool set);
    void setParentWindow(UkuiWindowInterface *parent);
    void setGeometry(const QRect &geometry);
    void setApplicationMenuPaths(const QString &service, const QString &object);
    wl_resource *resourceForParent(UkuiWindowInterface *parent, Resource *child) const;

    quint32 windowId = 0;
    QHash<SurfaceInterface *, QRect> startupGeometries;
    QHash<SurfaceInterface *, QRect> minimizedGeometries;
    UkuiWindowManagementInterface *wm;

    bool unmapped = false;
    UkuiWindowInterface *parentWindow = nullptr;
    QMetaObject::Connection parentWindowDestroyConnection;
    QStringList virtualDesktops;
    QStringList UkuiActivities;
    QRect geometry;
    UkuiWindowInterface *q;
    QString m_title;
    QString m_appId;
    quint32 m_pid = 0;
    QString m_themedIconName;
    QString m_appServiceName;
    QString m_appObjectPath;
    QIcon m_icon;
    quint32 m_state = 0;
    QString uuid;

protected:
    void ukui_window_bind_resource(Resource *resource) override;
    void ukui_window_set_state(Resource *resource, uint32_t flags, uint32_t state) override;
    void ukui_window_set_startup_geometry(Resource *resource, wl_resource *entry, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    void ukui_window_set_minimized_geometry(Resource *resource, wl_resource *panel, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    void ukui_window_unset_minimized_geometry(Resource *resource, wl_resource *panel) override;
    void ukui_window_close(Resource *resource) override;
    void ukui_window_request_move(Resource *resource) override;
    void ukui_window_request_resize(Resource *resource) override;
    void ukui_window_destroy(Resource *resource) override;
    void ukui_window_get_icon(Resource *resource, int32_t fd) override;
    void ukui_window_request_enter_virtual_desktop(Resource *resource, const QString &id) override;
    void ukui_window_request_enter_new_virtual_desktop(Resource *resource) override;
    void ukui_window_request_leave_virtual_desktop(Resource *resource, const QString &id) override;
    void ukui_window_request_enter_activity(Resource *resource, const QString &id) override;
    void ukui_window_request_leave_activity(Resource *resource, const QString &id) override;
    void ukui_window_send_to_output(Resource *resource, struct wl_resource *output) override;
};

/*********************************
 * UkuiWindowManagementInterfacePrivate
 *********************************/
UkuiWindowManagementInterfacePrivate::UkuiWindowManagementInterfacePrivate(UkuiWindowManagementInterface *_q, Display *display)
    : QtWaylandServer::ukui_window_management(*display, s_version)
    , q(_q)
{
}

void UkuiWindowManagementInterfacePrivate::sendShowingDesktopState()
{
    const auto clientResources = resourceMap();
    for (auto resource : clientResources) {
        sendShowingDesktopState(resource->handle);
    }
}

void UkuiWindowManagementInterfacePrivate::sendShowingDesktopState(wl_resource *r)
{
    uint32_t s = 0;
    switch (state) {
    case UkuiWindowManagementInterface::ShowingDesktopState::Enabled:
        s = QtWaylandServer::ukui_window_management::show_desktop_enabled;
        break;
    case UkuiWindowManagementInterface::ShowingDesktopState::Disabled:
        s = QtWaylandServer::ukui_window_management::show_desktop_disabled;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    send_show_desktop_changed(r, s);
}

void UkuiWindowManagementInterfacePrivate::sendStackingOrderChanged()
{
    const auto clientResources = resourceMap();
    for (auto resource : clientResources) {
        sendStackingOrderChanged(resource->handle);
    }
}

void UkuiWindowManagementInterfacePrivate::sendStackingOrderChanged(wl_resource *r)
{
    send_stacking_order_changed(r, QByteArray::fromRawData(reinterpret_cast<const char *>(stackingOrder.constData()), sizeof(uint32_t) * stackingOrder.size()));
}

void UkuiWindowManagementInterfacePrivate::sendStackingOrderUuidsChanged()
{
    const auto clientResources = resourceMap();
    for (auto resource : clientResources) {
        sendStackingOrderUuidsChanged(resource->handle);
    }
}

void UkuiWindowManagementInterfacePrivate::sendStackingOrderUuidsChanged(wl_resource *r)
{
    QString uuids;
    for (const auto &uuid : qAsConst(stackingOrderUuids)) {
        uuids += uuid;
        uuids += QLatin1Char(';');
    }
    // Remove the trailing ';', on the receiving side this is interpreted as an empty uuid.
    if (stackingOrderUuids.size() > 0) {
        uuids.remove(uuids.length() - 1, 1);
    }
    send_stacking_order_changed(r, uuids);
}

void UkuiWindowManagementInterfacePrivate::ukui_window_management_bind_resource(Resource *resource)
{
    for (auto window : windows) {
        send_window_created(resource->handle, window->d->uuid);
    }
    sendStackingOrderChanged(resource->handle);
}

void UkuiWindowManagementInterfacePrivate::ukui_window_management_destroy_resource(Resource *resource)
{
}

void UkuiWindowManagementInterfacePrivate::ukui_window_management_show_desktop(Resource *resource, uint32_t state)
{

    UkuiWindowManagementInterface::ShowingDesktopState s = UkuiWindowManagementInterface::ShowingDesktopState::Disabled;
    switch (state) {
    case UKUI_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED:
        s = UkuiWindowManagementInterface::ShowingDesktopState::Enabled;
        break;
    case UKUI_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED:
    default:
        s = UkuiWindowManagementInterface::ShowingDesktopState::Disabled;
        break;
    }
    Q_EMIT q->requestChangeShowingDesktop(s);
}

void UkuiWindowManagementInterfacePrivate::ukui_window_management_create_window(Resource *resource, uint32_t id, const QString &internal_window_uuid)
{
    auto it = std::find_if(windows.constBegin(), windows.constEnd(), [internal_window_uuid](UkuiWindowInterface *window) {
        return window->d->uuid == internal_window_uuid;
    });
    if (it == windows.constEnd()) {
        // create a temp window just for the resource, bind then immediately delete it, sending an unmap event
        UkuiWindowInterface window(q, q);
        window.d->add(resource->client(), id, resource->version());
        return;
    }
    (*it)->d->add(resource->client(), id, resource->version());
}

/*********************************
 * UkuiWindowManagementInterface
 *********************************/
UkuiWindowManagementInterface::UkuiWindowManagementInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new UkuiWindowManagementInterfacePrivate(this, display))
{
}

UkuiWindowManagementInterface::~UkuiWindowManagementInterface() = default;

void UkuiWindowManagementInterface::setShowingDesktopState(UkuiWindowManagementInterface::ShowingDesktopState state)
{
    if (d->state == state) {
        return;
    }
    d->state = state;
    d->sendShowingDesktopState();
}

UkuiWindowInterface *UkuiWindowManagementInterface::createWindow(QObject *parent, const QUuid &uuid)
{
    UkuiWindowInterface *window = new UkuiWindowInterface(this, parent);

    window->d->uuid = uuid.toString();
    window->d->windowId = ++d->windowIdCounter; // NOTE the window id is deprecated

    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_window_created(resource->handle, window->d->uuid);
    }
    d->windows << window;
    connect(window, &QObject::destroyed, this, [this, window] {
        d->windows.removeAll(window);
    });
    return window;
}

QList<UkuiWindowInterface *> UkuiWindowManagementInterface::windows() const
{
    return d->windows;
}

void UkuiWindowManagementInterface::setStackingOrder(const QVector<quint32> &stackingOrder)
{
    if (d->stackingOrder == stackingOrder) {
        return;
    }
    d->stackingOrder = stackingOrder;
    d->sendStackingOrderChanged();
}

void UkuiWindowManagementInterface::setStackingOrderUuids(const QVector<QString> &stackingOrderUuids)
{
    if (d->stackingOrderUuids == stackingOrderUuids) {
        return;
    }
    d->stackingOrderUuids = stackingOrderUuids;
    d->sendStackingOrderUuidsChanged();
}

void UkuiWindowManagementInterface::setVirtualDesktopManagementInterface(PlasmaVirtualDesktopManagementInterface *manager)
{
    if (d->virtualDesktopManagementInterface == manager) {
        return;
    }
    d->virtualDesktopManagementInterface = manager;
}

PlasmaVirtualDesktopManagementInterface *UkuiWindowManagementInterface::virtualDesktopManagementInterface() const
{
    return d->virtualDesktopManagementInterface;
}

/*********************************
 * UkuiWindowInterfacePrivate
 *********************************/
UkuiWindowInterfacePrivate::UkuiWindowInterfacePrivate(UkuiWindowManagementInterface *wm, UkuiWindowInterface *q)
    : QtWaylandServer::ukui_window()
    , wm(wm)
    , q(q)
{
}

UkuiWindowInterfacePrivate::~UkuiWindowInterfacePrivate()
{
    unmap();
}

void UkuiWindowInterfacePrivate::ukui_window_bind_resource(Resource *resource)
{
    for (const auto &desk : virtualDesktops) {
        send_virtual_desktop_entered(resource->handle, desk);
    }
    if (!m_appId.isEmpty()) {
        send_app_id_changed(resource->handle, m_appId);
    }
    if (m_pid != 0) {
        send_pid_changed(resource->handle, m_pid);
    }
    if (!m_title.isEmpty()) {
        send_title_changed(resource->handle, m_title);
    }
    if (!m_appObjectPath.isEmpty() || !m_appServiceName.isEmpty()) {
        send_application_menu(resource->handle, m_appServiceName, m_appObjectPath);
    }
    send_state_changed(resource->handle, m_state);
    if (!m_themedIconName.isEmpty()) {
        send_themed_icon_name_changed(resource->handle, m_themedIconName);
    } else if (!m_icon.isNull()) {
        send_icon_changed(resource->handle);
    }

    send_parent_window(resource->handle, resourceForParent(parentWindow, resource));
    send_geometry(resource->handle, geometry.x(), geometry.y(), geometry.width(), geometry.height());
    send_initial_state(resource->handle);
}

void UkuiWindowInterfacePrivate::ukui_window_set_state(Resource *resource, uint32_t flags, uint32_t state)
{

    if (flags & UKUI_WINDOW_STATE_ACTIVE) {
        Q_EMIT q->activeRequested(state & UKUI_WINDOW_STATE_ACTIVE);
    }
    if (flags & UKUI_WINDOW_STATE_MINIMIZED) {
        Q_EMIT q->minimizedRequested(state & UKUI_WINDOW_STATE_MINIMIZED);
    }
    if (flags & UKUI_WINDOW_STATE_MAXIMIZED) {
        Q_EMIT q->maximizedRequested(state & UKUI_WINDOW_STATE_MAXIMIZED);
    }
    if (flags & UKUI_WINDOW_STATE_FULLSCREEN) {
        Q_EMIT q->fullscreenRequested(state & UKUI_WINDOW_STATE_FULLSCREEN);
    }
    if (flags & UKUI_WINDOW_STATE_KEEP_ABOVE) {
        Q_EMIT q->keepAboveRequested(state & UKUI_WINDOW_STATE_KEEP_ABOVE);
    }
    if (flags & UKUI_WINDOW_STATE_KEEP_BELOW) {
        Q_EMIT q->keepBelowRequested(state & UKUI_WINDOW_STATE_KEEP_BELOW);
    }
    if (flags & UKUI_WINDOW_STATE_ON_ALL_DESKTOPS) {
        Q_EMIT q->onAllDesktopsRequested(state & UKUI_WINDOW_STATE_ON_ALL_DESKTOPS);
    }
    if (flags & UKUI_WINDOW_STATE_DEMANDS_ATTENTION) {
        Q_EMIT q->demandsAttentionRequested(state & UKUI_WINDOW_STATE_DEMANDS_ATTENTION);
    }
    if (flags & UKUI_WINDOW_STATE_CLOSEABLE) {
        Q_EMIT q->closeableRequested(state & UKUI_WINDOW_STATE_CLOSEABLE);
    }
    if (flags & UKUI_WINDOW_STATE_MINIMIZABLE) {
        Q_EMIT q->minimizeableRequested(state & UKUI_WINDOW_STATE_MINIMIZABLE);
    }
    if (flags & UKUI_WINDOW_STATE_MAXIMIZABLE) {
        Q_EMIT q->maximizeableRequested(state & UKUI_WINDOW_STATE_MAXIMIZABLE);
    }
    if (flags & UKUI_WINDOW_STATE_FULLSCREENABLE) {
        Q_EMIT q->fullscreenableRequested(state & UKUI_WINDOW_STATE_FULLSCREENABLE);
    }
    if (flags & UKUI_WINDOW_STATE_SHADEABLE) {
        Q_EMIT q->shadeableRequested(state & UKUI_WINDOW_STATE_SHADEABLE);
    }
    if (flags & UKUI_WINDOW_STATE_SHADED) {
        Q_EMIT q->shadedRequested(state & UKUI_WINDOW_STATE_SHADED);
    }
    if (flags & UKUI_WINDOW_STATE_MOVABLE) {
        Q_EMIT q->movableRequested(state & UKUI_WINDOW_STATE_MOVABLE);
    }
    if (flags & UKUI_WINDOW_STATE_RESIZABLE) {
        Q_EMIT q->resizableRequested(state & UKUI_WINDOW_STATE_RESIZABLE);
    }
    if (flags & UKUI_WINDOW_STATE_VIRTUAL_DESKTOP_CHANGEABLE) {
        Q_EMIT q->virtualDesktopChangeableRequested(state & UKUI_WINDOW_STATE_VIRTUAL_DESKTOP_CHANGEABLE);
    }
    if (flags & UKUI_WINDOW_STATE_ACCEPT_FOCUS) {
        Q_EMIT q->acceptFocusRequested(state & UKUI_WINDOW_STATE_ACCEPT_FOCUS);
    }
    if (flags & UKUI_WINDOW_STATE_SKIPTASKBAR) {
        Q_EMIT q->skipTaskbarRequested(state & UKUI_WINDOW_STATE_SKIPTASKBAR);
    }
    if (flags & UKUI_WINDOW_STATE_SKIPSWITCHER) {
        Q_EMIT q->skipSwitcherRequested(state & UKUI_WINDOW_STATE_SKIPSWITCHER);
    }
    if (flags & UKUI_WINDOW_STATE_MODALITY) {
        Q_EMIT q->modalityRequested(state & UKUI_WINDOW_STATE_MODALITY);
    }
}

void UkuiWindowInterfacePrivate::ukui_window_set_startup_geometry(Resource *resource, wl_resource *entry, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{

    SurfaceInterface *entrySurface = SurfaceInterface::get(entry);

    if (!entrySurface) {
        return;
    }

    if (startupGeometries.value(entrySurface) == QRect(x, y, width, height)) {
        return;
    }

    startupGeometries[entrySurface] = QRect(x, y, width, height);
    Q_EMIT q->startupGeometriesChanged();
    QObject::connect(entrySurface, &QObject::destroyed, q, [this, entrySurface]() {
        if (startupGeometries.remove(entrySurface)) {
            Q_EMIT q->startupGeometriesChanged();
        }
    });
}

void UkuiWindowInterfacePrivate::ukui_window_set_minimized_geometry(Resource *resource, wl_resource *panel, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{

    SurfaceInterface *panelSurface = SurfaceInterface::get(panel);

    if (!panelSurface) {
        return;
    }

    if (minimizedGeometries.value(panelSurface) == QRect(x, y, width, height)) {
        return;
    }

    minimizedGeometries[panelSurface] = QRect(x, y, width, height);
    Q_EMIT q->minimizedGeometriesChanged();
    QObject::connect(panelSurface, &QObject::destroyed, q, [this, panelSurface]() {
        if (minimizedGeometries.remove(panelSurface)) {
            Q_EMIT q->minimizedGeometriesChanged();
        }
    });
}

void UkuiWindowInterfacePrivate::ukui_window_unset_minimized_geometry(Resource *resource, wl_resource *panel)
{

    SurfaceInterface *panelSurface = SurfaceInterface::get(panel);

    if (!panelSurface) {
        return;
    }
    if (!minimizedGeometries.contains(panelSurface)) {
        return;
    }
    minimizedGeometries.remove(panelSurface);
    Q_EMIT q->minimizedGeometriesChanged();
}

void UkuiWindowInterfacePrivate::ukui_window_close(Resource *resource)
{

    Q_EMIT q->closeRequested();
}

void UkuiWindowInterfacePrivate::ukui_window_request_move(Resource *resource)
{

    Q_EMIT q->moveRequested();
}

void UkuiWindowInterfacePrivate::ukui_window_request_resize(Resource *resource)
{

    Q_EMIT q->resizeRequested();
}

void UkuiWindowInterfacePrivate::ukui_window_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void UkuiWindowInterfacePrivate::ukui_window_get_icon(Resource *resource, int32_t fd)
{

    QtConcurrent::run(
        [fd](const QIcon &icon) {
        QFile file;
        file.open(fd, QIODevice::WriteOnly, QFileDevice::AutoCloseHandle);
        QDataStream ds(&file);
        ds << icon;
        file.close();
    },
        m_icon);
}

void UkuiWindowInterfacePrivate::ukui_window_request_enter_virtual_desktop(Resource *resource, const QString &id)
{

    Q_EMIT q->enterUkuiVirtualDesktopRequested(id);
}

void UkuiWindowInterfacePrivate::ukui_window_request_enter_new_virtual_desktop(Resource *resource)
{

    Q_EMIT q->enterNewUkuiVirtualDesktopRequested();
}

void UkuiWindowInterfacePrivate::ukui_window_request_leave_virtual_desktop(Resource *resource, const QString &id)
{

    Q_EMIT q->leaveUkuiVirtualDesktopRequested(id);
}

void UkuiWindowInterfacePrivate::ukui_window_request_enter_activity(Resource *resource, const QString &id)
{

    Q_EMIT q->enterUkuiActivityRequested(id);
}

void UkuiWindowInterfacePrivate::ukui_window_request_leave_activity(Resource *resource, const QString &id)
{

    Q_EMIT q->leaveUkuiActivityRequested(id);
}

void UkuiWindowInterfacePrivate::ukui_window_send_to_output(Resource *resource, struct wl_resource *output)
{

    Q_EMIT q->sendToOutput(KWaylandServer::OutputInterface::get(output));
}

void UkuiWindowInterfacePrivate::unmap()
{
    if (unmapped) {
        return;
    }
    unmapped = true;
    const auto clientResources = resourceMap();

    for (auto resource : clientResources) {
        send_unmapped(resource->handle);
    }
}

wl_resource *UkuiWindowInterfacePrivate::resourceForParent(UkuiWindowInterface *parent, Resource *child) const
{
    if (!parent) {
        return nullptr;
    }

    const auto parentResource = parent->d->resourceMap();

    for (auto resource : parentResource) {
        if (child->client() == resource->client()) {
            return resource->handle;
        }
    }
    return nullptr;
}

void UkuiWindowInterfacePrivate::setState(ukui_window_state flag, bool set)
{
    quint32 newState = m_state;
    if (set) {
        newState |= flag;
    } else {
        newState &= ~flag;
    }
    if (newState == m_state) {
        return;
    }
    m_state = newState;
    const auto clientResources = resourceMap();

    for (auto resource : clientResources) {
        send_state_changed(resource->handle, m_state);
    }
}

void UkuiWindowInterfacePrivate::setTitle(const QString &title)
{
    if (m_title == title) {
        return;
    }
    m_title = title;
    const auto clientResources = resourceMap();

    for (auto resource : clientResources) {
        send_title_changed(resource->handle, m_title);
    }
}

void UkuiWindowInterfacePrivate::setAppId(const QString &appId)
{
    if (m_appId == appId) {
        return;
    }

    m_appId = appId;
    const auto clientResources = resourceMap();

    for (auto resource : clientResources) {
        send_app_id_changed(resource->handle, m_appId);
    }
}

void UkuiWindowInterfacePrivate::setPid(quint32 pid)
{
    if (m_pid == pid) {
        return;
    }
    m_pid = pid;
    const auto clientResources = resourceMap();

    for (auto resource : clientResources) {
        send_pid_changed(resource->handle, pid);
    }
}

void UkuiWindowInterfacePrivate::setThemedIconName(const QString &iconName)
{
    if (m_themedIconName == iconName) {
        return;
    }
    m_themedIconName = iconName;
    const auto clientResources = resourceMap();

    for (auto resource : clientResources) {
        send_themed_icon_name_changed(resource->handle, m_themedIconName);
    }
}

void UkuiWindowInterfacePrivate::setIcon(const QIcon &icon)
{
    m_icon = icon;
    setThemedIconName(m_icon.name());

    const auto clientResources = resourceMap();
    for (auto resource : clientResources) {
        send_icon_changed(resource->handle);
    }
}

void UkuiWindowInterfacePrivate::setParentWindow(UkuiWindowInterface *window)
{
    if (parentWindow == window) {
        return;
    }
    QObject::disconnect(parentWindowDestroyConnection);
    parentWindowDestroyConnection = QMetaObject::Connection();
    parentWindow = window;
    if (parentWindow) {
        parentWindowDestroyConnection = QObject::connect(window, &QObject::destroyed, q, [this] {
            parentWindow = nullptr;
            parentWindowDestroyConnection = QMetaObject::Connection();
            const auto clientResources = resourceMap();
            for (auto resource : clientResources) {
                send_parent_window(resource->handle, nullptr);
            }
        });
    }
    const auto clientResources = resourceMap();
    for (auto resource : clientResources) {
        send_parent_window(resource->handle, resourceForParent(window, resource));
    }
}

void UkuiWindowInterfacePrivate::setGeometry(const QRect &geo)
{
    if (geometry == geo) {
        return;
    }
    geometry = geo;
    if (!geometry.isValid()) {
        return;
    }

    const auto clientResources = resourceMap();
    for (auto resource : clientResources) {
        send_geometry(resource->handle, geometry.x(), geometry.y(), geometry.width(), geometry.height());
    }
}

void UkuiWindowInterfacePrivate::setApplicationMenuPaths(const QString &service, const QString &object)
{
    if (m_appServiceName == service && m_appObjectPath == object) {
        return;
    }
    m_appServiceName = service;
    m_appObjectPath = object;
    const auto clientResources = resourceMap();
    for (auto resource : clientResources) {
        send_application_menu(resource->handle, service, object);
    }
}

/*********************************
 * UkuiWindowInterface
 *********************************/
UkuiWindowInterface::UkuiWindowInterface(UkuiWindowManagementInterface *wm, QObject *parent)
    : QObject(parent)
    , d(new UkuiWindowInterfacePrivate(wm, this))
{
}

UkuiWindowInterface::~UkuiWindowInterface() = default;

void UkuiWindowInterface::setAppId(const QString &appId)
{
    d->setAppId(appId);
}

void UkuiWindowInterface::setPid(quint32 pid)
{
    d->setPid(pid);
}

void UkuiWindowInterface::setTitle(const QString &title)
{
    d->setTitle(title);
}

void UkuiWindowInterface::unmap()
{
    d->unmap();
}

QHash<SurfaceInterface *, QRect> UkuiWindowInterface::startupGeometries() const
{
    return d->startupGeometries;
}

QHash<SurfaceInterface *, QRect> UkuiWindowInterface::minimizedGeometries() const
{
    return d->minimizedGeometries;
}

void UkuiWindowInterface::setActive(bool set)
{
    d->setState(UKUI_WINDOW_STATE_ACTIVE, set);
}

void UkuiWindowInterface::setFullscreen(bool set)
{
    d->setState(UKUI_WINDOW_STATE_FULLSCREEN, set);
}

void UkuiWindowInterface::setKeepAbove(bool set)
{
    d->setState(UKUI_WINDOW_STATE_KEEP_ABOVE, set);
}

void UkuiWindowInterface::setKeepBelow(bool set)
{
    d->setState(UKUI_WINDOW_STATE_KEEP_BELOW, set);
}

void UkuiWindowInterface::setMaximized(bool set)
{
    d->setState(UKUI_WINDOW_STATE_MAXIMIZED, set);
}

void UkuiWindowInterface::setMinimized(bool set)
{
    d->setState(UKUI_WINDOW_STATE_MINIMIZED, set);
}

void UkuiWindowInterface::setOnAllDesktops(bool set)
{
    d->setState(UKUI_WINDOW_STATE_ON_ALL_DESKTOPS, set);
}

void UkuiWindowInterface::setDemandsAttention(bool set)
{
    d->setState(UKUI_WINDOW_STATE_DEMANDS_ATTENTION, set);
}

void UkuiWindowInterface::setCloseable(bool set)
{
    d->setState(UKUI_WINDOW_STATE_CLOSEABLE, set);
}

void UkuiWindowInterface::setFullscreenable(bool set)
{
    d->setState(UKUI_WINDOW_STATE_FULLSCREENABLE, set);
}

void UkuiWindowInterface::setMaximizeable(bool set)
{
    d->setState(UKUI_WINDOW_STATE_MAXIMIZABLE, set);
}

void UkuiWindowInterface::setMinimizeable(bool set)
{
    d->setState(UKUI_WINDOW_STATE_MINIMIZABLE, set);
}

void UkuiWindowInterface::setSkipTaskbar(bool set)
{
    d->setState(UKUI_WINDOW_STATE_SKIPTASKBAR, set);
}

void UkuiWindowInterface::setSkipSwitcher(bool skip)
{
    d->setState(UKUI_WINDOW_STATE_SKIPSWITCHER, skip);
}

void UkuiWindowInterface::setIcon(const QIcon &icon)
{
    d->setIcon(icon);
}

void UkuiWindowInterface::addVirtualDesktop(const QString &id)
{
    // don't add a desktop we're not sure it exists
    if (!d->wm->virtualDesktopManagementInterface() || d->virtualDesktops.contains(id)) {
        return;
    }

    PlasmaVirtualDesktopInterface *desktop = d->wm->virtualDesktopManagementInterface()->desktop(id);

    if (!desktop) {
        return;
    }

    d->virtualDesktops << id;

    // if the desktop dies, remove it from or list
    connect(desktop, &QObject::destroyed, this, [this, id]() {
        removeVirtualDesktop(id);
    });

    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_virtual_desktop_entered(resource->handle, id);
    }
}

void UkuiWindowInterface::removeVirtualDesktop(const QString &id)
{
    if (!d->virtualDesktops.contains(id)) {
        return;
    }

    d->virtualDesktops.removeAll(id);
    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_virtual_desktop_left(resource->handle, id);
    }

    // we went on all desktops
    if (d->virtualDesktops.isEmpty()) {
        setOnAllDesktops(true);
    }
}

QStringList UkuiWindowInterface::virtualDesktops() const
{
    return d->virtualDesktops;
}

void UkuiWindowInterface::addUkuiActivity(const QString &id)
{
    if (d->UkuiActivities.contains(id)) {
        return;
    }

    d->UkuiActivities << id;

    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_activity_entered(resource->handle, id);
    }
}

void UkuiWindowInterface::removeUkuiActivity(const QString &id)
{
    if (!d->UkuiActivities.removeOne(id)) {
        return;
    }

    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_activity_left(resource->handle, id);
    }
}

QStringList UkuiWindowInterface::ukuiActivities() const
{
    return d->UkuiActivities;
}

void UkuiWindowInterface::setShadeable(bool set)
{
    d->setState(UKUI_WINDOW_STATE_SHADEABLE, set);
}

void UkuiWindowInterface::setShaded(bool set)
{
    d->setState(UKUI_WINDOW_STATE_SHADED, set);
}

void UkuiWindowInterface::setMovable(bool set)
{
    d->setState(UKUI_WINDOW_STATE_MOVABLE, set);
}

void UkuiWindowInterface::setResizable(bool set)
{
    d->setState(UKUI_WINDOW_STATE_RESIZABLE, set);
}

void UkuiWindowInterface::setVirtualDesktopChangeable(bool set)
{
    d->setState(UKUI_WINDOW_STATE_VIRTUAL_DESKTOP_CHANGEABLE, set);
}

void UkuiWindowInterface::setParentWindow(UkuiWindowInterface *parentWindow)
{
    d->setParentWindow(parentWindow);
}

void UkuiWindowInterface::setGeometry(const QRect &geometry)
{
    d->setGeometry(geometry);
}

void UkuiWindowInterface::setApplicationMenuPaths(const QString &serviceName, const QString &objectPath)
{
    d->setApplicationMenuPaths(serviceName, objectPath);
}

quint32 UkuiWindowInterface::internalId() const
{
    return d->windowId;
}

QString UkuiWindowInterface::uuid() const
{
    return d->uuid;
}

class UkuiWindowActivationFeedbackInterfacePrivate : public QtWaylandServer::org_ukui_activation_feedback
{
public:
    UkuiWindowActivationFeedbackInterfacePrivate(Display *display);

protected:
    void org_ukui_activation_feedback_destroy(Resource *resource) override;
};

class UkuiWindowActivationInterfacePrivate : public QtWaylandServer::org_ukui_activation
{
public:
    UkuiWindowActivationInterfacePrivate(UkuiWindowActivationInterface *q)
        : QtWaylandServer::org_ukui_activation()
        , q(q)
    {
    }

    UkuiWindowActivationInterface *const q;
};

UkuiWindowActivationFeedbackInterfacePrivate::UkuiWindowActivationFeedbackInterfacePrivate(Display *display)
    : QtWaylandServer::org_ukui_activation_feedback(*display, s_activationVersion)
{
}

void UkuiWindowActivationFeedbackInterfacePrivate::org_ukui_activation_feedback_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

UkuiWindowActivationFeedbackInterface::UkuiWindowActivationFeedbackInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new UkuiWindowActivationFeedbackInterfacePrivate(display))
{
}

UkuiWindowActivationFeedbackInterface::~UkuiWindowActivationFeedbackInterface()
{
}

UkuiWindowActivationInterface *UkuiWindowActivationFeedbackInterface::createActivation(const QString &appid)
{
    auto activation = new UkuiWindowActivationInterface;
    const auto resources = d->resourceMap();
    for (auto resource : resources) {
        auto activationResource = activation->d->add(resource->client(), resource->version());
        d->send_activation(resource->handle, activationResource->handle);
    }
    activation->sendAppId(appid);
    return activation;
}

UkuiWindowActivationInterface::UkuiWindowActivationInterface()
    : d(new UkuiWindowActivationInterfacePrivate(this))
{
}

UkuiWindowActivationInterface::~UkuiWindowActivationInterface()
{
    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_finished(resource->handle);
    }
}

void UkuiWindowActivationInterface::sendAppId(const QString &appid)
{
    const auto clientResources = d->resourceMap();
    for (auto resource : clientResources) {
        d->send_app_id(resource->handle, appid);
    }
}

}
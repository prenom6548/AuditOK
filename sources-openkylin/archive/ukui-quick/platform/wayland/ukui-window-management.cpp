/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include "ukui-window-management.h"

#include "wayland-ukui-window-management-client-protocol.h"
#include "wayland-pointer_p.h"

#include <QFutureWatcher>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
namespace UkuiQuick::WaylandClient {
class Q_DECL_HIDDEN UkuiWindowManagement::Private
{
public:
    Private(UkuiWindowManagement *q);
    WaylandPointer<ukui_window_management, ukui_window_management_destroy> wm;
    EventQueue *queue = nullptr;
    bool showingDesktop = false;
    QList<UkuiWindow *> windows;
    UkuiWindow *activeWindow = nullptr;
    QVector<quint32> stackingOrder;
    QVector<QByteArray> stackingOrderUuids;

    void setup(ukui_window_management *wm);

private:
    static void showDesktopCallback(void *data, ukui_window_management *ukui_window_management, uint32_t state);

    static void windowCallback(void *data, struct ukui_window_management *ukui_window_management, const char *uuid);
    static void windowWithUuidCallback(void *data, struct ukui_window_management *ukui_window_management, uint32_t id, const char *uuid);
    static void stackingOrderUuidsCallback(void *data, struct ukui_window_management *ukui_window_management, const char *uuids);
    void setShowDesktop(bool set);
    void windowCreated(ukui_window *id, quint32 internalId, const char *uuid);
    void setStackingOrder(const QVector<quint32> &ids);
    void setStackingOrder(const QVector<QByteArray> &uuids);

    static struct ukui_window_management_listener s_listener;
    UkuiWindowManagement *q;
};

class Q_DECL_HIDDEN UkuiWindow::Private
{
public:
    Private(ukui_window *window, quint32 internalId, const char *uuid, UkuiWindow *q);
    WaylandPointer<ukui_window, ukui_window_destroy> window;
    quint32 internalId; ///< @deprecated
    QByteArray uuid;
    QString title;
    QString appId;
    quint32 desktop = 0;
    bool active = false;
    bool minimized = false;
    bool maximized = false;
    bool fullscreen = false;
    bool keepAbove = false;
    bool keepBelow = false;
    bool onAllDesktops = false;
    bool demandsAttention = false;
    bool closeable = false;
    bool minimizeable = false;
    bool maximizeable = false;
    bool fullscreenable = false;
    bool skipTaskbar = false;
    bool skipSwitcher = false;
    bool shadeable = false;
    bool shaded = false;
    bool movable = false;
    bool resizable = false;
    bool acceptFocus = false;
    bool modality = false;
    bool virtualDesktopChangeable = false;
    QIcon icon;
    UkuiWindowManagement *wm = nullptr;
    bool unmapped = false;
    QPointer<UkuiWindow> parentWindow;
    QMetaObject::Connection parentWindowUnmappedConnection;
    QStringList ukuiVirtualDesktops;
    QStringList ukuiActivities;
    QRect geometry;
    quint32 pid = 0;
    QString applicationMenuServiceName;
    QString applicationMenuObjectPath;
    bool highlight = false;

private:
    static void titleChangedCallback(void *data, ukui_window *window, const char *title);
    static void appIdChangedCallback(void *data, ukui_window *window, const char *app_id);
    static void pidChangedCallback(void *data, ukui_window *window, uint32_t pid);
    static void stateChangedCallback(void *data, ukui_window *window, uint32_t state);
    static void themedIconNameChangedCallback(void *data, ukui_window *window, const char *name);
    static void unmappedCallback(void *data, ukui_window *window);
    static void initialStateCallback(void *data, ukui_window *window);
    static void parentWindowCallback(void *data, ukui_window *window, ukui_window *parent);
    static void windowGeometryCallback(void *data, ukui_window *window, int32_t x, int32_t y, uint32_t width, uint32_t height);
    static void iconChangedCallback(void *data, ukui_window *ukui_window);
    static void virtualDesktopEnteredCallback(void *data, ukui_window *ukui_window, const char *id);
    static void virtualDesktopLeftCallback(void *data, ukui_window *ukui_window, const char *id);
    static void appmenuChangedCallback(void *data, ukui_window *ukui_window, const char *service_name, const char *object_path);
    static void activityEnteredCallback(void *data, ukui_window *ukui_window, const char *id);
    static void activityLeftCallback(void *data, ukui_window *ukui_window, const char *id);
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
    void setVirtualDesktopChangeable(bool set);
    void setAcceptFocus(bool set);
    void setModality(bool set);
    void setParentWindow(UkuiWindow *parentWindow);
    void setPid(const quint32 pid);

    static Private *cast(void *data)
    {
        return reinterpret_cast<Private *>(data);
    }

    UkuiWindow *q;

    static struct ukui_window_listener s_listener;
};

UkuiWindowManagement::Private::Private(UkuiWindowManagement *q)
    : q(q)
{
}

ukui_window_management_listener UkuiWindowManagement::Private::s_listener = {
    .show_desktop_changed = showDesktopCallback,
    .stacking_order_changed = stackingOrderUuidsCallback,
    .window_created = windowCallback,
};

void UkuiWindowManagement::Private::setup(ukui_window_management *windowManagement)
{
    Q_ASSERT(!wm);
    Q_ASSERT(windowManagement);
    wm.setup(windowManagement);
    ukui_window_management_add_listener(windowManagement, &s_listener, this);
}

void UkuiWindowManagement::Private::showDesktopCallback(void *data, ukui_window_management *ukui_window_management, uint32_t state)
{
    auto wm = reinterpret_cast<UkuiWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == ukui_window_management);
    switch (state) {
        case UKUI_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED:
            wm->setShowDesktop(true);
        break;
        case UKUI_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED:
            wm->setShowDesktop(false);
        break;
        default:
            Q_UNREACHABLE();
        break;
    }
}

void UkuiWindowManagement::Private::setShowDesktop(bool set)
{
    if (showingDesktop == set) {
        return;
    }
    showingDesktop = set;
    Q_EMIT q->showingDesktopChanged(showingDesktop);
}

void UkuiWindowManagement::Private::windowCallback(void *data, ukui_window_management *interface, const char *_uuid)
{
    QByteArray uuid(_uuid);
    auto wm = reinterpret_cast<UkuiWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == interface);
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(0);
    QObject::connect(
        timer,
        &QTimer::timeout,
        wm->q,
        [timer, wm, uuid] {
        // internalId 不用了，以uuid为唯一标识
        wm->windowCreated(ukui_window_management_create_window(wm->wm, uuid), 0, uuid);
        timer->deleteLater();
    },
        Qt::QueuedConnection);
    timer->start();
}

// not used
void UkuiWindowManagement::Private::windowWithUuidCallback(void *data, ukui_window_management *interface, uint32_t id, const char *_uuid)
{
    QByteArray uuid(_uuid);
    auto wm = reinterpret_cast<UkuiWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == interface);
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(0);
    QObject::connect(
        timer,
        &QTimer::timeout,
        wm->q,
        [timer, wm, id, uuid] {
        wm->windowCreated(ukui_window_management_create_window(wm->wm, uuid), id, uuid);
        timer->deleteLater();
    },
        Qt::QueuedConnection);
    timer->start();
}

void UkuiWindowManagement::Private::windowCreated(ukui_window *id, quint32 internalId, const char *uuid)
{
    if (queue) {
        queue->addProxy(id);
    }
    UkuiWindow *window = new UkuiWindow(q, id, internalId, uuid);
    window->d->wm = q;
    windows << window;

    const auto windowRemoved = [this, window] {
        windows.removeAll(window);
        if (activeWindow == window) {
            activeWindow = nullptr;
            Q_EMIT q->activeWindowChanged();
        }
    };

    QObject::connect(window, &QObject::destroyed, q, windowRemoved);
    QObject::connect(window, &UkuiWindow::unmapped, q, windowRemoved);
    QObject::connect(window, &UkuiWindow::activeChanged, q, [this, window] {
        if (window->d->unmapped) {
            return;
        }
        if (window->isActive()) {
            if (activeWindow == window) {
                return;
            }
            activeWindow = window;
            Q_EMIT q->activeWindowChanged();
        } else {
            if (activeWindow == window) {
                activeWindow = nullptr;
                Q_EMIT q->activeWindowChanged();
            }
        }
    });
}

void UkuiWindowManagement::Private::stackingOrderUuidsCallback(void *data, ukui_window_management *interface, const char *uuids)
{
    auto wm = reinterpret_cast<UkuiWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == interface);
    wm->setStackingOrder(QByteArray(uuids).split(';').toVector());
}

void UkuiWindowManagement::Private::setStackingOrder(const QVector<quint32> &ids)
{
    if (stackingOrder == ids) {
        return;
    }
    stackingOrder = ids;
    Q_EMIT q->stackingOrderChanged();
}

void UkuiWindowManagement::Private::setStackingOrder(const QVector<QByteArray> &uuids)
{
    if (stackingOrderUuids == uuids) {
        return;
    }
    stackingOrderUuids = uuids;
    Q_EMIT q->stackingOrderUuidsChanged();
}

UkuiWindowManagement::UkuiWindowManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

UkuiWindowManagement::~UkuiWindowManagement()
{
    release();
}

void UkuiWindowManagement::destroy()
{
    if (!d->wm) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->wm.destroy();
}

void UkuiWindowManagement::release()
{
    if (!d->wm) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->wm.release();
}

void UkuiWindowManagement::setup(ukui_window_management *wm)
{
    d->setup(wm);
}

void UkuiWindowManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *UkuiWindowManagement::eventQueue()
{
    return d->queue;
}

bool UkuiWindowManagement::isValid() const
{
    return d->wm.isValid();
}

UkuiWindowManagement::operator ukui_window_management *()
{
    return d->wm;
}

UkuiWindowManagement::operator ukui_window_management *() const
{
    return d->wm;
}

void UkuiWindowManagement::hideDesktop()
{
    setShowingDesktop(false);
}

void UkuiWindowManagement::showDesktop()
{
    setShowingDesktop(true);
}

void UkuiWindowManagement::setShowingDesktop(bool show)
{
    ukui_window_management_show_desktop(d->wm,
                                        show ? UKUI_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED
                                             : UKUI_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED);
}

bool UkuiWindowManagement::isShowingDesktop() const
{
    return d->showingDesktop;
}

QList<UkuiWindow *> UkuiWindowManagement::windows() const
{
    return d->windows;
}

UkuiWindow *UkuiWindowManagement::activeWindow() const
{
    return d->activeWindow;
}

QVector<quint32> UkuiWindowManagement::stackingOrder() const
{
    return d->stackingOrder;
}

QVector<QByteArray> UkuiWindowManagement::stackingOrderUuids() const
{
    return d->stackingOrderUuids;
}

ukui_window_listener UkuiWindow::Private::s_listener = {
    .title_changed = titleChangedCallback,
    .app_id_changed = appIdChangedCallback,
    .state_changed = stateChangedCallback,
    .themed_icon_name_changed = themedIconNameChangedCallback,
    .unmapped = unmappedCallback,
    .initial_state = initialStateCallback,
    .parent_window = parentWindowCallback,
    .geometry = windowGeometryCallback,
    .icon_changed = iconChangedCallback,
    .pid_changed = pidChangedCallback,
    .virtual_desktop_entered = virtualDesktopEnteredCallback,
    .virtual_desktop_left = virtualDesktopLeftCallback,
    .application_menu = appmenuChangedCallback,
    .activity_entered = activityEnteredCallback,
    .activity_left = activityLeftCallback};

void UkuiWindow::Private::appmenuChangedCallback(void *data, ukui_window *window, const char *service_name, const char *object_path)
{
    Q_UNUSED(window)

    Private *p = cast(data);

    p->applicationMenuServiceName = QString::fromUtf8(service_name);
    p->applicationMenuObjectPath = QString::fromUtf8(object_path);

    Q_EMIT p->q->applicationMenuChanged();
}

void UkuiWindow::Private::parentWindowCallback(void *data, ukui_window *window, ukui_window *parent)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const auto windows = p->wm->windows();
    auto it = std::find_if(windows.constBegin(), windows.constEnd(), [parent](const UkuiWindow *w) {
        return *w == parent;
    });
    p->setParentWindow(it != windows.constEnd() ? *it : nullptr);
}

void UkuiWindow::Private::windowGeometryCallback(void *data, ukui_window *window, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    QRect geo(x, y, width, height);
    if (geo == p->geometry) {
        return;
    }
    p->geometry = geo;
    Q_EMIT p->q->geometryChanged();
}

void UkuiWindow::Private::setParentWindow(UkuiWindow *parent)
{
    const auto old = parentWindow;
    QObject::disconnect(parentWindowUnmappedConnection);
    if (parent && !parent->d->unmapped) {
        parentWindow = QPointer<UkuiWindow>(parent);
        parentWindowUnmappedConnection = QObject::connect(parent, &UkuiWindow::unmapped, q, [this] {
            setParentWindow(nullptr);
        });
    } else {
        parentWindow = QPointer<UkuiWindow>();
        parentWindowUnmappedConnection = QMetaObject::Connection();
    }
    if (parentWindow.data() != old.data()) {
        Q_EMIT q->parentWindowChanged();
    }
}

void UkuiWindow::Private::initialStateCallback(void *data, ukui_window *window)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    if (!p->unmapped) {
        Q_EMIT p->wm->windowCreated(p->q);
    }
}

void UkuiWindow::Private::titleChangedCallback(void *data, ukui_window *window, const char *title)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const QString t = QString::fromUtf8(title);
    if (p->title == t) {
        return;
    }
    p->title = t;
    Q_EMIT p->q->titleChanged();
}

void UkuiWindow::Private::appIdChangedCallback(void *data, ukui_window *window, const char *appId)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const QString s = QString::fromUtf8(appId);
    if (s == p->appId) {
        return;
    }
    p->appId = s;
    Q_EMIT p->q->appIdChanged();
}

void UkuiWindow::Private::pidChangedCallback(void *data, ukui_window *window, uint32_t pid)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    if (p->pid == static_cast<quint32>(pid)) {
        return;
    }
    p->pid = pid;
}

void UkuiWindow::Private::unmappedCallback(void *data, ukui_window *window)
{
    auto p = cast(data);
    Q_UNUSED(window);
    p->unmapped = true;
    Q_EMIT p->q->unmapped();
    p->q->deleteLater();
}

void UkuiWindow::Private::virtualDesktopEnteredCallback(void *data, ukui_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->ukuiVirtualDesktops << stringId;
    Q_EMIT p->q->ukuiVirtualDesktopEntered(stringId);
    if (p->ukuiVirtualDesktops.count() == 1) {
        Q_EMIT p->q->onAllDesktopsChanged();
    }
}

void UkuiWindow::Private::virtualDesktopLeftCallback(void *data, ukui_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->ukuiVirtualDesktops.removeAll(stringId);
    Q_EMIT p->q->ukuiVirtualDesktopLeft(stringId);
    if (p->ukuiVirtualDesktops.isEmpty()) {
        Q_EMIT p->q->onAllDesktopsChanged();
    }
}

void UkuiWindow::Private::activityEnteredCallback(void *data, ukui_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->ukuiActivities << stringId;
    Q_EMIT p->q->ukuiActivityEntered(stringId);
}

void UkuiWindow::Private::activityLeftCallback(void *data, ukui_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->ukuiActivities.removeAll(stringId);
    Q_EMIT p->q->ukuiActivityLeft(stringId);
}

void UkuiWindow::Private::stateChangedCallback(void *data, ukui_window *window, uint32_t state)
{
    auto p = cast(data);

    Q_UNUSED(window);
    p->setActive(state & UKUI_WINDOW_STATE_ACTIVE);
    p->setMinimized(state & UKUI_WINDOW_STATE_MINIMIZED);
    p->setMaximized(state & UKUI_WINDOW_STATE_MAXIMIZED);
    p->setFullscreen(state & UKUI_WINDOW_STATE_FULLSCREEN);
    p->setKeepAbove(state & UKUI_WINDOW_STATE_KEEP_ABOVE);
    p->setKeepBelow(state & UKUI_WINDOW_STATE_KEEP_BELOW);
    p->setOnAllDesktops(state & UKUI_WINDOW_STATE_ON_ALL_DESKTOPS);
    p->setDemandsAttention(state & UKUI_WINDOW_STATE_DEMANDS_ATTENTION);
    p->setCloseable(state & UKUI_WINDOW_STATE_CLOSEABLE);
    p->setFullscreenable(state & UKUI_WINDOW_STATE_FULLSCREENABLE);
    p->setMaximizeable(state & UKUI_WINDOW_STATE_MAXIMIZABLE);
    p->setMinimizeable(state & UKUI_WINDOW_STATE_MINIMIZABLE);
    p->setSkipTaskbar(state & UKUI_WINDOW_STATE_SKIPTASKBAR);
    p->setSkipSwitcher(state & UKUI_WINDOW_STATE_SKIPSWITCHER);
    p->setShadeable(state & UKUI_WINDOW_STATE_SHADEABLE);
    p->setShaded(state & UKUI_WINDOW_STATE_SHADED);
    p->setMovable(state & UKUI_WINDOW_STATE_MOVABLE);
    p->setResizable(state & UKUI_WINDOW_STATE_RESIZABLE);
    p->setVirtualDesktopChangeable(state & UKUI_WINDOW_STATE_VIRTUAL_DESKTOP_CHANGEABLE);
}

void UkuiWindow::Private::themedIconNameChangedCallback(void *data, ukui_window *window, const char *name)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString themedName = QString::fromUtf8(name);
    if (!themedName.isEmpty()) {
        QIcon icon = QIcon::fromTheme(themedName);
        p->icon = icon;
    } else {
        p->icon = QIcon();
    }
    Q_EMIT p->q->iconChanged();
}

static int readData(int fd, QByteArray &data)
{
    char buf[4096];
    int retryCount = 0;
    int n;
    while (true) {
        n = read(fd, buf, 4096);
        if (n == -1 && (errno == EAGAIN) && ++retryCount < 1000) {
            usleep(1000);
        } else {
            break;
        }
    }
    if (n > 0) {
        data.append(buf, n);
        n = readData(fd, data);
    }
    return n;
}

void UkuiWindow::Private::iconChangedCallback(void *data, ukui_window *window)
{
    auto p = cast(data);
    Q_UNUSED(window);
    int pipeFds[2];
    if (pipe2(pipeFds, O_CLOEXEC | O_NONBLOCK) != 0) {
        return;
    }
    ukui_window_get_icon(p->window, pipeFds[1]);
    close(pipeFds[1]);
    const int pipeFd = pipeFds[0];
    auto readIcon = [pipeFd]() -> QIcon {
        QByteArray content;
        if (readData(pipeFd, content) != 0) {
            close(pipeFd);
            return QIcon();
        }
        close(pipeFd);
        if (qgetenv("XDG_SESSION_DESKTOP") == QString("kylin-wlcom")) {
            quint32 width, height;
            memcpy(&width, content.constData(), sizeof(width));
            memcpy(&height, content.constData() + sizeof(width), sizeof(height));

            if (content.size() - 8 != width * height * 4) {
                return QIcon();
            }
            QImage image(width, height, QImage::Format_ARGB32);
            if (!image.bits()) {
                return QIcon();
            }
            memcpy(image.bits(), content.constData() + 8, content.size() - 8);
            QIcon icon = QIcon(QPixmap::fromImage(image));
            return icon;
        } else {
            QDataStream ds(content);
            QIcon icon;
            ds >> icon;
            return icon;
        }
    };
    QFutureWatcher<QIcon> *watcher = new QFutureWatcher<QIcon>(p->q);
    QObject::connect(watcher, &QFutureWatcher<QIcon>::finished, p->q, [p, watcher] {
        watcher->deleteLater();
        QIcon icon = watcher->result();
        if (!icon.isNull()) {
            p->icon = icon;
        } else {
            p->icon = QIcon::fromTheme(QStringLiteral("wayland"));
        }
        Q_EMIT p->q->iconChanged();
    });
    watcher->setFuture(QtConcurrent::run(readIcon));
}

void UkuiWindow::Private::setActive(bool set)
{
    if (active == set) {
        return;
    }
    active = set;
    Q_EMIT q->activeChanged();
}

void UkuiWindow::Private::setFullscreen(bool set)
{
    if (fullscreen == set) {
        return;
    }
    fullscreen = set;
    Q_EMIT q->fullscreenChanged();
}

void UkuiWindow::Private::setKeepAbove(bool set)
{
    if (keepAbove == set) {
        return;
    }
    keepAbove = set;
    Q_EMIT q->keepAboveChanged();
}

void UkuiWindow::Private::setKeepBelow(bool set)
{
    if (keepBelow == set) {
        return;
    }
    keepBelow = set;
    Q_EMIT q->keepBelowChanged();
}

void UkuiWindow::Private::setMaximized(bool set)
{
    if (maximized == set) {
        return;
    }
    maximized = set;
    Q_EMIT q->maximizedChanged();
}

void UkuiWindow::Private::setMinimized(bool set)
{
    if (minimized == set) {
        return;
    }
    minimized = set;
    Q_EMIT q->minimizedChanged();
}

void UkuiWindow::Private::setOnAllDesktops(bool set)
{
    if (onAllDesktops == set) {
        return;
    }
    onAllDesktops = set;
    Q_EMIT q->onAllDesktopsChanged();
}

void UkuiWindow::Private::setDemandsAttention(bool set)
{
    if (demandsAttention == set) {
        return;
    }
    demandsAttention = set;
    Q_EMIT q->demandsAttentionChanged();
}

void UkuiWindow::Private::setCloseable(bool set)
{
    if (closeable == set) {
        return;
    }
    closeable = set;
    Q_EMIT q->closeableChanged();
}

void UkuiWindow::Private::setFullscreenable(bool set)
{
    if (fullscreenable == set) {
        return;
    }
    fullscreenable = set;
    Q_EMIT q->fullscreenableChanged();
}

void UkuiWindow::Private::setMaximizeable(bool set)
{
    if (maximizeable == set) {
        return;
    }
    maximizeable = set;
    Q_EMIT q->maximizeableChanged();
}

void UkuiWindow::Private::setMinimizeable(bool set)
{
    if (minimizeable == set) {
        return;
    }
    minimizeable = set;
    Q_EMIT q->minimizeableChanged();
}

void UkuiWindow::Private::setSkipTaskbar(bool skip)
{
    if (skipTaskbar == skip) {
        return;
    }
    skipTaskbar = skip;
    Q_EMIT q->skipTaskbarChanged();
}

void UkuiWindow::Private::setSkipSwitcher(bool skip)
{
    if (skipSwitcher == skip) {
        return;
    }
    skipSwitcher = skip;
    Q_EMIT q->skipSwitcherChanged();
}

void UkuiWindow::Private::setShadeable(bool set)
{
    if (shadeable == set) {
        return;
    }
    shadeable = set;
    Q_EMIT q->shadeableChanged();
}

void UkuiWindow::Private::setShaded(bool set)
{
    if (shaded == set) {
        return;
    }
    shaded = set;
    Q_EMIT q->shadedChanged();
}

void UkuiWindow::Private::setMovable(bool set)
{
    if (movable == set) {
        return;
    }
    movable = set;
    Q_EMIT q->movableChanged();
}

void UkuiWindow::Private::setResizable(bool set)
{
    if (resizable == set) {
        return;
    }
    resizable = set;
    Q_EMIT q->resizableChanged();
}

void UkuiWindow::Private::setVirtualDesktopChangeable(bool set)
{
    if (virtualDesktopChangeable == set) {
        return;
    }
    virtualDesktopChangeable = set;
    Q_EMIT q->virtualDesktopChangeableChanged();
}

void UkuiWindow::Private::setAcceptFocus(bool set)
{
    if (acceptFocus == set) {
        return;
    }
    acceptFocus = set;
    Q_EMIT q->acceptFocusChanged();
}

void UkuiWindow::Private::setModality(bool set)
{
    if (modality == set) {
        return;
    }
    modality = set;
    Q_EMIT q->modalityChanged();
}

UkuiWindow::Private::Private(ukui_window *w, quint32 internalId, const char *uuid, UkuiWindow *q)
    : internalId(internalId)
    , uuid(uuid)
    , q(q)
{
    Q_ASSERT(!this->uuid.isEmpty());
    window.setup(w);
    ukui_window_add_listener(w, &s_listener, this);
}

UkuiWindow::UkuiWindow(UkuiWindowManagement *parent, ukui_window *window, quint32 internalId, const char *uuid)
    : QObject(parent)
    , d(new Private(window, internalId, uuid, this))
{
}

UkuiWindow::~UkuiWindow()
{
    release();
}

void UkuiWindow::destroy()
{
    d->window.destroy();
}

void UkuiWindow::release()
{
    d->window.release();
}

bool UkuiWindow::isValid() const
{
    return d->window.isValid();
}

UkuiWindow::operator ukui_window *() const
{
    return d->window;
}

UkuiWindow::operator ukui_window *()
{
    return d->window;
}

QString UkuiWindow::appId() const
{
    return d->appId;
}

quint32 UkuiWindow::pid() const
{
    return d->pid;
}

QString UkuiWindow::title() const
{
    return d->title;
}

quint32 UkuiWindow::virtualDesktop() const
{
    return d->desktop;
}

bool UkuiWindow::isActive() const
{
    return d->active;
}

bool UkuiWindow::isFullscreen() const
{
    return d->fullscreen;
}

bool UkuiWindow::isKeepAbove() const
{
    return d->keepAbove;
}

bool UkuiWindow::isKeepBelow() const
{
    return d->keepBelow;
}

bool UkuiWindow::isMaximized() const
{
    return d->maximized;
}

bool UkuiWindow::isMinimized() const
{
    return d->minimized;
}

bool UkuiWindow::isOnAllDesktops() const
{
    if (ukui_window_get_version(d->window) < 8) {
        return d->onAllDesktops;
    } else {
        return d->ukuiVirtualDesktops.isEmpty();
    }
}

bool UkuiWindow::isDemandingAttention() const
{
    return d->demandsAttention;
}

bool UkuiWindow::isCloseable() const
{
    return d->closeable;
}

bool UkuiWindow::isFullscreenable() const
{
    return d->fullscreenable;
}

bool UkuiWindow::isMaximizeable() const
{
    return d->maximizeable;
}

bool UkuiWindow::isMinimizeable() const
{
    return d->minimizeable;
}

bool UkuiWindow::skipTaskbar() const
{
    return d->skipTaskbar;
}

bool UkuiWindow::skipSwitcher() const
{
    return d->skipSwitcher;
}

QIcon UkuiWindow::icon() const
{
    return d->icon;
}

bool UkuiWindow::isShadeable() const
{
    return d->shadeable;
}

bool UkuiWindow::isShaded() const
{
    return d->shaded;
}

bool UkuiWindow::isResizable() const
{
    return d->resizable;
}

bool UkuiWindow::isMovable() const
{
    return d->movable;
}

bool UkuiWindow::isVirtualDesktopChangeable() const
{
    return d->virtualDesktopChangeable;
}

QString UkuiWindow::applicationMenuObjectPath() const
{
    return d->applicationMenuObjectPath;
}

QString UkuiWindow::applicationMenuServiceName() const
{
    return d->applicationMenuServiceName;
}

void UkuiWindow::requestActivate()
{
    ukui_window_set_state(d->window, UKUI_WINDOW_STATE_ACTIVE, UKUI_WINDOW_STATE_ACTIVE);
}

void UkuiWindow::requestClose()
{
    ukui_window_close(d->window);
}

void UkuiWindow::requestMove()
{
    ukui_window_request_move(d->window);
}

void UkuiWindow::requestResize()
{
    ukui_window_request_resize(d->window);
}

void UkuiWindow::requestVirtualDesktop(quint32 desktop)
{
    // not used
}

void UkuiWindow::requestToggleKeepAbove()
{
    if (d->keepAbove) {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_KEEP_ABOVE, 0);
    } else {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_KEEP_ABOVE, UKUI_WINDOW_STATE_KEEP_ABOVE);
    }
}

void UkuiWindow::requestDemandAttention()
{
    if (d->demandsAttention) {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_DEMANDS_ATTENTION, 0);
    } else {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_DEMANDS_ATTENTION, UKUI_WINDOW_STATE_DEMANDS_ATTENTION);
    }
}

void UkuiWindow::requestToggleKeepBelow()
{
    if (d->keepBelow) {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_KEEP_BELOW, 0);
    } else {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_KEEP_BELOW, UKUI_WINDOW_STATE_KEEP_BELOW);
    }
}

void UkuiWindow::requestToggleMinimized()
{
    if (d->minimized) {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_MINIMIZED, 0);
    } else {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_MINIMIZED, UKUI_WINDOW_STATE_MINIMIZED);
    }
}

void UkuiWindow::requestToggleMaximized()
{
    if (d->maximized) {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_MAXIMIZED, 0);
    } else {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_MAXIMIZED, UKUI_WINDOW_STATE_MAXIMIZED);
    }
}

void UkuiWindow::setStartupGeometry(Surface *surface, const QRect &geometry)
{
    ukui_window_set_startup_geometry(d->window, *surface, geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void UkuiWindow::setMinimizedGeometry(Surface *panel, const QRect &geom)
{
    ukui_window_set_minimized_geometry(d->window, *panel, geom.x(), geom.y(), geom.width(), geom.height());
}

void UkuiWindow::unsetMinimizedGeometry(Surface *panel)
{
    ukui_window_unset_minimized_geometry(d->window, *panel);
}

void UkuiWindow::requestToggleShaded()
{
    if (d->shaded) {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_SHADED, 0);
    } else {
        ukui_window_set_state(d->window, UKUI_WINDOW_STATE_SHADED, UKUI_WINDOW_STATE_SHADED);
    }
}

quint32 UkuiWindow::internalId() const
{
    return d->internalId;
}

QByteArray UkuiWindow::uuid() const
{
    return d->uuid;
}

QPointer<UkuiWindow> UkuiWindow::parentWindow() const
{
    return d->parentWindow;
}

QRect UkuiWindow::geometry() const
{
    return d->geometry;
}

void UkuiWindow::requestEnterVirtualDesktop(const QString &id)
{
    ukui_window_request_enter_virtual_desktop(d->window, id.toUtf8());
}

void UkuiWindow::requestEnterNewVirtualDesktop()
{
    ukui_window_request_enter_new_virtual_desktop(d->window);
}

void UkuiWindow::requestLeaveVirtualDesktop(const QString &id)
{
    ukui_window_request_leave_virtual_desktop(d->window, id.toUtf8());
}

QStringList UkuiWindow::ukuiVirtualDesktops() const
{
    return d->ukuiVirtualDesktops;
}

void UkuiWindow::requestEnterActivity(const QString &id)
{
    ukui_window_request_enter_activity(d->window, id.toUtf8());
}

void UkuiWindow::requestLeaveActivity(const QString &id)
{
    ukui_window_request_leave_activity(d->window, id.toUtf8());
}

QStringList UkuiWindow::ukuiActivities() const
{
    return d->ukuiActivities;
}

void UkuiWindow::sendToOutput(KWayland::Client::Output *output) const
{
    if (ukui_window_get_version(d->window) >= UKUI_WINDOW_SEND_TO_OUTPUT_SINCE_VERSION) {
        ukui_window_send_to_output(d->window, *output);
    }
}

void UkuiWindow::setHighlight()
{
    if (d->highlight)
        return;

    ukui_window_highlight(d->window);
    d->highlight = true;
}

void UkuiWindow::unsetHightlight()
{
    if (!d->highlight)
        return;

    ukui_window_unset_highlight(d->window);
    d->highlight = false;
}

bool UkuiWindow::isHighlight()
{
    return d->highlight;
}
}
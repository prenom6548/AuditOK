/*
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2009 Matthieu Gallien <matthieu_gallien@yahoo.fr>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "status-notifier-item.h"
#include "status-notifier-item-private.h"

#include <qt5xdg/XdgIcon>
#include <netinet/in.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#include <QDebug>
#include <QThread>
#include <QFile>

using namespace UkuiSni;

UkuiDBusMenuImporter::UkuiDBusMenuImporter(const QString &service, const QString &path, QObject *parent)
    :DBusMenuImporter(service, path, parent)
{
}

QIcon UkuiDBusMenuImporter::iconForName(const QString &iconName)
{
    return QIcon::fromTheme(iconName);
}

StatusNotifierItemPrivate::StatusNotifierItemPrivate(const QString &service, StatusNotifierItem *q, QObject *parent)
        : QObject(parent)
        , m_refreshing(false)
        , m_needsReRefreshing(false)
        , q(q)
{
    m_serviceNamePath = service;
}

void StatusNotifierItemPrivate::init()
{
    qDBusRegisterMetaType<DbusImageStruct>();
    qDBusRegisterMetaType<DbusImageVector>();
    qDBusRegisterMetaType<DbusToolTipStruct>();
    //service = service name + path(/StatusNotifierItem)
    int slash = m_serviceNamePath.indexOf('/');
    QString path;
    if (slash == -1) {
        m_serviceName = m_serviceNamePath;
        path = QStringLiteral("/StatusNotifierItem");
    } else {
        m_serviceName = m_serviceNamePath.left(slash);
        path = m_serviceNamePath.mid(slash);
    }
    m_display = displayFormPid(m_serviceNamePath.section("-", 1, 1).toInt());

    m_statusNotifierItemInterface = new org::kde::StatusNotifierItem(m_serviceName, path, QDBusConnection::sessionBus());

    m_refreshTimer.setSingleShot(true);
    m_refreshTimer.setInterval(10);
    connect(&m_refreshTimer, &QTimer::timeout, this, &StatusNotifierItemPrivate::performRefresh);

    if (!m_serviceName.isEmpty() && m_statusNotifierItemInterface->isValid()) {
        connect(m_statusNotifierItemInterface, &OrgKdeStatusNotifierItem::NewTitle, this, &StatusNotifierItemPrivate::refresh);
        connect(m_statusNotifierItemInterface, &OrgKdeStatusNotifierItem::NewIcon, this, &StatusNotifierItemPrivate::refresh);
        connect(m_statusNotifierItemInterface, &OrgKdeStatusNotifierItem::NewAttentionIcon, this, &StatusNotifierItemPrivate::refresh);
        connect(m_statusNotifierItemInterface, &OrgKdeStatusNotifierItem::NewOverlayIcon, this, &StatusNotifierItemPrivate::refresh);
        connect(m_statusNotifierItemInterface, &OrgKdeStatusNotifierItem::NewToolTip, this, &StatusNotifierItemPrivate::refresh);
        connect(m_statusNotifierItemInterface, &OrgKdeStatusNotifierItem::NewStatus, this, &StatusNotifierItemPrivate::syncStatus);
        connect(m_statusNotifierItemInterface, &OrgKdeStatusNotifierItem::NewMenu, this, &StatusNotifierItemPrivate::refreshMenu);
        refresh();
    } else {
        qWarning() << "Invalid notifierItemService:" << m_serviceNamePath;
        m_valid = false;
    }
}

void StatusNotifierItemPrivate::contextMenu(int x, int y)
{
    if (m_menuImporter) {
        m_menuImporter->updateMenu();
        m_pendingMenuToShow ++;
    } else {
        qWarning() << "DBusMenu interface not available, falling back to calling ContextMenu()";
        if (m_statusNotifierItemInterface && m_statusNotifierItemInterface->isValid()) {
            m_statusNotifierItemInterface->call(QDBus::NoBlock, QStringLiteral("ContextMenu"), x, y);
        }
    }
}

void StatusNotifierItemPrivate::activate(int x, int y)
{
    if (m_statusNotifierItemInterface && m_statusNotifierItemInterface->isValid()) {
        QDBusMessage message = QDBusMessage::createMethodCall(m_statusNotifierItemInterface->service(),
                                                              m_statusNotifierItemInterface->path(),
                                                              m_statusNotifierItemInterface->interface(),
                                                              QStringLiteral("Activate"));

        message << x << y;
        QDBusPendingCall call = m_statusNotifierItemInterface->connection().asyncCall(message);
        auto *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &StatusNotifierItemPrivate::activateCallback);
    }
}

void StatusNotifierItemPrivate::refresh()
{
    if (!m_refreshTimer.isActive()) {
        m_refreshTimer.start();
    }
}

void StatusNotifierItemPrivate::performRefresh()
{
    if (m_refreshing) {
        m_needsReRefreshing = true;
        return;
    }

    m_refreshing = true;
    QDBusMessage message = QDBusMessage::createMethodCall(m_statusNotifierItemInterface->service(),
                                                      m_statusNotifierItemInterface->path(),
                                                      QStringLiteral("org.freedesktop.DBus.Properties"),
                                                      QStringLiteral("GetAll"));

    message << m_statusNotifierItemInterface->interface();
    QDBusPendingCall call = m_statusNotifierItemInterface->connection().asyncCall(message);
    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &StatusNotifierItemPrivate::refreshCallback);
}

void StatusNotifierItemPrivate::syncStatus(const QString &status)
{
    m_status = status;
    Q_EMIT q->dataUpdated();
}

void StatusNotifierItemPrivate::refreshMenu()
{
    if (m_menuImporter) {
        delete m_menuImporter;
        m_menuImporter = nullptr;
    }
    refresh();
}

void StatusNotifierItemPrivate::refreshCallback(QDBusPendingCallWatcher *call)
{
    m_refreshing = false;
    if (m_needsReRefreshing) {
        m_needsReRefreshing = false;
        performRefresh();
        call->deleteLater();
        return;
    }

    QDBusPendingReply<QVariantMap> reply = *call;
    if (reply.isError()) {
        m_valid = false;
        qDebug() << "Fail to refresh" << m_serviceName;
    } else {
        QVariantMap properties = reply.argumentAt<0>();
        m_category = properties[QStringLiteral("Category")].toString();
        m_id = properties[QStringLiteral("Id")].toString();
        m_title = properties[QStringLiteral("Title")].toString();
        m_status = properties[QStringLiteral("Status")].toString();
        m_windowId = properties[QStringLiteral("WindowId")].toString();
        m_itemIsMenu = properties[QStringLiteral("ItemIsMenu")].toBool();
        m_attentionMovieName = properties[QStringLiteral("AttentionMovieName")].toString();
        m_iconThemePath = properties[QStringLiteral("IconThemePath")].toString();

        //TODO support overlayIcon
        m_overlayIcon = loadIcon(properties, m_overlayIconName, QStringLiteral("OverlayIconPixmap"));
        m_overlayIconName = properties[QStringLiteral("OverlayIconName")].toString();

        m_iconName = properties[QStringLiteral("IconName")].toString();
        m_icon = loadIcon(properties, m_iconName, QStringLiteral("IconPixmap"));
        m_attentionIconName = properties[QStringLiteral("AttentionIconName")].toString();
        m_attentionIcon = loadIcon(properties, m_attentionIconName, QStringLiteral("AttentionIconPixmap"));

        DbusToolTipStruct toolTip;
        properties[QStringLiteral("ToolTip")].value<QDBusArgument>() >> toolTip;
        if (toolTip.title.isEmpty()) {
            m_toolTipTitle = QString();
            m_toolTipSubTitle = QString();
            m_toolTipIcon = QIcon();
        } else {
            QIcon toolTipIcon;
            if (toolTip.image.empty()) {
                toolTipIcon = QIcon::fromTheme(toolTip.icon);
            } else {
                toolTipIcon = imageVectorToPixmap(toolTip.image);
            }
            m_toolTipTitle = toolTip.title;
            m_toolTipSubTitle = toolTip.subTitle;
            if (toolTipIcon.isNull() || toolTipIcon.availableSizes().isEmpty()) {
                m_toolTipIcon = QIcon();
            } else {
                m_toolTipIcon = toolTipIcon;
            }
        }
        // Menu
        if (!m_menuImporter) {
            QString menuObjectPath = properties[QStringLiteral("Menu")].value<QDBusObjectPath>().path();
            if (!menuObjectPath.isEmpty()) {
                if (menuObjectPath == QLatin1String("/NO_DBUSMENU")) {
                    qWarning() << "DBusMenu disabled for this item" << m_serviceNamePath;
                } else {
                    m_menuImporter = new UkuiDBusMenuImporter(m_statusNotifierItemInterface->service(), menuObjectPath, this);
                    connect(m_menuImporter, &UkuiDBusMenuImporter::menuUpdated, this, [this](QMenu *menu) {
                        if(m_pendingMenuToShow && menu == m_menuImporter->menu()) {
                            --m_pendingMenuToShow;
                            Q_EMIT q->contextMenuReady(m_menuImporter->menu());
                        }
                    });

                    if (m_menuImporter->menu()) {
                        connect(m_menuImporter->menu(), &QMenu::aboutToShow, this, [this] {
                             Q_EMIT q->menuStateChange(true);
                        });
                        connect(m_menuImporter->menu(), &QMenu::aboutToHide, this, [this] {
                             Q_EMIT q->menuStateChange(false);
                        });
                    }
                }
            }
        }
    }
    Q_EMIT q->dataUpdated();
    call->deleteLater();
}

QIcon StatusNotifierItemPrivate::loadIcon(const QVariantMap &properties, const QString &iconName, const QString &pixmapKey)
{
    if(!m_iconThemePath.isEmpty()) {
        auto list = QIcon::fallbackSearchPaths();
        list.append(m_iconThemePath);
        list.removeDuplicates();
        QIcon::setFallbackSearchPaths(list);
    }

    if(!iconName.isEmpty()) {
        QIcon icon;
        if(iconName.startsWith("/")) {
            if(QFile::exists(iconName)) {
                icon = QIcon(iconName);
            }
        } else {
            icon = QIcon::fromTheme(iconName);
            if (icon.isNull()) {
                icon = XdgIcon::fromTheme(iconName);
            }
        }
        if(!icon.isNull()) {
            return icon;
        }
    }

    DbusImageVector image;
    properties[pixmapKey].value<QDBusArgument>() >> image;
    if(!image.isEmpty()) {
        QIcon icon = imageVectorToPixmap(image);
        if(!icon.isNull()) {
            return icon;
        }
    }
    return {};
}

QPixmap StatusNotifierItemPrivate::DbusImageStructToPixmap(const DbusImageStruct &image) const
{
    // swap from network byte order if we are little endian
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        uint *uintBuf = (uint *)image.data.data();
        for (uint i = 0; i < image.data.size() / sizeof(uint); ++i) {
            *uintBuf = ntohl(*uintBuf);
            ++uintBuf;
        }
    }
    if (image.width == 0 || image.height == 0) {
        return {};
    }

    // avoid a deep copy of the image data
    // we need to keep a reference to the image.data alive for the lifespan of the image, even if the image is copied
    // we create a new QByteArray with a shallow copy of the original data on the heap, then delete this in the QImage cleanup
    auto dataRef = new QByteArray(image.data);

    QImage iconImage(
            reinterpret_cast<const uchar *>(dataRef->data()),
            image.width,
            image.height,
            QImage::Format_ARGB32,
            [](void *ptr) {
                delete static_cast<QByteArray *>(ptr);
            },
            dataRef);
    return QPixmap::fromImage(iconImage);
}

QIcon StatusNotifierItemPrivate::imageVectorToPixmap(const DbusImageVector &vector) const
{
    QIcon icon;
    for (const auto & i : vector) {
        icon.addPixmap(DbusImageStructToPixmap(i));
    }
    return icon;
}

void StatusNotifierItemPrivate::activateCallback(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<void> reply = *call;
    Q_EMIT q->activateResult(!reply.isError());
    call->deleteLater();
}

void StatusNotifierItemPrivate::secondaryActivate(int x, int y)
{
    if (m_statusNotifierItemInterface && m_statusNotifierItemInterface->isValid()) {
        m_statusNotifierItemInterface->call(QDBus::NoBlock, QStringLiteral("SecondaryActivate"), x, y);
    }
}

void StatusNotifierItemPrivate::scroll(int delta, const QString &direction)
{
    if (m_statusNotifierItemInterface && m_statusNotifierItemInterface->isValid()) {
        m_statusNotifierItemInterface->call(QDBus::NoBlock, QStringLiteral("Scroll"), delta, direction);
    }
}

void StatusNotifierItemPrivate::provideXdgActivationToken(const QString &token)
{
    if (m_statusNotifierItemInterface && m_statusNotifierItemInterface->isValid()) {
        m_statusNotifierItemInterface->ProvideXdgActivationToken(token);
    }
}

QString StatusNotifierItemPrivate::displayFormPid(int pid)
{
    QFile environFile(QStringLiteral("/proc/%1/environ").arg(QString::number(pid)));
    if (environFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray DISPLAY  = QX11Info::isPlatformX11()? QByteArrayLiteral("DISPLAY") : QByteArrayLiteral("WAYLAND_DISPLAY");
        const auto lines = environFile.readAll().split('\0');
        for (const QByteArray &line : lines) {
            const int equalsIdx = line.indexOf('=');
            if (equalsIdx <= 0) {
                continue;
            }
            const QByteArray key = line.left(equalsIdx);
            if (key == DISPLAY) {
                const QByteArray value = line.mid(equalsIdx + 1);
                return value;
            }
        }
    }
    return {};
}

StatusNotifierItem::StatusNotifierItem(QObject *parent) : QObject(parent)
        ,d(new StatusNotifierItemPrivate({}, this, this))
{
}

StatusNotifierItem::StatusNotifierItem(const QString &service, QObject *parent): QObject(parent)
        ,d(new StatusNotifierItemPrivate(service, this, this))
{
    d->init();
}

void StatusNotifierItem::setServiceName(const QString &service)
{
    if(d) {
        delete d;
    }
    d = new StatusNotifierItemPrivate(service, this, this);
    d->init();
}

void StatusNotifierItem::setPid(const uint &pid)
{
    d->m_pid = pid;
}

void StatusNotifierItem::contextMenu(int x, int y)
{
    d->contextMenu(x, y);
}

void StatusNotifierItem::activate(int x, int y)
{
    d->activate(x, y);
}

void StatusNotifierItem::secondaryActivate(int x, int y)
{
    d->secondaryActivate(x, y);
}

void StatusNotifierItem::scroll(int delta, const QString &direction)
{
    d->scroll(delta, direction);
}

void StatusNotifierItem::provideXdgActivationToken(const QString &token)
{
    d->provideXdgActivationToken(token);
}

QString StatusNotifierItem::service()
{
    return d->m_serviceName;
}

QIcon StatusNotifierItem::attentionIcon() const
{
    return d->m_attentionIcon;
}

QString StatusNotifierItem::attentionIconName() const
{
    return d->m_attentionIconName;
}

QString StatusNotifierItem::attentionMovieName() const
{
    return d->m_attentionMovieName;
}

QString StatusNotifierItem::category() const
{
    return d->m_category;
}

QIcon StatusNotifierItem::icon() const
{
    return d->m_icon;
}

QString StatusNotifierItem::iconName() const
{
    return d->m_iconName;
}

QString StatusNotifierItem::id() const
{
    return d->m_id;
}

bool StatusNotifierItem::itemIsMenu() const
{
    return d->m_itemIsMenu;
}

QIcon StatusNotifierItem::overlayIcon() const
{
    return d->m_overlayIcon;
}

QString StatusNotifierItem::overlayIconName() const
{
    return d->m_overlayIconName;
}

QString StatusNotifierItem::status() const
{
    return d->m_status;
}

QString StatusNotifierItem::title() const
{
    return d->m_title;
}

QVariant StatusNotifierItem::toolTipIcon() const
{
    return d->m_toolTipIcon;
}

QString StatusNotifierItem::toolTipSubTitle() const
{
    return d->m_toolTipSubTitle;
}

QString StatusNotifierItem::toolTipTitle() const
{
    return d->m_toolTipTitle;
}

QString StatusNotifierItem::windowId() const
{
    return d->m_windowId;
}

QString StatusNotifierItem::display() const
{
    return d->m_display;
}

uint StatusNotifierItem::pid() const
{
    return d->m_pid;
}

QString StatusNotifierItem::iconThemePath() const
{
    return d->m_iconThemePath;
}

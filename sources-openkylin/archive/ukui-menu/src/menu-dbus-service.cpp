/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "menu-dbus-service.h"
#include "menuadaptor.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDBusConnectionInterface>
#include <QGuiApplication>

#define MENU_CORE_DBUS_SERVICE          "org.ukui.menu"
#define MENU_CORE_DBUS_PATH             "/org/ukui/menu"
#define MENU_CORE_DBUS_INTERFACE        "org.ukui.menu"

using namespace UkuiMenu;

// GCOVR_EXCL_START
QString MenuDbusService::displayFromPid(uint pid)
{
    QFile environFile(QStringLiteral("/proc/%1/environ").arg(QString::number(pid)));
    if (environFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray DISPLAY = QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))
                ? QByteArrayLiteral("WAYLAND_DISPLAY") : QByteArrayLiteral("DISPLAY");
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

MenuDbusService::MenuDbusService(const QString &display, QObject *parent) : QObject(parent), m_display(display)
{
    bool isServiceRegistered = QDBusConnection::sessionBus().interface()->isServiceRegistered(MENU_CORE_DBUS_SERVICE);
    qDebug() << "menu service is registered:" << isServiceRegistered << ", display:" << display;

    if (isServiceRegistered) {
        initWatcher();

    } else {
        bool success = registerService();
        if (!success) {
            initWatcher();
        }
        qDebug() << "menu service register:" << success;
    }
}

void MenuDbusService::WinKeyResponse()
{
    uint callerPid = QDBusConnection::sessionBus().interface()->servicePid(message().service()).value();
    QString display = MenuDbusService::displayFromPid(callerPid);

    active(display);
}
// GCOVR_EXCL_STOP

void MenuDbusService::active(const QString &display)
{
    if (display.isEmpty() || (display == m_display)) {
        Q_EMIT menuActive();
        return;
    }

    Q_EMIT activeRequest(display);
}

bool MenuDbusService::visible() const
{
    return m_visible;
}

void MenuDbusService::setVisible(bool visible)
{
    if (m_visible == visible) return;
    m_visible = visible;
    sendPropertiesChangedSignal("Visible", m_visible);
}

void MenuDbusService::sendPropertiesChangedSignal(const QString &propName, const QVariant &propValue)
{
    QVariantMap changedProps;
    changedProps.insert(propName, propValue);

    QDBusMessage signal = QDBusMessage::createSignal("/org/ukui/menu",
                                                     "org.freedesktop.DBus.Properties",
                                                     "PropertiesChanged");
    signal << "org.ukui.menu" << changedProps << QStringList();
    QDBusConnection::sessionBus().send(signal);
}

void MenuDbusService::activeMenu(QString display)
{
    if (display == m_display) {
        Q_EMIT menuActive();
    }
}

// GCOVR_EXCL_START
bool MenuDbusService::registerService()
{
    new MenuAdaptor(this);

    if (!QDBusConnection::sessionBus().registerObject(MENU_CORE_DBUS_PATH, this)) {
        qWarning() << "MenuDbusService : register dbus object failed .";
        return false;
    }

    QDBusReply<QDBusConnectionInterface::RegisterServiceReply> reply =
        QDBusConnection::sessionBus().interface()->registerService(MENU_CORE_DBUS_SERVICE,
                                                                   QDBusConnectionInterface::ReplaceExistingService,
                                                                   QDBusConnectionInterface::DontAllowReplacement);

    if (reply.value() == QDBusConnectionInterface::ServiceNotRegistered) {
        qWarning() << "MenuDbusService : register dbus service failed .";
        return false;
    }

    return true;
}

void MenuDbusService::onServiceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    qDebug() << "serviceOwnerChanged:" << service << oldOwner << newOwner;
    if (newOwner.isEmpty()) {
        bool success = registerService();
        if (success) {
            disConnectActiveRequest();
            delete m_watcher;
            m_watcher = nullptr;
        }
        qDebug() << "try to register service:" << success;
        return;
    }

    uint newOwnerPid = QDBusConnection::sessionBus().interface()->servicePid(newOwner);
    qDebug() << "newOwnerPid:" << newOwnerPid << ", myPid:" << QCoreApplication::applicationPid() << ", display:" << m_display;
//        if (newOwnerPid == QCoreApplication::applicationPid()) {
//            qDebug() << "Becoming a new service";
//        }
}

void MenuDbusService::connectToActiveRequest()
{
    QDBusConnection::sessionBus().connect(MENU_CORE_DBUS_SERVICE,
                                          MENU_CORE_DBUS_PATH,
                                          MENU_CORE_DBUS_INTERFACE,
                                          "activeRequest",
                                          this,
                                          SLOT(activeMenu(QString)));
}

void MenuDbusService::disConnectActiveRequest()
{
    QDBusConnection::sessionBus().disconnect(MENU_CORE_DBUS_SERVICE,
                                             MENU_CORE_DBUS_PATH,
                                             MENU_CORE_DBUS_INTERFACE,
                                             "activeRequest",
                                             this,
                                             SLOT(activeMenu(QString)));
}

void MenuDbusService::initWatcher()
{
    m_watcher = new QDBusServiceWatcher(MENU_CORE_DBUS_SERVICE,QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange);
    connect(m_watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &MenuDbusService::onServiceOwnerChanged);
    connectToActiveRequest();
}
// GCOVR_EXCL_STOP

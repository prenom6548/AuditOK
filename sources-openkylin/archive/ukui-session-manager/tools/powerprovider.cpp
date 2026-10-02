/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 * Copyright: Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
 *            2012 Razor team
 * Authors:
 *   Christian Surlykke <christian@surlykke.dk>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */
#include "powerprovider.h"

#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusPendingCall>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QDebug>

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QObject>
#include <QDBusReply>
#include <QSharedPointer>
#include <QTimer>
// #include "loginedusers.h"

#define LIGHTDM_SERVICE "org.freedesktop.DisplayManager"
#define LIGHTDM_PATH "/org/freedesktop/DisplayManager"
#define LIGTHDM_INTERFACE "org.freedesktop.DisplayManager.Seat"
#define LIGHTDM_SESSION_INTERFACE "org.freedesktop.DisplayManager.Session"

#define SYSTEMD_SERVICE "org.freedesktop.login1"
#define SYSTEMD_PATH "/org/freedesktop/login1"
#define SYSTEMD_INTERFACE "org.freedesktop.login1.Manager"
#define SYSTEMD_SESSION_INTERFACE "org.freedesktop.login1.Session"

#define UKUI_SERVICE "org.gnome.SessionManager"
#define UKUI_PATH "/org/gnome/SessionManager"
#define UKUI_INTERFACE "org.gnome.SessionManager"

#define UKUI_SCREENSAVER_SERVICE "org.ukui.ScreenSaver"
#define UKUI_SCREENSAVER_PATH "/"
#define UKUI_SCREENSAVER_INTERFACE "org.ukui.ScreenSaver"

#define PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

bool messageBoxCheck()
{
    QMessageBox msgBox;
    //    msgBox.setWindowTitle(QObject::tr("conform"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    //    msgBox.setModal(false);
    msgBox.setText(QObject::tr("some applications are running and they don't want you to do this."));
    QPushButton *stillButton = msgBox.addButton(QObject::tr("Still to do!"), QMessageBox::ActionRole);
    QPushButton *giveupButton = msgBox.addButton(QObject::tr("give up"), QMessageBox::RejectRole);

    //    QStringList usrlist = getLoginedUsers();
    //    QList<QString>::Iterator it = usrlist.begin(),itend = usrlist.end();
    //    for(;it != itend;it++){
    //        qDebug()<<*it;
    //    }

    msgBox.exec();

    if (msgBox.clickedButton() == stillButton) {
        qDebug() << "Still to do!";
        return true;
    } else if (msgBox.clickedButton() == giveupButton) {
        qDebug() << "give up";
        return false;
    } else {
        return false;
    }
}

static bool dbusCall(
    const QString &service,
    const QString &path,
    const QString &interface,
    const QDBusConnection &connection,
    const QString &method)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path << interface << method;
        return false;
    }

    QDBusMessage msg = dbus.call(method);

    if (!msg.errorName().isEmpty()) {
        qWarning() << "Dbus error: " << msg;
    }

    return msg.arguments().isEmpty() || msg.arguments().constFirst().isNull() || msg.arguments().constFirst().toBool();
}

static bool dbusCallSystemd(
    const QString &service,
    const QString &path,
    const QString &interface,
    const QDBusConnection &connection,
    const QString &method,
    bool needBoolArg)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path << interface << method;
        return false;
    }

    // QDBusMessage msg = dbus.call(method, needBoolArg ? QVariant(true) : QVariant());
    // 在华为相关的机器上，如果调用的method没有参数，但仍传入一个空值，会调用失败
    QDBusMessage msg;
    if (needBoolArg) {
        msg = dbus.call(method, QVariant(true));
    } else {
        msg = dbus.call(method);
    }

    if (!msg.errorName().isEmpty()) {
        qWarning() << "Debus error: " << msg;
    }

    if (msg.arguments().isEmpty() || msg.arguments().constFirst().isNull()) {
        return false;
    }

    QString response = msg.arguments().constFirst().toString();
    qDebug() << "systemd:" << method << "=" << response;

    // this need to be resolved:
    // while canReboot and canPoweroff return challenge,users click Reboot or shutdown,
    // we call logout ,then Reboot(true) and Poweroff(true) is called,
    // if user click cancel in the polkit messagebox,then Reboot or Poweroff is canceled,
    // but logout has been executed.
    return response == QLatin1String("yes") || response == QLatin1String("challenge");
}

bool dbusGetProperty(
    const QString &service,
    const QString &path,
    const QString &interface,
    const QDBusConnection &connection,
    const QString &property)
{
    QDBusInterface dbus(service, path, PROPERTIES_INTERFACE, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusGetProperty: QDBusinterface is invalid" << service << path << interface << property;
        return false;
    }

    QDBusReply<QDBusVariant> reply = dbus.call(QStringLiteral("Get"), interface, property);
    if (!reply.isValid()) {
        qWarning() << "dbusGetProperty Get failed" << service << path << interface << property << reply.error();
        return false;
    }

    QVariant value = reply.value().variant();
    qDebug() << property << "=" << value;
    return value.toBool();
}

PowerProvider::PowerProvider(QObject *parent) : QObject(parent) {}

PowerProvider::~PowerProvider() {}

namespace {

QString currentUserName()
{
    const QByteArray user = qgetenv("USER");
    if (!user.isEmpty())
        return QString::fromLocal8Bit(user);
    struct passwd *pw = getpwuid(getuid());
    if (pw && pw->pw_name)
        return QString::fromLocal8Bit(pw->pw_name);
    return QString();
}

QList<QDBusObjectPath> displayManagerObjectPaths(const QString &property)
{
    QDBusInterface dmProps(
        QLatin1String(LIGHTDM_SERVICE),
        QLatin1String(LIGHTDM_PATH),
        QLatin1String(PROPERTIES_INTERFACE),
        QDBusConnection::systemBus());
    // Seats/Sessions: ao
    QDBusReply<QDBusVariant> reply = dmProps.call("Get", LIGHTDM_SERVICE, property);
    if (!reply.isValid()) {
        qWarning() << "Unable to get DisplayManager" << property << reply.error();
        return {};
    }
    return qdbus_cast<QList<QDBusObjectPath>>(reply.value().variant());
}

QString matchSeatPathById(const QList<QDBusObjectPath> &seats, const QString &seatId)
{
    if (seatId.isEmpty())
        return QString();
    for (const QDBusObjectPath &seatPath : seats) {
        const QString leaf = seatPath.path().section(QLatin1Char('/'), -1);
        if (leaf.compare(seatId, Qt::CaseInsensitive) == 0)
            return seatPath.path();
    }
    return QString();
}

/*
 * ukui-session is often started as a systemd --user service (e.g. kylin-wlcom.service).
 * That process belongs to the logind manager session, which has an empty Seat, and
 * does not inherit XDG_SEAT_PATH. GetSession("auto") is therefore the wrong source.
 */
QString logindSeatIdForCurrentProcess()
{
    QDBusInterface loginMgr(
        QLatin1String(SYSTEMD_SERVICE),
        QLatin1String(SYSTEMD_PATH),
        QLatin1String(SYSTEMD_INTERFACE),
        QDBusConnection::systemBus());
    if (!loginMgr.isValid())
        return QString();

    QDBusReply<QDBusObjectPath> byAuto = loginMgr.call("GetSession", QStringLiteral("auto"));
    if (!byAuto.isValid()) {
        qDebug() << "GetSession(auto) failed:" << byAuto.error();
        return QString();
    }

    QDBusInterface props(
        QLatin1String(SYSTEMD_SERVICE),
        byAuto.value().path(),
        QLatin1String(PROPERTIES_INTERFACE),
        QDBusConnection::systemBus());
    QDBusReply<QDBusVariant> seatVar = props.call("Get", SYSTEMD_SESSION_INTERFACE, "Seat");
    if (!seatVar.isValid()) {
        qDebug() << "get Session.Seat failed:" << seatVar.error();
        return QString();
    }
    // Session.Seat: (so)
    const QDBusArgument arg = qvariant_cast<QDBusArgument>(seatVar.value().variant());
    QString seatId;
    QDBusObjectPath seatPath;
    arg.beginStructure();
    arg >> seatId >> seatPath;
    arg.endStructure();
    return seatId;
}

QString seatPathFromDisplayManagerSessions(const QString &userName)
{
    if (userName.isEmpty())
        return QString();

    const QList<QDBusObjectPath> sessions = displayManagerObjectPaths(QStringLiteral("Sessions"));
    for (const QDBusObjectPath &sessionPath : sessions) {
        QDBusInterface sessionProps(
            QLatin1String(LIGHTDM_SERVICE),
            sessionPath.path(),
            QLatin1String(PROPERTIES_INTERFACE),
            QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> nameVar
            = sessionProps.call("Get", LIGHTDM_SESSION_INTERFACE, QStringLiteral("UserName"));
        if (!nameVar.isValid())
            continue;
        if (nameVar.value().variant().toString() != userName)
            continue;

        QDBusReply<QDBusVariant> seatVar
            = sessionProps.call("Get", LIGHTDM_SESSION_INTERFACE, QStringLiteral("Seat"));
        if (!seatVar.isValid())
            continue;
        // Session.Seat: o
        const QString seatPath = qdbus_cast<QDBusObjectPath>(seatVar.value().variant()).path();
        if (!seatPath.isEmpty())
            return seatPath;
    }
    return QString();
}

QString resolveDisplayManagerSeatPath()
{
    const QByteArray envPath = qgetenv("XDG_SEAT_PATH");
    if (!envPath.isEmpty()) {
        return envPath;
    }
    const QList<QDBusObjectPath> seats = displayManagerObjectPaths(QStringLiteral("Seats"));

    QString seatId = QString::fromLocal8Bit(qgetenv("XDG_SEAT"));
    if (seatId.isEmpty())
        seatId = logindSeatIdForCurrentProcess();
    const QString matched = matchSeatPathById(seats, seatId);
    if (!matched.isEmpty())
        return matched;

    const QString fromSession = seatPathFromDisplayManagerSessions(currentUserName());
    if (!fromSession.isEmpty()) {
        qDebug() << "resolved DisplayManager seat from session:" << fromSession;
        return fromSession;
    }

    if (seats.size() == 1) {
        qDebug() << "using the only DisplayManager seat" << seats.first().path();
        return seats.first().path();
    }

    if (!seats.isEmpty()) {
        qWarning() << "ambiguous DisplayManager seats, using" << seats.first().path();
        return seats.first().path();
    }

    qWarning() << "no DisplayManager seat available";
    return QString();
}

} // namespace

/************************************************
  SystemdProvider

  http://www.freedesktop.org/wiki/Software/systemd/logind
 ************************************************/

SystemdProvider::SystemdProvider(QObject *parent) : PowerProvider(parent) {}

SystemdProvider::~SystemdProvider() {}

bool SystemdProvider::canSwitchUser() const
{
    QString property = "CanSwitch";
    QString xdg_seat_path = resolveDisplayManagerSeatPath();
    if (xdg_seat_path.isEmpty()) {
        qWarning() << "DisplayManager seat unresolved, CanSwitch returns false";
        return false;
    }
    return dbusGetProperty(
        QLatin1String(LIGHTDM_SERVICE),
        xdg_seat_path,
        QLatin1String(LIGTHDM_INTERFACE),
        QDBusConnection::systemBus(),
        property);
}

bool SystemdProvider::canAction(UkuiPower::Action action) const
{
    QString command;

    switch (action) {
        case UkuiPower::PowerSwitchUser:
            return canSwitchUser();
        case UkuiPower::PowerReboot:
            command = QLatin1String("CanReboot");
            break;

        case UkuiPower::PowerShutdown:
            command = QLatin1String("CanPowerOff");
            break;

        case UkuiPower::PowerSuspend:
            command = QLatin1String("CanSuspend");
            break;

        case UkuiPower::PowerHibernate:
            command = QLatin1String("CanHibernate");
            break;

        default:
            return false;
    }

    // canAction should be always silent because it can freeze
    // g_main_context_iteration Qt event loop in QMessageBox
    // on panel startup if there is no DBUS running.
    return dbusCallSystemd(
        QLatin1String(SYSTEMD_SERVICE),
        QLatin1String(SYSTEMD_PATH),
        QLatin1String(SYSTEMD_INTERFACE),
        QDBusConnection::systemBus(),
        command,
        false);
}

bool SystemdProvider::doSwitchUser()
{
    bool isinhibited = false;
    QDBusInterface *interface = new QDBusInterface(
        "org.gnome.SessionManager",
        "/org/gnome/SessionManager",
        "org.gnome.SessionManager",
        QDBusConnection::sessionBus());
    quint32 inhibit_switchuser = 2;
    QDBusReply<bool> reply = interface->call("IsInhibited", inhibit_switchuser);
    if (reply.isValid()) {
        // use the returned value
        qDebug() << "Is inhibit by someone: " << reply.value();
        isinhibited = reply.value();
    } else {
        qDebug() << reply.value();
    }

    if (isinhibited == true) {
        isinhibited = !messageBoxCheck();
    }

    if (isinhibited == false) {

        //        QDBusInterface dbus("org.ukui.KWin", "/Compositor", "org.ukui.kwin.Compositing",
        //        QDBusConnection::sessionBus());

        //        if (!dbus.isValid()) {
        //            qWarning() << "dbusCall: QDBusInterface is invalid";
        //            return false;
        //        }

        //        dbus.call("suspend");

        bool useScreensaver = dbusCall(
            QLatin1String(UKUI_SCREENSAVER_SERVICE),
            UKUI_SCREENSAVER_PATH,
            QLatin1String(UKUI_SCREENSAVER_INTERFACE),
            QDBusConnection::sessionBus(),
            "CheckAppVersion");
        if (useScreensaver) {
            QDBusMessage msg = QDBusMessage::createMethodCall(UKUI_SCREENSAVER_SERVICE, UKUI_SCREENSAVER_PATH, UKUI_SCREENSAVER_INTERFACE, "SwitchUser");
            QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(msg);

            // 使用父对象管理内存，避免泄漏
            QObject *parent = new QObject();
            QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall, parent);
            QTimer *timeoutTimer = new QTimer(parent);
            timeoutTimer->setSingleShot(true);
            timeoutTimer->start(3000);

            // 使用标志位防止重复删除
            bool isFinished = false;

            QObject::connect(watcher, &QDBusPendingCallWatcher::finished,
                             parent, [parent, &isFinished, watcher, timeoutTimer]() {
                if (isFinished) return;
                isFinished = true;
                timeoutTimer->stop();
                QDBusPendingReply<> reply = *watcher;
                if (reply.isValid()) {
                    qDebug() << "SwitchUser DBus call succeeded";
                } else {
                    qDebug() << "SwitchUser DBus call failed:" << reply.error().message();
                }
                parent->deleteLater();
            });

            QObject::connect(timeoutTimer, &QTimer::timeout,
                             parent, [parent, &isFinished, watcher]() {
                if (isFinished) return;
                isFinished = true;
                qDebug() << "SwitchUser DBus call timeout";
                parent->deleteLater();
            });

            return true;
        }

        QString command = "SwitchToGreeter";
        QString xdg_seat_path = resolveDisplayManagerSeatPath();
        if (xdg_seat_path.isEmpty()) {
            qWarning() << "SwitchToGreeter skipped: DisplayManager seat path unresolved";
            return false;
        }
        return dbusCall(
            QLatin1String(LIGHTDM_SERVICE),
            xdg_seat_path,
            QLatin1String(LIGTHDM_INTERFACE),
            QDBusConnection::systemBus(),
            command);
    }
    return false;
}

bool SystemdProvider::doAction(UkuiPower::Action action)
{

    qDebug() << "SystemdProvider::doAction:" << action;
    QString command;

    switch (action) {
        case UkuiPower::PowerSwitchUser:
            return doSwitchUser();
        case UkuiPower::PowerReboot:
            command = QLatin1String("Reboot");
            break;
        case UkuiPower::PowerShutdown:
            command = QLatin1String("PowerOff");
            break;
        case UkuiPower::PowerSuspend:
            command = QLatin1String("Suspend");
            break;
        case UkuiPower::PowerHibernate:
            command = QLatin1String("Hibernate");
            break;
        default:
            return false;
    }

    return dbusCallSystemd(
        QLatin1String(SYSTEMD_SERVICE),
        QLatin1String(SYSTEMD_PATH),
        QLatin1String(SYSTEMD_INTERFACE),
        QDBusConnection::systemBus(),
        command,
        true);
}

UKUIProvider::UKUIProvider(QObject *parent) : PowerProvider(parent) {}

UKUIProvider::~UKUIProvider() {}

bool UKUIProvider::canAction(UkuiPower::Action action) const
{
    // 这里是调用ukui-session注册的d-bus检测是否有inhibitor存在
    bool isinhibited = false;
    QDBusInterface *interface = new QDBusInterface(
        "org.gnome.SessionManager",
        "/org/gnome/SessionManager",
        "org.gnome.SessionManager",
        QDBusConnection::sessionBus());
    quint32 inhibit_logout = 1;
    QDBusReply<bool> reply = interface->call("IsInhibited", inhibit_logout);
    if (reply.isValid()) {
        // use the returned value
        qDebug() << "Is inhibit by someone: " << reply.value();
        isinhibited = reply.value();
    } else {
        qDebug() << reply.value();
    }

    if (isinhibited == true) {
        isinhibited = !messageBoxCheck();
    }

    return !isinhibited;
}

bool UKUIProvider::doAction(UkuiPower::Action action)
{
    QString command;
    switch (action) {
        case UkuiPower::PowerLogout:
            command = QLatin1String("logout");
            break;
        case UkuiPower::PowerReboot:
            command = QLatin1String("reboot");
            break;
        case UkuiPower::PowerShutdown:
            command = QLatin1String("powerOff");
            break;
        default:
            return false;
    }

    qDebug() << "ukuipowerprovider call D-Bus session";
    return dbusCall(
        QLatin1String(UKUI_SERVICE),
        QLatin1String(UKUI_PATH),
        QLatin1String(UKUI_INTERFACE),
        QDBusConnection::sessionBus(),
        command);
}

bool UKUIProvider::doLSAction(UkuiPower::Action action, QString param /* = ""*/)
{
    qDebug() << "call LockScreen D-Bus";
    QString SS_DBUS_SERVICE = "org.ukui.ScreenSaver";
    QString SS_DBUS_PATH = "/";
    QString SS_DBUS_INTERFACE = "org.ukui.ScreenSaver";
    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    QString sessionDbus = QString("%1%2").arg(SS_DBUS_SERVICE).arg(displayNum);
    QSharedPointer<QDBusInterface> screensaverInterface(
        new QDBusInterface(sessionDbus, SS_DBUS_PATH, SS_DBUS_INTERFACE));

    if (!screensaverInterface->isValid()) {
        screensaverInterface
            = QSharedPointer<QDBusInterface>(new QDBusInterface(SS_DBUS_SERVICE, SS_DBUS_PATH, SS_DBUS_INTERFACE));
    }

    bool m_flag = false;
    QDBusReply<bool> reply = screensaverInterface->call("CheckAppVersion");
    if (reply.isValid()) {
        // use the returned value
        m_flag = reply.value();
        qDebug() << "CheckAppVersion is " << m_flag;
    } else {
        qDebug() << "CheckAppVersion function is not existed";
    }
    if (m_flag) {
        switch (action) {
            case UkuiPower::PowerLogout:
                screensaverInterface->asyncCall("Logout");
                break;
            case UkuiPower::PowerReboot:
                screensaverInterface->asyncCall("Reboot");
                break;
            case UkuiPower::PowerShutdown:
                screensaverInterface->asyncCall("PowerOff");
                break;
            case UkuiPower::PowerHibernate:
                screensaverInterface->asyncCall("Hibernate");
                break;
            case UkuiPower::PowerSuspend:
                screensaverInterface->asyncCall("Suspend");
                break;
            case UkuiPower::PowerSwitchUser:
                screensaverInterface->asyncCall("SwitchUser");
                break;
            case UkuiPower::PowerSwitchToUser:
                screensaverInterface->asyncCall("SwitchToUser", param);
                break;
            default:
                return false;
        }
        return true;
    } else {
        return false;
    }
}

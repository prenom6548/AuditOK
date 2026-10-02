/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "sessionhelper.h"
#include <sys/file.h>
#include <pwd.h>
#include <QString>
#include <QDebug>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusInterface>
#include <QProcess>

#include "login1helper.h"
#include "lock-backend/gsettingshelper.h"

const static QString sessionService = QStringLiteral("org.gnome.SessionManager");
const static QString sessionPath = QStringLiteral("/org/gnome/SessionManager");
const static QString sessionInterface = QStringLiteral("org.gnome.SessionManager");

QDBusArgument &InhibitInfo::operator<<(QDBusArgument &argument, const InhibitInfo::InhibitorInfo &mystruct)
{
    argument.beginStructure();
    argument << mystruct.name << mystruct.icon;
    argument.endStructure();
    return argument;
}

const QDBusArgument &InhibitInfo::operator>>(const QDBusArgument &argument, InhibitInfo::InhibitorInfo &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.name >> mystruct.icon;
    argument.endStructure();
    return argument;
}

SessionHelper::SessionHelper(QObject *parent) : QObject(parent)
{
    m_sessionInterface
        = new QDBusInterface(sessionService, sessionPath, sessionInterface, QDBusConnection::sessionBus(), this);
}

SessionHelper::~SessionHelper() {}

bool SessionHelper::canAction(Action action)
{
    //以下为代码结构调整
    QString command;
    switch (action) {
        case PowerSwitchUser:
            command = QLatin1String("canSwitch");
            break;
        case PowerHibernate:
            command = QLatin1String("canHibernate");
            break;
        case PowerSuspend:
            command = QLatin1String("canSuspend");
            break;
        case PowerMonitorOff:
            command = QLatin1String("canLockscreen");
            break;
        case PowerLogout:
            command = QLatin1String("canLogout");
            break;
        case PowerReboot:
            command = QLatin1String("canReboot");
            break;
        case PowerShutdown:
            command = QLatin1String("canPowerOff");
            break;
        default:
            break;
    }

    if (!m_sessionInterface->isValid()) {
        qWarning() << "dbusCall: Session QDBusInterface is invalid";
        return false;
    }

    QDBusReply<bool> testReply = m_sessionInterface->call(QLatin1String("canLockscreen"));
    if (!testReply.isValid()) {
        //解决老版本升级到新版本接口不兼容的问题，在session接口不存在的情况下，调用systemd的接口
        QDBusError error = testReply.error();
        Login1Helper *m_login1Helper = new Login1Helper(this);
        if (error.type() == QDBusError::UnknownMethod) {
            qInfo() << "updating ! old ukui-session dose not have canAction method";
            if (action == PowerLogout || action == PowerMonitorOff) {
                return true;
            }
            return m_login1Helper->canAction(action);
        }
        qInfo() << "dbus error";
        return false;
    }

    QDBusReply<bool> reply = m_sessionInterface->call(command);

    qInfo() << "command = " << command << "reply = " << m_sessionInterface->call(command);

    return reply.value();
}

bool SessionHelper::doAction(const QString &powerManagerfunc)
{
    if (!m_sessionInterface->isValid()) {
        qWarning() << "dbusCall: Session QDBusInterface is invalid";
        return false;
    }

    up_to_time = new QTimer();
    up_to_time->setSingleShot(true);

//    qDebug() << "playShutdownMusic(powerManagerfunc) = " << playShutdownMusic(powerManagerfunc);

    if (!playShutdownMusic(powerManagerfunc)) {
        doPowerManager(powerManagerfunc);
    }
    return true;
}

void SessionHelper::doPowerManager(const QString &powerManagerfunc)
{
    QString command;
    if (powerManagerfunc == "Hibernate") {
        command = QLatin1String("hibernate");
    } else if (powerManagerfunc == "Suspend") {
        command = QLatin1String("suspend");
    } else if (powerManagerfunc == "Reboot") {
        command = QLatin1String("reboot");
    } else if (powerManagerfunc == "PowerOff") {
        command = QLatin1String("powerOff");
    } else if (powerManagerfunc == "Logout") {
        command = QLatin1String("logout");
    } else if (powerManagerfunc == "SwitchUser") {
        command = QLatin1String("switchUser");
    } else {
        return;
    }
    QDBusMessage mes = m_sessionInterface->call(command);

    if (!(mes.errorName().isEmpty())) {
        Login1Helper *m_login1Helper = new Login1Helper(this);
        //本来应该判断错误类别，考虑到运行效率，不做该判断
        m_login1Helper->setPowerManager(powerManagerfunc);
    }
}

QStringList SessionHelper::getLockCheckStatus(QString type)
{
    qDBusRegisterMetaType<InhibitInfo::InhibitorInfo>();

    QVector<InhibitInfo::InhibitorInfo> resVec;

    if (!m_sessionInterface->isValid()) {
        qDebug() << "session interface invalid!";
    }

    QDBusMessage result = m_sessionInterface->call("ListInhibitor", QVariant(type));
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        InhibitInfo::InhibitorInfo inhibtor;
        dbusArgs >> inhibtor;
        resVec.push_back(inhibtor);
    }
    dbusArgs.endArray();
    QStringList lockCheckList;
    for (auto iter = resVec.begin(); iter != resVec.end(); ++iter) {
        lockCheckList.append(iter->icon);
        lockCheckList.append(iter->name);
    }
    // qDebug() << "lockCheckList = " << lockCheckList;

    return lockCheckList;
}

bool SessionHelper::playShutdownMusic(const QString &powerManagerfunc)
{
    // up_to_time and soundplayer can not be define out of this if().
    // otherwise run ukui-session-tools --suspend with segmente error.
    // because they will be delate at the end of the playShutdownMusic().
    bool play_music = false;
    QDBusMessage msg;
    GSettingsHelper *gsettingsHelper = new GSettingsHelper(this);
    if (powerManagerfunc == "Reboot" || powerManagerfunc == "PowerOff") {
        play_music = gsettingsHelper->GetSessionConf(KEY_SESSION_POWEROFF_MUSIC).toBool();
    } else if (powerManagerfunc == "Logout") {
        play_music = gsettingsHelper->GetSessionConf(KEY_SESSION_LOGOUT_MUSIC).toBool();
        msg = m_sessionInterface->call("emitStartLogout");
        if (!msg.errorName().isEmpty()) {
            qWarning() << "Dbus error: " << msg;
        }
    } else {
        if (powerManagerfunc == "SwitchUser") {
            msg = m_sessionInterface->call("emitPrepareForSwitchuser");
            if (!msg.errorName().isEmpty()) {
                qWarning() << "Dbus error: " << msg;
            }
        }
        return false;
    }

    if (play_music) {
        QObject::connect(up_to_time, &QTimer::timeout, [=]() {
            doPowerManager(powerManagerfunc);
            return true;
        });
        QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
        if (powerManagerfunc == "Reboot" || powerManagerfunc == "PowerOff") {
            if (xdg_session_type == "wayland") {
                QProcess::startDetached("paplay /usr/share/ukui/ukui-session-manager/shutdown.wav");
            } else {
                QProcess::startDetached("aplay /usr/share/ukui/ukui-session-manager/shutdown.wav");
            }
            up_to_time->start(5000);
        } else if (powerManagerfunc == "Logout") {
            if (xdg_session_type == "wayland") {
                QProcess::startDetached("paplay /usr/share/ukui/ukui-session-manager/logout.wav");
            } else {
                QProcess::startDetached("aplay /usr/share/ukui/ukui-session-manager/logout.wav");
            }
            up_to_time->start(2000);
        } else {
            qDebug() << "error num";
            return false;
        }
    }
    return play_music;
}

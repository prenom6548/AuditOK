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
#include "switchuserutils.h"

#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#include <systemd/sd-login.h>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QProcess>
#include <QStandardPaths>
#include <QString>
#include "global_utils.h"

SwitchUserUtils::SwitchUserUtils() {}

QString SwitchUserUtils::GetCurUserName()
{
    QString strUserName = "";
    struct passwd *pw = NULL;
    pw = getpwuid(getuid());
    if (pw != NULL) {
        strUserName = QString(pw->pw_name);
    } else {
        qWarning() << "SwitchUserUtils:get user name failed";
    }

    return strUserName;
}

int SwitchUserUtils::GetUidByName(QString strUserName)
{
    if (strUserName.isEmpty()) {
        return (unsigned int)-1;
    }

    struct passwd *pwbufp = getpwnam(strUserName.toLatin1().data());

    return pwbufp ? pwbufp->pw_uid : (unsigned int)-1;
}

UserDisplayIfInfo SwitchUserUtils::GetUserUDII(QString strUserName)
{
    UserDisplayIfInfo userDisplayIfInfo;
    // 获取所有的session路径
    QList<QDBusObjectPath> listObjPath;
    QDBusInterface iface(
        "org.freedesktop.DisplayManager",
        "/org/freedesktop/DisplayManager",
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus());
    QDBusMessage result = iface.call("Get", "org.freedesktop.DisplayManager", "Sessions");
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "Get Sessions:" << result.errorMessage();
        return userDisplayIfInfo;
    }
    QList<QVariant> variantList = result.arguments();
    if (variantList.size() <= 0) {
        qDebug() << "Get Sessions List is null!";
        return userDisplayIfInfo;
    }
    QDBusVariant dbusVar = variantList[0].value<QDBusVariant>();
    const QDBusArgument &dbusArg = dbusVar.variant().value<QDBusArgument>();
    if (dbusArg.currentType() == QDBusArgument::ArrayType) {
        QDBusObjectPath objPath;
        dbusArg.beginArray();
        while (!dbusArg.atEnd()) {
            dbusArg >> objPath;
            listObjPath.append(objPath);
        }
        dbusArg.endArray();
    }

    // 获取各session的用户名，过滤指定用户的信息
    for (auto objPath : listObjPath) {
        QDBusInterface iface(
            "org.freedesktop.DisplayManager",
            objPath.path(),
            "org.freedesktop.DBus.Properties",
            QDBusConnection::systemBus());
        QDBusMessage result = iface.call("Get", "org.freedesktop.DisplayManager.Session", "UserName");
        if (result.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "Get Session username :" << result.errorMessage();
            continue;
        }
        QList<QVariant> variantList = result.arguments();
        if (variantList.size() <= 0) {
            qDebug() << "Get Session user info is null!";
            continue;
        }
        QDBusVariant dbusUNameVar = variantList[0].value<QDBusVariant>();
        QString userName = dbusUNameVar.variant().value<QString>();
        if (strUserName == userName) {
            result = iface.call("Get", "org.freedesktop.DisplayManager.Session", "Seat");
            if (result.type() == QDBusMessage::ErrorMessage) {
                qWarning() << "Get Session seat :" << result.errorMessage();
                continue;
            }
            QList<QVariant> variantList = result.arguments();
            if (variantList.size() <= 0) {
                qDebug() << "Get Session seatis null!";
                continue;
            }
            QDBusVariant dbusSeatVar = variantList[0].value<QDBusVariant>();
            QDBusObjectPath strSeat = dbusSeatVar.variant().value<QDBusObjectPath>();
            if (!strSeat.path().isEmpty()) {
                userDisplayIfInfo.strSeatPath = strSeat.path();
                userDisplayIfInfo.strUserName = strUserName;
                userDisplayIfInfo.strSessionPath = objPath.path();
            }
        } else {
            continue;
        }
    }
    return userDisplayIfInfo;
}

int SwitchUserUtils::SwitchToUserSession(QString seatPath, UserDisplayIfInfo &toUDII)
{
    // 检查Seat路径是否合规
    if (QDBusObjectPath(seatPath).path().isEmpty()) {
        qWarning() << "SwitchToUserSession failed, invalid seatpath:" << seatPath;
        return 1;
    }
    QDBusInterface ifaceDM(
        "org.freedesktop.DisplayManager",
        seatPath,
        "org.freedesktop.DisplayManager.Seat",
        QDBusConnection::systemBus());
    if (!ifaceDM.isValid()) {
        qWarning() << "SwitchUser seat interface invalid!";
        return 2;
    }
    // 截取lightdm seat路径中最后字段与systemd中Seat匹配(systemd接口状态异常时，可在/run/systemd/目录下查看实际状态)
    QString strSeatID = seatPath.split("/").last();
    qDebug() << "SeatId:" << strSeatID << ",UName:" << toUDII.strUserName << ",USessionPath:" << toUDII.strSessionPath
             << ",USeatPath:" << toUDII.strSeatPath;
    if (!toUDII.strUserName.isEmpty() && !toUDII.strSessionPath.isEmpty()) { // 指定了切换的用户名，且其有显示会话
        char **sessions = NULL;
        char *user_display = NULL;
        QString strUserDisplay = "", strSessionDisplay = "";
        QStringList listSessions;
        QString strNewUserSession = "";
        // 获取用户的所有session id
        sd_uid_get_sessions(GetUidByName(toUDII.strUserName), 0, &sessions);
        // 获取用户的主display id
        sd_uid_get_display(GetUidByName(toUDII.strUserName), &user_display);
        // 释放systemd接口调用占用的资源
        if (sessions) {
            char **k = sessions;
            while (*k) {
                listSessions.append(QString("%1").arg(*k));
                k++;
            }
            for (k = sessions; *k; k++)
                free(*k);
            free(sessions);
            sessions = NULL;
        }
        if (user_display) {
            strUserDisplay = QString("%1").arg(user_display);
            free(user_display);
            user_display = NULL;
        }

        qDebug() << "user display: " << strUserDisplay;
        qDebug() << "user sessions: " << listSessions;

        /**
         * 遍历systemd中用户所有的session，过滤掉Display为空的session
         * （wayland环境可能Dispaly为空，需再确认Type是否为wayland）
         **/
        for (int n = 0; n < listSessions.size(); n++) {
            char *session_display = NULL;
            sd_session_get_display(listSessions.at(n).toLatin1().data(), &session_display);
            if (session_display) {
                strSessionDisplay = QString("%1").arg(session_display);
                free(session_display);
                session_display = NULL;
            }
            // 支持wayland的环境
            if (!qgetenv("WAYLAND_DISPLAY").isEmpty()) {
                QString strSession = "";
                bool isNumber = true;
                listSessions.at(n).toDouble(&isNumber);
                // 会话id是否为纯数字，是则增加前缀_3,否则不加前缀
                if (isNumber) {
                    strSession = QString("/org/freedesktop/login1/session/_3%1").arg(listSessions.at(n));
                } else {
                    strSession = QString("/org/freedesktop/login1/session/%1").arg(listSessions.at(n));
                }
                // 检查session type 是否匹配
                QDBusInterface iface(
                    "org.freedesktop.login1",
                    strSession,
                    "org.freedesktop.DBus.Properties",
                    QDBusConnection::systemBus());
                QDBusMessage result = iface.call("Get", "org.freedesktop.login1.Session", "Type");
                if (result.type() == QDBusMessage::ErrorMessage) {
                    qWarning() << "Get Session Type failed:" << result.errorMessage();
                    return 2;
                }
                QList<QVariant> variantList = result.arguments();
                if (variantList.size() <= 0) {
                    qDebug() << "Get Session type info is null!";
                    return 2;
                }
                QDBusVariant dbusTypeVar = variantList[0].value<QDBusVariant>();
                QString sessionType = dbusTypeVar.variant().value<QString>();
                if (strSessionDisplay.isEmpty() && sessionType != "wayland") {
                    continue;
                }
            } else {
                if (strSessionDisplay.isEmpty()) {
                    /**
                     * 最后一个会话id时，如果该用户的Display不为空，则也认为最后一个会话有效
                     **/
                    if ((n + 1) < listSessions.size() || strUserDisplay.isEmpty()) {
                        continue;
                    }
                    strSessionDisplay = strUserDisplay;
                }
            }
            QString strUserSeatID = "";
            char *seat = NULL;
            sd_session_get_seat(listSessions.at(n).toLatin1().data(), &seat);
            if (seat) {
                strUserSeatID = QString("%1").arg(seat);
                free(seat);
                seat = NULL;
            }
            qDebug() << "SrcSeat:" << strSeatID << ",DesSeat:" << strUserSeatID << ",Session:" << listSessions.at(n)
                     << ",Display:" << strSessionDisplay;
            // 有效会话的SeatId与lightdm中的SeatId匹配则此会话为即将激活的会话
            if (!strUserSeatID.isEmpty() && (QString::compare(strUserSeatID, strSeatID, Qt::CaseInsensitive) == 0)) {
                strNewUserSession = listSessions.at(n);
                qDebug() << "SwitchUser: find sessions path:" << strNewUserSession << "," << strUserSeatID;
                break;
            }
        }
        if (strNewUserSession.isEmpty()) {
            qWarning() << "SwitchUser: can not find user session" << toUDII.strUserName;
            return 3;
        }
        // 激活用户的会话
        QDBusInterface iface(
            "org.freedesktop.login1",
            "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager",
            QDBusConnection::systemBus());
        QDBusMessage result = iface.call("ActivateSession", strNewUserSession);
        if (result.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "SwitchUser ActivateSession failed:" << result.errorMessage();
            return 2;
        }
        return 0;
    } else if (!toUDII.strUserName.isEmpty()) {
        if (!isGreeterMode()) {
            QDBusMessage result = ifaceDM.call("SwitchToUser", toUDII.strUserName, toUDII.strSessionPath);
            if (result.type() == QDBusMessage::ErrorMessage) {
                qWarning() << "SwitchUser SwitchToUser failed:" << result.errorMessage();
                return 2;
            }
        } else {
            return 4;
        }
    } else {
        if (!isGreeterMode()) {
            QDBusMessage result = ifaceDM.call("SwitchToGreeter");
            if (result.type() == QDBusMessage::ErrorMessage) {
                qWarning() << "SwitchUser SwitchToGreeter failed:" << result.errorMessage();
                return 2;
            }
        } else {
            return 5;
        }
    }
    return 0;
}

bool SwitchUserUtils::SwitchToUserLock()
{
    QString strLockCommand = QStandardPaths::findExecutable("ukui-screensaver-command");
    if (!strLockCommand.isEmpty()) {
        QProcess::execute(strLockCommand, QStringList() << "-S");
        return true;
    }
    return false;
}

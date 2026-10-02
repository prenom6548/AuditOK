/*
 * Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
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

#ifndef SESSIONHELPER_H
#define SESSIONHELPER_H

#include <QFile>
#include <QTimer>
#include <QVector>
#include <QDBusArgument>
#include <QSharedPointer>
#include "../common/definetypes.h"
#include "../common/windowdata.h"

namespace InhibitInfo
{
struct InhibitorInfo
{
    QString name;
    QString icon;
    int uid;
};

    QDBusArgument &operator<<(QDBusArgument &argument, const InhibitInfo::InhibitorInfo &mystruct);

    const QDBusArgument &operator>>(const QDBusArgument &argument, InhibitInfo::InhibitorInfo &mystruct);

    QDebug operator<<(QDebug debug, const InhibitInfo::InhibitorInfo &info);
} // namespace InhibitInfo

Q_DECLARE_METATYPE(InhibitInfo::InhibitorInfo)

namespace SystemdInhibitor {
    struct Inhibitor {
        QString action;
        QString name;
        QString reason;
        QString mode;
        int uid;
        int pid;
    };

    QDBusArgument &operator<<(QDBusArgument &argument, const SystemdInhibitor::Inhibitor &mystruct);

    const QDBusArgument &operator>>(const QDBusArgument &argument, SystemdInhibitor::Inhibitor &mystruct);
}

Q_DECLARE_METATYPE(SystemdInhibitor::Inhibitor)

class QDBusInterface;
class Login1Helper;

class SessionHelper : public QObject
{
    Q_OBJECT
public:
    explicit SessionHelper(QSharedPointer<Login1Helper> login1Helper, bool isLockStartupMode = false, QObject *parent = nullptr);

    virtual ~SessionHelper();

    QVector<InhibitInfo::InhibitorInfo> getLockCheckStatus(QString type);

    QVector<WindowInfo::WindowInfo> getAppWindows();

    bool canAction(Action action);

    bool doAction(const QString &powerManagerfunc);

    void setClosePhase(bool inClosePhase);

    bool getIsLegacy();

    void setAuthenticateCompeteState(int state);

    int getAuthenticateCompeteState();

Q_SIGNALS:
    void blockLogoutInhibitedChanged();

    void windowAdd(QString winId, QString appId);

    void windowRemove(QString winId);

    void sessionLegacyChanged();

private Q_SLOTS:
    void onSessionDbusActiveChanged(const QString &strServerName, bool isActive);

    void onLogoutHibitedAddWatcherMessage();

    void onLogoutHibitedRemoveWatcherMessage();

private:
    void init();

    void doPowerManager(const QString &powerManagerfunc);

    bool playShutdownMusic(const QString &powerManagerfunc);

    void getSystemdInhibitor(QString type, QVector<InhibitInfo::InhibitorInfo> &inhibitorVec);

    static void findNameAndIcon(QString &inhibitorName, InhibitInfo::InhibitorInfo &inhibitor);
private:
    QDBusInterface *m_sessionInterface = nullptr;

    QDBusInterface *m_wlcomInterface = nullptr;

    QTimer *m_timerPlayMusic = nullptr;

    QSharedPointer<Login1Helper> m_login1Helper = nullptr;

    bool m_inClosePhase = false;

    bool m_isLegacy = false; //存在兼容旧版本的情况，需要判断session是否支持监听窗口开关

    bool m_isLockStartupMode = false;
};

#endif // SESSIONHELPER_H

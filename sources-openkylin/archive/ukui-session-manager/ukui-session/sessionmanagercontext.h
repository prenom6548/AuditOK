/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *               2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef SESSIONINHIBITCONTEXT_H
#define SESSIONINHIBITCONTEXT_H

#include "../tools/ukuipower.h"
#include "../tools/powerprovider.h"
#include "modulemanager.h"
#include "usminhibit.h"
#include "ukuilockinfo.h"
#include "abnormalwindow.h"
#include <QDBusServiceWatcher>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDebug>
#include <KIdleTime>
#include "screensaverproxy.h"

#ifdef signals
#undef signals
#endif

class SessionDBusAdaptor;

enum LockStatus
{
    SCREEN_SAVER,
    SCREEN_LOCK,
    SCREEN_SAVER_LOCK,
    SCREEN_UNLOCK,
    UNDEFINED,
};

enum SaverStates
{
    UNDEFINE = -1,
    ISSTARTUP,
    AUTHCOMPLETE,
};

class SessionManagerDBusContext : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    SessionManagerDBusContext(ModuleManager *manager, QObject *object = nullptr);
    ~SessionManagerDBusContext() override;

Q_SIGNALS:
    void moduleStateChanged(QString moduleName, bool state);
    void inhibitadded(quint32 flags);
    void inhibitremove(quint32 flags);
    void StartLogout();
    void PrepareForSwitchuser();
    void PrepareForPhase2();
    void logoutInhibitAdd();
    void logoutInhibitRemove();
    void windowAdd(QString windId, QString appId);
    void windowRemove(QString winId);

public Q_SLOTS:
    Q_NOREPLY void startupfinished(const QString &appName, const QString &string);

    bool canLogout();

    bool canSwitch();

    bool canLockscreen();

    bool canHibernate();

    bool canSuspend();

    bool canReboot();

    bool canPowerOff();

    Q_NOREPLY void switchUser();

    Q_NOREPLY void execComplete(const QString type);

    Q_NOREPLY void switchToUser(QString strUserName);

    Q_NOREPLY void hibernate();

    Q_NOREPLY void suspend();

    Q_NOREPLY void logout();

    Q_NOREPLY void reboot();

    Q_NOREPLY void powerOff();

    Q_NOREPLY void startModule(const QString &name);

    Q_NOREPLY void stopModule(const QString &name);

    Q_NOREPLY void closeWindows(const QString &power);

    bool startApp(const QString &name, const QStringList &args);

    uint Inhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags);

    Q_NOREPLY void Uninhibit(uint cookie);

    QStringList GetInhibitors();

    QStringList GetAppWindows();

    QVector<InhibitInfo::InhibitorInfo> ListInhibitor(QString type);

    QVector<InhibitInfoAndUid::InhibitorInfo> ListInhibitorAndUid(QString type);

    bool IsSessionRunning();

    QString GetSessionName();

    bool IsInhibited(quint32 flags);

    int GetScreenLockStatus();

    Q_NOREPLY void showBlackWindow();

    Q_NOREPLY void hideBlackWindow();

    Q_NOREPLY void showEmergencyWindow();

    Q_NOREPLY void setSessionEnv(const QString &key, const QString &value);

    Q_NOREPLY void emitStartLogout();

    Q_NOREPLY void emitPrepareForSwitchuser();

    Q_NOREPLY void emitPrepareForPhase2();

    Q_NOREPLY void simulateUserActivity();

    Q_NOREPLY int getAuthenticateCompeteState();

    Q_NOREPLY void setAuthenticateCompeteState(int authState);

    void on_serviceUnregistered(const QString &serviceName);

private slots:
    void onWindowAdded(QString winId);

    void onWindowRemoved(QString winId);

    void onScreenLock();

    void onScreenUnLock();

    void onScreenSaver();

    void onScreenSaverLock();

    void onLogin1UnLockScreen();

    void onScreensaverServiceStatusChanged(bool isLockStartup);

    void onSaverBackendAbnormaled();

private:
    void executeBoxadm();

    void stopUkuisessionTarget();

    void logoutComplete();

    void rebootComplete();

    void shutdownComplete();

    QStringList listFileList();

    void execPro(QStringList proList);

    bool isInnerCaller();

    bool checkAuthorization(const QString &actionId);

    /**
     * @brief 内部调用模拟用户活动，绕过DBus鉴权检查
     *        仅在 inhibit 状态变化时由信号触发，不需要 DBus 调用上下文
     */
    void doSimulateUserActivity();

    void ukuiServerComplete(QString type);

    void callCloseWindows();

    void initSaverConnect();

    int getSaverState();

    void setSaverState();

    bool releaseKyInhibit(quint32 &id);

    quint32 requestKyInhibit(const QString &what, const QString &who, const QString &why, const QString &mode);

    /**
     * @brief 设置冻结使能
     * bool enable   true 冻结   false 解冻
     */
    void setFreezeEnable(bool enable);
    
private:
    ModuleManager *mManager;
    usminhibit *minhibit;
    PowerProvider *m_systemdProvider;
    UKUIProvider *m_ukuiProvider;

    QHash<QString, QList<quint32>> m_hashInhibitionServices;
    QDBusServiceWatcher *m_serviceWatcher = nullptr;

    QString m_strSessionPath = "";
    QHash<QString, QString> m_windows;

    AbnormalWindow *m_abnormalWindow = nullptr;

    bool m_saverHasConnect = false;
    ScreenSaverProxy *m_saverInterface = nullptr;
    LockStatus m_lockStatus = UNDEFINED;
    QProcess m_saverProcess;
    int m_isAuthenticateCompete = SaverStates::UNDEFINE;
    bool m_isSaverBackendAbnormal = false;

    quint32 m_kyInhibitId = 0;
};

#endif // SESSIONINHIBITCONTEXT_H

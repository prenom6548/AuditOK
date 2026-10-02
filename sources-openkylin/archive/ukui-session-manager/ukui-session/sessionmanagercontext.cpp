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
#include "sessionmanagercontext.h"
#include "sessiondbusadaptor.h"
#include "../tools/lockchecker.h"
#include <QMessageBox>
#include <QPushButton>
#include <QDBusMetaType>
#include <QDBusAbstractAdaptor>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <polkit-qt5-1/PolkitQt1/Authority>

#define WLCOM_SERVICE_BUSNAME "com.kylin.Wlcom"
#define WLCOM_SERVICE_PATH "/com/kylin/Wlcom/View"
#define WLCOM_SERVICE_INTERFACE "com.kylin.Wlcom.View"

#define SS_DBUS_SERVICE "org.ukui.ScreenSaver"
#define SS_DBUS_PATH "/"
#define SS_DBUS_INTERFACE "org.ukui.ScreenSaver"

#define LOGIN1_DBUS_SERVICE "org.freedesktop.login1"
#define LOGIN1_DBUS_PATH "/org/freedesktop/login1"
#define LOGIN1_DBUS_INTERFACE "org.freedesktop.login1.Manager"
#define LOGIN1_DBUS_SESSION_INTERFACE "org.freedesktop.login1.Session"

#define UNIAUTH_DBUS_SERVICE "org.ukui.UniauthBackend"
#define UNIAUTH_DBUS_PATH "/org/ukui/UniauthBackend"
#define UNIAUTH_DBUS_INTERFACE "org.ukui.UniauthBackend"

#define KYINHIBIT_DBUS_SERVICE "org.freedesktop.kyinhibit"
#define KYINHIBIT_DBUS_PATH "/org/freedesktop/kyinhibit"
#define KYINHIBIT_DBUS_INTERFACE "org.freedesktop.kyinhibit.Manager"

SessionManagerDBusContext::SessionManagerDBusContext(ModuleManager *manager, QObject *parent)
    : QObject(parent)
    , mManager(manager)
    , minhibit(new usminhibit())
    , m_systemdProvider(new SystemdProvider())
    , m_ukuiProvider(new UKUIProvider())
    , m_serviceWatcher(new QDBusServiceWatcher(this))
{
    new SessionDBusAdaptor(this);

    connect(mManager, &ModuleManager::moduleStateChanged, this, &SessionManagerDBusContext::moduleStateChanged);
    connect(mManager, &ModuleManager::finished, this, &SessionManagerDBusContext::emitPrepareForPhase2);
    connect(
        mManager,
        &ModuleManager::screensaverServiceStatusChanged,
        this,
        &SessionManagerDBusContext::onScreensaverServiceStatusChanged);
    connect(
        mManager, &ModuleManager::saverBackendAbnormaled, this, &SessionManagerDBusContext::onSaverBackendAbnormaled);
    connect(minhibit, &usminhibit::inhibitRemove, this, &SessionManagerDBusContext::doSimulateUserActivity);
    connect(minhibit, &usminhibit::inhibitAdd, this, &SessionManagerDBusContext::doSimulateUserActivity);
    connect(minhibit, &usminhibit::logoutInhibitAdd, this, &SessionManagerDBusContext::logoutInhibitAdd);
    connect(minhibit, &usminhibit::logoutInhibitRemove, this, &SessionManagerDBusContext::logoutInhibitRemove);
    connect(mManager, &ModuleManager::windowAdded, this, &SessionManagerDBusContext::onWindowAdded);
    connect(mManager, &ModuleManager::windowRemoved, this, &SessionManagerDBusContext::onWindowRemoved);
    m_serviceWatcher->setConnection(QDBusConnection::sessionBus());
    m_serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(
        m_serviceWatcher,
        &QDBusServiceWatcher::serviceUnregistered,
        this,
        &SessionManagerDBusContext::on_serviceUnregistered);

    // 注册类型
    qDBusRegisterMetaType<InhibitInfo::InhibitorInfo>();
    qDBusRegisterMetaType<QVector<InhibitInfo::InhibitorInfo>>();
    qDBusRegisterMetaType<InhibitInfoAndUid::InhibitorInfo>();
    qDBusRegisterMetaType<QVector<InhibitInfoAndUid::InhibitorInfo>>();
    qDBusRegisterMetaType<SystemdInhibitor::Inhibitor>();

    if (!m_abnormalWindow) {
        m_abnormalWindow = new AbnormalWindow();
    }
}

bool SessionManagerDBusContext::checkAuthorization(const QString& actionId)
{
    // 确保当前有有效的 D-Bus 调用上下文
    QDBusMessage msg = message();
    if (msg.type() == QDBusMessage::InvalidMessage) {
        qWarning() << "checkAuthorization called without valid D-Bus call context for action:" << actionId;
        sendErrorReply(QDBusError::AccessDenied,
                       "Access denied: no valid D-Bus call context");
        return false;
    }

    // 1. Get the caller's PID from the D-Bus session bus connection
    QDBusConnectionInterface *iface = connection().interface();
    if (!iface) {
        qWarning() << "checkAuthorization: unable to get D-Bus connection interface";
        sendErrorReply(QDBusError::AccessDenied, "Access denied: unable to determine caller");
        return false;
    }
    unsigned int pid = iface->servicePid(msg.service()).value();

    // 2. Create Polkit Subject based on Unix PID
    PolkitQt1::UnixProcessSubject subject(static_cast<qint64>(pid));

    // 3. Check authorization (no user interaction — policy decides)
    PolkitQt1::Authority::Result result =
        PolkitQt1::Authority::instance()->checkAuthorizationSync(
            actionId, subject, PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::Yes) {
        return true;
    }

    // 4. Authorization denied — send D-Bus error reply
    sendErrorReply(QDBusError::AccessDenied,
                   "Access denied: operation not permitted from remote or inactive session");
    return false;
}

SessionManagerDBusContext::~SessionManagerDBusContext() {}

Q_NOREPLY void SessionManagerDBusContext::startupfinished(const QString &appName, const QString &string)
{
    qDebug() << "  startupfinished appName = " << appName;
    if (appName == "ukui-screensaver-backend") {
        qDebug() << " m_lockStatus = " << m_lockStatus << " getAuthenticateCompeteState() = " << getAuthenticateCompeteState();
        if (!m_saverHasConnect) {
            initSaverConnect();
            if (m_isAuthenticateCompete == SaverStates::ISSTARTUP) {
                if (!m_abnormalWindow) {
                    m_abnormalWindow = new AbnormalWindow();
                }
                m_abnormalWindow->setGeometry(QApplication::desktop()->geometry());
                m_abnormalWindow->showAbnormalWindow();
            }
        } else if ((m_lockStatus == SCREEN_LOCK
                    || m_lockStatus == SCREEN_SAVER_LOCK)) { // 锁屏后端崩溃时处于锁屏或者屏保状态，重新拉起锁屏进程
            m_saverInterface->call("Lock");
        } else if (m_lockStatus == SCREEN_SAVER) {
            m_saverInterface->call("ShowScreensaver");
        }
    }
    return mManager->startupfinished(appName, string);
}

bool SessionManagerDBusContext::canSwitch()
{
    if (!Ukuilockinfo::getCfgValue(Ukuilockinfo::buttonType::switchuserBtn)
        || !m_systemdProvider->canAction(UkuiPower::PowerSwitchUser)) {
        return false;
    }

    if (!QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.globalManager")) {
        return true;
    }

    QGSettings globalSettings("org.ukui.SettingsDaemon.plugins.globalManager");
    if (globalSettings.get("disable-power-operation").toBool()) {
        return false;
    }

    QStringList hideList = globalSettings.get("disable-shutdown-option").toStringList();
    if (hideList.contains(QStringLiteral("switchuser"))) {
        return false;
    }

    return true; // GCOV_EXCL_LINE
}

bool SessionManagerDBusContext::canLockscreen()
{
    if (!Ukuilockinfo::getCfgValue(Ukuilockinfo::buttonType::lockscreenBtn)) {
        return false;
    }

    // 检查schema是否存在
    if (!QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.globalManager")) {
        return true;
    }

    QGSettings globalSettings("org.ukui.SettingsDaemon.plugins.globalManager");
    if (globalSettings.get("disable-power-operation").toBool()) {
        return false;
    }

    QStringList hideList = globalSettings.get("disable-shutdown-option").toStringList();
    if (hideList.contains(QStringLiteral("lockscreen"))) {
        return false;
    }

    return true; // GCOV_EXCL_LINE
}

bool SessionManagerDBusContext::canHibernate()
{
    if (!Ukuilockinfo::getCfgValue(Ukuilockinfo::buttonType::hibernateBtn)
        || !m_systemdProvider->canAction(UkuiPower::PowerHibernate)) {
        return false;
    }

    if (!QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.globalManager")) {
        return true;
    }

    QGSettings globalSettings("org.ukui.SettingsDaemon.plugins.globalManager");
    if (globalSettings.get("disable-power-operation").toBool()) {
        return false;
    }

    QStringList hideList = globalSettings.get("disable-shutdown-option").toStringList();
    if (hideList.contains(QStringLiteral("hibernate"))) {
        return false;
    }

    return true; // GCOV_EXCL_LINE
}

bool SessionManagerDBusContext::canSuspend()
{
    if (!Ukuilockinfo::getCfgValue(Ukuilockinfo::buttonType::suspendBtn)
        || !m_systemdProvider->canAction(UkuiPower::PowerSuspend)) {
        return false;
    }

    if (!QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.globalManager")) {
        return true;
    }

    QGSettings globalSettings("org.ukui.SettingsDaemon.plugins.globalManager");
    if (globalSettings.get("disable-power-operation").toBool()) {
        return false;
    }

    QStringList hideList = globalSettings.get("disable-shutdown-option").toStringList();
    if (hideList.contains(QStringLiteral("suspend"))) {
        return false;
    }

    return true; // GCOV_EXCL_LINE
}

bool SessionManagerDBusContext::canLogout()
{
    // 暂时都返回true
    if (!Ukuilockinfo::getCfgValue(Ukuilockinfo::buttonType::logoutBtn)) {
        return false;
    }

    if (!QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.globalManager")) {
        return true;
    }

    QGSettings globalSettings("org.ukui.SettingsDaemon.plugins.globalManager");
    if (globalSettings.get("disable-power-operation").toBool()) {
        return false;
    }

    QStringList hideList = globalSettings.get("disable-shutdown-option").toStringList();
    if (hideList.contains(QStringLiteral("logout"))) {
        return false;
    }

    return true; // GCOV_EXCL_LINE
}

bool SessionManagerDBusContext::canReboot()
{
    if (!Ukuilockinfo::getCfgValue(Ukuilockinfo::buttonType::rebootBtn)
        || !m_systemdProvider->canAction(UkuiPower::PowerReboot)) {
        return false;
    }

    if (!QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.globalManager")) {
        return true;
    }

    QGSettings globalSettings("org.ukui.SettingsDaemon.plugins.globalManager");
    if (globalSettings.get("disable-power-operation").toBool()) {
        return false;
    }

    QStringList hideList = globalSettings.get("disable-shutdown-option").toStringList();
    if (hideList.contains(QStringLiteral("restart"))) {
        return false;
    }

    return true; // GCOV_EXCL_LINE
}

bool SessionManagerDBusContext::canPowerOff()
{
    if (!Ukuilockinfo::getCfgValue(Ukuilockinfo::buttonType::shutdownBtn)
        || !m_systemdProvider->canAction(UkuiPower::PowerShutdown)) {
        return false;
    }

    if (!QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.globalManager")) {
        return true;
    }

    QGSettings globalSettings("org.ukui.SettingsDaemon.plugins.globalManager");
    if (globalSettings.get("disable-power-operation").toBool()) {
        return false;
    }

    QStringList hideList = globalSettings.get("disable-shutdown-option").toStringList();
    if (hideList.contains(QStringLiteral("shutdown"))) {
        return false;
    }

    return true; // GCOV_EXCL_LINE
}

// GCOV_EXCL_START:The interface is to other applications through remote calls, it is inconvenient to use a single test
// method to execute
Q_NOREPLY void SessionManagerDBusContext::switchUser()
{
    if (!checkAuthorization("org.gnome.SessionManager.switch-user")) {
        return;
    }
    m_systemdProvider->doAction(UkuiPower::PowerSwitchUser);
}

Q_NOREPLY void SessionManagerDBusContext::switchToUser(QString strUserName)
{
    if (canSwitch()) {
        m_ukuiProvider->doLSAction(UkuiPower::PowerSwitchToUser, strUserName);
    } else {
        qDebug() << "Can't switch user";
    }
}

Q_NOREPLY void SessionManagerDBusContext::hibernate()
{
    if (!checkAuthorization("org.gnome.SessionManager.hibernate")) {
        return;
    }
    
    if (isInnerCaller()) {
        m_systemdProvider->doAction(UkuiPower::PowerHibernate); // GCOV_EXCL_LINE
    } else {
        m_ukuiProvider->doLSAction(UkuiPower::PowerHibernate);
    }
}

Q_NOREPLY void SessionManagerDBusContext::suspend()
{
    if (!checkAuthorization("org.gnome.SessionManager.suspend")) {
        return;
    }

    if (isInnerCaller()) {
        m_systemdProvider->doAction(UkuiPower::PowerSuspend); // GCOV_EXCL_LINE
    } else {
        qDebug() << "exec ukui-session-tools suspend";
        QProcess::startDetached("ukui-session-tools --suspend");
    }
}

Q_NOREPLY void SessionManagerDBusContext::closeWindows(const QString &power)
{
    if (!checkAuthorization("org.gnome.SessionManager.close-windows")) {
        return;
    }

    if (isInnerCaller()) {
        qDebug() << "ukui-session" << power << "interface called";
        if (mManager->getIsShutingDown()) {
            qDebug() << "already performing" << power;
            return;
        }

        mManager->setIsShutingDown(true);

        m_kyInhibitId = requestKyInhibit("sleep", "ukui-session", "for shutdown", "block");

        QDBusInterface ukuismserverFace(
            "org.ukui.ukuismserver", "/UKUISMServer", "org.ukui.ukuismserver", QDBusConnection::sessionBus());

        if (!ukuismserverFace.isValid()) {
            qDebug() << "ukuisminterface not valid";
            return;
        }

        // 分级冻结应用唤醒
        setFreezeEnable(false);

        // QDBusAbstractInterface有超时机制，默认是25秒，25秒后没有返回值，会给出一个带有错误信息的返回值，供调用方判断做出下一步动作，也正是因此KDE桌面不会在某个应用的提醒保存页面一直
        // 停留，而是会继续往下执行注销操作，可以通过setTimerout设置超时时间，实验证明确实如此
        QList<QVariant> argumentList;
        QDBusPendingReply<bool> closeSessionReply
            = ukuismserverFace.asyncCallWithArgumentList(QStringLiteral("closeSession"), argumentList);
        auto watcher = new QDBusPendingCallWatcher(closeSessionReply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [closeSessionReply, watcher, power, this]() {
            watcher->deleteLater();
            if (closeSessionReply.isError()) {
                qDebug() << "ukuismserver failed to complete logout with error" << closeSessionReply.error().name()
                         << " continuining with" << power;
                callCloseWindows();
            }
            if (closeSessionReply.value()) {
                qDebug() << "ukuismserver close session completed";
                callCloseWindows();
            } else {
                mManager->setIsShutingDown(false);
                qDebug() << power << "cancelled";
            }
        });
    } else {
        qDebug() << "exec ukui-session-tools" << power;
        QString cmd = "ukui-session-tools --" + power;
        QProcess::startDetached(cmd);
    }
}

Q_NOREPLY void SessionManagerDBusContext::logout()
{
    if (!checkAuthorization("org.gnome.SessionManager.logout")) {
        return;
    }

    if (isInnerCaller()) {
        qDebug() << "ukui-session logout interface called";
        if (mManager->getIsShutingDown()) {
            qDebug() << "already performing logout";
            return;
        }

        mManager->setIsShutingDown(true);

        QDBusInterface ukuismserverFace(
            "org.ukui.ukuismserver", "/UKUISMServer", "org.ukui.ukuismserver", QDBusConnection::sessionBus());

        if (!ukuismserverFace.isValid()) {
            qDebug() << "ukuisminterface not valid";
            return;
        }

        setFreezeEnable(false);

        // QDBusAbstractInterface有超时机制，默认是25秒，25秒后没有返回值，会给出一个带有错误信息的返回值，供调用方判断做出下一步动作，也正是因此KDE桌面不会在某个应用的提醒保存页面一直
        // 停留，而是会继续往下执行注销操作，可以通过setTimerout设置超时时间，实验证明确实如此
        QList<QVariant> argumentList;
        QDBusPendingReply<bool> closeSessionReply
            = ukuismserverFace.asyncCallWithArgumentList(QStringLiteral("closeSession"), argumentList);
        auto watcher = new QDBusPendingCallWatcher(closeSessionReply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [closeSessionReply, watcher, this]() {
            watcher->deleteLater();
            if (closeSessionReply.isError()) {
                qDebug() << "ukuismserver failed to complete logout with error" << closeSessionReply.error().name()
                         << " continuining with logout";
                ukuiServerComplete("logout");
            }
            if (closeSessionReply.value()) {
                qDebug() << "ukuismserver close session completed";
                ukuiServerComplete("logout");
            } else {
                mManager->setIsShutingDown(false);
                setFreezeEnable(true);
                qDebug() << "logout cancelled";
            }
        });
    } else {
        qDebug() << "exec ukui-session-tools logout";
        QProcess::startDetached("ukui-session-tools --logout");
    }
}

Q_NOREPLY void SessionManagerDBusContext::reboot()
{
    if (!checkAuthorization("org.gnome.SessionManager.reboot")) {
        return;
    }

    if (isInnerCaller()) {
        qDebug() << "ukui-session reboot interface called";
        if (mManager->getIsShutingDown()) {
            qDebug() << "already performing reboot";
            return;
        }

        mManager->setIsShutingDown(true);

        m_kyInhibitId = requestKyInhibit("sleep", "ukui-session", "for shutdown", "block");

        QDBusInterface ukuismserverFace(
            "org.ukui.ukuismserver", "/UKUISMServer", "org.ukui.ukuismserver", QDBusConnection::sessionBus());

        if (!ukuismserverFace.isValid()) {
            qDebug() << "ukuisminterface not valid";
            return;
        }

        setFreezeEnable(false);

        // QDBusAbstractInterface有超时机制，默认是25秒，25秒后没有返回值，会给出一个带有错误信息的返回值，供调用方判断做出下一步动作，也正是因此KDE桌面不会在某个应用的提醒保存页面一直
        // 停留，而是会继续往下执行注销操作，可以通过setTimerout设置超时时间，实验证明确实如此
        QList<QVariant> argumentList;
        QDBusPendingReply<bool> closeSessionReply
            = ukuismserverFace.asyncCallWithArgumentList(QStringLiteral("closeSession"), argumentList);
        auto watcher = new QDBusPendingCallWatcher(closeSessionReply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [closeSessionReply, watcher, this]() {
            watcher->deleteLater();
            if (closeSessionReply.isError()) {
                qDebug() << "ukuismserver failed to complete logout with error" << closeSessionReply.error().name()
                         << " continuining with reboot";
                ukuiServerComplete("reboot");
            }
            if (closeSessionReply.value()) {
                qDebug() << "ukuismserver close session completed";
                ukuiServerComplete("reboot");
            } else {
                mManager->setIsShutingDown(false);
                setFreezeEnable(true);
                qDebug() << "logout cancelled";
            }
            releaseKyInhibit(m_kyInhibitId);
        });
    } else {
        qDebug() << "exec ukui-session-tools reboot";
        QProcess::startDetached("ukui-session-tools --reboot");
    }
}

Q_NOREPLY void SessionManagerDBusContext::powerOff()
{
    if (!checkAuthorization("org.gnome.SessionManager.poweroff")) {
        return;
    }

    if (isInnerCaller()) {
        qDebug() << "ukui-session poweroff interface called";
        if (mManager->getIsShutingDown()) {
            qDebug() << "already performing poweroff";
            return;
        }

        mManager->setIsShutingDown(true); // GCOV_EXCL_LINE

        m_kyInhibitId = requestKyInhibit("sleep", "ukui-session", "for shutdown", "block");

        QDBusInterface ukuismserverFace(
            "org.ukui.ukuismserver", "/UKUISMServer", "org.ukui.ukuismserver", QDBusConnection::sessionBus());

        if (!ukuismserverFace.isValid()) {           // GCOV_EXCL_LINE
            qDebug() << "ukuisminterface not valid"; // GCOV_EXCL_LINE
            return;                                  // GCOV_EXCL_LINE
        }

        setFreezeEnable(false);

        // QDBusAbstractInterface有超时机制，默认是25秒，25秒后没有返回值，会给出一个带有错误信息的返回值，供调用方判断做出下一步动作，也正是因此KDE桌面不会在某个应用的提醒保存页面一直
        // 停留，而是会继续往下执行注销操作，可以通过setTimerout设置超时时间，实验证明确实如此
        QList<QVariant> argumentList;
        QDBusPendingReply<bool> closeSessionReply
            = ukuismserverFace.asyncCallWithArgumentList(QStringLiteral("closeSession"), argumentList);
        auto watcher = new QDBusPendingCallWatcher(closeSessionReply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [closeSessionReply, watcher, this]() {
            watcher->deleteLater();
            if (closeSessionReply.isError()) {
                qDebug() << "ukuismserver failed to complete logout with error" << closeSessionReply.error().name()
                         << " continuining with powerOff";
                ukuiServerComplete("shutdown");
            }
            if (closeSessionReply.value()) {
                qDebug() << "ukuismserver close session completed";
                ukuiServerComplete("shutdown");
            } else {
                mManager->setIsShutingDown(false);
                setFreezeEnable(true);
                qDebug() << "logout cancelled";
            }
            releaseKyInhibit(m_kyInhibitId);
        });
    } else {
        qDebug() << "exec ukui-session-tools --shutdown";
        QProcess::startDetached("ukui-session-tools --shutdown");
    }
}

void SessionManagerDBusContext::callCloseWindows()
{
    mManager->setIsShutingDown(false);
    QDBusInterface wlcomFace(
        WLCOM_SERVICE_BUSNAME, WLCOM_SERVICE_PATH, WLCOM_SERVICE_INTERFACE, QDBusConnection::sessionBus());

    if (!wlcomFace.isValid()) {            // GCOV_EXCL_LINE
        qDebug() << "wlcomface not valid"; // GCOV_EXCL_LINE
        return;                            // GCOV_EXCL_LINE
    }
    QList<QVariant> argumentList;
    QDBusPendingReply<bool> closeWindowsReply
        = wlcomFace.asyncCallWithArgumentList(QStringLiteral("CloseWindows"), argumentList);
    auto watcher = new QDBusPendingCallWatcher(closeWindowsReply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [closeWindowsReply, watcher, this]() {
        watcher->deleteLater();
        if (closeWindowsReply.isError()) {
            qDebug() << "wlcom failed to complete close window" << closeWindowsReply.error().name();
        }
        qDebug() << "call CloseWindows success";
    });
}

void SessionManagerDBusContext::ukuiServerComplete(QString type)
{
    mManager->setIsShutingDown(false);
    QDBusInterface wlcomFace(
        WLCOM_SERVICE_BUSNAME, WLCOM_SERVICE_PATH, WLCOM_SERVICE_INTERFACE, QDBusConnection::sessionBus());

    if (!wlcomFace.isValid()) {            // GCOV_EXCL_LINE
        qDebug() << "wlcomface not valid"; // GCOV_EXCL_LINE
        if (type == "logout") {
            logoutComplete();
        } else if (type == "shutdown") {
            shutdownComplete();
        } else if (type == "reboot") {
            rebootComplete();
        }
    } else {
        QList<QVariant> argumentList;
        QDBusPendingReply<bool> closeWindowsReply
        = wlcomFace.asyncCallWithArgumentList(QStringLiteral("CloseWindows"), argumentList);
        auto watcher = new QDBusPendingCallWatcher(closeWindowsReply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [closeWindowsReply, watcher, type, this]() {
            watcher->deleteLater();
            if (closeWindowsReply.isError()) {
                qDebug() << "wlcom failed to complete close window" << closeWindowsReply.error().name();
            }
            if (closeWindowsReply.value()) {
                qDebug() << "wlcom close window completed";
                if (type == "logout") {
                    logoutComplete();
                } else if (type == "shutdown") {
                    shutdownComplete();
                } else if (type == "reboot") {
                    rebootComplete();
                }
            } else {
                qDebug() << "logout cancelled";
            }
        }); 
    }
}

void SessionManagerDBusContext::initSaverConnect()
{
    qDebug() << " initSaverConnect !!! ";
    m_saverInterface = new ScreenSaverProxy(SS_DBUS_SERVICE, SS_DBUS_PATH, QDBusConnection::sessionBus(), this);
    if (!m_saverInterface || !m_saverInterface->isValid()) {
        qDebug() << "m_saverInterface not valid";
        return;
    }

    connect(m_saverInterface, &ScreenSaverProxy::prepareForLock, this, &SessionManagerDBusContext::onScreenLock);

    bool result
        = connect(m_saverInterface, &ScreenSaverProxy::unlock, this, &SessionManagerDBusContext::onScreenUnLock);
    if (result) {
        qDebug() << "Successfully connected to unlock signal";
    } else {
        qDebug() << "Failed to connect to unlock signal";
    }

    connect(m_saverInterface, &ScreenSaverProxy::SessionIdle, this, &SessionManagerDBusContext::onScreenSaver);

    connect(m_saverInterface, &ScreenSaverProxy::ScreenSaverLock, this, &SessionManagerDBusContext::onScreenSaverLock);

    QDBusInterface *m_loginMgrInterface = new QDBusInterface(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        QDBusConnection::systemBus(),
        this);

    QDBusReply<QDBusObjectPath> sessionPath = m_loginMgrInterface->call("GetSession", "auto");
    if (!sessionPath.isValid()) {
        qWarning() << "Get session error:" << sessionPath.error();
    } else {
        QString session = sessionPath.value().path();
        QDBusConnection::systemBus().connect(
            LOGIN1_DBUS_SERVICE,
            session,
            LOGIN1_DBUS_SESSION_INTERFACE,
            QStringLiteral("Unlock"),
            this,
            SLOT(onLogin1UnLockScreen()));
    }
    delete m_loginMgrInterface;
    m_saverHasConnect = true;
}

int SessionManagerDBusContext::getAuthenticateCompeteState()
{
    return m_isAuthenticateCompete;
}

void SessionManagerDBusContext::setAuthenticateCompeteState(int state)
{
    m_isAuthenticateCompete = state;
}

void SessionManagerDBusContext::onScreensaverServiceStatusChanged(bool isLockStartup)
{
    qDebug() << " onScreensaverServiceStatusChanged !!! << m_lockStatus = " << m_lockStatus;
    if (!m_saverHasConnect) {
        initSaverConnect();
        if (m_isAuthenticateCompete == SaverStates::ISSTARTUP) {
            if (!m_abnormalWindow) {
                m_abnormalWindow = new AbnormalWindow();
            }
            m_abnormalWindow->setGeometry(QApplication::desktop()->geometry());
            m_abnormalWindow->showAbnormalWindow();
        }
    } else if ((m_lockStatus == SCREEN_LOCK || m_lockStatus == SCREEN_SAVER_LOCK
                || m_lockStatus == UNDEFINED)) { // 锁屏后端崩溃时处于锁屏或者屏保状态，重新拉起锁屏进程
        m_saverInterface->call("Lock");
    } else if (m_lockStatus == SCREEN_SAVER) {
        m_saverInterface->call("ShowScreensaver");
    }
}

void SessionManagerDBusContext::onSaverBackendAbnormaled()
{
    if (m_lockStatus != SCREEN_UNLOCK && m_lockStatus != UNDEFINED) {
        m_isSaverBackendAbnormal = true; // 锁屏后端多次崩溃不再拉起时，记录状态
        showEmergencyWindow();
    }
}

int SessionManagerDBusContext::GetScreenLockStatus()
{
    return m_lockStatus;
}

Q_NOREPLY void SessionManagerDBusContext::showBlackWindow()
{
    if (m_abnormalWindow) {
        qDebug() << "showBlackWindow!!!";
        m_abnormalWindow->setGeometry(QApplication::desktop()->geometry());
        m_abnormalWindow->showAbnormalWindow();
    }
}

Q_NOREPLY void SessionManagerDBusContext::hideBlackWindow()
{
    if (m_abnormalWindow) {
        // m_abnormalWindow->hide();
        m_abnormalWindow->hideAbnormalWindow();
    }
}

Q_NOREPLY void SessionManagerDBusContext::showEmergencyWindow()
{
    if (m_abnormalWindow) {
        m_abnormalWindow->setGeometry(QApplication::desktop()->geometry());
        m_abnormalWindow->showEmergencyWindow();
    }
}

Q_NOREPLY void SessionManagerDBusContext::execComplete(const QString type)
{
    if (type == "logout") {
        logoutComplete();
    } else if (type == "powerOff") {
        shutdownComplete();
    } else if (type == "reboot") {
        rebootComplete();
    } else if (type == "cancel") {
        qDebug() << "User cancel execComplete";
        mManager->setIsShutingDown(false);
        releaseKyInhibit(m_kyInhibitId);
        //取消电源操作恢复冻结
        setFreezeEnable(true);
    }
}

Q_NOREPLY void SessionManagerDBusContext::startModule(const QString &name)
{
    if (!checkAuthorization("org.gnome.SessionManager.start-module")) {
       return;
    }
    mManager->startProcess(name, true);
}

Q_NOREPLY void SessionManagerDBusContext::stopModule(const QString &name)
{
    if (!checkAuthorization("org.gnome.SessionManager.stop-module")) {
        return;
    }
    mManager->stopProcess(name);
}

bool SessionManagerDBusContext::startApp(const QString &name, const QStringList &args)
{
    if (!checkAuthorization("org.gnome.SessionManager.start-app")) {
       return false;
    }
    return mManager->startProcess(name, args);
}

uint SessionManagerDBusContext::Inhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags)
{
    uint result = minhibit->addInhibit(app_id, toplevel_xid, reason, flags);
    if (result < 0) {
        return 0;
    }

    QString dbusService = message().service();
    qDebug() << "SessionManagerDBusContext message().service():" << dbusService;

    QStringList keys = m_hashInhibitionServices.keys();
    if (!keys.contains(dbusService)) {
        QList<quint32> cookies;
        cookies.append(result);
        m_hashInhibitionServices.insert(dbusService, cookies);
        m_serviceWatcher->addWatchedService(dbusService);
        qDebug() << "m_serviceWatcher services:..." << m_serviceWatcher->watchedServices();
    } else {
        for (auto iter = m_hashInhibitionServices.begin(); iter != m_hashInhibitionServices.end(); iter++) {
            qDebug() << "Inhibit iter.key()" << iter.key() << dbusService;

            if (iter.key() == dbusService) {
                QList<quint32> cookies = iter.value();
                if (!cookies.contains(result)) {
                    cookies.append(result);
                    m_hashInhibitionServices[dbusService] = cookies;
                    break;
                }
            }
        }
    }
    qDebug() << "Inhibit m_hashInhibitionServices..." << m_hashInhibitionServices;
    emit this->inhibitadded(flags);
    return result;
}

Q_NOREPLY void SessionManagerDBusContext::Uninhibit(uint cookie)
{
    uint result = minhibit->unInhibit(cookie);
    if (result > 0) {
        emit inhibitremove(result);
    }
    for (auto iter = m_hashInhibitionServices.begin(); iter != m_hashInhibitionServices.end(); iter++) {
        QList<quint32> cookies = iter.value();
        for (int i = 0; i < cookies.length(); i++) {
            if (cookie == cookies[i]) {
                qDebug() << "Uninhibit cookies:" << cookies << "cookie:" << cookie;
                if (cookies.length() > 1) {
                    cookies.removeAt(i);
                    m_hashInhibitionServices[iter.key()] = cookies;
                } else {
                    m_serviceWatcher->removeWatchedService(iter.key());
                    m_hashInhibitionServices.remove(iter.key());
                    qDebug() << "Uninhibit m_hashInhibitionServices...:" << m_hashInhibitionServices;
                }
                qDebug() << "Uninhibit m_hashInhibitionServices:" << m_hashInhibitionServices;
                return;
            }
        }
    }
}

QStringList SessionManagerDBusContext::GetInhibitors()
{
    return minhibit->getInhibitor();
}

QStringList SessionManagerDBusContext::GetAppWindows()
{
    return m_windows.values();
}

QVector<InhibitInfo::InhibitorInfo> SessionManagerDBusContext::ListInhibitor(QString type)
{
    QVector<InhibitInfoAndUid::InhibitorInfo> resultAndUid;
    if (type == QString("logout")) {
        resultAndUid = Ukuilockinfo::listInhibitorInfo(Ukuilockinfo::InhibitorType::logout);
    } else if (type == QString("sleep")) {
        resultAndUid = Ukuilockinfo::listInhibitorInfo(Ukuilockinfo::InhibitorType::suspend);
    } else if (type == QString("shutdown")) {
        resultAndUid = Ukuilockinfo::listInhibitorInfo(Ukuilockinfo::InhibitorType::shutdown);
    }

    // 预分配 + 索引赋值，避免扩容触发 QString move 导致数据损坏
    QVector<InhibitInfo::InhibitorInfo> result(resultAndUid.size());
    for (int i = 0; i < resultAndUid.size(); ++i) {
        result[i].name = resultAndUid.at(i).name;
        result[i].icon = resultAndUid.at(i).icon;
    }
    return result;
}

QVector<InhibitInfoAndUid::InhibitorInfo> SessionManagerDBusContext::ListInhibitorAndUid(QString type)
{
    QVector<InhibitInfoAndUid::InhibitorInfo> result;

    if (type == QString("logout")) {
        result = Ukuilockinfo::listInhibitorInfo(Ukuilockinfo::InhibitorType::logout);
    } else if (type == QString("sleep")) {
        result = Ukuilockinfo::listInhibitorInfo(Ukuilockinfo::InhibitorType::suspend);
    } else if (type == QString("shutdown")) {
        result = Ukuilockinfo::listInhibitorInfo(Ukuilockinfo::InhibitorType::shutdown);
    }
    return result;
}

bool SessionManagerDBusContext::IsSessionRunning()
{
    QString xdg_session_desktop = qgetenv("XDG_SESSION_DESKTOP").toLower();
    if (xdg_session_desktop == "ukui" || xdg_session_desktop == "ukui-wayland") {
        return true;
    }
    return false;
}

QString SessionManagerDBusContext::GetSessionName()
{
    QString xdg_session_desktop = qgetenv("XDG_SESSION_DESKTOP").toLower();
    if (xdg_session_desktop == "ukui" || xdg_session_desktop == "ukui-wayland"
        || xdg_session_desktop == "kylin-wlcom") {
        return "ukui-session";
    }

    return "error";
}

bool SessionManagerDBusContext::IsInhibited(quint32 flags)
{
    return minhibit->isInhibited(flags);
}

void SessionManagerDBusContext::setSessionEnv(const QString &key, const QString &value)
{
    if (!checkAuthorization("org.gnome.SessionManager.set-session-env")) {
        return;
    }
    qDebug() << "set env " << key << " as " << value;
    qputenv(key.toLatin1(), value.toLatin1());
}

Q_NOREPLY void SessionManagerDBusContext::emitStartLogout()
{
    if (!checkAuthorization("org.gnome.SessionManager.logout")) {
        return;
    }
    qDebug() << "emit  StartLogout";
    emit StartLogout();
}

Q_NOREPLY void SessionManagerDBusContext::emitPrepareForSwitchuser()
{
    if (!checkAuthorization("org.gnome.SessionManager.switch-user")) {
        return;
    }
    qDebug() << "emit  PrepareForSwitchuser";
    emit PrepareForSwitchuser();
}

Q_NOREPLY void SessionManagerDBusContext::emitPrepareForPhase2()
{
    qDebug() << "emit  PrepareForPhase2";
    emit PrepareForPhase2();
}

void SessionManagerDBusContext::doSimulateUserActivity()
{
    qDebug() << "simulate User Activity (internal)";
    KIdleTime::instance()->simulateUserActivity();
}

Q_NOREPLY void SessionManagerDBusContext::simulateUserActivity()
{
    if (!checkAuthorization("org.gnome.SessionManager.simulate-user-activity")) {
        return;
    }
    doSimulateUserActivity();
}

void SessionManagerDBusContext::on_serviceUnregistered(const QString &serviceName)
{
    qDebug() << "onServiceUnregistered..." << serviceName;

    for (auto iter = m_hashInhibitionServices.begin(); iter != m_hashInhibitionServices.end(); iter++) {
        if (iter.key() == serviceName) {
            QList<quint32> cookies = iter.value();
            for (auto i = 0; i < cookies.length(); i++) {
                quint32 cookie = cookies[i];
                qDebug() << "on_serviceUnregistered cookie:" << cookie;
                Uninhibit(cookie);
            }
            return;
        }
    }
    qDebug() << "on_serviceUnregistered unfind serviceName:" << serviceName;
}
// GCOV_EXCL_STOP

void SessionManagerDBusContext::onScreenLock()
{
    if (m_abnormalWindow) {
        qDebug() << "onScreenLock!!!";
        m_abnormalWindow->setGeometry(QApplication::desktop()->geometry());
        m_abnormalWindow->showAbnormalWindow();
    }
    m_lockStatus = SCREEN_LOCK;
}

void SessionManagerDBusContext::onScreenUnLock()
{
    if (m_abnormalWindow) {
        qDebug() << "onScreenUnLock!!!";
        m_abnormalWindow->hideAbnormalWindow();
        //  防止在锁屏崩溃多次显示提示界面后，空闲进入屏保后，解锁进入桌面，快捷键仍然没有释放
        m_abnormalWindow->blockSomeShortCut(false);
    }
    m_lockStatus = SCREEN_UNLOCK;
    mManager->onLockScrenUnlock();
}

void SessionManagerDBusContext::onScreenSaver()
{
    if (m_abnormalWindow) {
        qDebug() << "onScreenSaver!!!";
        m_abnormalWindow->setGeometry(QApplication::desktop()->geometry());
        m_abnormalWindow->showAbnormalWindow();
    }
    m_lockStatus = SCREEN_SAVER;
}

void SessionManagerDBusContext::onScreenSaverLock()
{
    if (m_abnormalWindow) {
        qDebug() << "onScreenSaverLock!!!";
        m_abnormalWindow->setGeometry(QApplication::desktop()->geometry());
        m_abnormalWindow->showAbnormalWindow();
    }
    m_lockStatus = SCREEN_SAVER_LOCK;
}

void SessionManagerDBusContext::onLogin1UnLockScreen()
{
    if (m_abnormalWindow) {
        qDebug() << "onLogin1UnLockScreen!!!";
        m_abnormalWindow->hideAbnormalWindow();
        m_abnormalWindow->blockSomeShortCut(false);
        m_lockStatus = SCREEN_UNLOCK;
    }
}

void SessionManagerDBusContext::executeBoxadm()
{
    QProcess *proc = new QProcess;
    QString argu = "--lockall";
    proc->start("boxadm", QStringList{ argu });
    proc->waitForFinished(-1);
    delete proc;
}

// 退出ukui-session.target
void SessionManagerDBusContext::stopUkuisessionTarget()
{
    auto msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.systemd1"),
        QStringLiteral("/org/freedesktop/systemd1"),
        QStringLiteral("org.freedesktop.systemd1.Manager"),
        QStringLiteral("StopUnit"));
    msg << QStringLiteral("ukui-session.target") << QStringLiteral("fail");
    QDBusConnection::sessionBus().call(msg);
}

// GCOV_EXCL_START:The interface is to other applications through remote calls, it is inconvenient to use a single test
// method to execute
void SessionManagerDBusContext::logoutComplete()
{
    // 注销前对文件保护箱加锁
    executeBoxadm();

    // 注销前执行指定目录脚本
    QStringList list = listFileList();
    if (list.isEmpty()) {
        qDebug() << "logouting file list is empty or dir is not exits.";
    } else {
        execPro(list);
    }

    // 调用logout
    qDebug() << "complete xsmp logout, call Terminal";

    if (m_strSessionPath.isEmpty()) {
        QDBusInterface dbus(
            "org.freedesktop.login1",
            "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager",
            QDBusConnection::systemBus());
        if (!dbus.isValid()) {
            qWarning() << "dbusCall: QDBusInterface is invalid org.freedesktop.login1";
            return;
        }

        QDBusReply<QDBusObjectPath> reply = dbus.call("GetSession", "auto");

        if (!reply.isValid()) {
            qDebug() << "GetSession error:" << reply.error().name() << reply.error().message();
            return;
        }
        m_strSessionPath = reply.value().path();
    }

    QDBusInterface interface(
        "org.freedesktop.login1", m_strSessionPath, "org.freedesktop.login1.Session", QDBusConnection::systemBus());
    if (!interface.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid org.freedesktop.login1";
        return;
    }

    interface.asyncCall("Terminate");

    stopUkuisessionTarget();
}

void SessionManagerDBusContext::rebootComplete()
{
    // 调用systemd的reboot
    qDebug() << "complete xsmp logout, call Reboot";
    QDBusInterface dbus(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        QDBusConnection::systemBus());
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid org.freedesktop.login1";
        return;
    }

    QDBusPendingReply<QString> reply;
    if (true) {
        reply = dbus.asyncCallWithArgumentList("Reboot", { QVariant(true) });
    } else {
        reply = dbus.asyncCall("Reboot");
    }
    auto watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [reply, watcher]() {
        watcher->deleteLater();
        if (reply.isError()) {
            qWarning() << "Debus error: " << reply.error();
        }
        if (!reply.value().isEmpty()) {
            QString ret = reply.value();
            if (ret == QLatin1String("yes") || ret == QLatin1String("challenge")) {
                qDebug() << "call systemd reboot success";
            }
        } else {
            qDebug() << "call systemd reboot failed";
        }
    });

    stopUkuisessionTarget();
}

void SessionManagerDBusContext::shutdownComplete()
{
    // 调用systemd的shutdown
    qDebug() << "complete xsmp logout, call powerOff";

    QDBusInterface dbus(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        QDBusConnection::systemBus());
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid org.freedesktop.login1";
        return;
    }

    QDBusPendingReply<QString> reply;
    if (true) {
        reply = dbus.asyncCallWithArgumentList("PowerOff", { QVariant(true) });
    } else {
        reply = dbus.asyncCall("PowerOff");
    }
    auto watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [reply, watcher]() {
        watcher->deleteLater();
        if (reply.isError()) {
            qWarning() << "Debus error: " << reply.error();
        }
        if (!reply.value().isEmpty()) {
            QString ret = reply.value();
            if (ret == QLatin1String("yes") || ret == QLatin1String("challenge")) {
                qDebug() << "call systemd shutdown success";
            }
        } else {
            qDebug() << "call systemd shutdown failed";
        }
    });

    stopUkuisessionTarget();
}
// GCOV_EXCL_STOP

QStringList SessionManagerDBusContext::listFileList()
{
    QDir dir("/etc/ukui/ukui-session/logout/");
    if (!dir.exists()) {
        qWarning("Cannot find the example directory");
        return QStringList();
    }
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    QStringList filelist;

    for (int i = list.size() - 1; i >= 0; --i) {
        QFileInfo fileInfo = list.at(i);
        filelist << fileInfo.fileName();
    }
    return filelist;
}

void SessionManagerDBusContext::execPro(QStringList proList)
{
    for (int i = 0; i < proList.size(); ++i) {
        qDebug() << "script name is " << proList.at(i);
        QString pro = "/etc/ukui/ukui-session/logout/" + proList.at(i);
        QProcess *exe = new QProcess();
        exe->execute(pro, QStringList());
        delete exe;
    }

    qDebug() << "script exec complete .";
}

bool SessionManagerDBusContext::isInnerCaller()
{
    QString clientService = message().service();
    QDBusConnectionInterface *intf = connection().interface();
    unsigned int pid = intf->servicePid(clientService).value();
    QFile file(QString("/proc/%1/cmdline").arg(pid));
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QString strExePath = QString::fromLocal8Bit(file.readAll()).split('\0').first();
        qDebug() << "exe path:" << strExePath << file.fileName();
        if (QStringList({ "/usr/bin/ukui-screensaver-backend",
                          "/usr/bin/ukui-screensaver-dialog",
                          "/usr/bin/ukui-session-tools" })
                .contains(strExePath)) {
            return true;
        }
    }
    return false;
}

void SessionManagerDBusContext::onWindowAdded(QString winId)
{
    if (WindowManager::skipTaskBar(winId)) {
        return;
    }
    QString appId = WindowManager::appId(winId);
    qint32 pid = WindowManager::pid(winId);

    QDBusInterface sessionInterface(
        "com.kylin.ProcessManager",
        "/com/kylin/ProcessManager/AppLauncher",
        "com.kylin.ProcessManager.AppLauncher",
        QDBusConnection::sessionBus());

    QDBusReply<QString> reply = sessionInterface.call("GetDesktopFileByPid", pid);

    if (reply.isValid()) {
        QString desktopFile = reply.value();
        qDebug() << "appId" << appId << "desktop" << desktopFile;
        if (desktopFile.isEmpty()) {
            desktopFile = appId;
        }
        m_windows.insert(winId, QString(winId + "_" + desktopFile));
        emit windowAdd(winId, appId);
    } else {
        qWarning() << "GetDesktopFileByPid faild:" << reply.error().message();
    }
}

void SessionManagerDBusContext::onWindowRemoved(QString winId)
{
    if (m_windows.contains(winId)) {
        m_windows.remove(winId);
        emit windowRemove(winId);
    }
}

bool SessionManagerDBusContext::releaseKyInhibit(quint32 &id)
{
    if (id == 0) {
        qDebug() << "Invalid kyInhibit ID (0), nothing to release";
        return false;
    }

    QDBusInterface interface(
        KYINHIBIT_DBUS_SERVICE, KYINHIBIT_DBUS_PATH, KYINHIBIT_DBUS_INTERFACE, QDBusConnection::systemBus());

    if (!interface.isValid()) {
        qDebug() << "kyInhibit interface is invalid, cannot release";
        return false;
    }

    qDebug() << "Releasing kyInhibit ID:" << id;
    QDBusReply<bool> reply = interface.call("kyUninhibit", id);

    if (reply.isValid()) {
        bool ret = reply.value();
        qDebug() << "kyUninhibit result:" << ret;

        if (ret) {
            // 成功释放后，将ID置为0
            id = 0;
            return true;
        }
    } else {
        qDebug() << "kyUninhibit call failed:" << reply.error().message();
    }

    return false;
}

quint32 SessionManagerDBusContext::requestKyInhibit(
    const QString &what, const QString &who, const QString &why, const QString &mode)
{
    QDBusInterface interface(
        KYINHIBIT_DBUS_SERVICE, KYINHIBIT_DBUS_PATH, KYINHIBIT_DBUS_INTERFACE, QDBusConnection::systemBus());

    if (!interface.isValid()) {
        qDebug() << "kyInhibit interface is invalid";
        return 0;
    }

    qDebug() << "Requesting kyInhibit - what:" << what << "who:" << who << "why:" << why;
    QDBusReply<uint> reply = interface.call("kyInhibit", what, who, why, mode);

    if (reply.isValid()) {
        quint32 id = reply.value();
        qDebug() << "kyInhibitId obtained:" << id;
        return id;
    } else {
        qDebug() << "kyInhibit call failed:" << reply.error().message();
        return 0;
    }
}

void SessionManagerDBusContext::setFreezeEnable(bool enable)
{
    QDBusInterface interface(
    "com.kylin.ProcessManagerDaemon",
    "/com/kylin/ProcessManagerDaemon/CgroupControl",
    "com.kylin.ProcessManagerDaemon.CgroupControl",
    QDBusConnection::systemBus());

    if (!interface.isValid()) {
        qDebug() << "ProcessManagerDaemon is invalid:" << interface.lastError().message();
        return;
    }

    // 使用QDBusInterface的call方法
    QDBusReply<void> reply = interface.call("SetFreezeEnable", enable);

    if (!reply.isValid()) {
        qDebug() << "SetFreezeEnable failed:" << reply.error().message();
        return;
    }
}

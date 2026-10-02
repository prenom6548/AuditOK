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
#include "dbusupperinterface.h"
#include "lightdmhelper.h"
#include "authpamthread.h"
#include "../../src/dbusifs/login1helper.h"
#include "../../src/common/definetypes.h"
#include "../../src/common/configuration.h"
#include "../../src/lock-backend/gsettingshelper.h"
#include "../../src/dbusifs/usdhelper.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextCodec>
#include <QVariant>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include "../../src/QtSingleApplication/qtlocalpeer.h"
#include "../../src/lock-backend/switchuserutils.h"
#include "../../src/lock-backend/pamauthenticate.h"
#include "../../src/dbusifs/upowerhelper.h"
#include "../../src/dbusifs/accountshelper.h"
#include "../../src/dbusifs/sessionhelper.h"
#include "../../src/dbusifs/systemupgradehelper.h"
#include "../../src/lock-backend/sessionwatcher.h"
#include "../../src/dbusifs/freedesktophelper.h"
#include "../../src/lock-backend/bioauthenticate.h"
#include "../../src/common/kyrsac.h"
#include "../../src/lock-backend/personalizeddata.h"
#ifdef SUPPORT_TABLET_MODE
#include "../../src/dbusifs/libinputswitchevent.h"
#endif
#include "../../src/dbusifs/uniauthservice.h"
#include "../../src/dbusifs/greeterconfig.h"
#include "../../src/dbusifs/networkmanagerhelper.h"
#include "../../src/dbusifs/filemanagerhelper.h"
#include "pamauthsessionserver.h"
#include "../../src/dbusifs/usdblockshortcut.h"
#include <syslog.h>

#define CONFIG_FILE "/usr/share/ukui-greeter/ukui-greeter.conf"

DbusUpperInterface::DbusUpperInterface(QObject *parent) : QObject(parent)
{
    m_bLockState = false;
    m_bSlpState = false;
}

DbusUpperInterface::~DbusUpperInterface()
{
    if (m_pamSessionServer) {
        m_pamSessionServer->stopSession();
    }
    // 断开所有信号，用disconnect的方法直接断开没有效果
    //    disconnect(this, nullptr, nullptr, nullptr);
    // 提前手动管理连接并逐个断开conect,避免在析构时因为信号槽的连接导致访问到已销毁的对象
    for (const auto &connection : m_connections) {
        QObject::disconnect(connection);
    }
}

void DbusUpperInterface::init()
{
    initData();
    initConnections();
    inhibit();
}

void DbusUpperInterface::initData()
{
    syslog(LOG_INFO, "[ukui-screensaver-backend] DbusUpperInterface initData begin!!");
    m_config = Configuration::instance(this);
    m_accountsHelper = QSharedPointer<AccountsHelper>(new AccountsHelper(this));
    m_lightDmHelper = QSharedPointer<LightDMHelper>(new LightDMHelper(m_accountsHelper, m_config, this));
    m_login1Helper = QSharedPointer<Login1Helper>(new Login1Helper(this));
    m_gsettingsHelper = QSharedPointer<GSettingsHelper>(new GSettingsHelper(this));
#ifdef ENABLE_BIO_BACKEND
    m_bioAuth = QSharedPointer<BioAuthenticate>(new BioAuthenticate(this));
#endif
    m_pamAuth = QSharedPointer<PamAuthenticate>(new PamAuthenticate(m_lightDmHelper, this));
    m_sessionHelper = QSharedPointer<SessionHelper>(new SessionHelper(m_login1Helper, m_isLockStartupMode, this));
#ifdef SUPPORT_TABLET_MODE
    m_libinputSwitchEvent = QSharedPointer<LibinputSwitchEvent>(new LibinputSwitchEvent(this));
#endif

    QTimer::singleShot(1000, this, &DbusUpperInterface::onDelayInit);
    syslog(LOG_INFO, "[ukui-screensaver-backend] DbusUpperInterface initData end!!");
}

void DbusUpperInterface::initConnections()
{
    if (m_lightDmHelper) {
        m_connections.append(connect(
            m_lightDmHelper.get(), &LightDMHelper::usersInfoChanged, this, &DbusUpperInterface::onUsersInfoChanged));
        m_connections.append(connect(
            m_lightDmHelper.get(),
            &LightDMHelper::currentSessionChanged,
            this,
            &DbusUpperInterface::onCurrentSessionChanged));
    }
    if (m_login1Helper) {
        m_connections.append(
            connect(m_login1Helper.get(), &Login1Helper::requestLock, this, &DbusUpperInterface::onLogin1ReqLock));
        m_connections.append(
            connect(m_login1Helper.get(), &Login1Helper::requestUnlock, this, &DbusUpperInterface::onLogin1ReqUnLock));
        m_connections.append(connect(
            m_login1Helper.get(), &Login1Helper::PrepareForSleep, this, &DbusUpperInterface::onLogin1PrepareForSleep));
        m_connections.append(connect(
            m_login1Helper.get(),
            &Login1Helper::sessionActiveChanged,
            this,
            &DbusUpperInterface::onLogin1SessionActiveChanged));
        m_connections.append(connect(
            m_login1Helper.get(),
            &Login1Helper::blockInhibitedChanged,
            this,
            &DbusUpperInterface::onBlockInhibitedChanged));
    }
    if (m_gsettingsHelper) {
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::lockScreenConfigChanged,
            this,
            &DbusUpperInterface::onLockScreenConfigChanged));
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::screenSaverConfigChanged,
            this,
            &DbusUpperInterface::onScreenSaverConfigChanged));
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::weatherKeysConfigChanged,
            this,
            &DbusUpperInterface::onScreenSaverConfigChanged));
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::powerManagerConfigChanged,
            this,
            &DbusUpperInterface::onPowerManagerConfigChanged));
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::mateBgConfigChanged,
            this,
            &DbusUpperInterface::onMateBgConfigChanged));
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::ukccPluginsConfigChanged,
            this,
            &DbusUpperInterface::onUkccPluginsConfigChanged));
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::themeStyleConfigChanged,
            this,
            &DbusUpperInterface::onThemeStyleConfigChanged));
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::sessionConfigChanged,
            this,
            &DbusUpperInterface::onSessionConfigChanged));
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::keyboardConfigChanged,
            this,
            &DbusUpperInterface::onKeyboardConfigChanged));
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::usdMediaKeysConfigChanged,
            this,
            &DbusUpperInterface::onUsdMediaKeysConfigChanged));
        m_connections.append(connect(
            m_gsettingsHelper.get(),
            &GSettingsHelper::usdMediaStateKeysConfigChanged,
            this,
            &DbusUpperInterface::onUsdMediaStateKeysConfigChanged));
    }
#ifdef ENABLE_BIO_BACKEND
    if (m_bioAuth) {
        m_connections.append(connect(
            m_bioAuth.get(), &BioAuthenticate::bioAuthShowMessage, this, &DbusUpperInterface::onBioAuthShowMessage));
        m_connections.append(connect(
            m_bioAuth.get(),
            &BioAuthenticate::bioServiceStatusChanged,
            this,
            &DbusUpperInterface::onBioServiceStatusChanged));
        m_connections.append(connect(
            m_bioAuth.get(), &BioAuthenticate::bioDeviceChanged, this, &DbusUpperInterface::onBioDeviceChanged));
        m_connections.append(connect(
            m_bioAuth.get(), &BioAuthenticate::bioAuthStateChanged, this, &DbusUpperInterface::onBioAuthStateChanged));
        m_connections.append(connect(
            m_bioAuth.get(), &BioAuthenticate::bioAuthFrameData, this, &DbusUpperInterface::onBioAuthFrameData));
        m_connections.append(connect(
            m_bioAuth.get(), &BioAuthenticate::bioAuthCompleted, this, &DbusUpperInterface::onBioAuthCompleted));
        if (m_pamAuth) {
            m_connections.append(connect(
                m_pamAuth.get(),
                &PamAuthenticate::authenticationComplete,
                m_bioAuth.get(),
                &BioAuthenticate::onPamAuthComplete));
        }
    }
#endif
    if (m_pamAuth) {
        m_connections.append(
            connect(m_pamAuth.get(), &PamAuthenticate::showMessage, this, &DbusUpperInterface::onPamShowMessage));
        m_connections.append(
            connect(m_pamAuth.get(), &PamAuthenticate::showPrompt, this, &DbusUpperInterface::onPamShowPrompt));
        m_connections.append(connect(
            m_pamAuth.get(), &PamAuthenticate::showBinaryPrompt, this, &DbusUpperInterface::onPamShowBinaryPrompt));
        m_connections.append(connect(
            m_pamAuth.get(), &PamAuthenticate::authenticationComplete, this, &DbusUpperInterface::onPamAuthCompleted));
    }
    if (m_sessionHelper) {
        m_connections.append(connect(
            m_sessionHelper.get(),
            &SessionHelper::blockLogoutInhibitedChanged,
            this,
            &DbusUpperInterface::onBlockLogoutInhibitedChanged));
        m_connections.append(
            connect(m_sessionHelper.get(), &SessionHelper::windowAdd, this, &DbusUpperInterface::onWindowAdded));
        m_connections.append(
            connect(m_sessionHelper.get(), &SessionHelper::windowRemove, this, &DbusUpperInterface::onWindowRemoved));
        m_connections.append(connect(
            m_sessionHelper.get(),
            &SessionHelper::sessionLegacyChanged,
            this,
            &DbusUpperInterface::onSessionLegacyChanged));
    }
    m_crashedTimer = new QTimer(this);
    connect(m_crashedTimer, &QTimer::timeout, this, [=] {
        qInfo() << "Saver process exitted and we could in no way recover from that!";
        m_crashedTimer->stop();
        m_remainingTime = 8000;
        m_isCrashedTootimes = true;
    });
    m_crashedTimer->setInterval(8000);
    m_connections.append(connect(
        &m_procLockDialog,
        SIGNAL(finished(int, QProcess::ExitStatus)),
        this,
        SLOT(onLockDialogProcExit(int, QProcess::ExitStatus))));
#ifdef SUPPORT_TABLET_MODE
    if (m_libinputSwitchEvent) {
        m_connections.append(connect(
            m_libinputSwitchEvent.get(),
            &LibinputSwitchEvent::tabletModeStatusChanged,
            this,
            &DbusUpperInterface::onTabletModeChanged));
    }
#endif
    m_connections.append(connect(
        GreeterConfig::instance(),
        &GreeterConfig::screensaverDatetimeChanged,
        this,
        &DbusUpperInterface::onScreensaverDatetimeChanged));
    m_connections.append(connect(
        GreeterConfig::instance(),
        &GreeterConfig::lockScreenDatetimeEnableChanged,
        this,
        &DbusUpperInterface::onScreenlockDatetimeChanged));
}

void DbusUpperInterface::onNameLost(const QString &serviceName)
{
    if (serviceName == "cn.kylinos.ScreenSaver")
        exit(0);
}

void DbusUpperInterface::GetInitData(QJsonObject &reqObj, QJsonObject &retObj)
{
    Q_UNUSED(reqObj);
    retObj["UsersInfo"] = GenerateUserInfoList();
    retObj["SessionsInfo"] = GenerateSessionInfoList();
    retObj["IsSessionActive"] = m_login1Helper->isSessionActive();
    retObj["DefAuthUser"] = GetDefaultAuthUser();
    retObj["CurUserName"] = GetCurrentUser();
    retObj["CurSession"] = GetCurrentSession();
    retObj["AgreementInfo"] = GenerateAgreementInfo();
    retObj["CapsLock"] = m_gsettingsHelper->GetKeyboardConf(KEY_CAPSLOCK_STATUS).toBool();
    retObj["MediaKeyAreaScreenShot"] = m_gsettingsHelper->GetUsdMediaKeys(KEY_AREA_SCREENSHOT).toString();
    retObj["MediaKeyAreaScreenShot2"] = m_gsettingsHelper->GetUsdMediaKeys(KEY_AREA_SCREENSHOT2).toString();
    retObj["MediaKeyScreenShot"] = m_gsettingsHelper->GetUsdMediaKeys(KEY_SCREEN_SHOT).toString();
    retObj["MediaKeyScreenShot2"] = m_gsettingsHelper->GetUsdMediaKeys(KEY_SCREEN_SHOT2).toString();
    retObj["MediaKeyWinScreenShot"] = m_gsettingsHelper->GetUsdMediaKeys(KEY_WINDOW_SCREENSHOT).toString();
    retObj["MediaKeyRfkillState"] = m_gsettingsHelper->GetUsdMediaKeys(KEY_RFKILL_STATE).toInt();
    retObj["PowerButton"] = m_gsettingsHelper->GetPowerManagerConf(KEY_BUTTON_POWER).toString();
    retObj["ScreenSaverMode"] = m_gsettingsHelper->GetLockScreenConf(KEY_MODE).toString();
    retObj["ScreenSaverThemes"] = getSaverTheme();
    retObj["ImageTransitionEffect"] = m_gsettingsHelper->GetLockScreenConf(KEY_IMAGE_TRANSITION_EFFECT).toInt();
    retObj["ImageSwitchInterval"] = m_gsettingsHelper->GetLockScreenConf(KEY_IMAGE_SWITCH_INTERVAL).toInt();
    retObj["BatteryArgs"] = GenerateBatteryArgsList();
    retObj["BatteryIconName"] = getBatteryIconName();
    retObj["IsBattery"] = getIsBattery();
    retObj["IsLegacy"] = getIsLegacy();
    retObj["SleepLockCheck"] = getSleepLockCheck();
    retObj["ShutdownLockCheck"] = getShutdownLockcheck();
    retObj["LogoutLockCheck"] = getLogoutLockCheck();
    retObj["GetAppWindows"] = getAppWindows();
    retObj["LockTimeout"] = m_gsettingsHelper->GetLockScreenConf(KEY_LOCK_TIMEOUT).toInt();
    retObj["IdleDelay"] = m_gsettingsHelper->GetLockScreenConf(KEY_IDLE_DELAY).toInt();
    retObj["LockEnable"] = m_gsettingsHelper->GetLockScreenConf(KEY_LOCK_ENABLED).toBool();
    retObj["SleepActivationEnable"] = m_gsettingsHelper->GetLockScreenConf(KEY_SLEEP_ACTIVATION_ENABLED).toBool();
    retObj["SystemFontSize"] = m_gsettingsHelper->GetThemeStyleConf(KEY_SYSTEM_FONT_SIZE).toInt();
    retObj["SystemFont"] = m_gsettingsHelper->GetThemeStyleConf(KEY_SYSTEM_FONT).toString();
    retObj["TabletMode"] = getCurTabletMode();
    retObj["LogoutMusic"] = m_gsettingsHelper->GetSessionConf(KEY_SESSION_LOGOUT_MUSIC).toBool();
    retObj["PoweroffMusic"] = m_gsettingsHelper->GetSessionConf(KEY_SESSION_POWEROFF_MUSIC).toBool();
    retObj["LockDatetimeEnable"] = GetLockDatetimeEnable();
    retObj["CanSwitchUser"] = GetCanSwitchUser();
}

bool DbusUpperInterface::GetSlpState()
{
    return ((m_procLockDialog.state() != QProcess::NotRunning) && m_bSlpState);
}

void DbusUpperInterface::LockStartupMode()
{
    if (GetLockState())
        return;

    m_isLockStartupMode = true;
    m_isCrashedTootimes = false;
    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "StartupLock";
        QDBusConnection::sessionBus().send(message);
        emitLockState(true);
        return;
    }

    qDebug() << "LockStartupMode requested";
    m_bLockState = false;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("XMODIFIERS", "@im=fcitx");
    env.insert("QT_IM_MODULE", "fcitx");
    m_procLockDialog.setEnvironment(env.toStringList());
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --lock-startup");
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(true);
}

bool DbusUpperInterface::GetBlankState()
{
    return m_bBlankState;
}

bool DbusUpperInterface::GetLockState()
{
    if (m_procLockDialog.state() == QProcess::NotRunning && !checkScreenDialogRunning()) {
        return false;
    } else {
        return m_bLockState;
    }
    return false;
}

void DbusUpperInterface::Lock()
{
    if (GetLockState())
        return;
    m_isCrashedTootimes = false;
    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "CmdLock";
        if (Configuration::checkIslivecdStatus() != 1) {
            QDBusConnection::sessionBus().send(message);
            message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "prepareForLock");
            QDBusConnection::sessionBus().send(message);
        }
        emitLockState(true);
        return;
    }
    qDebug() << "Lock requested";
    m_bLockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --lock");
    qDebug() << cmd;

    m_procLockDialog.start(cmd);

    if (Configuration::checkIslivecdStatus() != 1) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "prepareForLock");
        QDBusConnection::sessionBus().send(message);
    }

    emitLockState(true);
}

void DbusUpperInterface::SwitchUser()
{
    if (GetLockState())
        return;

    m_isCrashedTootimes = false;
    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "CmdSwitchUser";
        QDBusConnection::sessionBus().send(message);
        emitLockState(true);
        return;
    }
    qDebug() << "SwitchUser requested";
    m_bLockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --switchuser");
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(true);
}

void DbusUpperInterface::SwitchToUser(QString strUserName)
{
    if (GetLockState())
        return;

    m_isCrashedTootimes = false;
    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << QString("CmdSwitchToUser&%1").arg(strUserName);
        QDBusConnection::sessionBus().send(message);
        emitLockState(true);
        return;
    }
    qDebug() << "SwitchToUser requested:" << strUserName;
    m_bLockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --switchtouser %1").arg(strUserName);
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(true);
}

bool DbusUpperInterface::CheckAppVersion()
{
    return true;
}

void DbusUpperInterface::SessionTools()
{
    if (GetLockState())
        return;

    m_isCrashedTootimes = false;
    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "CmdSessionTools";
        QDBusConnection::sessionBus().send(message);
        emitLockState(false, true);
        return;
    }
    m_bLockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --session-tools");
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(false, true);
}

void DbusUpperInterface::AppBlockWindow(QString actionType, bool isHardInhibit)
{
    bool isLegacy = getIsLegacy();
    qDebug() << "backend appblock" << actionType;
    QVector<InhibitInfo::InhibitorInfo> lockcheck;
    if (actionType == "PowerOff" || actionType == "Reboot" || actionType == "Logout") {
        if (!isLegacy) {
            lockcheck = m_sessionHelper->getLockCheckStatus("shutdown") + m_sessionHelper->getLockCheckStatus("logout");
            if (lockcheck.isEmpty() && m_windowList.isEmpty()) {
                return;
            }
        } else {
            if (isHardInhibit) {
                lockcheck = m_sessionHelper->getLockCheckStatus("shutdown");
            } else {
                lockcheck = m_sessionHelper->getLockCheckStatus("logout");
            }
            if (lockcheck.isEmpty()) {
                return;
            }
        }
    } else if (actionType == "Suspend" || actionType == "Hibernate") {
        lockcheck = m_sessionHelper->getLockCheckStatus("sleep");
        if (lockcheck.isEmpty()) {
            return;
        }
    }

    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        if (!isLegacy) {
            if (actionType == "Reboot")
                message << "CmdRestartAppBlock";
            else if (actionType == "PowerOff")
                message << "CmdPowerOffAppBlock";
            else if (actionType == "Suspend")
                message << "CmdSuspendAppBlock";
            else if (actionType == "Hibernate")
                message << "CmdHibernateAppBlock";
            else if (actionType == "Logout")
                message << "CmdLogoutAppBlock";
            else
                return;
        } else {
            if (isHardInhibit) {
                if (actionType == "Reboot")
                    message << "CmdRestartAppBlock";
                else if (actionType == "PowerOff")
                    message << "CmdPowerOffAppBlock";
                else if (actionType == "Suspend")
                    message << "CmdSuspendAppBlock";
                else if (actionType == "Hibernate")
                    message << "CmdHibernateAppBlock";
                else if (actionType == "Logout")
                    message << "CmdLogoutAppBlock";
                else
                    return;
            } else {
                if (actionType == "Reboot")
                    message << "CmdRestartAppSoftBlock";
                else if (actionType == "PowerOff")
                    message << "CmdPowerOffAppSoftBlock";
                else if (actionType == "Logout")
                    message << "CmdLogoutAppSoftBlock";
                else
                    return;
            }
        }
        QDBusConnection::sessionBus().send(message);
        emitLockState(false, true);
        return;
    }
    if (actionType != "Reboot" && actionType != "PowerOff" && actionType != "Suspend" && actionType != "Hibernate"
        && actionType != "Logout")
        return;
    m_bLockState = false;
    QString cmd;
    cmd = QString("/usr/bin/ukui-screensaver-dialog --app-block %1").arg(actionType);
    qDebug() << cmd;
    m_procLockDialog.start(cmd);
    emitLockState(false, true);
}

void DbusUpperInterface::shutdownAnimation(QString actionType)
{
    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        if (actionType == "Reboot")
            message << "CmdRestartAnimation";
        else if (actionType == "PowerOff")
            message << "CmdPowerOffAnimation";
        else if (actionType == "Logout")
            message << "CmdLogoutAnimation";
        else
            return;
        QDBusConnection::sessionBus().send(message);
        emitLockState(false, true);
        return;
    }
    if (actionType != "Reboot" && actionType != "PowerOff" && actionType != "Logout")
        return;
    m_bLockState = false;
    QString cmd;
    cmd = QString("/usr/bin/ukui-screensaver-dialog --shutdown-animation %1").arg(actionType);
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(false, true);
}

void DbusUpperInterface::MultiUserBlockWindow(QString actionType)
{
    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        if (actionType == "Reboot")
            message << "CmdRestartMulTiUserBlock";
        else if (actionType == "PowerOff")
            message << "CmdPowerOffMulTiUserBlock";
        else
            return;
        QDBusConnection::sessionBus().send(message);
        emitLockState(false, true);
        return;
    }
    if (actionType != "Reboot" && actionType != "PowerOff")
        return;
    m_bLockState = false;
    QString cmd;
    cmd = QString("/usr/bin/ukui-screensaver-dialog --multiusers-block %1").arg(actionType);
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(false, true);
}

void DbusUpperInterface::Suspend()
{
    if (m_sessionHelper) {
        QVector<InhibitInfo::InhibitorInfo> lockCheckList = m_sessionHelper->getLockCheckStatus("sleep");
        if (lockCheckList.count() == 0) {
            doPrepareForSleep(true, "Suspend");
        } else {
            AppBlockWindow("Suspend");
        }
    }
}

void DbusUpperInterface::Hibernate()
{
    if (m_sessionHelper) {
        QVector<InhibitInfo::InhibitorInfo> lockCheckList = m_sessionHelper->getLockCheckStatus("sleep");
        if (lockCheckList.count() == 0) {
            doPrepareForSleep(true, "Hibernate");
        } else {
            AppBlockWindow("Hibernate");
        }
    }
}

void DbusUpperInterface::Reboot()
{
    if (m_sessionHelper) {
        if (!getIsLegacy()) {
            if (m_lightDmHelper && m_lightDmHelper->getLoginUserCount() > 1) {
                MultiUserBlockWindow("Reboot");
            } else {
                m_sessionHelper->doAction("Reboot");
                shutdownAnimation("Reboot");
            }
        } else {
            QVector<InhibitInfo::InhibitorInfo> lockCheckList = m_sessionHelper->getLockCheckStatus("shutdown");
            if (lockCheckList.count() == 0) {
                QVector<InhibitInfo::InhibitorInfo> logoutLockCheckList = m_sessionHelper->getLockCheckStatus("logout");
                if (logoutLockCheckList.count() == 0) {
                    if (m_lightDmHelper && m_lightDmHelper->getLoginUserCount() > 1) {
                        MultiUserBlockWindow("Reboot");
                    } else {
                        m_sessionHelper->doAction("Reboot");
                    }
                } else {
                    AppBlockWindow("Logout", false);
                }
            } else {
                AppBlockWindow("Reboot");
            }
        }
    }
}

void DbusUpperInterface::PowerOff()
{
    if (m_sessionHelper) {
        if (!getIsLegacy()) {
            if (m_lightDmHelper && m_lightDmHelper->getLoginUserCount() > 1) {
                MultiUserBlockWindow("PowerOff");
            } else {
                m_sessionHelper->doAction("PowerOff");
                shutdownAnimation("PowerOff");
            }
        } else {
            QVector<InhibitInfo::InhibitorInfo> lockCheckList = m_sessionHelper->getLockCheckStatus("shutdown");
            if (lockCheckList.count() == 0) {
                QVector<InhibitInfo::InhibitorInfo> logoutLockCheckList = m_sessionHelper->getLockCheckStatus("logout");
                if (logoutLockCheckList.count() == 0) {
                    if (m_lightDmHelper && m_lightDmHelper->getLoginUserCount() > 1) {
                        MultiUserBlockWindow("PowerOff");
                    } else {
                        m_sessionHelper->doAction("PowerOff");
                    }
                } else {
                    AppBlockWindow("Logout", false);
                }
            } else {
                AppBlockWindow("PowerOff");
            }
        }
    }
}

void DbusUpperInterface::Logout()
{
    if (m_sessionHelper) {
        if (!getIsLegacy()) {
            m_sessionHelper->doAction("Logout");
            shutdownAnimation("Logout");
        } else {
            QVector<InhibitInfo::InhibitorInfo> lockCheckList = m_sessionHelper->getLockCheckStatus("shutdown");
            if (lockCheckList.count() == 0) {
                QVector<InhibitInfo::InhibitorInfo> logoutLockCheckList = m_sessionHelper->getLockCheckStatus("logout");
                if (logoutLockCheckList.count() == 0) {
                    m_sessionHelper->doAction("Logout");
                } else {
                    AppBlockWindow("Logout", false);
                }
            } else {
                AppBlockWindow("Logout");
            }
        }
    }
}

void DbusUpperInterface::UnLock()
{
    qDebug() << "manual unlcok!!";
    if (m_login1Helper) {
        m_login1Helper->Unlock();
    } else {
        if (m_procLockDialog.state() != QProcess::NotRunning)
            m_procLockDialog.kill();
    }
}

bool DbusUpperInterface::LockByBlank(int nType)
{
    qDebug() << "LockByBlank:" << nType;
    bool isNeedLock = true;

    if (!checkStatus(SESSION_STATUS_SCREENCLOSE)) {
        qDebug() << "m_nStatus:" << m_nStatus << " can't do screenclose lock!";
        if (nType == 0) {
            int closeDisplayTime = m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_DISPLAY_AC).toInt();
            int nIdleDelay = m_gsettingsHelper->GetScreenSaverConf(KEY_IDLE_DELAY).toInt();
            bool isCloseActive = m_gsettingsHelper->GetScreenSaverConf(KEY_CLOSE_ACTIVATION_ENABLED).toBool();
            qDebug() << "DelayState:" << closeDisplayTime << nIdleDelay << isCloseActive;
            if (-1 == closeDisplayTime || !isCloseActive)
                return false;
            if (-1 != nIdleDelay && closeDisplayTime == nIdleDelay * 60) {               // 屏保与关屏时间一致
                if (!m_gsettingsHelper->GetScreenSaverConf(KEY_LOCK_ENABLED).toBool()) { // 仅启动屏保
                    isNeedLock = false;
                }
            } else {
                return false;
            }
        } else {
            qDebug() << "Don't need blank lock:" << nType;
            return false;
        }
    }
    if (isNeedLock) {
        qDebug() << "LockByBlank ---";
        m_bLockState = false;
        m_bSlpState = false;
        if (m_gsettingsHelper->GetLockScreenConf(KEY_LOCK_TIMEOUT).toInt() != -1 && isNeedLock) {
            // 空闲关屏且关屏时间与睡眠时间一致时直接锁定
            if (nType == 0
                && ((m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_DISPLAY_AC).toInt()
                     == m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_COMPUTER_AC).toInt())
                    && m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_COMPUTER_AC).toInt() != -1)) {
                qDebug() << "close delay same with sleep delay then lock!";
                if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
                    QDBusMessage message;
                    message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
                    QString strParam = QString("CloseLock&delay=%1&hasLock=%2").arg(1000).arg(isNeedLock ? 1 : 0);
                    message << strParam;
                    QDBusConnection::sessionBus().send(message);
                } else {
                    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --blank --delay %1 --has-lock %2")
                                      .arg(1000)
                                      .arg(isNeedLock ? 1 : 0);
                    qDebug() << cmd;
                    m_procLockDialog.start(cmd);
                }
                emitLockState(true);
            } else {
                delayLockScreen();
            }
        } else {
            if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
                QDBusMessage message;
                message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
                QString strParam = QString("CloseLock&delay=%1&hasLock=%2").arg(2000).arg(isNeedLock ? 1 : 0);
                message << strParam;
                QDBusConnection::sessionBus().send(message);
            } else {
                QString cmd = QString("/usr/bin/ukui-screensaver-dialog --blank --delay %1 --has-lock %2")
                                  .arg(2000)
                                  .arg(isNeedLock ? 1 : 0);
                qDebug() << cmd;
                m_procLockDialog.start(cmd);
            }
            emitLockState(true);
        }
        return true;
    } else {
        return false;
    }
}

void DbusUpperInterface::SetLockState()
{
    m_bLockState = true;
}

void DbusUpperInterface::ClearLockState()
{
    m_bLockState = false;
}

void DbusUpperInterface::onShowBlankScreensaver()
{
    if (GetLockState())
        return;

    m_isCrashedTootimes = false;
    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "SleepLock";
        QDBusConnection::sessionBus().send(message);
        return;
    }

    qDebug() << "lock and show screensaver";
    m_bLockState = false;
    m_bSlpState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --blank");
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(true);
}

void DbusUpperInterface::ShowScreensaver()
{
    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        if (!m_bLockState) {
            emitLockState(true);
        }
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "Screensaver";
        QDBusConnection::sessionBus().send(message);
        return;
    }

    qDebug() << "only show screensaver";
    m_bLockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --screensaver");
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(true);
}

void DbusUpperInterface::LockScreensaver()
{
    m_isCrashedTootimes = false;
    if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
        if (!m_bLockState) {
            emitLockState(true);
        }
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "ScreensaverLock";
        QDBusConnection::sessionBus().send(message);
        return;
    }

    m_bSlpState = true;
    qDebug() << "lock and show screensaver";
    m_bLockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --lock-screensaver");
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(true);
}

bool DbusUpperInterface::checkScreenDialogRunning()
{
    bool isRunning = false;
    QString strDisplay = "";
    if (QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        strDisplay = QLatin1String(getenv("WAYLAND_DISPLAY"));
    } else {
        strDisplay = QLatin1String(getenv("DISPLAY"));
    }
    QString id = QString("ukui-screensaver-dialog" + strDisplay);
    QtLocalPeer *lPeer = new QtLocalPeer(this, id);
    isRunning = lPeer->isClient();
    delete lPeer;
    return isRunning;
}

void DbusUpperInterface::emitLockState(bool val, bool isSessionTools)
{
    if (!isSessionTools) {
        static bool s_isLocked = false;
        QDBusMessage message;
        if (val && !s_isLocked) {
            int state = SaverStatesForAbnormal::SaverAUTHCOMPLETE;
            if (m_sessionHelper) {
                state = m_sessionHelper->getAuthenticateCompeteState();
            }
            if (state != SaverStatesForAbnormal::SaverISSTARTUP) {
                message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "lock");
                QDBusConnection::sessionBus().send(message);
                if (Configuration::checkIslivecdStatus() != 1) {
                    if (m_isSessionIdle && !m_gsettingsHelper->GetLockScreenConf(KEY_LOCK_ENABLED).toBool()) {
                        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SessionIdle");
                        QDBusConnection::sessionBus().send(message);
                    } else {
                        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "prepareForLock");
                        QDBusConnection::sessionBus().send(message);
                    }
                }
            }
            s_isLocked = true;
            connectUpowerBatterySignals();
        } else if (!val && s_isLocked) {
            disconnectUpowerBatterySignals();
            message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "unlock");
            QDBusConnection::sessionBus().send(message);
            s_isLocked = false;
            if (m_sessionHelper) {
                m_sessionHelper->setAuthenticateCompeteState(SaverStatesForAbnormal::SaverAUTHCOMPLETE);
            }
        }
        if (s_isLocked != m_bBlockShortcutState) {
	    blockAllShortcuts(s_isLocked);
            m_bBlockShortcutState = val;
            qDebug() << " block all shortcut "
                     << "blockShortcut states = " << m_bBlockShortcutState;
        }
    } else {
        if (val != m_bBlockShortcutState) {
	    blockAllShortcuts(val);
            m_bBlockShortcutState = val;
            qDebug() << " block all shortcut "
                     << "blockShortcut states = " << m_bBlockShortcutState;
        }
        if (val) {
            connectUpowerBatterySignals();
        } else {
            disconnectUpowerBatterySignals();
        }
        if (val) {
            connectUpowerBatterySignals();
        } else {
            disconnectUpowerBatterySignals();
        }
    }
}

QString DbusUpperInterface::GetInformation(QString strJson)
{
    QJsonParseError jsonParseError;
    QJsonObject retObj;
    retObj["Ret"] = 0;
    retObj["Message"] = "Success";
    qDebug() << "Json:" << strJson;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError) {
        retObj["Ret"] = -1;
        retObj["Message"] = "Parse Request json failed!!";
        qWarning() << "Parse request json failed!!";
    } else {
        QJsonObject rootObj = jsonDoc.object();
        if (rootObj.isEmpty()) {
            retObj["Ret"] = -1;
            retObj["Message"] = "Request Json is null!!";
            qWarning() << "Request Json is null!!";
        } else {
            int cmdId = rootObj.value("CmdId").toInt();
            qInfo() << "CmdId: " << cmdId;
            switch (cmdId) {
                case LOCK_CMD_ID_GET_USERINFO_LIST:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = GenerateUserInfoList();
                    break;
                case LOCK_CMD_ID_GET_DEFAULT_AUTH_USER:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = GetDefaultAuthUser();
                    break;
                case LOCK_CMD_ID_GET_CURRENT_USER:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = GetCurrentUser();
                    break;
                case LOCK_CMD_ID_GET_SESSIONS_LIST:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = GenerateSessionInfoList();
                    break;
                case LOCK_CMD_ID_GET_CURRENT_SESSION:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = GetCurrentSession();
                    break;
                case LOCK_CMD_ID_LOGIN1_IS_SESSION_ACTIVE:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = m_login1Helper->isSessionActive();
                    break;
                case LOCK_CMD_ID_GET_AGREEMENT:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = GenerateAgreementInfo();
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_LOCKSCREEN_CONF:
                    retObj["CmdId"] = cmdId;
                    GetLockScreenConf(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_SCREENSAVER_CONF:
                    retObj["CmdId"] = cmdId;
                    GetScreenSaverConf(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_DEFAULT_PHOTOPATH:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getSaverPhotoPath(KEY_PHOTO_DEFAULT_PATH);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_CUSTOM_PHOTOPATH:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getSaverPhotoPath(KEY_PHOTO_CUSTOM_PATH);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_POWERMANAGER_CONF:
                    retObj["CmdId"] = cmdId;
                    GetPowerManagerConf(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_MATEBG_CONF:
                    retObj["CmdId"] = cmdId;
                    GetMateBgConf(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_UKCCPLUGINS_CONF:
                    retObj["CmdId"] = cmdId;
                    GetUkccPluginsConf(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_THEMESTYLE_CONF:
                    retObj["CmdId"] = cmdId;
                    GetThemeStyleConf(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_SESSION_CONF:
                    retObj["CmdId"] = cmdId;
                    GetSessionConf(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_KEYBOARD_CONF:
                    retObj["CmdId"] = cmdId;
                    GetKeyboardConf(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_PAMAUTH_IS_AUTHENTICATED:
                    retObj["CmdId"] = cmdId;
                    IsPamAuthenticated(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_PAMAUTH_IS_INAUTHTICATION:
                    retObj["CmdId"] = cmdId;
                    IsPamInAuthentication(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_PAMAUTH_GET_AUTHUSER:
                    retObj["CmdId"] = cmdId;
                    PamAuthenticationUser(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_PAMAUTH_GET_AUTOLOGINUSERHINT:
                    retObj["CmdId"] = cmdId;
                    GetAutologinUserHint(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIAKEYS_CONF:
                    retObj["CmdId"] = cmdId;
                    GetUsdMediaKeys(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIA_STATE_KEYS_CONF:
                    retObj["CmdId"] = cmdId;
                    GetUsdMediaStateKeys(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANHIBERNATE:
                    retObj["CmdId"] = cmdId;
                    if (m_login1Helper)
                        retObj["Content"] = m_login1Helper->isCanHibernate();
                    break;
                case LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANPOWEROFF:
                    retObj["CmdId"] = cmdId;
                    if (m_login1Helper)
                        retObj["Content"] = m_login1Helper->isCanPowerOff();
                    break;
                case LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANREBOOT:
                    retObj["CmdId"] = cmdId;
                    if (m_login1Helper)
                        retObj["Content"] = m_login1Helper->isCanReboot();
                    break;
                case LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANSUSPEND:
                    retObj["CmdId"] = cmdId;
                    if (m_login1Helper)
                        retObj["Content"] = m_login1Helper->isCanSuspend();
                    break;
                case LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANLOGOUT:
                    retObj["CmdId"] = cmdId;
                    if (m_sessionHelper)
                        retObj["Content"] = m_sessionHelper->canAction(PowerLogout);
                    break;
                case LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANLOCKSCREEN:
                    retObj["CmdId"] = cmdId;
                    if (m_sessionHelper)
                        retObj["Content"] = m_sessionHelper->canAction(PowerMonitorOff);
                    break;
                case LOCK_CMD_ID_SYSTEM_UPGRADE_CHECK:
                    retObj["CmdId"] = cmdId;
                    if (m_systemsUpgradeHelper) {
                        const auto upgradeInfo = m_systemsUpgradeHelper->checkSystemUpgrade();
                        retObj["Content"] = upgradeInfo.first;
                        retObj["UpgradeTime"] = upgradeInfo.second;
                    } else {
                        retObj["Content"] = false;
                        retObj["UpgradeTime"] = QStringLiteral("NoUpdate");
                    }
                    break;
                case LOCK_CMD_ID_SYSTEM_UPGRADE_CHECK_HIDE:
                    if (m_systemsUpgradeHelper) {
                        retObj["CmdId"] = cmdId;
                        retObj["Content"] = m_systemsUpgradeHelper->checkHideUpdate();
                    }
                    break;
                case LOCK_CMD_ID_UPOWER_BATTERY_STATUS:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getBatteryIconName();
                    break;
                case LOCK_CMD_ID_UPOWER_IS_BATTERY:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getIsBattery();
                    break;
                case LOCK_CMD_ID_UPOWER_BATTERY:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = GenerateBatteryArgsList();
                    break;
                case LOCK_CMD_ID_SESSION_GET_LEGACY_COMPONENTS:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getIsLegacy();
                    break;
                case LOCK_CMD_ID_SESSION_GET_LOGOUT_LOCKCHECK:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getLogoutLockCheck();
                    break;
                case LOCK_CMD_ID_SESSION_GET_SLEEP_LOCKCHECK:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getSleepLockCheck();
                    break;
                case LOCK_CMD_ID_SESSION_GET_SHUTDOWN_LOCKCHECK:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getShutdownLockcheck();
                    break;
                case LOCK_CMD_ID_SESSION_GET_OPEN_WINDOWS_LOCKCHECK:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getAppWindows();
                    break;
                case LOCK_CMD_ID_LOCK_SCREEN_GET_THEMES:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getSaverTheme();
                    break;
                case LOCK_CMD_ID_BIOAUTH_GET_AVAILABLE_DEVICES:
                    retObj["CmdId"] = cmdId;
                    BioGetAvailableDevices(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_BIOAUTH_GET_DISABLED_DEVICES:
                    retObj["CmdId"] = cmdId;
                    BioGetDisabledDevices(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_BIOAUTH_GET_STATE:
                    retObj["CmdId"] = cmdId;
                    BioGetBioAuthState(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_BIOAUTH_GET_CURDEVICE:
                    retObj["CmdId"] = cmdId;
                    BioGetCurBioInfo(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_BIOAUTH_FIND_DEVICE_BY_ID:
                    retObj["CmdId"] = cmdId;
                    BioFindDeviceById(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_BIOAUTH_FIND_DEVICE_BY_NAME:
                    retObj["CmdId"] = cmdId;
                    BioFindDeviceByName(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_BIOAUTH_GET_DEFAULT_DEVICE:
                    retObj["CmdId"] = cmdId;
                    BioGetDefaultDevice(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GET_PUBLIC_KEY:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getPublicEncrypt();
                    break;
                case LOCK_CMD_ID_TABLET_MODE:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getCurTabletMode();
                    break;
                case LOCK_CMD_ID_NETWORK_CONNECT_STATE:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getNetworkConnectState();
                    break;
                case LOCK_CMD_ID_GET_WEATHER_DATA:
                    retObj["CmdId"] = cmdId;
                    GetWeatherKeys(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GET_INIT_DATA:
                    retObj["CmdId"] = cmdId;
                    GetInitData(rootObj, retObj);
                    break;
                case LOCK_CMD_ID_GET_SAVER_DATETIME_ENABLE_DATA:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = GetSaverDatetimeEnable();
                    break;
                case LOCK_CMD_ID_GET_LOCK_DATETIME_ENABLE_DATA:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = GetLockDatetimeEnable();
                    break;
                case LOCK_CMD_ID_CAN_SWITCH_USER:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = GetCanSwitchUser();
                    break;
                default:
                    qInfo() << "不支持的CmdId";
                    retObj["Ret"] = -2;
                    retObj["Message"] = "Unsupported CmdId!!";
                    break;
            }
        }
    }
    return QString(QJsonDocument(retObj).toJson());
}

int DbusUpperInterface::SetInformation(QString strJson)
{
    QJsonParseError jsonParseError;
    int nRet = 0;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse info json failed!!";
        nRet = -1;
    } else {
        QJsonObject rootObj = jsonDoc.object();
        if (rootObj.isEmpty()) {
            qWarning() << "Info Json is null!!";
            nRet = -1;
        } else {
            int cmdId = rootObj.value("CmdId").toInt();
            qInfo() << "CmdId: " << cmdId;
            switch (cmdId) {
                case LOCK_CMD_ID_SET_USER:
                    nRet = SetCurrentUser(rootObj);
                    break;
                case LOCK_CMD_ID_SWITCH_TO_USER:
                    nRet = switchToUser(rootObj);
                    break;
                case LOCK_CMD_ID_SET_SESSION:
                    nRet = SetCurrentSession(rootObj);
                    break;
                case LOCK_CMD_ID_START_SESSION:
                    nRet = StartSession(rootObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_SET_LOCKSCREEN_CONF:
                    nRet = SetLockScreenConf(rootObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_SET_SCREENSAVER_CONF:
                    nRet = SetScreenSaverConf(rootObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_SET_POWERMANAGER_CONF:
                    nRet = SetPowerManagerConf(rootObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_SET_MATEBG_CONF:
                    nRet = SetMateBgConf(rootObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_SET_UKCCPLUGINS_CONF:
                    nRet = SetUkccPluginsConf(rootObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_SET_THEMESTYLE_CONF:
                    nRet = SetThemeStyleConf(rootObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_SET_SESSION_CONF:
                    nRet = SetSessionConf(rootObj);
                    break;
                case LOCK_CMD_ID_PAMAUTH_AUTHENTICATE:
                    nRet = PamAuthenticateUser(rootObj);
                    break;
                case LOCK_CMD_ID_PAMAUTH_RESPOND:
                    nRet = PamRespond(rootObj);
                    break;
                case LOCK_CMD_ID_PAMAUTH_AUTHENTICATE_CANCEL:
                    nRet = PamAuthenticateCancel(rootObj);
                    break;
                case LOCK_CMD_ID_USD_MEDIAKEYS:
                    nRet = usdExternalDoAction(rootObj);
                    break;
                case LOCK_CMD_ID_LOGIN1_SET_POWER_MANAGER:
                    nRet = SetPowerManager(rootObj);
                    break;
                case LOCK_CMD_ID_SESSION_SET_IN_CALL_MANAGER:
                    nRet = SetClosePhase(rootObj);
                    break;
                case LOCK_CMD_ID_LOCK_STATE_CHANGED:
                    nRet = lockStateChanged(rootObj);
                    break;
                case LOCK_CMD_ID_BIOAUTH_STARTAUTH:
                    nRet = BioStartAuth(rootObj);
                    break;
                case LOCK_CMD_ID_BIOAUTH_STOPAUTH:
                    nRet = BioStopAuth(rootObj);
                    break;
                case LOCK_CMD_ID_KWIN_BLOCK_SHORTCUT:
                    nRet = blockShortcut(rootObj);
                    break;
                case LOCK_CMD_ID_GSETTINGS_SET_USD_MEDIA_STATE_KEYS_CONF:
                    nRet = SetUsdMediaStateKeys(rootObj);
                    break;
                case LOCK_CMD_ID_SET_SAVER_DATETIME_ENABLE_DATA:
                    SetSaverDatetimeEnable(rootObj.value("Content").toBool());
                    break;
                case LOCK_CMD_ID_SET_LOCK_DATETIME_ENABLE_DATA:
                    SetLockDatetimeEnable(rootObj.value("Content").toBool());
                    break;
                default:
                    qInfo() << "不支持的CmdId";
                    nRet = -2;
                    break;
            }
        }
    }
    return nRet;
}

QJsonObject DbusUpperInterface::GenerateAgreementInfo()
{
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);
    settings.setIniCodec(QTextCodec::codecForName("utf-8"));

    QJsonObject objAgreementInfo;

    if (settings.contains("Greeter/showLoginPrompt")) {
        objAgreementInfo["showLoginPrompt"] = settings.value("Greeter/showLoginPrompt").toBool();
    }

    if (settings.contains("Greeter/hideTitle")) {
        objAgreementInfo["hideTitle"] = settings.value("Greeter/hideTitle").toBool();
    }

    if (settings.contains("Greeter/promptTitle")) {
        QString promptTitle = settings.value("Greeter/promptTitle").toString();
        objAgreementInfo["promptTitle"] = promptTitle;
    }

    if (settings.contains("Greeter/promptText")) {
        QString promptText = settings.value("Greeter/promptText").toString();
        objAgreementInfo["promptText"] = promptText;
    }

    if (settings.contains("Greeter/promptTextFilePath")) {
        QString promptTextFilePath = settings.value("Greeter/promptTextFilePath").toString();
        objAgreementInfo["promptTextFilePath"] = promptTextFilePath;
    }

    return objAgreementInfo;
}

QJsonArray DbusUpperInterface::GenerateUserInfoList()
{
    QJsonArray jsonArray;
    if (m_lightDmHelper) {
        QList<UserInfoPtr> usersInfo = m_lightDmHelper->getUsersInfo();
        for (auto userInfo : usersInfo) {
            QJsonObject objUserInfo;
            objUserInfo["uid"] = (double)userInfo->uid();
            objUserInfo["name"] = userInfo->name();
            objUserInfo["fullName"] = userInfo->fullName();
            objUserInfo["loggedIn"] = userInfo->isLoggedIn();
            objUserInfo["backGround"] = userInfo->backGround();
            objUserInfo["lang"] = userInfo->lang();
            objUserInfo["headImg"] = userInfo->headImage();
            objUserInfo["systemAccount"] = userInfo->isSystemAccount();
            KylinUserDatePtr ptr = KYLINUSERDATAMNG::instance()->GetUserPtr(userInfo->name());
            if (nullptr != ptr) {
                if (!ptr->backgroundPath().isEmpty()) {
                    objUserInfo["greeterBackGround"] = ptr->backgroundPath();
                }
                objUserInfo["dateType"] = ptr->dateType();
                objUserInfo["fontSize"] = ptr->fontSize();
                objUserInfo["timeType"] = ptr->timeType();
                objUserInfo["color"] = ptr->color();
            }
            if (isCurUserSelf(userInfo->name()) && m_gsettingsHelper) {
                QString strBackground = m_gsettingsHelper->GetLockScreenConf(KEY_BACKGROUND).toString();
                if (!strBackground.isEmpty() && QFile(strBackground).exists()) {
                    objUserInfo["greeterBackGround"] = strBackground;
                }
            }
            jsonArray.append(objUserInfo);
        }
    }
    return jsonArray;
}

int DbusUpperInterface::GetLoggedInUsersCount()
{
    int loggedinUsersCount = 0;
    if (m_lightDmHelper) {
        QList<UserInfoPtr> usersInfo = m_lightDmHelper->getUsersInfo();
        if (!usersInfo.isEmpty()) {
            for (auto user : usersInfo) {
                if (user->isLoggedIn()) {
                    loggedinUsersCount++;
                }
            }
        }
    }
    return loggedinUsersCount;
}

QJsonArray DbusUpperInterface::GenerateBatteryArgsList()
{
    QJsonArray jsonArray;
    if (m_upowerHelper) {
        QStringList batteryArgs = m_upowerHelper->getBatteryArgs();
        for (auto battery : batteryArgs) {
            QJsonObject objBatteryInfo;
            objBatteryInfo["key"] = battery;
            jsonArray.append(objBatteryInfo);
        }
    }
    return jsonArray;
}

QJsonArray DbusUpperInterface::getSleepLockCheck()
{
    QJsonArray jsonArray;
    if (m_sessionHelper) {
        QVector<InhibitInfo::InhibitorInfo> lockCheckList = m_sessionHelper->getLockCheckStatus("sleep");
        for (auto it = lockCheckList.constBegin(); it != lockCheckList.constEnd(); ++it) {
            QJsonObject objBatteryInfo;
            objBatteryInfo["name"] = it->name;
            objBatteryInfo["icon"] = it->icon;
            objBatteryInfo["uid"] = it->uid;
            jsonArray.append(objBatteryInfo);
        }
    }
    return jsonArray;
}

QJsonArray DbusUpperInterface::getLogoutLockCheck()
{
    QJsonArray jsonArray;
    if (m_sessionHelper) {
        QVector<InhibitInfo::InhibitorInfo> lockCheckList = m_sessionHelper->getLockCheckStatus("logout");
        for (auto it = lockCheckList.constBegin(); it != lockCheckList.constEnd(); ++it) {
            QJsonObject objBatteryInfo;
            objBatteryInfo["name"] = it->name;
            objBatteryInfo["icon"] = it->icon;
            objBatteryInfo["uid"] = it->uid;
            jsonArray.append(objBatteryInfo);
        }
    }
    return jsonArray;
}

QJsonArray DbusUpperInterface::getShutdownLockcheck()
{
    QJsonArray jsonArray;
    if (m_sessionHelper) {
        QVector<InhibitInfo::InhibitorInfo> lockCheckList = m_sessionHelper->getLockCheckStatus("shutdown");
        for (auto it = lockCheckList.constBegin(); it != lockCheckList.constEnd(); ++it) {
            QJsonObject objBatteryInfo;
            objBatteryInfo["name"] = it->name;
            objBatteryInfo["icon"] = it->icon;
            objBatteryInfo["uid"] = it->uid;
            jsonArray.append(objBatteryInfo);
        }
    }
    return jsonArray;
}

QJsonArray DbusUpperInterface::getAppWindows()
{
    QJsonArray jsonArray;
    if (m_sessionHelper) {
        QVector<WindowInfo::WindowInfo> apps = m_sessionHelper->getAppWindows();
        for (auto it = apps.constBegin(); it != apps.constEnd(); ++it) {
            QJsonObject windowObj;
            windowObj["wid"] = it->wid;
            windowObj["icon"] = it->icon;
            windowObj["name"] = it->name;
            jsonArray.append(windowObj);
        }
        qDebug() << "Found" << jsonArray.size() << "app windows";
    } else {
        qWarning() << "m_sessionHelper is null!";
    }
    return jsonArray;
}

QJsonArray DbusUpperInterface::getSaverTheme()
{
    QJsonArray jsonArray;
    if (m_gsettingsHelper) {
        QStringList themeList = m_gsettingsHelper->GetLockScreenConf(KEY_THEMES).toStringList();
        for (auto list : themeList) {
            QJsonObject objBatteryInfo;
            objBatteryInfo["key"] = list;
            jsonArray.append(objBatteryInfo);
        }
    }
    return jsonArray;
}

QString DbusUpperInterface::getBatteryIconName()
{
    QString iconName;
    if (m_upowerHelper) {
        iconName = m_upowerHelper->getBatteryIconName();
    }
    return iconName;
}

bool DbusUpperInterface::getIsLegacy()
{
    bool isLegacy = false;
    if (m_sessionHelper) {
        isLegacy = m_sessionHelper->getIsLegacy();
    }
    return isLegacy;
}

bool DbusUpperInterface::getIsBattery()
{
    bool isBattery = false;
    if (m_upowerHelper) {
        isBattery = m_upowerHelper->getIsBattery();
    }
    return isBattery;
}

QString DbusUpperInterface::GetDefaultAuthUser()
{
    QString strUserName = "";
    if (!isGreeterMode()) {
        strUserName = getenv("USER");
    } else {
        // 获取lightdm传过来的被选中的用户
        WillLoginUserInfo willSwitchUser = UniAuthService::instance()->getWillSwitchUser();
        bool selectManualLogin = GreeterConfig::instance()->getSelectManualLoginDefault();
        QString selectedUser = "";
        if (willSwitchUser.strUserName.isEmpty()
            || !UniAuthService::instance()->isUserNameValid(willSwitchUser.strUserName)) {
            qDebug() << "WillSwitchUser is NULL or invalid:" << willSwitchUser.strUserName;
            selectedUser = m_lightDmHelper->selectUserHint();
        }
        UniAuthService::instance()->SwitchToGreeterUser("");
        if (!willSwitchUser.strUserName.isEmpty()) {
            strUserName = willSwitchUser.strUserName;
            qDebug() << "WillSwitchUser:" << strUserName;
        } else if (!selectedUser.isEmpty()) {
            qDebug() << "SelectUserHint: " << selectedUser << selectManualLogin;
            if (!selectManualLogin) {
                if (m_lightDmHelper->findUserByName(selectedUser)) {
                    strUserName = selectedUser;
                } else {
                    qDebug() << "SlelectUerHint invalid:" << selectedUser;
                }
            }
        }
        if (!strUserName.isEmpty()) {
            // do nothing
        } else if (selectManualLogin && m_lightDmHelper->findUserByName("*login")) {
            qDebug() << "Select Manual login";
            strUserName = "*login";
        } else if (m_lightDmHelper->selectGuestHint()) {
            qDebug() << "SelectGuest";
            strUserName = "*guest";
        } else if (m_lightDmHelper->getUsersInfo().count() == 1) { // 如果只有一个用户,默认选中
            strUserName = m_lightDmHelper->getUsersInfo()[0]->name();
        } else {
            QString lastLoginUser = UniAuthService::instance()->GetLastLoginUser();
            if (m_lightDmHelper->findUserByName(lastLoginUser)) {
                strUserName = lastLoginUser;
            } else {
                if (m_lightDmHelper->getUsersInfo().count() > 0) {
                    strUserName = m_lightDmHelper->getUsersInfo()[0]->name();
                } else {
                    qWarning() << "Have no user to select!!";
                }
            }
        }
    }
    return strUserName;
}

int DbusUpperInterface::StartSession(const QJsonObject &objInfo)
{
    Q_UNUSED(objInfo);
    m_lightDmHelper->startSession();
    return 0;
}

QString DbusUpperInterface::GetCurrentSession()
{
    return m_lightDmHelper->session();
}

int DbusUpperInterface::SetCurrentSession(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strSessionName = objInfo.value("Content").toString();
    if (!strSessionName.isEmpty()) {
        if (m_lightDmHelper->setSession(strSessionName)) {
            nRet = 0;
        }
    }
    return nRet;
}

void DbusUpperInterface::onCurrentSessionChanged(QString strSession)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_CURRENT_SESSION_CHANGED;
    retObj["Content"] = strSession;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

QJsonArray DbusUpperInterface::GenerateSessionInfoList()
{
    QJsonArray jsonArray;
    if (m_lightDmHelper) {
        QList<QString> sessionsInfo = m_lightDmHelper->getSessionsInfo();
        for (auto session : sessionsInfo) {
            QJsonObject objSessionInfo;
            objSessionInfo["key"] = session;
            jsonArray.append(objSessionInfo);
        }
    }
    return jsonArray;
}

QString DbusUpperInterface::GetCurrentUser()
{
    return m_lightDmHelper->getCurrentUser();
}

int DbusUpperInterface::SetCurrentUser(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strUserName = objInfo.value("Content").toString();
    if (!strUserName.isEmpty()) {
        if (m_lightDmHelper->setCurrentUser(strUserName)) {
            nRet = 0;
            if (m_accountsHelper) {
                QString strSession = m_accountsHelper->getUserSessionByName(strUserName);
                if (!strSession.isEmpty()) {
                    m_lightDmHelper->setSession(strSession);
                }
            }
        }
    }
    if (nRet >= 0) {
        QJsonObject retObj;
        retObj["CmdId"] = LOCK_CMD_ID_CURRENT_USER_CHANGED;
        retObj["Content"] = GetCurrentUser();
        SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
    }
    return nRet;
}

bool DbusUpperInterface::getCurTabletMode()
{
    bool tabletMode = false;
#ifdef SUPPORT_TABLET_MODE
    if (m_libinputSwitchEvent) {
        tabletMode = m_libinputSwitchEvent->geInitDevicesStatus();
    }
#endif
    return tabletMode;
}

int DbusUpperInterface::getNetworkConnectState()
{
    int state = -1;
    if (m_networkManagerHelper) {
        state = m_networkManagerHelper->getNetWorkConnectStatus();
    }
    return state;
}

bool DbusUpperInterface::GetSaverDatetimeEnable()
{
    QString currentUser = GetCurrentUser();
    if (currentUser.isEmpty()) {
        currentUser = qgetenv("USER");
    }
    return GreeterConfig::instance()->getScreensaverDatetimeEnable(currentUser);
}

void DbusUpperInterface::SetSaverDatetimeEnable(bool enable)
{
    GreeterConfig::instance()->setScreensaverDatetimeEnable(enable);
}

bool DbusUpperInterface::GetLockDatetimeEnable()
{
    QString currentUser = GetCurrentUser();
    if (currentUser.isEmpty()) {
        currentUser = qgetenv("USER");
    }
    return GreeterConfig::instance()->getLockScreenDatetimeEnable(currentUser);
}

void DbusUpperInterface::SetLockDatetimeEnable(bool enable)
{
    GreeterConfig::instance()->setLockScreenDatetimeEnable(enable);
}

void DbusUpperInterface::SetCrashedTimerStop()
{
    if (m_crashedTimer->isActive() && !m_isCrashedTootimes) {
        m_remainingTime = m_crashedTimer->remainingTime();
        qInfo() << "m_remainingTime = " << m_crashedTimer->remainingTime()
                << "intervalTime = " << m_crashedTimer->interval();
        m_crashedTimer->stop();
    }
}

int DbusUpperInterface::switchToUser(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strUserName = objInfo.value("Content").toString();
    if (!strUserName.isEmpty()) {
        QString strSwitchSeatPath = "";
        QString strDesUserName = strUserName;
        QString strCurUserName = SwitchUserUtils::GetCurUserName();
        if (strCurUserName.isEmpty()) {
            qDebug() << "GetCurUserName failed!";
            nRet = 1;
        } else {
            UserDisplayIfInfo curUdii = SwitchUserUtils::GetUserUDII(strCurUserName);
            UserDisplayIfInfo desUdii = { "" };
            if (!strDesUserName.isEmpty()) {
                desUdii = SwitchUserUtils::GetUserUDII(strDesUserName);
                if (desUdii.strUserName.isEmpty()) {
                    desUdii.strUserName = strDesUserName;
                }
            }

            if (!curUdii.strSeatPath.isEmpty()) {
                strSwitchSeatPath = curUdii.strSeatPath;
            }
            if (!desUdii.strSeatPath.isEmpty()) {
                strSwitchSeatPath = desUdii.strSeatPath;
            }

            if (strSwitchSeatPath.isEmpty()) {
                qWarning() << "switch user: switch seat path invalid!";
                nRet = 1;
            } else {
                nRet = SwitchUserUtils::SwitchToUserSession(strSwitchSeatPath, desUdii);
                qDebug() << "Switch user result:" << nRet;
            }
        }
    }
    return nRet;
}

bool DbusUpperInterface::GetCanSwitchUser()
{
    return SwitchUserUtils::CanSwitchUser();
}

void DbusUpperInterface::onUsersInfoChanged()
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_USERINFO_CHANGED;
    retObj["Content"] = GenerateUserInfoList();
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

bool DbusUpperInterface::SendUpdateInfoSig(QString strJson)
{
    QDBusMessage message;
    message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "UpdateInformation");
    message << strJson;
    return QDBusConnection::sessionBus().send(message);
}

void DbusUpperInterface::onLogin1ReqLock()
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_LOGIN1_REQ_LOCK;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
    // LockScreensaver();
    // 调用屏保会导致在两个已登录用户切锁屏，会出现屏保界面，因此改为调用锁屏
    if (!m_isCrashedTootimes) {
        Lock();
    }
}

void DbusUpperInterface::onLogin1ReqUnLock()
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_LOGIN1_REQ_UNLOCK;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::doPrepareForSleep(bool isSleep, QString strAction /*= ""*/)
{
    qInfo() << "doPrepareForSleep begin:" << isSleep << strAction;
    m_strAction = strAction;
    if (isSleep) {
        if (!checkStatus(SESSION_STATUS_SLEEPED)) {
            uninhibit();
            if (!strAction.isEmpty() && m_sessionHelper) {
                m_sessionHelper->doAction(strAction);
            }
            qInfo() << "doPrepareForSleep done:" << isSleep;
            return;
        }

        if (m_bBlankState) {
            qDebug() << "Is blank state";
            qInfo() << "doPrepareForSleep done:" << isSleep;
            return;
        }

        stopDelayLockScreen(false);
        m_bBlankState = true;
        if (checkScreenDialogRunning() || GetLockState() || GetSlpState()) {
            QDBusMessage message;
            message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
            message << "SleepLock";
            QDBusConnection::sessionBus().send(message);
        } else {
            this->onShowBlankScreensaver();
        }

        if (m_bLockState) {
            uninhibit();
            if (!strAction.isEmpty() && m_sessionHelper) {
                m_sessionHelper->doAction(strAction);
            }
            qInfo() << "doPrepareForSleep done:" << isSleep;
        } else {
            if (!m_timer) {
                m_timer = new QTimer(this);
                qDebug() << "m_bLockState " << m_bLockState;
                auto connection = connect(m_timer, &QTimer::timeout, this, &DbusUpperInterface::handleTimeout);
                m_connections.append(connection);
            }
            m_timer->start(200);
        }
    } else {
        m_nStatus &= ~SESSION_STATUS_SLEEPED;
        m_bBlankState = false;
        inhibit();
        qInfo() << "doPrepareForSleep done:" << isSleep;
    }
}

void DbusUpperInterface::handleTimeout()
{
    m_timerCount += 1;
    if (m_bLockState || m_timerCount > 20) {
        m_timer->stop();
        m_timerCount = 0;
        uninhibit();
        if (!m_strAction.isEmpty() && m_sessionHelper) {
            m_sessionHelper->doAction(m_strAction);
        }
        qInfo() << "doPrepareForSleep done: true timeout";
    }
}

void DbusUpperInterface::onLogin1PrepareForSleep(bool isSleep)
{
    qInfo() << "onLogin1PrepareForSleep:" << isSleep;
    doPrepareForSleep(isSleep);
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_LOGIN1_PREPAREFORSLEEP;
    retObj["IsSleep"] = isSleep;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onLogin1SessionActiveChanged(bool isActive)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_LOGIN1_SESSION_ACTIVE_CHANGED;
    retObj["IsActive"] = isActive;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onBlockLogoutInhibitedChanged()
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_LOGIN1_BLOCKLOGOUTINHIBITED_CHANGED;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onSessionLegacyChanged()
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_SESSION_LEGACY_COMPONENTS_CHANGED;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onBlockInhibitedChanged(QString blockInhibited)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_LOGIN1_BLOCKINHIBITED_CHANGED;
    retObj["BlockInhibited"] = blockInhibited;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onShowShutdownAnimation(QString command)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_SESSION_SHOW_SHUTDOWN_ANIMATION;
    retObj["Command"] = command;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onBatteryStatusChanged(QString iconName)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_UPOWER_BATTERY_STATUS;
    retObj["IconName"] = iconName;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onSessionIdleReceived()
{
    if (!checkStatus(SESSION_STATUS_SCREENSAVER)) {
        qDebug() << "m_nStatus:" << m_nStatus << " can't do screensaver!";
        return;
    }
    if (m_procLockDialog.state() != QProcess::NotRunning)
        return;

    m_isCrashedTootimes = false;
    qDebug() << "emit SessionIdle";
    m_bLockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --session-idle");
    qDebug() << cmd;
    m_procLockDialog.start(cmd);

    if (Configuration::checkIslivecdStatus() != 1) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "prepareForLock");
        QDBusConnection::sessionBus().send(message);
    }
    emitLockState(true);
    if (Configuration::checkIslivecdStatus() != 1 && !m_gsettingsHelper->GetLockScreenConf(KEY_LOCK_ENABLED).toBool()) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SessionIdle");
        QDBusConnection::sessionBus().send(message);
    }
}

void DbusUpperInterface::onSessionIdleExit()
{
    m_nStatus &= ~SESSION_STATUS_SCREENSAVER;
    m_nStatus &= ~SESSION_STATUS_SCREENCLOSE;
    stopDelayLockScreen();
}

void DbusUpperInterface::delayLockScreen()
{
    if (!m_timerLock) {
        m_timerLock = new QTimer(this);
        m_connections.append(connect(m_timerLock, &QTimer::timeout, this, &DbusUpperInterface::onLockScreenTimeout));
    }
    int nLockTimeout = m_gsettingsHelper->GetLockScreenConf(KEY_LOCK_TIMEOUT).toInt();
    qDebug() << "LockTimeout:" << nLockTimeout;
    if (nLockTimeout != -1) {
        stopDelayLockScreen();
        if (m_procLockDialog.state() != QProcess::NotRunning || checkScreenDialogRunning()) {
            QDBusMessage message;
            message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
            QString strParam = QString("CloseLock&delay=%1&hasLock=1").arg(nLockTimeout * 1000);
            message << strParam;
            QDBusConnection::sessionBus().send(message);
        } else {
            QString cmd
                = QString("/usr/bin/ukui-screensaver-dialog --blank --delay %1 --has-lock 1").arg(nLockTimeout * 1000);
            qDebug() << cmd;
            m_procLockDialog.start(cmd);
        }
        emitLockState(true);
        m_timerLock->start(m_gsettingsHelper->GetLockScreenConf(KEY_LOCK_TIMEOUT).toInt() * 1000);
    }
}

void DbusUpperInterface::stopDelayLockScreen(bool withUnlock /*= true*/)
{
    if (m_timerLock && m_timerLock->isActive()) {
        if (withUnlock) {
            UnLock();
        }
        m_timerLock->stop();
    }
}

void DbusUpperInterface::onLockScreenTimeout()
{
    m_timerLock->stop();
}

void DbusUpperInterface::onBatteryChanged(QStringList batteryArgs)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_UPOWER_BATTERY;
    QJsonArray jsonArray;
    for (auto args : batteryArgs) {
        QJsonObject objBatteryInfo;
        objBatteryInfo["key"] = args;
        jsonArray.append(objBatteryInfo);
    }
    retObj["Content"] = jsonArray;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onUPowerServiceReady()
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_UPOWER_SERVICE_READY;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::connectUpowerBatterySignals()
{
    if (!m_upowerHelper) {
        m_pendingBatteryConnect = true;
        qDebug() << "m_upowerHelper not initialized yet, deferring battery signal connection";
        return;
    }
    if (m_upowerBatteryListening) {
        return;
    }
    m_upowerBatteryConnections.append(connect(
        m_upowerHelper.get(),
        &UpowerHelper::batteryStatusChanged,
        this,
        &DbusUpperInterface::onBatteryStatusChanged));
    m_upowerBatteryConnections.append(
        connect(m_upowerHelper.get(), &UpowerHelper::batteryChanged, this, &DbusUpperInterface::onBatteryChanged));
    m_upowerBatteryListening = true;
    pushImmediateBatteryInfo();
}

void DbusUpperInterface::disconnectUpowerBatterySignals()
{
    m_pendingBatteryConnect = false;
    if (!m_upowerBatteryListening) {
        return;
    }
    for (const QMetaObject::Connection &c : m_upowerBatteryConnections) {
        QObject::disconnect(c);
    }
    m_upowerBatteryConnections.clear();
    m_upowerBatteryListening = false;
}

void DbusUpperInterface::pushImmediateBatteryInfo()
{
    if (!m_upowerHelper) {
        return;
    }
    onUPowerServiceReady();
    onBatteryChanged(m_upowerHelper->getBatteryArgs());
    onBatteryStatusChanged(m_upowerHelper->getBatteryIconName());
}

void DbusUpperInterface::onLidStateChanged(bool isClosed)
{
    if (!isClosed) {
        if (m_gsettingsHelper->GetPowerManagerConf(KEY_LOCK_BLANKSCREEN).toBool()) {
            m_nStatus &= ~SESSION_STATUS_SCREENCLOSE;
        }
        stopDelayLockScreen();
    }
}

void DbusUpperInterface::onLockScreenConfigChanged(QString strKey, QVariant value)
{
    if (strKey == KEY_BACKGROUND) {
        onUsersInfoChanged();
    }
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_GSETTINGS_LOCKSCREEN_CONF_CHANGED;
    retObj["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            retObj["Value"] = value.toBool();
            break;
        case QVariant::Int:
            retObj["Value"] = value.toInt();
            break;
        case QVariant::String:
            retObj["Value"] = value.toString();
            break;
        case QVariant::UInt:
            retObj["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            retObj["Value"] = value.toDouble();
            break;
        default:
            return;
    }

    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onScreenSaverConfigChanged(QString strKey, QVariant value)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_GSETTINGS_SCREENSAVER_CONF_CHANGED;
    retObj["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            retObj["Value"] = value.toBool();
            break;
        case QVariant::Int:
            retObj["Value"] = value.toInt();
            break;
        case QVariant::String:
            retObj["Value"] = value.toString();
            break;
        case QVariant::UInt:
            retObj["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            retObj["Value"] = value.toDouble();
            break;
        default:
            return;
    }

    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onPowerManagerConfigChanged(QString strKey, QVariant value)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_GSETTINGS_POWERMANAGER_CONF_CHANGED;
    retObj["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            retObj["Value"] = value.toBool();
            break;
        case QVariant::Int:
            retObj["Value"] = value.toInt();
            break;
        case QVariant::String:
            retObj["Value"] = value.toString();
            break;
        case QVariant::UInt:
            retObj["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            retObj["Value"] = value.toDouble();
            break;
        default:
            return;
    }

    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onMateBgConfigChanged(QString strKey, QVariant value)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_GSETTINGS_MATEBG_CONF_CHANGED;
    retObj["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            retObj["Value"] = value.toBool();
            break;
        case QVariant::Int:
            retObj["Value"] = value.toInt();
            break;
        case QVariant::String:
            retObj["Value"] = value.toString();
            break;
        case QVariant::UInt:
            retObj["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            retObj["Value"] = value.toDouble();
            break;
        default:
            return;
    }

    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onUkccPluginsConfigChanged(QString strKey, QVariant value)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_GSETTINGS_UKCCPLUGINS_CONF_CHANGED;
    retObj["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            retObj["Value"] = value.toBool();
            break;
        case QVariant::Int:
            retObj["Value"] = value.toInt();
            break;
        case QVariant::String:
            retObj["Value"] = value.toString();
            break;
        case QVariant::UInt:
            retObj["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            retObj["Value"] = value.toDouble();
            break;
        default:
            return;
    }
    if (strKey == KEY_DATE) {
        m_config->setShareConfigValue("dateType", value.toString());
    } else if (strKey == KEY_HOUR_SYSTEM) {
        m_config->setShareConfigValue("timeType", value.toInt());
    }

    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onThemeStyleConfigChanged(QString strKey, QVariant value)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_GSETTINGS_THEMESTYLE_CONF_CHANGED;
    retObj["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            retObj["Value"] = value.toBool();
            break;
        case QVariant::Int:
            retObj["Value"] = value.toInt();
            break;
        case QVariant::String:
            retObj["Value"] = value.toString();
            break;
        case QVariant::UInt:
            retObj["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            retObj["Value"] = value.toDouble();
            break;
        default:
            return;
    }
    if (strKey == KEY_SYSTEM_FONT_SIZE) {
        m_config->setShareConfigValue("fontSize", value.toDouble());
    } else if (strKey == KEY_THEME_COLOR) {
        m_config->setShareConfigValue("themeColor", value.toString());
    }
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onSessionConfigChanged(QString strKey, QVariant value)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_GSETTINGS_SESSION_CONF_CHANGED;
    retObj["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            retObj["Value"] = value.toBool();
            break;
        case QVariant::Int:
            retObj["Value"] = value.toInt();
            break;
        case QVariant::String:
            retObj["Value"] = value.toString();
            break;
        case QVariant::UInt:
            retObj["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            retObj["Value"] = value.toDouble();
            break;
        default:
            return;
    }

    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onKeyboardConfigChanged(QString strKey, QVariant value)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_GSETTINGS_KEYBOARD_CONF_CHANGED;
    retObj["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            retObj["Value"] = value.toBool();
            break;
        case QVariant::Int:
            retObj["Value"] = value.toInt();
            break;
        case QVariant::String:
            retObj["Value"] = value.toString();
            break;
        case QVariant::UInt:
            retObj["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            retObj["Value"] = value.toDouble();
            break;
        default:
            return;
    }
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onUsdMediaKeysConfigChanged(QString strKey, QVariant value)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_GSETTINGS_USD_MEDIAKEYS_CONF_CHANGED;
    retObj["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            retObj["Value"] = value.toBool();
            break;
        case QVariant::Int:
            retObj["Value"] = value.toInt();
            break;
        case QVariant::String:
            retObj["Value"] = value.toString();
            break;
        case QVariant::UInt:
            retObj["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            retObj["Value"] = value.toDouble();
            break;
        default:
            return;
    }
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onUsdMediaStateKeysConfigChanged(QString strKey, QVariant value)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_GSETTINGS_USD_MEDIA_STATE_KEYS_CONF_CHANGED;
    retObj["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            retObj["Value"] = value.toBool();
            break;
        case QVariant::Int:
            retObj["Value"] = value.toInt();
            break;
        case QVariant::String:
            retObj["Value"] = value.toString();
            break;
        case QVariant::UInt:
            retObj["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            retObj["Value"] = value.toDouble();
            break;
        default:
            return;
    }
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::GetScreenSaverConf(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetScreenSaverConf(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

QJsonArray DbusUpperInterface::getSaverPhotoPath(QString key)
{
    QJsonArray jsonArray;
    if (m_gsettingsHelper) {
        QStringList pathList = m_gsettingsHelper->GetScreenSaverConf(key).toStringList();
        for (auto list : pathList) {
            QJsonObject objPathInfo;
            objPathInfo["key"] = list;
            jsonArray.append(objPathInfo);
        }
    }
    return jsonArray;
}

int DbusUpperInterface::SetScreenSaverConf(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strKey = objInfo.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = objInfo.value("Value").toVariant();
        if (value.isValid()) {
            if (m_gsettingsHelper->SetScreenSaverConf(strKey, value)) {
                nRet = 0;
            }
        }
    }
    return nRet;
}

void DbusUpperInterface::GetLockScreenConf(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetLockScreenConf(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

int DbusUpperInterface::SetLockScreenConf(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strKey = objInfo.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = objInfo.value("Value").toVariant();
        if (value.isValid()) {
            if (m_gsettingsHelper->SetLockScreenConf(strKey, value)) {
                nRet = 0;
            }
        }
    }
    return nRet;
}

void DbusUpperInterface::GetPowerManagerConf(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetPowerManagerConf(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

int DbusUpperInterface::SetPowerManagerConf(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strKey = objInfo.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = objInfo.value("Value").toVariant();
        if (value.isValid()) {
            if (m_gsettingsHelper->SetPowerManagerConf(strKey, value)) {
                nRet = 0;
            }
        }
    }
    return nRet;
}

void DbusUpperInterface::GetMateBgConf(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetMateBgConf(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

int DbusUpperInterface::SetMateBgConf(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strKey = objInfo.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = objInfo.value("Value").toVariant();
        if (value.isValid()) {
            if (m_gsettingsHelper->SetMateBgConf(strKey, value)) {
                nRet = 0;
            }
        }
    }
    return nRet;
}

void DbusUpperInterface::GetUkccPluginsConf(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetUkccPluginsConf(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

int DbusUpperInterface::SetUkccPluginsConf(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strKey = objInfo.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = objInfo.value("Value").toVariant();
        if (value.isValid()) {
            if (m_gsettingsHelper->SetUkccPluginsConf(strKey, value)) {
                nRet = 0;
            }
        }
    }
    return nRet;
}

void DbusUpperInterface::GetThemeStyleConf(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetThemeStyleConf(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

int DbusUpperInterface::SetThemeStyleConf(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strKey = objInfo.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = objInfo.value("Value").toVariant();
        if (value.isValid()) {
            if (m_gsettingsHelper->SetThemeStyleConf(strKey, value)) {
                nRet = 0;
            }
        }
    }
    return nRet;
}

void DbusUpperInterface::GetSessionConf(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetSessionConf(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

int DbusUpperInterface::SetSessionConf(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strKey = objInfo.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = objInfo.value("Value").toVariant();
        if (value.isValid()) {
            if (m_gsettingsHelper->SetSessionConf(strKey, value)) {
                nRet = 0;
            }
        }
    }
    return nRet;
}

void DbusUpperInterface::GetKeyboardConf(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetKeyboardConf(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

int DbusUpperInterface::SetKeyboardConf(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strKey = objInfo.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = objInfo.value("Value").toVariant();
        if (value.isValid()) {
            if (m_gsettingsHelper->SetKeyboardConf(strKey, value)) {
                nRet = 0;
            }
        }
    }
    return nRet;
}

void DbusUpperInterface::onLockDialogProcExit(int exitCode, QProcess::ExitStatus exitStatus)
{
    qInfo() << "ukui-screensaver-dialog is exited,exitCode is " << exitCode;
    disconnectUpowerBatterySignals();
    if (m_bBlockShortcutState) {
	blockAllShortcuts(false);
        m_bBlockShortcutState = false;
    }
    QDBusInterface *sessionInterface = new QDBusInterface(
        SESSION_DBUS_SERVICE, SESSION_DBUS_PATH, SESSION_DBUS_INTERFACE, QDBusConnection::sessionBus(), this);
    if (!sessionInterface->isValid()) {
        qWarning() << "Failed to create valid D-Bus interface";
        delete sessionInterface;
        return;
    }
    if (Configuration::checkIslivecdStatus()) {
        return ;
    }
    qDebug() << "Greeter process exit unregular. Restarting lock.";
    if (!m_isCrashedTootimes) {
        // Perhaps it crashed due to a graphics driver issue, force software rendering now
        qDebug() << "Trying to lock again with software rendering";
        QDBusReply<int> status = sessionInterface->call("getScreenLockStatus");
        if (!status.isValid()) {
            qWarning() << "getScreenLockStatus error:" << status.error();
        } else {
            if (m_crashedTimer) {
                qInfo() << "m_remainingTime = " << m_remainingTime << "status = " << status;
                m_crashedTimer->start(m_remainingTime);
            }
            if (status == LOCK || status == SAVER_LOCK) {
                Lock();
            } else if (status == SCREEN_SAVER_STATUS) {
                onSessionIdleReceived();
            }
        }
    } else { // 不再放在定时器结束后立马展示异常界面，放在定时器结束后，锁屏再次崩溃再展示 bug#275869
        QDBusReply<int> status = sessionInterface->call("getScreenLockStatus");
        if (!(status == UNLOCK)) {
            QDBusMessage sessionMsg = sessionInterface->call("showEmergencyWindow");
            if (sessionMsg.type() == QDBusMessage::ErrorMessage)
                qDebug() << sessionMsg.errorMessage();
        }
    }
    delete sessionInterface;
}

bool DbusUpperInterface::checkStatus(int nStatus)
{
    if (!m_gsettingsHelper) {
        return false;
    }
    switch (nStatus) {
        case SESSION_STATUS_SLEEPED: {
            m_nStatus |= nStatus;
            if (!m_gsettingsHelper->GetLockScreenConf(KEY_SLEEP_ACTIVATION_ENABLED).toBool()) {
                return false;
            }
        } break;
        case SESSION_STATUS_SCREENCLOSE: {
            m_nStatus |= nStatus;
            if ((m_nStatus & SESSION_STATUS_SLEEPED)
                || !m_gsettingsHelper->GetLockScreenConf(KEY_CLOSE_ACTIVATION_ENABLED).toBool()) {
                return false;
            }
            if (((m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_COMPUTER_AC).toInt()
                  == m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_DISPLAY_AC).toInt())
                 && m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_COMPUTER_AC).toInt() != -1)
                && !m_gsettingsHelper->GetLockScreenConf(KEY_SLEEP_ACTIVATION_ENABLED).toBool()) {
                qDebug() << "Sleep same with Close and Sleep disable!";
                return false;
            }
        } break;
        case SESSION_STATUS_SCREENSAVER: {
            m_nStatus |= nStatus;
            if ((m_nStatus & SESSION_STATUS_SLEEPED) || (m_nStatus & SESSION_STATUS_SCREENCLOSE)) {
                return false;
            }
            if (((m_gsettingsHelper->GetLockScreenConf(KEY_IDLE_DELAY).toInt() * 60
                  == m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_COMPUTER_AC).toInt())
                 && m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_COMPUTER_AC).toInt() != -1)
                && !m_gsettingsHelper->GetLockScreenConf(KEY_SLEEP_ACTIVATION_ENABLED).toBool()) {
                qDebug() << "Sleep same with idle and Sleep disable!";
                return false;
            }
            if (((m_gsettingsHelper->GetLockScreenConf(KEY_IDLE_DELAY).toInt() * 60
                  == m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_DISPLAY_AC).toInt())
                 && m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_DISPLAY_AC).toInt()
                        != -1) /* && !m_gsettingsHelper->GetLockScreenConf(KEY_CLOSE_ACTIVATION_ENABLED).toBool()*/) {
                qDebug() << "Close same with idle do nothing!";
                return false;
            }
        } break;
        default:
            return false;
    }
    return true;
}

void DbusUpperInterface::inhibit()
{
    if (m_inhibitFileDescriptor.isValid()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        QStringLiteral("Inhibit"));
    message.setArguments(QVariantList({ QStringLiteral("sleep"),
                                        "Screen Locker Backend",
                                        "Ensuring that the screen gets locked before going to sleep",
                                        QStringLiteral("delay") }));
    QDBusPendingReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(message);
    if (!reply.isValid()) {
        return;
    }
    reply.value().swap(m_inhibitFileDescriptor);
}

void DbusUpperInterface::uninhibit()
{
    if (!m_inhibitFileDescriptor.isValid()) {
        return;
    }

    m_inhibitFileDescriptor = QDBusUnixFileDescriptor();
}

void DbusUpperInterface::IsPamInAuthentication(QJsonObject &reqObj, QJsonObject &retObj)
{
    Q_UNUSED(reqObj);
    if (m_pamAuth) {
        retObj["Content"] = m_pamAuth->inAuthentication();
    } else {
        retObj["Content"] = false;
    }
}

void DbusUpperInterface::IsPamAuthenticated(QJsonObject &reqObj, QJsonObject &retObj)
{
    Q_UNUSED(reqObj);
    if (m_pamAuth) {
        retObj["Content"] = m_pamAuth->isAuthenticated();
    } else {
        retObj["Content"] = false;
    }
}

void DbusUpperInterface::PamAuthenticationUser(QJsonObject &reqObj, QJsonObject &retObj)
{
    Q_UNUSED(reqObj);
    if (m_pamAuth) {
        retObj["Content"] = m_pamAuth->authenticationUser();
    } else {
        retObj["Content"] = "";
    }
}

int DbusUpperInterface::PamAuthenticateUser(const QJsonObject &objInfo)
{
    int nRet = -1;
    QVariant varUserName = objInfo.value("UserName");
    if (varUserName.isValid() && m_pamAuth) {
        m_pamAuth->authenticate(varUserName.toString());
        nRet = 0;
    }
    return nRet;
}

void DbusUpperInterface::GetAutologinUserHint(QJsonObject &reqObj, QJsonObject &retObj)
{
    Q_UNUSED(reqObj);
    if (m_pamAuth) {
        retObj["Content"] = m_pamAuth->autologinUserHint();
    } else {
        retObj["Content"] = "";
    }
}

int DbusUpperInterface::PamAuthenticateCancel(const QJsonObject &objInfo)
{
    Q_UNUSED(objInfo);
    int nRet = -1;
    if (m_pamAuth) {
        m_pamAuth->cancelAuthentication();
        if (m_pamSessionServer) {
            m_pamSessionServer->stopSession();
        }
        nRet = 0;
    }
    return 0;
}

int DbusUpperInterface::PamRespond(const QJsonObject &objInfo)
{
    int nRet = -1;
    QVariant varRespond = objInfo.value("Content");
    if (varRespond.isValid() && m_pamAuth) {
        QString strDecrypt = varRespond.toString();
        if (strDecrypt != BIOMETRIC_IGNORE && !priKey.isEmpty()) {
            QByteArray buffer;
            QByteArray byteEncrypt = QByteArray::fromBase64(strDecrypt.toLatin1());
            if (rsac.decryptData(byteEncrypt, buffer, priKey)) {
                strDecrypt = buffer;
            }
        }
        m_pamAuth->respond(strDecrypt);
        nRet = 0;
    }
    return nRet;
}

void DbusUpperInterface::onPamShowMessage(QString strMsg, int type)
{
    QJsonObject pipeObj;
    pipeObj["Message"] = strMsg;
    pipeObj["MsgType"] = type;
    if (m_pamSessionServer && m_pamSessionServer->writePacket(LOCK_CMD_ID_PAMAUTH_SHOWMESSAGE, pipeObj)) {
        return;
    }
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_PAMAUTH_SHOWMESSAGE;
    retObj["Message"] = strMsg;
    retObj["MsgType"] = type;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onPamShowPrompt(QString strPrompt, int type)
{
    QJsonObject pipeObj;
    pipeObj["Prompt"] = strPrompt;
    pipeObj["PromptType"] = type;
    if (m_pamSessionServer && m_pamSessionServer->writePacket(LOCK_CMD_ID_PAMAUTH_SHOWPROMPT, pipeObj)) {
        return;
    }
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_PAMAUTH_SHOWPROMPT;
    retObj["Prompt"] = strPrompt;
    retObj["PromptType"] = type;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onPamShowBinaryPrompt(QVariant value, int type)
{
    QJsonObject pipeObj;
    pipeObj["BinaryPromptType"] = type;
    pipeObj["BinaryVal"] = value.toJsonValue();
    if (m_pamSessionServer && m_pamSessionServer->writePacket(LOCK_CMD_ID_PAMAUTH_SHOWPBINARYROMPT, pipeObj)) {
        return;
    }
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_PAMAUTH_SHOWPBINARYROMPT;
    retObj["BinaryPromptType"] = type;
    retObj["BinaryVal"] = value.toJsonValue();
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onPamAuthCompleted()
{
    if (m_pamSessionServer && m_pamSessionServer->writePacket(LOCK_CMD_ID_PAMAUTH_AUTH_COMPLETED)) {
        m_pamSessionServer->stopSession();
        return;
    }
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_PAMAUTH_AUTH_COMPLETED;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onBioAuthShowMessage(QString strMsg)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_BIOAUTH_SHOW_MESSAGE;
    retObj["Message"] = strMsg;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onBioAuthStateChanged(int nState)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_BIOAUTH_AUTHSTATE_CHANGED;
    retObj["State"] = nState;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onBioServiceStatusChanged(bool bValid)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_BIOAUTH_SERVICE_STATUS_CHANGED;
    retObj["IsValid"] = bValid;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onBioDeviceChanged()
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_BIOAUTH_DEVICE_CHANGED;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onBioAuthFrameData(QString strData)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_BIOAUTH_FRAME_DATA;
    retObj["Data"] = strData;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onBioAuthCompleted(int nUid, bool isSuccess, int nError, int nMaxFailedTime, int nFailedTime)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_BIOAUTH_COMPLETE;
    retObj["UId"] = nUid;
    retObj["IsSuccess"] = isSuccess;
    retObj["ErrorNo"] = nError;
    retObj["MaxFailedTime"] = nMaxFailedTime;
    retObj["FailedTime"] = nFailedTime;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onTabletModeChanged(bool tabletMode)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_TABLET_MODE_CHANGED;
    retObj["mode"] = tabletMode;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onNetWorkConnectStateChanged(int state)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_NETWORK_CONNECT_STATE_CHANGED;
    retObj["state"] = state;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onScreensaverDatetimeChanged(bool isEnable, QString strUserName)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_SET_SAVER_DATETIME_ENABLE_DATA_CHANGED;
    retObj["enable"] = isEnable;
    retObj["username"] = strUserName;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onScreenlockDatetimeChanged(bool isEnable, QString strUserName)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_SET_LOCK_DATETIME_ENABLE_DATA_CHANGED;
    retObj["enable"] = isEnable;
    retObj["username"] = strUserName;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::BioGetAvailableDevices(QJsonObject &reqObj, QJsonObject &retObj)
{
#ifdef ENABLE_BIO_BACKEND
    QVariant varUserId = reqObj.value("UserId");
    if (varUserId.isValid() && m_bioAuth) {
        DeviceMap bioDevInfos = m_bioAuth->getAvailableDevices(varUserId.toInt());
        QJsonArray objArray;
        DeviceMap::iterator itValidDev = bioDevInfos.begin();
        for (; itValidDev != bioDevInfos.end(); itValidDev++) {
            QJsonObject objDevInfo;
            objDevInfo["id"] = itValidDev.value()->id;
            objDevInfo["ShortName"] = itValidDev.value()->shortName;
            objDevInfo["FullName"] = itValidDev.value()->fullName;
            objDevInfo["DriverEnable"] = itValidDev.value()->driverEnable;
            objDevInfo["DeviceNum"] = itValidDev.value()->deviceNum;
            objDevInfo["DeviceType"] = itValidDev.value()->deviceType;
            objDevInfo["StorageType"] = itValidDev.value()->storageType;
            objDevInfo["EigType"] = itValidDev.value()->eigType;
            objDevInfo["VerifyType"] = itValidDev.value()->verifyType;
            objDevInfo["IdentifyType"] = itValidDev.value()->identifyType;
            objDevInfo["BusType"] = itValidDev.value()->busType;
            objDevInfo["DeviceStatus"] = itValidDev.value()->deviceStatus;
            objDevInfo["OpsStatus"] = itValidDev.value()->OpsStatus;
            objArray.append(objDevInfo);
        }
        retObj["Content"] = objArray;
    } else {
        retObj["Content"] = QJsonArray();
    }
#endif
}

void DbusUpperInterface::BioGetDisabledDevices(QJsonObject &reqObj, QJsonObject &retObj)
{
#ifdef ENABLE_BIO_BACKEND
    QVariant varUserId = reqObj.value("UserId");
    if (varUserId.isValid() && m_bioAuth) {
        QList<int> disabledInfos = m_bioAuth->getDisabledDevices(varUserId.toInt());
        QJsonArray objArray;
        for (auto devType : disabledInfos) {
            objArray.append(devType);
        }
        retObj["Content"] = objArray;
    } else {
        retObj["Content"] = QJsonArray();
    }
#endif
}

void DbusUpperInterface::BioGetBioAuthState(QJsonObject &reqObj, QJsonObject &retObj)
{
#ifdef ENABLE_BIO_BACKEND
    Q_UNUSED(reqObj);
    if (m_bioAuth) {
        retObj["Content"] = m_bioAuth->getBioAuthState();
    } else {
        retObj["Content"] = BIOAUTH_IDLE;
    }
#endif
}

void DbusUpperInterface::BioGetCurBioInfo(QJsonObject &reqObj, QJsonObject &retObj)
{
#ifdef ENABLE_BIO_BACKEND
    Q_UNUSED(reqObj);
    if (m_bioAuth) {
        DeviceInfo bioDevInfo = m_bioAuth->getCurBioInfo();
        QJsonObject objDevInfo;
        objDevInfo["id"] = bioDevInfo.id;
        objDevInfo["ShortName"] = bioDevInfo.shortName;
        objDevInfo["FullName"] = bioDevInfo.fullName;
        objDevInfo["DriverEnable"] = bioDevInfo.driverEnable;
        objDevInfo["DeviceNum"] = bioDevInfo.deviceNum;
        objDevInfo["DeviceType"] = bioDevInfo.deviceType;
        objDevInfo["StorageType"] = bioDevInfo.storageType;
        objDevInfo["EigType"] = bioDevInfo.eigType;
        objDevInfo["VerifyType"] = bioDevInfo.verifyType;
        objDevInfo["IdentifyType"] = bioDevInfo.identifyType;
        objDevInfo["BusType"] = bioDevInfo.busType;
        objDevInfo["DeviceStatus"] = bioDevInfo.deviceStatus;
        objDevInfo["OpsStatus"] = bioDevInfo.OpsStatus;
        retObj["Content"] = objDevInfo;
    } else {
        retObj["Content"] = QJsonObject();
    }
#endif
}

void DbusUpperInterface::BioGetDefaultDevice(QJsonObject &reqObj, QJsonObject &retObj)
{
#ifdef ENABLE_BIO_BACKEND
    qDebug() << "BioGetDefaultDevice:" << reqObj;
    QVariant varUserId = reqObj.value("UserId");
    QVariant varUserName = reqObj.value("UserName");
    if (varUserId.isValid() && varUserName.isValid() && m_bioAuth) {
        if (reqObj.contains("DevType")) {
            QVariant varDevType = reqObj.value("DevType");
            retObj["Content"]
                = m_bioAuth->getDefaultDevice(varUserId.toInt(), varUserName.toString(), varDevType.toInt());
        } else {
            retObj["Content"] = m_bioAuth->getDefaultDevice(varUserId.toInt(), varUserName.toString());
        }
    } else {
        retObj["Content"] = "";
    }
#endif
}

void DbusUpperInterface::BioFindDeviceById(QJsonObject &reqObj, QJsonObject &retObj)
{
#ifdef ENABLE_BIO_BACKEND
    QVariant varUserId = reqObj.value("UserId");
    QVariant varDevId = reqObj.value("DevId");
    if (varUserId.isValid() && varDevId.isValid() && m_bioAuth) {
        DeviceInfoPtr ptrDevInfo = m_bioAuth->findDeviceById(varUserId.toInt(), varDevId.toInt());
        QJsonObject objDevInfo;
        if (ptrDevInfo) {
            objDevInfo["id"] = ptrDevInfo->id;
            objDevInfo["ShortName"] = ptrDevInfo->shortName;
            objDevInfo["FullName"] = ptrDevInfo->fullName;
            objDevInfo["DriverEnable"] = ptrDevInfo->driverEnable;
            objDevInfo["DeviceNum"] = ptrDevInfo->deviceNum;
            objDevInfo["DeviceType"] = ptrDevInfo->deviceType;
            objDevInfo["StorageType"] = ptrDevInfo->storageType;
            objDevInfo["EigType"] = ptrDevInfo->eigType;
            objDevInfo["VerifyType"] = ptrDevInfo->verifyType;
            objDevInfo["IdentifyType"] = ptrDevInfo->identifyType;
            objDevInfo["BusType"] = ptrDevInfo->busType;
            objDevInfo["DeviceStatus"] = ptrDevInfo->deviceStatus;
            objDevInfo["OpsStatus"] = ptrDevInfo->OpsStatus;
        }
        retObj["Content"] = objDevInfo;
    } else {
        retObj["Content"] = QJsonObject();
    }
#endif
}

void DbusUpperInterface::initPublicEncrypt()
{
    m_initEncrypt = QtConcurrent::run([=]() {
        if (pubKey.isEmpty()) {
            rsac.genKeyPair(priKey, pubKey, 2048);
        }
    });
}

QString DbusUpperInterface::getPublicEncrypt()
{
    if (pubKey.isEmpty()) {
        initPublicEncrypt();
        m_initEncrypt.waitForFinished();
    }
    if (pubKey.isEmpty()) {
        rsac.genKeyPair(priKey, pubKey, 2048);
    }
    return QString(pubKey);
}

bool DbusUpperInterface::sendPassword(const QString username, QByteArray password)
{
    if (priKey.isEmpty())
        return false;

    QByteArray decryptText;
    rsac.decryptData(password, decryptText, priKey); // 解密

    if (GetLockState()) {
        if (m_pamAuth && m_pamAuth->inAuthentication() && (m_pamAuth->authenticationUser() == username)) {
            m_pamAuth->respond(QString(decryptText));
        } else if (m_pamAuth) {
            m_pamAuth->authenticate(username);
            QTimer::singleShot(100, this, [=]() { m_pamAuth->respond(QString(decryptText)); });
        }
    }

    return true;
}

void DbusUpperInterface::BioFindDeviceByName(QJsonObject &reqObj, QJsonObject &retObj)
{
    QVariant varUserId = reqObj.value("UserId");
    QVariant varDevName = reqObj.value("DevName");
    if (varUserId.isValid() && varDevName.isValid() && m_bioAuth) {
        DeviceInfoPtr ptrDevInfo = m_bioAuth->findDeviceByName(varUserId.toInt(), varDevName.toString());
        QJsonObject objDevInfo;
        if (ptrDevInfo) {
            objDevInfo["id"] = ptrDevInfo->id;
            objDevInfo["ShortName"] = ptrDevInfo->shortName;
            objDevInfo["FullName"] = ptrDevInfo->fullName;
            objDevInfo["DriverEnable"] = ptrDevInfo->driverEnable;
            objDevInfo["DeviceNum"] = ptrDevInfo->deviceNum;
            objDevInfo["DeviceType"] = ptrDevInfo->deviceType;
            objDevInfo["StorageType"] = ptrDevInfo->storageType;
            objDevInfo["EigType"] = ptrDevInfo->eigType;
            objDevInfo["VerifyType"] = ptrDevInfo->verifyType;
            objDevInfo["IdentifyType"] = ptrDevInfo->identifyType;
            objDevInfo["BusType"] = ptrDevInfo->busType;
            objDevInfo["DeviceStatus"] = ptrDevInfo->deviceStatus;
            objDevInfo["OpsStatus"] = ptrDevInfo->OpsStatus;
        }
        retObj["Content"] = objDevInfo;
    } else {
        retObj["Content"] = QJsonObject();
    }
}

int DbusUpperInterface::BioStartAuth(const QJsonObject &objInfo)
{
    int nRet = -1;
    QVariant varUserId = objInfo.value("UserId");
    QVariant varDevId = objInfo.value("DevId");
    if (varUserId.isValid() && varDevId.isValid() && m_bioAuth) {
        m_bioAuth->startAuth(varUserId.toInt(), varDevId.toInt());
        nRet = 0;
    }
    return nRet;
}

int DbusUpperInterface::BioStopAuth(const QJsonObject &objInfo)
{
    Q_UNUSED(objInfo);
    int nRet = -1;
    if (m_bioAuth) {
        m_bioAuth->stopAuth();
        nRet = 0;
    }
    return nRet;
}

void DbusUpperInterface::GetUsdMediaKeys(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetUsdMediaKeys(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

void DbusUpperInterface::GetUsdMediaStateKeys(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetUsdMediaStateKeys(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

int DbusUpperInterface::SetUsdMediaStateKeys(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strKey = objInfo.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = objInfo.value("Value").toVariant();
        if (value.isValid()) {
            if (m_gsettingsHelper->SetUsdMediaStateKeys(strKey, value)) {
                nRet = 0;
            }
        }
    }
    return nRet;
}

void DbusUpperInterface::GetWeatherKeys(QJsonObject &reqObj, QJsonObject &retObj)
{
    QString strKey = reqObj.value("Key").toString();
    if (!strKey.isEmpty()) {
        QVariant value = m_gsettingsHelper->GetWeatherKeys(strKey);
        switch (value.type()) {
            case QVariant::Bool:
                retObj[strKey] = value.toBool();
                return;
            case QVariant::Int:
                retObj[strKey] = value.toInt();
                return;
            case QVariant::String:
                retObj[strKey] = value.toString();
                qDebug() << retObj[strKey];
                return;
            case QVariant::UInt:
                retObj[strKey] = value.toDouble();
                return;
            case QVariant::Double:
                retObj[strKey] = value.toDouble();
                return;
            default:
                break;
        }
    }
    retObj["Ret"] = -2;
    retObj["Message"] = "Key is invalid!!";
}

bool DbusUpperInterface::usdExternalDoAction(const QJsonObject &objInfo)
{
    if (!m_usdHelper) {
        m_usdHelper = QSharedPointer<UsdHelper>(new UsdHelper(this));
    }
    int nRet = -1;
    QString actionType = objInfo.value("Content").toString();
    if (m_usdHelper->usdExternalDoAction(actionType)) {
        nRet = 0;
    }
    return nRet;
}

bool DbusUpperInterface::blockShortcut(const QJsonObject &objInfo)
{
    int nRet = -1;
    bool isBlock = objInfo.value("Content").toBool();
    if (blockAllShortcuts(isBlock)) {
        nRet = 0;
    }
    return nRet;
}

bool DbusUpperInterface::lockStateChanged(const QJsonObject &objInfo)
{
    bool state = objInfo.value("Content").toBool();
    bool isSessionTools = objInfo.value("SessionTools").toBool();
    m_isSessionIdle = objInfo.value("SessionIdle").toBool();
    qInfo() << " state = " << state << " m_bLockState = " << m_bLockState << "isSessionIdle = " << m_isSessionIdle;
    if (m_bLockState != state && !isSessionTools) {
        m_bLockState = state;
        if (Configuration::checkIslivecdStatus() != 1) {
            if (m_bLockState && m_isSessionIdle && !m_gsettingsHelper->GetLockScreenConf(KEY_LOCK_ENABLED).toBool()) {
                QDBusMessage message;
                message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SessionIdle");
                QDBusConnection::sessionBus().send(message);
                SetCrashedTimerStop();
            } else if (m_bLockState) {
                QDBusMessage message;
                message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "prepareForLock");
                QDBusConnection::sessionBus().send(message);
                SetCrashedTimerStop();
            }
        }
    }
    emitLockState(state, isSessionTools);
    return m_bLockState;
}

bool DbusUpperInterface::SetClosePhase(const QJsonObject &objInfo)
{
    bool inCall = objInfo.value("InCall").toBool();
    m_sessionHelper->setClosePhase(inCall);
    return true;
}

int DbusUpperInterface::SetPowerManager(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strFuncName = objInfo.value("Content").toString();
    bool isForceExecute = objInfo.value("Force").toBool();
    bool isSessionTools = objInfo.value("IsSessionTools").toBool(false);
    if (!getIsLegacy()) {
        bool isCloseWindow = objInfo.value("IsCloseWindow").toBool(false);
        bool alreadyShowMulUsers = objInfo.value("AlreadyShowMulUsers").toBool(false);
        if (!strFuncName.isEmpty()) {
            qWarning() << __LINE__ << __FUNCTION__ << strFuncName << isForceExecute << isSessionTools << isCloseWindow
                       << alreadyShowMulUsers;
            if (strFuncName != "UpgradeThenRestart" && strFuncName != "UpgradeThenShutdown" && m_sessionHelper) {
                if (!isGreeterMode()) {
                    if (strFuncName == "Reboot" || strFuncName == "PowerOff" || strFuncName == "Logout") {
                        if (isCloseWindow) {
                            m_sessionHelper->doAction(strFuncName);
                            shutdownAnimation(strFuncName);
                            return -1;
                        } else {
                            m_sessionHelper->setClosePhase(true);
                        }
                    }

                    if (!isForceExecute && !isSessionTools) {
                        if (strFuncName == QStringLiteral("RebootMultiuser")) {
                            strFuncName = "Reboot";
                            m_sessionHelper->doAction(strFuncName);
                            shutdownAnimation(strFuncName);
                            return -1;
                        } else if (
                            strFuncName == QStringLiteral("PowerOffMultiuser")) { // strFuncName.contains("PowerOff")
                            strFuncName = "PowerOff";
                            m_sessionHelper->doAction(strFuncName);
                            shutdownAnimation(strFuncName);
                            return -1;
                        }

                        if (strFuncName == "Hibernate" || strFuncName == "Suspend") {
                            QVector<InhibitInfo::InhibitorInfo> sleepLockcheck
                                = m_sessionHelper->getLockCheckStatus("sleep");
                            if (!sleepLockcheck.isEmpty()) {
                                AppBlockWindow(strFuncName);
                                return -1;
                            }
                        }

                        if (strFuncName == "Reboot" || strFuncName == "PowerOff" || strFuncName == "Logout") {
                            if (GetLoggedInUsersCount() > 1 && strFuncName != "Logout" && !alreadyShowMulUsers) {
                                qDebug() << "multiuserblockwindow";
                                if (strFuncName == "Reboot") {
                                    MultiUserBlockWindow("Reboot");
                                } else if (strFuncName == "PowerOff") {
                                    MultiUserBlockWindow("PowerOff");
                                }
                                return -1;
                            }

                            QVector<InhibitInfo::InhibitorInfo> shutdownLockcheck
                                = m_sessionHelper->getLockCheckStatus("shutdown");
                            QVector<InhibitInfo::InhibitorInfo> logoutLockcheck
                                = m_sessionHelper->getLockCheckStatus("logout");
                            int count = shutdownLockcheck.count() + logoutLockcheck.count() + m_windowList.count();
                            qDebug() << "inhibit count" << count << shutdownLockcheck.count() << logoutLockcheck.count()
                                     << m_windowList.count();
                            if (count > 0) {
                                AppBlockWindow(strFuncName);
                                return -1;
                            }
                        }
                    }

                    qDebug() << "doaction" << strFuncName;
                    if (!isForceExecute) {
                        m_sessionHelper->doAction(strFuncName);
                    } else {
                        if (m_login1Helper) {
                            m_login1Helper->setPowerManager(strFuncName);
                        } else {
                            m_sessionHelper->doAction(strFuncName);
                        }
                    }
                } else if (m_login1Helper) {
                    m_login1Helper->setPowerManager(strFuncName);
                }
            } else if (strFuncName == "UpgradeThenRestart" && m_systemsUpgradeHelper) {
                m_systemsUpgradeHelper->doUpgradeThenRboot();
            } else if (strFuncName == "UpgradeThenShutdown" && m_systemsUpgradeHelper) {
                m_systemsUpgradeHelper->doUpgradeThenShutdown();
            }
            nRet = 0;
        }
    } else {
        if (!strFuncName.isEmpty()) {
            qWarning() << __LINE__ << __FUNCTION__ << strFuncName << isForceExecute;
            if (strFuncName != "UpgradeThenRestart" && strFuncName != "UpgradeThenShutdown" && m_sessionHelper) {
                if (!isGreeterMode()) {
                    if (!isForceExecute && !isSessionTools) {
                        if (strFuncName == "Hibernate" || strFuncName == "Suspend") {
                            QVector<InhibitInfo::InhibitorInfo> sleepLockcheck
                                = m_sessionHelper->getLockCheckStatus("sleep");
                            if (!sleepLockcheck.isEmpty()) {
                                AppBlockWindow(strFuncName);
                                return -1;
                            }
                        }
                        if (strFuncName == "Reboot" || strFuncName == "PowerOff" || strFuncName == "Logout") {
                            QVector<InhibitInfo::InhibitorInfo> shutdownLockcheck
                                = m_sessionHelper->getLockCheckStatus("shutdown");
                            if (shutdownLockcheck.count() > 0) {
                                AppBlockWindow(strFuncName);
                                return -1;
                            }
                            QVector<InhibitInfo::InhibitorInfo> logoutLockcheck
                                = m_sessionHelper->getLockCheckStatus("logout");
                            if (logoutLockcheck.count() > 0) {
                                qDebug() << "logoutlockcheck";
                                AppBlockWindow(strFuncName, false);
                                return -1;
                            }
                            if (GetLoggedInUsersCount() > 1 && strFuncName != "Logout") {
                                qDebug() << "multiuserblockwindow";
                                if (strFuncName == "Reboot") {
                                    MultiUserBlockWindow("Reboot");
                                } else if (strFuncName == "PowerOff") {
                                    MultiUserBlockWindow("PowerOff");
                                }
                                return -1;
                            }
                        }
                    }

                    // 当存在多用户且存在软阻塞的情况下，且用户选择的是关机或重启，并且用户在软阻塞场景下点击了确认按钮
                    if (strFuncName == "RebootSoftBlock" || strFuncName == "PoweroffSoftBlock") {
                        if (strFuncName == "RebootSoftBlock") {
                            strFuncName = "Reboot";
                        } else if (strFuncName == "PoweroffSoftBlock") {
                            strFuncName = "PowerOff";
                        }
                        if (GetLoggedInUsersCount() > 1) {
                            qDebug() << "multiuserblockwindow";
                            if (strFuncName == "Reboot") {
                                MultiUserBlockWindow("Reboot");
                            } else if (strFuncName == "PowerOff") {
                                MultiUserBlockWindow("PowerOff");
                            }
                            return -1;
                        }
                    }

                    qDebug() << "doaction" << strFuncName;
                    if (!isForceExecute) {
                        m_sessionHelper->doAction(strFuncName);
                    } else {
                        if (m_login1Helper) {
                            m_login1Helper->setPowerManager(strFuncName);
                        } else {
                            m_sessionHelper->doAction(strFuncName);
                        }
                    }
                } else if (m_login1Helper) {
                    m_login1Helper->setPowerManager(strFuncName);
                }
            } else if (strFuncName == "UpgradeThenRestart" && m_systemsUpgradeHelper) {
                m_systemsUpgradeHelper->doUpgradeThenRboot();
            } else if (strFuncName == "UpgradeThenShutdown" && m_systemsUpgradeHelper) {
                m_systemsUpgradeHelper->doUpgradeThenShutdown();
            }
            nRet = 0;
        }
    }
    return nRet;
}

bool DbusUpperInterface::ShowFolder(QStringList paths)
{
    int nRet = -1;
    if (m_fileManagerHelper->showFolders(paths)) {
        nRet = 0;
    }
    return nRet;
}

QSharedPointer<GSettingsHelper> DbusUpperInterface::getGSettingsInstance()
{
    return m_gsettingsHelper;
}

void DbusUpperInterface::CheckTurnOnScreen()
{
    if (m_sessionWatcher && m_sessionWatcher->isSessionIdle()) {
        QProcess *procTurnOn = new QProcess(this);
        auto finishedSignal = static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished);
        connect(procTurnOn, finishedSignal, this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
            qDebug() << procTurnOn->arguments() << "finished:" << exitCode << exitStatus;
            procTurnOn->deleteLater();
        });
        if ("wayland" == qgetenv("XDG_SESSION_TYPE")) {
            QProcessEnvironment procEnv = QProcessEnvironment::systemEnvironment();
            procEnv.insert("QT_QPA_PLATFORM", "wayland");
            procTurnOn->setProcessEnvironment(procEnv);
            procTurnOn->start("kscreen-doctor", { "--dpms", "on" });
        } else {
            procTurnOn->start("xset", { "dpms", "force", "on" });
        }
        m_sessionWatcher->simulateSessionActive();
        qDebug() << "The display is turned on" << qgetenv("XDG_SESSION_TYPE");
    } else {
        qDebug() << "session not in idle";
    }
}

void DbusUpperInterface::onDelayInit()
{
    syslog(LOG_INFO, "[ukui-screensaver-backend] onDelayInit!!");
    m_upowerHelper = QSharedPointer<UpowerHelper>(new UpowerHelper(this));
    if (m_upowerHelper) {
        m_connections.append(connect(
            m_upowerHelper.get(), &UpowerHelper::lidStateChanged, this, &DbusUpperInterface::onLidStateChanged));
        m_upowerHelper->init();
        // 电池信息补连，connectUpowerBatterySignals 可能先于 onDelayInit 被调用，
        // 当时 m_upowerHelper 尚未初始化，信号无法绑定，这里检查并重新连接
        if (m_pendingBatteryConnect) {
            m_pendingBatteryConnect = false;
            connectUpowerBatterySignals();
        }
    }
    if (!isGreeterMode()) {
        m_sessionWatcher = QSharedPointer<SessionWatcher>(new SessionWatcher(m_gsettingsHelper, this));
    }
    if (m_sessionWatcher) {
        m_connections.append(connect(
            m_sessionWatcher.get(), &SessionWatcher::sessionIdle, this, &DbusUpperInterface::onSessionIdleReceived));
        m_connections.append(connect(m_sessionWatcher.get(), &SessionWatcher::sessionIdle, this, [=]() {
            QDBusMessage message;
            message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
            message << "SessionIdle";
            QDBusConnection::sessionBus().send(message);
        }));
        m_connections.append(connect(
            m_sessionWatcher.get(), &SessionWatcher::sessionIdleExit, this, &DbusUpperInterface::onSessionIdleExit));
    }
    m_networkManagerHelper = QSharedPointer<NetWorkManagerHelper>(new NetWorkManagerHelper(this));
    m_fileManagerHelper = QSharedPointer<FileManagerHelper>(new FileManagerHelper(this));
    m_systemsUpgradeHelper = QSharedPointer<SystemUpgradeHelper>(new SystemUpgradeHelper(this));
    if (!isGreeterMode()) {
        m_config->initShareConfig();
        m_config->setShareConfigValue("timeType", m_gsettingsHelper->GetUkccPluginsConf(KEY_HOUR_SYSTEM).toInt());
        m_config->setShareConfigValue("dateType", m_gsettingsHelper->GetUkccPluginsConf(KEY_DATE).toString());
        m_config->setShareConfigValue(
            "fontSize", m_gsettingsHelper->GetThemeStyleConf(KEY_SYSTEM_FONT_SIZE).toDouble());
        m_config->setShareConfigValue("themeColor", m_gsettingsHelper->GetThemeStyleConf(KEY_THEME_COLOR).toString());
    }
    syslog(LOG_INFO, "[ukui-screensaver-backend] onDelayInit end!!");
}

void DbusUpperInterface::onWindowAdded(QString winId, QString appId)
{
    if (winId.isEmpty()) {
        return;
    }

    m_windowList.insert(winId, appId);
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_SESSION_WINDOWLIST_CHANGED;
    retObj["operation"] = QString("add");
    retObj["winId"] = winId;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onWindowRemoved(QString winId)
{
    if (winId.isEmpty() || m_windowList.isEmpty()) {
        return;
    }
    if (m_windowList.contains(winId)) {
        m_windowList.take(winId);
    }
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_SESSION_WINDOWLIST_CHANGED;
    retObj["operation"] = QString("remove");
    retObj["winId"] = winId;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

QDBusUnixFileDescriptor DbusUpperInterface::PamAuthenticateByName(const QString &username)
{
    if (!m_pamSessionServer) {
        m_pamSessionServer = new PamAuthSessionServer(m_pamAuth.get(), this);
        if (!m_pamSessionServer) {
            return QDBusUnixFileDescriptor();
        }
    }
    return m_pamSessionServer->startSession(username);
}

bool DbusUpperInterface::blockAllShortcuts(bool isBlocked)
{
    if (isBlocked) {
        // usd shortcut
        if (!m_usdBlockShortCut) {
            m_usdBlockShortCut = new USDBlockShortCut(this);
            qInfo() << "init usd blockAllShortcut !!! ";
        }
    } else {
        // usd shortcut
        if (m_usdBlockShortCut) {
            delete m_usdBlockShortCut;
            m_usdBlockShortCut = nullptr;
            qInfo() << "release usd blockAllShortcut !!! ";
        }
    }
    return true;
}

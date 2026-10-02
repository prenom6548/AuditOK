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
#include "login1helper.h"
#include "definetypes.h"
#include "configuration.h"
#include "gsettingshelper.h"
#include "usdhelper.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextCodec>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include "qtlocalpeer.h"
#include "switchuserutils.h"
#include "pamauthenticate.h"
#include "upowerhelper.h"
#include "accountshelper.h"
#include "sessionhelper.h"
#include "systemupgradehelper.h"
#include "sessionwatcher.h"
#include "freedesktophelper.h"
#include "bioauthenticate.h"
#include "rsac.h"
#include "kglobalaccelhelper.h"
#include "personalizeddata.h"
#include "libinputswitchevent.h"

#define CONFIG_FILE "/usr/share/ukui-greeter/ukui-greeter.conf"

DbusUpperInterface::DbusUpperInterface(QObject *parent) : QObject(parent)
{
    initData();
    initConnections();
    inhibit();
}

void DbusUpperInterface::initData()
{
    m_bLockState = false;
    m_bSlpState = false;
    m_config = Configuration::instance(this);
    m_accountsHelper = new AccountsHelper(this);
    m_lightDmHelper = new LightDMHelper(m_accountsHelper, m_config, this);
    m_login1Helper = new Login1Helper(this);
    m_gsettingsHelper = new GSettingsHelper(this);
    m_bioAuth = new BioAuthenticate(this);
    m_pamAuth = new PamAuthenticate(m_lightDmHelper, this);
    m_usdHelper = new UsdHelper(this);
    m_upowerHelper = new UpowerHelper(this);
    m_sessionHelper = new SessionHelper(this);
    m_systemsUpgradeHelper = new SystemUpgradeHelper(this);
    m_sessionWatcher = new SessionWatcher(m_gsettingsHelper, this);
    m_kglobalHelper = new KglobalAccelHelper(this);
    m_libinputSwitchEvent = new LibinputSwitchEvent(this);

    m_config->initShareConfig();
    m_config->setShareConfigValue("timeType", m_gsettingsHelper->GetUkccPluginsConf(KEY_HOUR_SYSTEM).toInt());
    m_config->setShareConfigValue("dateType", m_gsettingsHelper->GetUkccPluginsConf(KEY_DATE).toString());
    m_config->setShareConfigValue("fontSize", m_gsettingsHelper->GetThemeStyleConf(KEY_SYSTEM_FONT_SIZE).toDouble());
    m_config->setShareConfigValue("themeColor", m_gsettingsHelper->GetThemeStyleConf(KEY_THEME_COLOR).toString());
}

void DbusUpperInterface::initConnections()
{
    if (m_lightDmHelper) {
        connect(m_lightDmHelper, &LightDMHelper::usersInfoChanged, this, &DbusUpperInterface::onUsersInfoChanged);
        connect(
            m_lightDmHelper, &LightDMHelper::currentSessionChanged, this, &DbusUpperInterface::onCurrentSessionChanged);
    }
    if (m_login1Helper) {
        connect(m_login1Helper, &Login1Helper::requestLock, this, &DbusUpperInterface::onLogin1ReqLock);
        connect(m_login1Helper, &Login1Helper::requestUnlock, this, &DbusUpperInterface::onLogin1ReqUnLock);
        connect(m_login1Helper, &Login1Helper::PrepareForSleep, this, &DbusUpperInterface::onLogin1PrepareForSleep);
        connect(
            m_login1Helper,
            &Login1Helper::sessionActiveChanged,
            this,
            &DbusUpperInterface::onLogin1SessionActiveChanged);
        connect(
            m_login1Helper, &Login1Helper::blockInhibitedChanged, this, &DbusUpperInterface::onBlockInhibitedChanged);
    }
    if (m_gsettingsHelper) {
        connect(
            m_gsettingsHelper,
            &GSettingsHelper::lockScreenConfigChanged,
            this,
            &DbusUpperInterface::onLockScreenConfigChanged);
        connect(
            m_gsettingsHelper,
            &GSettingsHelper::screenSaverConfigChanged,
            this,
            &DbusUpperInterface::onScreenSaverConfigChanged);
        connect(
            m_gsettingsHelper,
            &GSettingsHelper::powerManagerConfigChanged,
            this,
            &DbusUpperInterface::onPowerManagerConfigChanged);
        connect(
            m_gsettingsHelper, &GSettingsHelper::mateBgConfigChanged, this, &DbusUpperInterface::onMateBgConfigChanged);
        connect(
            m_gsettingsHelper,
            &GSettingsHelper::ukccPluginsConfigChanged,
            this,
            &DbusUpperInterface::onUkccPluginsConfigChanged);
        connect(
            m_gsettingsHelper,
            &GSettingsHelper::themeStyleConfigChanged,
            this,
            &DbusUpperInterface::onThemeStyleConfigChanged);
        connect(
            m_gsettingsHelper,
            &GSettingsHelper::sessionConfigChanged,
            this,
            &DbusUpperInterface::onSessionConfigChanged);
        connect(
            m_gsettingsHelper,
            &GSettingsHelper::keyboardConfigChanged,
            this,
            &DbusUpperInterface::onKeyboardConfigChanged);
        connect(
            m_gsettingsHelper,
            &GSettingsHelper::usdMediaKeysConfigChanged,
            this,
            &DbusUpperInterface::onUsdMediaKeysConfigChanged);
        connect(
            m_gsettingsHelper,
            &GSettingsHelper::usdMediaStateKeysConfigChanged,
            this,
            &DbusUpperInterface::onUsdMediaStateKeysConfigChanged);
    }
    if (m_bioAuth) {
        connect(m_bioAuth, &BioAuthenticate::bioAuthShowMessage, this, &DbusUpperInterface::onBioAuthShowMessage);
        connect(
            m_bioAuth, &BioAuthenticate::bioServiceStatusChanged, this, &DbusUpperInterface::onBioServiceStatusChanged);
        connect(m_bioAuth, &BioAuthenticate::bioDeviceChanged, this, &DbusUpperInterface::onBioDeviceChanged);
        connect(m_bioAuth, &BioAuthenticate::bioAuthStateChanged, this, &DbusUpperInterface::onBioAuthStateChanged);
        connect(m_bioAuth, &BioAuthenticate::bioAuthFrameData, this, &DbusUpperInterface::onBioAuthFrameData);
        connect(m_bioAuth, &BioAuthenticate::bioAuthCompleted, this, &DbusUpperInterface::onBioAuthCompleted);
        if (m_pamAuth) {
            connect(
                m_pamAuth, &PamAuthenticate::authenticationComplete, m_bioAuth, &BioAuthenticate::onPamAuthComplete);
        }
    }
    if (m_pamAuth) {
        connect(m_pamAuth, &PamAuthenticate::showMessage, this, &DbusUpperInterface::onPamShowMessage);
        connect(m_pamAuth, &PamAuthenticate::showPrompt, this, &DbusUpperInterface::onPamShowPrompt);
        connect(m_pamAuth, &PamAuthenticate::authenticationComplete, this, &DbusUpperInterface::onPamAuthCompleted);
    }
    if (m_upowerHelper) {
        connect(m_upowerHelper, &UpowerHelper::batteryStatusChanged, this, &DbusUpperInterface::onBatteryStatusChanged);
        connect(m_upowerHelper, &UpowerHelper::batteryChanged, this, &DbusUpperInterface::onBatteryChanged);
        connect(m_upowerHelper, &UpowerHelper::lidStateChanged, this, &DbusUpperInterface::onLidStateChanged);
    }
    if (m_sessionWatcher) {
        connect(m_sessionWatcher, &SessionWatcher::sessionIdle, this, &DbusUpperInterface::onSessionIdleReceived);
        connect(m_sessionWatcher, &SessionWatcher::sessionIdle, this, [=]() {
            QDBusMessage message;
            message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
            message << "SessionIdle";
            QDBusConnection::sessionBus().send(message);
        });
        connect(m_sessionWatcher, &SessionWatcher::sessionIdleExit, this, &DbusUpperInterface::onSessionIdleExit);
    }
    connect(
        &m_procLockDialog,
        SIGNAL(finished(int, QProcess::ExitStatus)),
        this,
        SLOT(onLockDialogProcExit(int, QProcess::ExitStatus)));
    if (m_libinputSwitchEvent) {
        connect(
            m_libinputSwitchEvent,
            &LibinputSwitchEvent::tabletModeStatusChanged,
            this,
            &DbusUpperInterface::onTabletModeChanged);
    }
}

void DbusUpperInterface::onNameLost(const QString &serviceName)
{
    if (serviceName == "cn.kylinos.ScreenSaver")
        exit(0);
}

bool DbusUpperInterface::GetSlpState()
{
    return ((m_procLockDialog.state() != QProcess::NotRunning) && m_bSlpState);
}

void DbusUpperInterface::LockStartupMode()
{
    if (GetLockState())
        return;
    if (m_procLockDialog.state() != QProcess::NotRunning) {
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
    if (m_procLockDialog.state() == QProcess::NotRunning) {
        return false;
    } else {
        return lockState;
    }
    return false;
}

void DbusUpperInterface::Lock()
{
    if (GetLockState())
        return;

    if (m_procLockDialog.state() != QProcess::NotRunning) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "CmdLock";
        QDBusConnection::sessionBus().send(message);
        emitLockState(true);
        return;
    }
    qDebug() << "Lock requested";
    m_bLockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --lock");
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(true);
}

void DbusUpperInterface::SwitchUser()
{
    if (GetLockState())
        return;

    if (m_procLockDialog.state() != QProcess::NotRunning) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "CmdLock";
        QDBusConnection::sessionBus().send(message);
        emitLockState(true);
        return;
    }
    qDebug() << "Lock requested";
    m_bLockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --switchuser");
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

    if (m_procLockDialog.state() != QProcess::NotRunning) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "CmdSessionTools";
        QDBusConnection::sessionBus().send(message);
        emitLockState(true);
        return;
    }
    m_bLockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --session-tools");
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(true);
}

void DbusUpperInterface::AppBlockWindow(QString actionType)
{
    QStringList lockcheck;
    if (actionType == "Shutdown" || actionType == "Restart" || actionType == "Logout") {
        lockcheck = m_sessionHelper->getLockCheckStatus("shutdown");
    } else if (actionType == "Suspend" || actionType == "Hibernate") {
        lockcheck = m_sessionHelper->getLockCheckStatus("sleep");
    }
    qDebug() << "lockCheck = " << lockcheck;
    if (lockcheck.isEmpty()) {
        return;
    }
    if (m_procLockDialog.state() != QProcess::NotRunning) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        if (actionType == "Restart")
            message << "CmdRestartAppBlock";
        else if (actionType == "Shutdown")
            message << "CmdPowerOffAppBlock";
        else if (actionType == "Suspend")
            message << "CmdSuspendAppBlock";
        else if (actionType == "Hibernate")
            message << "CmdHibernateAppBlock";
        else if (actionType == "Logout")
            message << "CmdLogoutAppBlock";
        else
            return;
        QDBusConnection::sessionBus().send(message);
        emitLockState(false);
        return;
    }
    if (actionType != "Restart" && actionType != "Shutdown" && actionType != "Suspend" && actionType != "Hibernate"
        && actionType != "Logout")
        return;
    m_bLockState = false;
    QString cmd;
    cmd = QString("/usr/bin/ukui-screensaver-dialog --app-block %1").arg(actionType);
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(false);
}

void DbusUpperInterface::MultiUserBlockWindow(QString actionType)
{
    if (m_procLockDialog.state() != QProcess::NotRunning) {
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        if (actionType == "Restart")
            message << "CmdRestartMulTiUserBlock";
        else if (actionType == "Shutdown")
            message << "CmdPowerOffMulTiUserBlock";
        else
            return;
        QDBusConnection::sessionBus().send(message);
        emitLockState(false);
        return;
    }
    if (actionType != "Restart" && actionType != "Shutdown")
        return;
    m_bLockState = false;
    QString cmd;
    cmd = QString("/usr/bin/ukui-screensaver-dialog --multiusers-block %1").arg(actionType);
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(false);
}

void DbusUpperInterface::Suspend()
{
    if (m_sessionHelper) {
        QStringList lockCheckList = m_sessionHelper->getLockCheckStatus("sleep");
        if (lockCheckList.count() == 0)
            m_sessionHelper->doAction("Suspend");
        else {
            AppBlockWindow("Suspend");
        }
    }
}

void DbusUpperInterface::Hibernate()
{
    if (m_sessionHelper) {
        QStringList lockCheckList = m_sessionHelper->getLockCheckStatus("sleep");
        if (lockCheckList.count() == 0)
            m_sessionHelper->doAction("Hibernate");
        else {
            AppBlockWindow("Hibernate");
        }
    }
}

void DbusUpperInterface::Reboot()
{
    if (m_sessionHelper) {
        QStringList lockCheckList = m_sessionHelper->getLockCheckStatus("shutdown");
        if (lockCheckList.count() == 0) {
            if (m_lightDmHelper && m_lightDmHelper->getLoginUserCount() > 1) {
                MultiUserBlockWindow("Restart");
            } else {
                m_sessionHelper->doAction("Reboot");
            }
        } else {
            AppBlockWindow("Restart");
        }
    }
}

void DbusUpperInterface::PowerOff()
{
    if (m_sessionHelper) {
        QStringList lockCheckList = m_sessionHelper->getLockCheckStatus("shutdown");
        if (lockCheckList.count() == 0) {
            if (m_lightDmHelper && m_lightDmHelper->getLoginUserCount() > 1) {
                MultiUserBlockWindow("Shutdown");
            } else {
                m_sessionHelper->doAction("PowerOff");
            }
        } else {
            AppBlockWindow("Shutdown");
        }
    }
}

void DbusUpperInterface::Logout()
{
    if (m_sessionHelper) {
        QStringList lockCheckList = m_sessionHelper->getLockCheckStatus("shutdown");
        if (lockCheckList.count() == 0)
            m_sessionHelper->doAction("Logout");
        else {
            AppBlockWindow("Logout");
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
        lockState = false;
        m_bSlpState = false;
        if (m_gsettingsHelper->GetLockScreenConf(KEY_LOCK_TIMEOUT).toInt() != -1 && isNeedLock) {
            // 空闲关屏且关屏时间与睡眠时间一致时直接锁定
            if (nType == 0
                && ((m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_DISPLAY_AC).toInt()
                     == m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_COMPUTER_AC).toInt())
                    && m_gsettingsHelper->GetPowerManagerConf(KEY_SLEEP_COMPUTER_AC).toInt() != -1)) {
                qDebug() << "close delay same with sleep delay then lock!";
                if (m_procLockDialog.state() != QProcess::NotRunning) {
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
            if (m_procLockDialog.state() != QProcess::NotRunning) {
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

void DbusUpperInterface::onShowBlankScreensaver()
{
    if (GetLockState())
        return;

    if (m_procLockDialog.state() != QProcess::NotRunning) {
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
    if (m_procLockDialog.state() != QProcess::NotRunning) {
        if (!lockState) {
            emitLockState(true);
        }
        QDBusMessage message;
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
        message << "Screensaver";
        QDBusConnection::sessionBus().send(message);
        return;
    }

    qDebug() << "only show screensaver";
    lockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --screensaver");
    qDebug() << cmd;

    m_procLockDialog.start(cmd);
    emitLockState(true);
}

void DbusUpperInterface::LockScreensaver()
{
    if (m_procLockDialog.state() != QProcess::NotRunning) {
        if (!lockState) {
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
    QString id = QString("ukui-screensaver-dialog" + QLatin1String(getenv("DISPLAY")));
    QtLocalPeer *lPeer = new QtLocalPeer(this, id);
    isRunning = lPeer->isClient();
    delete lPeer;
    return isRunning;
}

void DbusUpperInterface::emitLockState(bool val)
{
    qDebug() << "emitLockState state = " << val;

    if (val != m_bBlockShortcutState) {
        m_kglobalHelper->blockShortcut(val);
        m_bBlockShortcutState = val;
        qDebug() << " block all shortcut " << "blockShortcut states = " << m_bBlockShortcutState;
    }

    QDBusMessage message;
    if (val) {
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "lock");
    } else {
        message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "unlock");
    }
    QDBusConnection::sessionBus().send(message);
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
                    if (m_systemsUpgradeHelper)
                        retObj["Content"] = m_systemsUpgradeHelper->checkSystemUpgrade();
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
                case LOCK_CMD_ID_SESSION_GET_SLEEP_LOCKCHECK:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getSleepLockCheck();
                    break;
                case LOCK_CMD_ID_SESSION_GET_SHUTDOWN_LOCKCHECK:
                    retObj["CmdId"] = cmdId;
                    retObj["Content"] = getShutdownLockcheck();
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
            KylinUserDatePtr ptr = KYLINUSERDATAMNG::getInstance()->GetUserPtr(userInfo->name());
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
        QStringList lockCheckList = m_sessionHelper->getLockCheckStatus("sleep");
        for (auto lockcheck : lockCheckList) {
            QJsonObject objBatteryInfo;
            objBatteryInfo["key"] = lockcheck;
            jsonArray.append(objBatteryInfo);
        }
    }
    return jsonArray;
}

QJsonArray DbusUpperInterface::getShutdownLockcheck()
{
    QJsonArray jsonArray;
    if (m_sessionHelper) {
        QStringList lockCheckList = m_sessionHelper->getLockCheckStatus("shutdown");
        for (auto lockcheck : lockCheckList) {
            QJsonObject objBatteryInfo;
            objBatteryInfo["key"] = lockcheck;
            jsonArray.append(objBatteryInfo);
        }
    }
    return jsonArray;
}

QJsonArray DbusUpperInterface::getSaverTheme()
{
    QJsonArray jsonArray;
    if (m_sessionHelper) {
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
        QString selectedUser = m_lightDmHelper->selectUserHint();
        if (!selectedUser.isEmpty()) {
            qDebug() << "SelectUserHint: " << selectedUser;
            if (m_lightDmHelper->findUserByName(selectedUser)) {
                strUserName = selectedUser;
            } else {
                qDebug() << "SlelectUerHint invalid:" << selectedUser;
            }
        }
        if (!strUserName.isEmpty()) {
            // do nothing
        } else if (m_lightDmHelper->selectGuestHint()) {
            qDebug() << "SelectGuest";
            strUserName = "*guest";
        } else if (m_lightDmHelper->getUsersInfo().count() == 1) { // 如果只有一个用户,默认选中
            strUserName = m_lightDmHelper->getUsersInfo()[0]->name();
        } else {
            QString lastLoginUser = m_config->getLastLoginUser();
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
    if (m_libinputSwitchEvent) {
        tabletMode = m_libinputSwitchEvent->geInitDevicesStatus();
    }
    return tabletMode;
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
    Lock();
}

void DbusUpperInterface::onLogin1ReqUnLock()
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_LOGIN1_REQ_UNLOCK;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onLogin1PrepareForSleep(bool isSleep)
{
    qDebug() << "onLogin1PrepareForSleep:" << isSleep << "--------";
    if (isSleep) {
        if (!checkStatus(SESSION_STATUS_SLEEPED)) {
            uninhibit();
            return;
        }

        m_bBlankState = true;
        if (checkScreenDialogRunning() || GetLockState() || GetSlpState()) {
            QDBusMessage message;
            message = QDBusMessage::createSignal(SS_DBUS_PATH, SS_DBUS_INTERFACE, "SecondRunParam");
            message << "SleepLock";
            QDBusConnection::sessionBus().send(message);
        } else {
            this->onShowBlankScreensaver();
        }

        if (!m_timer) {
            m_timer = new QTimer(this);
            qDebug() << "m_bLockState " << m_bLockState;
            connect(m_timer, &QTimer::timeout, this, [&] {
                m_timerCount += 1;
                if (m_bLockState || m_timerCount > 20) {
                    m_timer->stop();
                    m_timerCount = 0;
                    uninhibit();
                }
            });
        }
        m_timer->start(200);
    } else {
        m_nStatus &= ~SESSION_STATUS_SLEEPED;
        m_bBlankState = false;
        inhibit();
    }
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

void DbusUpperInterface::onBlockInhibitedChanged(QString blockInhibited)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_LOGIN1_BLOCKINHIBITED_CHANGED;
    retObj["BlockInhibited"] = blockInhibited;
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

    qDebug() << "emit SessionIdle";
    lockState = false;
    QString cmd = QString("/usr/bin/ukui-screensaver-dialog --session-idle");
    qDebug() << cmd;
    m_procLockDialog.start(cmd);
    emitLockState(true);
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
        connect(m_timerLock, &QTimer::timeout, this, &DbusUpperInterface::onLockScreenTimeout);
    }
    int nLockTimeout = m_gsettingsHelper->GetLockScreenConf(KEY_LOCK_TIMEOUT).toInt();
    qDebug() << "LockTimeout:" << nLockTimeout;
    if (nLockTimeout != -1) {
        stopDelayLockScreen();
        if (m_procLockDialog.state() != QProcess::NotRunning) {
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

void DbusUpperInterface::stopDelayLockScreen()
{
    if (m_timerLock && m_timerLock->isActive()) {
        UnLock();
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
    if (exitStatus == QProcess::CrashExit)
        qWarning() << m_procLockDialog.errorString();
    emitLockState(false);
    lockState = false;
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

int DbusUpperInterface::PamAuthenticateCancel(const QJsonObject &objInfo)
{
    Q_UNUSED(objInfo);
    int nRet = -1;
    if (m_pamAuth) {
        m_pamAuth->cancelAuthentication();
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
        if (!priKey.isEmpty()) {
            QByteArray buffer;
            QByteArray byteEncrypt = QByteArray::fromBase64(strDecrypt.toLatin1());
            if (rsac.decrypt(byteEncrypt, buffer, priKey)) {
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
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_PAMAUTH_SHOWMESSAGE;
    retObj["Message"] = strMsg;
    retObj["MsgType"] = type;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onPamShowPrompt(QString strPrompt, int type)
{
    QJsonObject retObj;
    retObj["CmdId"] = LOCK_CMD_ID_PAMAUTH_SHOWPROMPT;
    retObj["Prompt"] = strPrompt;
    retObj["PromptType"] = type;
    SendUpdateInfoSig(QString(QJsonDocument(retObj).toJson()));
}

void DbusUpperInterface::onPamAuthCompleted()
{
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

void DbusUpperInterface::BioGetAvailableDevices(QJsonObject &reqObj, QJsonObject &retObj)
{
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
}

void DbusUpperInterface::BioGetDisabledDevices(QJsonObject &reqObj, QJsonObject &retObj)
{
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
}

void DbusUpperInterface::BioGetBioAuthState(QJsonObject &reqObj, QJsonObject &retObj)
{
    Q_UNUSED(reqObj);
    if (m_bioAuth) {
        retObj["Content"] = m_bioAuth->getBioAuthState();
    } else {
        retObj["Content"] = BIOAUTH_IDLE;
    }
}

void DbusUpperInterface::BioGetCurBioInfo(QJsonObject &reqObj, QJsonObject &retObj)
{
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
}

void DbusUpperInterface::BioGetDefaultDevice(QJsonObject &reqObj, QJsonObject &retObj)
{
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
}

void DbusUpperInterface::BioFindDeviceById(QJsonObject &reqObj, QJsonObject &retObj)
{
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
}

QString DbusUpperInterface::getPublicEncrypt()
{
    if (pubKey.isEmpty()) {
        rsac.generateKeyPair(priKey, pubKey, 2048);
    }
    return QString(pubKey);
}

bool DbusUpperInterface::sendPassword(const QString username, QByteArray password)
{
    if (priKey.isEmpty())
        return false;

    QByteArray decryptText;
    rsac.decrypt(password, decryptText, priKey); // 解密

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

bool DbusUpperInterface::usdExternalDoAction(const QJsonObject &objInfo)
{
    int nRet = -1;
    int actionType = objInfo.value("Content").toInt();
    if (m_usdHelper->usdExternalDoAction(actionType)) {
        nRet = 0;
    }
    return nRet;
}

bool DbusUpperInterface::blockShortcut(const QJsonObject &objInfo)
{
    int nRet = -1;
    bool isBlock = objInfo.value("Content").toBool();
    if (m_kglobalHelper->blockShortcut(isBlock)) {
        nRet = 0;
    }
    return nRet;
}

bool DbusUpperInterface::lockStateChanged(const QJsonObject &objInfo)
{
    int nRet = -1;
    bool state = objInfo.value("Content").toBool();
    if (lockState != state) {
        lockState = state;
        emitLockState(state);
    }
    return lockState;
}

int DbusUpperInterface::SetPowerManager(const QJsonObject &objInfo)
{
    int nRet = -1;
    QString strFuncName = objInfo.value("Content").toString();
    if (!strFuncName.isEmpty()) {
        qWarning() << __LINE__ << __FUNCTION__ << strFuncName;
        if (strFuncName != "UpgradeThenRestart" && strFuncName != "UpgradeThenShutdown" && m_sessionHelper)
            if (!isGreeterMode()) {
                m_sessionHelper->doAction(strFuncName);
            } else if (m_login1Helper) {
                m_login1Helper->setPowerManager(strFuncName);
            } else if (strFuncName == "UpgradeThenRestart" && m_systemsUpgradeHelper) {
                m_systemsUpgradeHelper->doUpgradeThenRboot();
            } else if (strFuncName == "UpgradeThenShutdown" && m_systemsUpgradeHelper) {
                m_systemsUpgradeHelper->doUpgradeThenShutdown();
            }
        nRet = 0;
    }

    return nRet;
}

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
#include "lockdialogperformer.h"
#include "lockdialogmodel.h"
#include "backenddbushelper.h"
#include "../common/plasma-shell-manager.h"
#include "utils.h"
#include "rsac.h"

LockDialogPerformer::LockDialogPerformer(LockDialogModel *model, QObject *parent)
    : QObject(parent), m_modelLockDialog(model)
{
    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    QString sessionDbus = QString("%1%2").arg(QString(SS_DBUS_SERVICE)).arg(displayNum);
    m_bdHelper = new BackendDbusHelper(sessionDbus, SS_DBUS_PATH, QDBusConnection::sessionBus(), this);
    if (!m_bdHelper->isValid()) {
        delete m_bdHelper;
        m_bdHelper = new BackendDbusHelper(SS_DBUS_SERVICE, SS_DBUS_PATH, QDBusConnection::sessionBus(), this);
    }
    initConnections();
    initData();
}

void LockDialogPerformer::initConnections()
{
    connect(m_bdHelper, &BackendDbusHelper::requestLock, m_modelLockDialog, &LockDialogModel::requestLockSession);
    connect(m_bdHelper, &BackendDbusHelper::requestUnlock, m_modelLockDialog, &LockDialogModel::requestUnlockSession);
    connect(m_bdHelper, &BackendDbusHelper::PrepareForSleep, m_modelLockDialog, &LockDialogModel::prepareForSleep);
    connect(
        m_bdHelper,
        &BackendDbusHelper::sessionActiveChanged,
        m_modelLockDialog,
        &LockDialogModel::onSessionActiveChanged);

    connect(m_bdHelper, &BackendDbusHelper::usersInfoChanged, m_modelLockDialog, &LockDialogModel::onUsersInfoChanged);
    connect(m_bdHelper, &BackendDbusHelper::currentUserChanged, m_modelLockDialog, &LockDialogModel::onCurUserChanged);
    connect(
        m_bdHelper,
        &BackendDbusHelper::currentSessionChanged,
        m_modelLockDialog,
        &LockDialogModel::onCurSessionChanged);

    connect(m_bdHelper, &BackendDbusHelper::blockInhibitedChanged, this, [=](const QString &blockInhibited) {
        m_modelLockDialog->updateSleepLockcheck(m_bdHelper->getSleepLockcheck());
        m_modelLockDialog->updateShutdownLockcheck(m_bdHelper->getShutdownLockcheck());
    });
    if (!m_modelLockDialog->isUseWayland()) {
        connect(
            m_bdHelper,
            &BackendDbusHelper::capslockConfChanged,
            m_modelLockDialog,
            &LockDialogModel::onCapslockStateChanged);
    }

    connect(
        m_bdHelper,
        &BackendDbusHelper::batteryStatusChanged,
        m_modelLockDialog,
        &LockDialogModel::onBatteryStatusChanged);
    connect(m_bdHelper, &BackendDbusHelper::batteryChanged, m_modelLockDialog, &LockDialogModel::batteryChanged);
    connect(m_bdHelper, &BackendDbusHelper::SecondRunParam, m_modelLockDialog, &LockDialogModel::onSecondRunParam);
    connect(
        m_bdHelper,
        &BackendDbusHelper::lockScreenConfChanged,
        m_modelLockDialog,
        &LockDialogModel::onLockScreenConfChanged);
    connect(
        m_bdHelper,
        &BackendDbusHelper::themeStyleConfChanged,
        m_modelLockDialog,
        &LockDialogModel::onThemeStyleConfChanged);
    connect(
        m_bdHelper, &BackendDbusHelper::sessionConfChanged, m_modelLockDialog, &LockDialogModel::onSessionConfChanged);

    // pam signals
    connect(m_bdHelper, SIGNAL(pamShowMessage(QString, int)), m_modelLockDialog, SIGNAL(pamShowMessage(QString, int)));
    connect(m_bdHelper, SIGNAL(pamShowPrompt(QString, int)), m_modelLockDialog, SIGNAL(pamShowPrompt(QString, int)));
    connect(m_bdHelper, SIGNAL(pamAuthCompleted()), m_modelLockDialog, SIGNAL(pamAuthCompleted()));

    // bio signals
    connect(
        m_bdHelper, SIGNAL(bioServiceStatusChanged(bool)), m_modelLockDialog, SIGNAL(bioServiceStatusChanged(bool)));
    connect(m_bdHelper, SIGNAL(bioDeviceChanged()), m_modelLockDialog, SIGNAL(bioDeviceChanged()));
    connect(m_bdHelper, SIGNAL(bioAuthShowMessage(QString)), m_modelLockDialog, SIGNAL(bioAuthShowMessage(QString)));
    connect(m_bdHelper, SIGNAL(bioAuthStateChanged(int)), m_modelLockDialog, SIGNAL(bioAuthStateChanged(int)));
    connect(m_bdHelper, SIGNAL(bioAuthFrameData(QString)), m_modelLockDialog, SIGNAL(bioAuthFrameData(QString)));
    connect(
        m_bdHelper,
        SIGNAL(bioAuthCompleted(int, bool, int, int, int)),
        m_modelLockDialog,
        SIGNAL(bioAuthCompleted(int, bool, int, int, int)));

    connect(
        m_bdHelper,
        &BackendDbusHelper::usdMediaKeysConfChanged,
        m_modelLockDialog,
        &LockDialogModel::onUsdMediaKeysChanged);
    connect(
        m_bdHelper,
        &BackendDbusHelper::usdMediaStateKeysConfChanged,
        m_modelLockDialog,
        &LockDialogModel::onUsdMediaStateKeysChanged);
    connect(
        m_bdHelper, &BackendDbusHelper::tabletModeChanged, m_modelLockDialog, &LockDialogModel::onTabletModeChanged);
    /// 通过信号直接调用槽
    connect(m_modelLockDialog, &LockDialogModel::setCurrentUser, m_bdHelper, &BackendDbusHelper::setCurrentUser);
    connect(m_modelLockDialog, &LockDialogModel::switchToUser, m_bdHelper, &BackendDbusHelper::switchToUser);
    connect(m_modelLockDialog, &LockDialogModel::setCurrentSession, m_bdHelper, &BackendDbusHelper::setCurrentSession);
    connect(m_modelLockDialog, &LockDialogModel::startSession, m_bdHelper, &BackendDbusHelper::startSession);
    connect(m_modelLockDialog, &LockDialogModel::pamAuthenticate, m_bdHelper, &BackendDbusHelper::pamAuthenticate);
    connect(m_modelLockDialog, &LockDialogModel::pamRespond, m_bdHelper, [=](const QString &strResponse) {
        QString strEncrypt = strResponse;
        QString strPublicKey = m_bdHelper->getPublicEncrypt();
        if (!strPublicKey.isEmpty()) {
            QByteArray buffer;
            RSAC rsac;
            if (rsac.encrypt(strEncrypt.toLatin1(), buffer, strPublicKey.toLatin1())) {
                strEncrypt = buffer.toBase64();
            }
        }
        m_bdHelper->pamRespond(strEncrypt);
    });
    connect(
        m_modelLockDialog,
        &LockDialogModel::pamAuthenticateCancel,
        m_bdHelper,
        &BackendDbusHelper::pamAuthenticateCancel);
    connect(
        m_modelLockDialog,
        &LockDialogModel::pamIsInAuthentication,
        m_bdHelper,
        &BackendDbusHelper::pamIsInAuthentication);
    connect(
        m_modelLockDialog, &LockDialogModel::pamIsAuthenticated, m_bdHelper, &BackendDbusHelper::pamIsAuthenticated);
    connect(
        m_modelLockDialog, &LockDialogModel::pamAuthenticateUser, m_bdHelper, &BackendDbusHelper::pamAuthenticateUser);
    connect(m_modelLockDialog, &LockDialogModel::GetBlankState, m_bdHelper, &BackendDbusHelper::GetBlankState);

    connect(m_modelLockDialog, &LockDialogModel::bioStartAuth, m_bdHelper, &BackendDbusHelper::bioStartAuth);
    connect(m_modelLockDialog, &LockDialogModel::bioStopAuth, m_bdHelper, &BackendDbusHelper::bioStopAuth);
    connect(
        m_modelLockDialog,
        &LockDialogModel::bioGetAvailableDevices,
        m_bdHelper,
        &BackendDbusHelper::bioGetAvailableDevices);
    connect(
        m_modelLockDialog,
        &LockDialogModel::bioGetDisabledDevices,
        m_bdHelper,
        &BackendDbusHelper::bioGetDisabledDevices);
    connect(
        m_modelLockDialog, &LockDialogModel::bioGetBioAuthState, m_bdHelper, &BackendDbusHelper::bioGetBioAuthState);
    connect(m_modelLockDialog, &LockDialogModel::bioGetCurBioInfo, m_bdHelper, &BackendDbusHelper::bioGetCurBioInfo);
    connect(m_modelLockDialog, &LockDialogModel::bioFindDeviceById, m_bdHelper, &BackendDbusHelper::bioFindDeviceById);
    connect(
        m_modelLockDialog, &LockDialogModel::bioFindDeviceByName, m_bdHelper, &BackendDbusHelper::bioFindDeviceByName);
    connect(
        m_modelLockDialog, &LockDialogModel::bioGetDefaultDevice, m_bdHelper, &BackendDbusHelper::bioGetDefaultDevice);

    connect(
        m_modelLockDialog, &LockDialogModel::usdExternalDoAction, m_bdHelper, &BackendDbusHelper::usdExternalDoAction);
    connect(m_modelLockDialog, &LockDialogModel::setPowerManager, m_bdHelper, &BackendDbusHelper::setPowerManager);
    connect(m_modelLockDialog, &LockDialogModel::lockStateChanged, m_bdHelper, &BackendDbusHelper::lockStateChanged);
    connect(m_modelLockDialog, &LockDialogModel::getPublicEncrypt, m_bdHelper, &BackendDbusHelper::getPublicEncrypt);
    connect(m_modelLockDialog, &LockDialogModel::sendPassword, m_bdHelper, &BackendDbusHelper::sendPassword);
    connect(m_modelLockDialog, &LockDialogModel::setUserThemeColor, m_bdHelper, &BackendDbusHelper::setThemeStyleConf);
    connect(m_modelLockDialog, &LockDialogModel::setRfkillState, m_bdHelper, &BackendDbusHelper::setUsdMediaStateKeys);
}

void LockDialogPerformer::initData()
{
    m_modelLockDialog->updateUsersInfo(m_bdHelper->getUsersInfo());
    m_modelLockDialog->updateSessionsInfo(m_bdHelper->getSessionsInfo());
    m_modelLockDialog->updateSessionState(m_bdHelper->isSessionActive());
    m_modelLockDialog->updateDefUserName(m_bdHelper->getDefaultAuthUser());
    m_modelLockDialog->updateCurUserName(m_bdHelper->getCurrentUser());
    m_modelLockDialog->updateCurSession(m_bdHelper->getCurrentSession());
    m_modelLockDialog->updateAgreementInfo(m_bdHelper->getAgreementInfo());
    m_modelLockDialog->updateCapslockState(m_bdHelper->getKeyboardConf(KEY_CAPSLOCK_STATUS).toBool());
    m_modelLockDialog->updateUsdMediaKeys(
        KEY_AREA_SCREENSHOT, m_bdHelper->getUsdMediaKeys(KEY_AREA_SCREENSHOT).toString());
    m_modelLockDialog->updateUsdMediaKeys(
        KEY_AREA_SCREENSHOT2, m_bdHelper->getUsdMediaKeys(KEY_AREA_SCREENSHOT2).toString());
    m_modelLockDialog->updateUsdMediaKeys(KEY_SCREEN_SHOT, m_bdHelper->getUsdMediaKeys(KEY_SCREEN_SHOT).toString());
    m_modelLockDialog->updateUsdMediaKeys(KEY_SCREEN_SHOT2, m_bdHelper->getUsdMediaKeys(KEY_SCREEN_SHOT2).toString());
    m_modelLockDialog->updateUsdMediaKeys(
        KEY_WINDOW_SCREENSHOT, m_bdHelper->getUsdMediaKeys(KEY_WINDOW_SCREENSHOT).toString());
#ifdef USDFIXED
    m_modelLockDialog->updateUsdMediaStateKeys(
        KEY_RFKILL_STATE, m_bdHelper->getUsdMediaStateKeys(KEY_RFKILL_STATE).toInt());
#endif
    m_modelLockDialog->updateScreensaverMode(m_bdHelper->getLockScreenConf(KEY_MODE).toString());
    m_modelLockDialog->updateScreensaverTheme(m_bdHelper->getSaverThemes());
    m_modelLockDialog->updateScreensaverImageTSEffect(
        m_bdHelper->getLockScreenConf(KEY_IMAGE_TRANSITION_EFFECT).toInt());
    m_modelLockDialog->updateScreensaverImageSwitchInterval(
        m_bdHelper->getLockScreenConf(KEY_IMAGE_SWITCH_INTERVAL).toInt());
    m_modelLockDialog->updateCanSwitchUser(m_bdHelper->getPowerManagerCanSwitchUser());
    m_modelLockDialog->updateCanHibernate(m_bdHelper->getPowerManagerCanHibernate());
    m_modelLockDialog->updateCanReboot(m_bdHelper->getPowerManagerCanReboot());
    m_modelLockDialog->updateCanPowerOff(m_bdHelper->getPowerManagerCanPowerOff());
    m_modelLockDialog->updateCanSuspend(m_bdHelper->getPowerManagerCanSuspend());
    m_modelLockDialog->updateCanLockScreen(m_bdHelper->getPowerManagerCanLockScreen());
    m_modelLockDialog->updateCanLogout(m_bdHelper->getPowerManagerCanLogout());
    m_modelLockDialog->updataCheckSystemUpgrade(m_bdHelper->checkSystemUpgrade());
    m_modelLockDialog->updateBatteryArgs(m_bdHelper->getBatteryArgs());
    m_modelLockDialog->updateBatteryIconName(m_bdHelper->getBatteryIconName());
    m_modelLockDialog->updateIsBattery(m_bdHelper->getIsBattery());
    m_modelLockDialog->updateSleepLockcheck(m_bdHelper->getSleepLockcheck());
    m_modelLockDialog->updateShutdownLockcheck(m_bdHelper->getShutdownLockcheck());
    m_modelLockDialog->updateLoggedInUsersCount();
    m_modelLockDialog->updateLockTimeout(m_bdHelper->getLockScreenConf(KEY_LOCK_TIMEOUT).toInt());
    m_modelLockDialog->updateLockEnabled(m_bdHelper->getLockScreenConf(KEY_LOCK_ENABLED).toBool());
    m_modelLockDialog->updateSleepActivationEnabled(
        m_bdHelper->getLockScreenConf(KEY_SLEEP_ACTIVATION_ENABLED).toBool());
    m_modelLockDialog->updateSystemFontSize(m_bdHelper->getThemeStyleConf(KEY_SYSTEM_FONT_SIZE).toDouble());
    m_modelLockDialog->updateSystemFont(m_bdHelper->getThemeStyleConf(KEY_SYSTEM_FONT).toString());
    m_modelLockDialog->updateTabletMode(m_bdHelper->getCurTabletMode());
    m_modelLockDialog->updateSessionLogoutMusic(m_bdHelper->getSessionConf(KEY_SESSION_LOGOUT_MUSIC).toBool());
    m_modelLockDialog->updateSessionPoweroffMusic(m_bdHelper->getSessionConf(KEY_SESSION_POWEROFF_MUSIC).toBool());
}

void LockDialogPerformer::initExtraData()
{
    if (m_modelLockDialog->isUseWayland()) {
        connect(
            PlasmaShellManager::getInstance(),
            &PlasmaShellManager::capslockStateChanged,
            m_modelLockDialog,
            &LockDialogModel::onCapslockStateChanged);
    }
}

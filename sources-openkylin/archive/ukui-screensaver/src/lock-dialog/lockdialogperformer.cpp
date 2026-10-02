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
#include "pamauthsessionclient.h"
#include "../common/plasma-shell-manager.h"
#include "utils.h"
#include "kyrsac.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <unistd.h>

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
    m_pamSession = new PamAuthSessionClient(this);
    initConnections();
    initData();
}

LockDialogPerformer::~LockDialogPerformer()
{
    if (m_pamSession) {
        m_pamSession->stop();
    }
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

    connect(m_bdHelper, &BackendDbusHelper::blockLogoutInhibitedChanged, this, [=]() {
        qDebug() << "update logoutcheck";
        m_modelLockDialog->updateLogoutLockcheck(m_bdHelper->getLogoutLockcheck());
    });
    connect(m_bdHelper, &BackendDbusHelper::blockInhibitedChanged, this, [=](const QString &blockInhibited) {
        m_modelLockDialog->updateSleepLockcheck(m_bdHelper->getSleepLockcheck());
        m_modelLockDialog->updateShutdownLockcheck(m_bdHelper->getShutdownLockcheck());
    });
    connect(m_bdHelper, &BackendDbusHelper::updateWindowList, this, [=](const QString &operation, const QString &winId) {
        m_modelLockDialog->updateAppWindows(m_bdHelper->getAppWindows());
        Q_EMIT m_modelLockDialog->windowChanged(operation, winId);
    });
    connect(m_bdHelper, &BackendDbusHelper::sessionLegacyChanged, this, [=]() {
        m_modelLockDialog->updateIsSessionLegacy(m_bdHelper->getIsSessionLegacy());
    });
    connect(
        m_modelLockDialog,
        &LockDialogModel::checkHideUpdate,
        m_bdHelper,
        &BackendDbusHelper::checkHideUpdate,
        Qt::DirectConnection);
    if (!m_modelLockDialog->isUseWayland()) {
        connect(
            m_bdHelper,
            &BackendDbusHelper::capslockConfChanged,
            m_modelLockDialog,
            &LockDialogModel::onCapslockStateChanged);
    }

    connect(m_bdHelper, &BackendDbusHelper::upowerServiceReady, m_modelLockDialog, [=]() {
        m_modelLockDialog->updateBatteryArgs(m_bdHelper->getBatteryArgs());
        m_modelLockDialog->updateBatteryIconName(m_bdHelper->getBatteryIconName());
        m_modelLockDialog->updateIsBattery(m_bdHelper->getIsBattery());
        Q_EMIT m_modelLockDialog->upowerServiceReady();
    });
    connect(
        m_bdHelper,
        &BackendDbusHelper::batteryStatusChanged,
        m_modelLockDialog,
        &LockDialogModel::onBatteryStatusChanged);
    connect(m_bdHelper, &BackendDbusHelper::batteryChanged, m_modelLockDialog, &LockDialogModel::onBatteryChanged);
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

    // pam prompt/message/completed 优先通过会话管道传递；当新接口失败回退旧认证时，仍走旧信号转发。
    m_pamSession->onShowMessage
        = [=](const QString &msg, int type) { Q_EMIT m_modelLockDialog->pamShowMessage(msg, type); };
    m_pamSession->onShowPrompt
        = [=](const QString &prompt, int type) { Q_EMIT m_modelLockDialog->pamShowPrompt(prompt, type); };
    m_pamSession->onShowBinaryPrompt
        = [=](const QVariant &val, int type) { Q_EMIT m_modelLockDialog->pamShowBinaryPrompt(val, type); };
    m_pamSession->onAuthCompleted = [=]() {
        m_pamSession->stop();
        Q_EMIT m_modelLockDialog->pamAuthCompleted();
    };
    connect(m_bdHelper, SIGNAL(pamShowMessage(QString, int)), m_modelLockDialog, SIGNAL(pamShowMessage(QString, int)));
    connect(m_bdHelper, SIGNAL(pamShowPrompt(QString, int)), m_modelLockDialog, SIGNAL(pamShowPrompt(QString, int)));
    connect(
        m_bdHelper,
        SIGNAL(pamShowBinaryPrompt(QVariant, int)),
        m_modelLockDialog,
        SIGNAL(pamShowBinaryPrompt(QVariant, int)));
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
        m_bdHelper,
        &BackendDbusHelper::powerManagerConfChanged,
        m_modelLockDialog,
        &LockDialogModel::onPowerManagerConfChanged);
    connect(
        m_bdHelper, &BackendDbusHelper::tabletModeChanged, m_modelLockDialog, &LockDialogModel::onTabletModeChanged);
    connect(
        m_bdHelper,
        &BackendDbusHelper::lockDatetimeEnableChanged,
        m_modelLockDialog,
        &LockDialogModel::onLockDatetimeEnableChanged);
    /// 通过信号直接调用槽
    connect(m_modelLockDialog, &LockDialogModel::setCurrentUser, m_bdHelper, &BackendDbusHelper::setCurrentUser);
    connect(m_modelLockDialog, &LockDialogModel::switchToUser, m_bdHelper, &BackendDbusHelper::switchToUser);
    connect(m_modelLockDialog, &LockDialogModel::setCurrentSession, m_bdHelper, &BackendDbusHelper::setCurrentSession);
    connect(m_modelLockDialog, &LockDialogModel::startSession, m_bdHelper, &BackendDbusHelper::startSession);
    connect(m_modelLockDialog, &LockDialogModel::pamAuthenticate, this, [=](const QString &strUserName) {
        m_pamSession->stop();
        qInfo() << "[PamAuthSessionClient] request pamAuthenticate user:" << strUserName;
        QDBusUnixFileDescriptor fd = m_bdHelper->pamAuthenticate(strUserName);
        if (fd.isValid()) {
            int localFd = dup(fd.fileDescriptor());
            m_pamSession->start(localFd);
        } else {
            qWarning() << "[PamAuthSessionClient] pamAuthenticate returned invalid fd";
        }
    });
    connect(m_modelLockDialog, &LockDialogModel::pamRespond, m_bdHelper, [=](const QString &strResponse) {
        if (m_pamSession && m_pamSession->isActive()) {
            if (m_pamSession->sendPacket(LOCK_CMD_ID_PAMAUTH_RESPOND, strResponse)) {
                return;
            }
        }
        QString strEncrypt = strResponse;
        if (strEncrypt != BIOMETRIC_IGNORE) {
            QString strPublicKey = m_bdHelper->getPublicEncrypt();
            if (!strPublicKey.isEmpty()) {
                QByteArray buffer;
                KyRSAC rsac;
                if (rsac.encryptData(strEncrypt.toLatin1(), buffer, strPublicKey.toLatin1())) {
                    strEncrypt = buffer.toBase64();
                }
            }
        }
        m_bdHelper->pamRespond(strEncrypt);
    });
    connect(m_modelLockDialog, &LockDialogModel::pamAuthenticateCancel, this, [=]() {
        if (m_pamSession && m_pamSession->isActive()) {
            m_pamSession->sendPacket(LOCK_CMD_ID_PAMAUTH_AUTHENTICATE_CANCEL);
        }
        m_pamSession->stop();
        m_bdHelper->pamAuthenticateCancel();
    });
    connect(
        m_modelLockDialog,
        &LockDialogModel::pamIsInAuthentication,
        m_bdHelper,
        &BackendDbusHelper::pamIsInAuthentication);
    connect(
        m_modelLockDialog, &LockDialogModel::pamIsAuthenticated, m_bdHelper, &BackendDbusHelper::pamIsAuthenticated);
    connect(
        m_modelLockDialog, &LockDialogModel::pamAuthenticateUser, m_bdHelper, &BackendDbusHelper::pamAuthenticateUser);
    connect(
        m_modelLockDialog,
        &LockDialogModel::getAutologinUserHint,
        m_bdHelper,
        &BackendDbusHelper::getAutologinUserHint);
    connect(m_modelLockDialog, &LockDialogModel::GetBlankState, m_bdHelper, &BackendDbusHelper::GetBlankState);
    connect(m_modelLockDialog, &LockDialogModel::CheckTurnOnScreen, m_bdHelper, &BackendDbusHelper::CheckTurnOnScreen);

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
    connect(m_modelLockDialog, &LockDialogModel::setClosePhase, m_bdHelper, &BackendDbusHelper::setClosePhase);
    connect(m_modelLockDialog, &LockDialogModel::lockStateChanged, m_bdHelper, &BackendDbusHelper::lockStateChanged);
    connect(m_modelLockDialog, &LockDialogModel::getPublicEncrypt, m_bdHelper, &BackendDbusHelper::getPublicEncrypt);
    connect(m_modelLockDialog, &LockDialogModel::sendPassword, m_bdHelper, &BackendDbusHelper::sendPassword);
    connect(m_modelLockDialog, &LockDialogModel::setUserThemeColor, m_bdHelper, &BackendDbusHelper::setThemeStyleConf);
    connect(m_modelLockDialog, &LockDialogModel::setRfkillState, m_bdHelper, &BackendDbusHelper::setUsdMediaStateKeys);
    // 锁定状态
    connect(m_modelLockDialog, &LockDialogModel::setLockState, m_bdHelper, &BackendDbusHelper::SetLockState);
    connect(m_modelLockDialog, &LockDialogModel::clearLockState, m_bdHelper, &BackendDbusHelper::ClearLockState);

    /*电源管理*/
    connect(
        m_modelLockDialog,
        &LockDialogModel::getCanHibernate,
        m_bdHelper,
        &BackendDbusHelper::getPowerManagerCanHibernate,
        Qt::DirectConnection);
    connect(
        m_modelLockDialog,
        &LockDialogModel::getCanSuspend,
        m_bdHelper,
        &BackendDbusHelper::getPowerManagerCanSuspend,
        Qt::DirectConnection);
    connect(
        m_modelLockDialog,
        &LockDialogModel::getCanReboot,
        m_bdHelper,
        &BackendDbusHelper::getPowerManagerCanReboot,
        Qt::DirectConnection);
    connect(
        m_modelLockDialog,
        &LockDialogModel::getCanPowerOff,
        m_bdHelper,
        &BackendDbusHelper::getPowerManagerCanPowerOff,
        Qt::DirectConnection);
    connect(
        m_modelLockDialog,
        &LockDialogModel::getCanLockScreen,
        m_bdHelper,
        &BackendDbusHelper::getPowerManagerCanLockScreen,
        Qt::DirectConnection);
    connect(
        m_modelLockDialog,
        &LockDialogModel::getCanSwitchUser,
        m_bdHelper,
        &BackendDbusHelper::getPowerManagerCanSwitchUser,
        Qt::DirectConnection);
    connect(
        m_modelLockDialog,
        &LockDialogModel::getCanLogout,
        m_bdHelper,
        &BackendDbusHelper::getPowerManagerCanLogout,
        Qt::DirectConnection);
    connect(
        m_modelLockDialog,
        &LockDialogModel::checkSystemUpgrade,
        m_bdHelper,
        &BackendDbusHelper::checkSystemUpgrade,
        Qt::DirectConnection);
    connect(
        m_modelLockDialog,
        &LockDialogModel::checkHideUpdate,
        m_bdHelper,
        &BackendDbusHelper::checkHideUpdate,
        Qt::DirectConnection);
}

void LockDialogPerformer::initData()
{
    QString strJsonInitData = m_bdHelper->getInitDataJson();
    if (!strJsonInitData.isEmpty()) {
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strJsonInitData.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qWarning() << "getInitDataJson Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qWarning() << "getInitDataJson Json is null!!";
            } else {
                if (objRes.contains("UsersInfo")) {
                    QList<UserInfoPtr> listUser;
                    QJsonArray jsonArray = objRes.value("UsersInfo").toArray();
                    if (!jsonArray.isEmpty()) {
                        for (auto jsonValue : jsonArray) {
                            QJsonObject objUser = jsonValue.toObject();
                            UserInfoPtr userPtr = std::make_shared<UserInfo>();
                            userPtr->updateUid((uid_t)objUser.value("uid").toDouble(-1));
                            userPtr->updateName(objUser.value("name").toString());
                            userPtr->updateFullName(objUser.value("fullName").toString());
                            userPtr->updateHeadImage(objUser.value("headImg").toString());
                            userPtr->updateLoggedIn(objUser.value("loggedIn").toBool(false));
                            userPtr->updateBackground(objUser.value("backGround").toString());
                            userPtr->updateLang(objUser.value("lang").toString());
                            userPtr->updateGreeterBackground(objUser.value("greeterBackGround").toString());
                            userPtr->updateBackgroundColor(objUser.value("color").toString());
                            userPtr->updateSystemAccount(objUser.value("systemAccount").toBool(false));
                            listUser.append(userPtr);
                        }
                    }
                    m_modelLockDialog->updateUsersInfo(listUser);
                }
                if (objRes.contains("SessionsInfo")) {
                    QList<QString> listSessions;
                    QJsonArray jsonArray = objRes.value("SessionsInfo").toArray();
                    if (!jsonArray.isEmpty()) {
                        for (auto jsonValue : jsonArray) {
                            QJsonObject objUser = jsonValue.toObject();
                            QString strSession = objUser.value("key").toString();
                            listSessions.append(strSession);
                        }
                    }
                    m_modelLockDialog->updateSessionsInfo(listSessions);
                }
                if (objRes.contains("IsSessionActive")) {
                    m_modelLockDialog->updateSessionState(objRes.value("IsSessionActive").toBool());
                }
                if (objRes.contains("IsLegacy")) {
                    m_modelLockDialog->updateIsSessionLegacy(objRes.value("IsLegacy").toBool());
                }
                if (objRes.contains("DefAuthUser")) {
                    m_modelLockDialog->updateDefUserName(objRes.value("DefAuthUser").toString());
                }
                if (objRes.contains("CurUserName")) {
                    m_modelLockDialog->updateCurUserName(objRes.value("CurUserName").toString());
                }
                if (objRes.contains("CurSession")) {
                    m_modelLockDialog->updateCurSession(objRes.value("CurSession").toString());
                }
                if (objRes.contains("AgreementInfo")) {
                    AgreementInfoPtr agreementInfo = nullptr;
                    QJsonObject jsonObject = objRes.value("AgreementInfo").toObject();

                    if (!jsonObject.isEmpty()) {
                        agreementInfo = std::make_shared<AgreementInfo>();
                        agreementInfo->updateShowLoginPrompt(jsonObject.value("showLoginPrompt").toBool());
                        agreementInfo->updateHideTitle(jsonObject.value("hideTitle").toBool());
                        agreementInfo->updatePromptTitle(jsonObject.value("promptTitle").toString());
                        agreementInfo->updatePromptText(jsonObject.value("promptText").toString());
                        agreementInfo->updatePromptTextFilePath(jsonObject.value("promptTextFilePath").toString());

                        qDebug() << "------------------------------------------------------------";
                        qDebug() << jsonObject.value("showLoginPrompt").toBool();
                        qDebug() << jsonObject.value("hideTitle").toBool();
                        qDebug() << jsonObject.value("promptTitle").toString();
                        qDebug() << jsonObject.value("promptText").toString();
                        qDebug() << jsonObject.value("promptTextFilePath").toString();
                        qDebug() << "------------------------------------------------------------";
                    }
                    if (!agreementInfo) {
                        agreementInfo = std::make_shared<AgreementInfo>();
                    }
                    m_modelLockDialog->updateAgreementInfo(agreementInfo);
                }
                if (objRes.contains("CapsLock")) {
                    m_modelLockDialog->updateCapslockState(objRes.value("CapsLock").toBool());
                }
                if (objRes.contains("MediaKeyAreaScreenShot")) {
                    m_modelLockDialog->updateUsdMediaKeys(
                        KEY_AREA_SCREENSHOT, objRes.value("MediaKeyAreaScreenShot").toString());
                }
                if (objRes.contains("MediaKeyAreaScreenShot2")) {
                    m_modelLockDialog->updateUsdMediaKeys(
                        KEY_AREA_SCREENSHOT2, objRes.value("MediaKeyAreaScreenShot2").toString());
                }
                if (objRes.contains("MediaKeyScreenShot")) {
                    m_modelLockDialog->updateUsdMediaKeys(
                        KEY_SCREEN_SHOT, objRes.value("MediaKeyScreenShot").toString());
                }
                if (objRes.contains("MediaKeyScreenShot2")) {
                    m_modelLockDialog->updateUsdMediaKeys(
                        KEY_SCREEN_SHOT2, objRes.value("MediaKeyScreenShot2").toString());
                }
                if (objRes.contains("MediaKeyWinScreenShot")) {
                    m_modelLockDialog->updateUsdMediaKeys(
                        KEY_WINDOW_SCREENSHOT, objRes.value("MediaKeyWinScreenShot").toString());
                }
                if (objRes.contains("MediaKeyRfkillState")) {
#ifdef USDFIXED
                    m_modelLockDialog->updateUsdMediaStateKeys(
                        KEY_RFKILL_STATE, objRes.value("MediaKeyRfkillState").toInt());
#endif
                }
                if (objRes.contains("PowerButton")) {
                    m_modelLockDialog->updatePowerButtonKey(KEY_BUTTON_POWER, objRes.value("PowerButton").toString());
                }
                if (objRes.contains("ScreenSaverMode")) {
                    m_modelLockDialog->updateScreensaverMode(objRes.value("ScreenSaverMode").toString());
                }
                if (objRes.contains("ScreenSaverThemes")) {
                    QList<QString> listThemes;
                    QJsonArray jsonArray = objRes.value("ScreenSaverThemes").toArray();
                    if (!jsonArray.isEmpty()) {
                        for (auto jsonValue : jsonArray) {
                            QJsonObject objUser = jsonValue.toObject();
                            QString strSession = objUser.value("key").toString();
                            qDebug() << "savertheme = " << strSession;
                            listThemes.append(strSession);
                        }
                    }
                    m_modelLockDialog->updateScreensaverTheme(listThemes);
                }
                if (objRes.contains("ImageTransitionEffect")) {
                    m_modelLockDialog->updateScreensaverImageTSEffect(objRes.value("ImageTransitionEffect").toInt());
                }
                if (objRes.contains("ImageSwitchInterval")) {
                    m_modelLockDialog->updateScreensaverImageSwitchInterval(
                        objRes.value("ImageSwitchInterval").toInt());
                }
                if (objRes.contains("BatteryArgs")) {
                    QStringList listBattery;
                    QJsonArray jsonArray = objRes.value("BatteryArgs").toArray();
                    if (!jsonArray.isEmpty()) {
                        for (auto jsonValue : jsonArray) {
                            QJsonObject objUser = jsonValue.toObject();
                            QString strBattery = objUser.value("key").toString();
                            listBattery.append(strBattery);
                        }
                    }
                    m_modelLockDialog->updateBatteryArgs(listBattery);
                }
                if (objRes.contains("BatteryIconName")) {
                    m_modelLockDialog->updateBatteryIconName(objRes.value("BatteryIconName").toString());
                }
                if (objRes.contains("IsBattery")) {
                    m_modelLockDialog->updateIsBattery(objRes.value("IsBattery").toBool());
                }
                if (objRes.contains("SleepLockCheck")) {
                    QVector<InhibitorInfo> sleepLockcheck;
                    QJsonArray jsonArray = objRes.value("SleepLockCheck").toArray();
                    if (!jsonArray.isEmpty()) {
                        for (const auto &jsonValue : jsonArray) {
                            QJsonObject objInhibitor = jsonValue.toObject();
                            InhibitorInfo info;
                            info.name = objInhibitor.value("name").toString();
                            info.icon = objInhibitor.value("icon").toString();
                            info.uid = objInhibitor.value("uid").toInt();
                            sleepLockcheck.append(info);
                        }
                    }
                    m_modelLockDialog->updateSleepLockcheck(sleepLockcheck);
                }
                if (objRes.contains("ShutdownLockCheck")) {
                    QVector<InhibitorInfo> shutdownLockcheck;
                    QJsonArray jsonArray = objRes.value("ShutdownLockCheck").toArray();
                    if (!jsonArray.isEmpty()) {
                        for (const auto &jsonValue : jsonArray) {
                            QJsonObject objInhibitor = jsonValue.toObject();
                            InhibitorInfo info;
                            info.name = objInhibitor.value("name").toString();
                            info.icon = objInhibitor.value("icon").toString();
                            info.uid = objInhibitor.value("uid").toInt();
                            shutdownLockcheck.append(info);
                        }
                    }
                    m_modelLockDialog->updateShutdownLockcheck(shutdownLockcheck);
                }
                if (objRes.contains("LogoutLockCheck")) {
                    QVector<InhibitorInfo> logoutLockCheck;
                    QJsonArray jsonArray = objRes.value("LogoutLockCheck").toArray();
                    if (!jsonArray.isEmpty()) {
                        for (const auto &jsonValue : jsonArray) {
                            QJsonObject objInhibitor = jsonValue.toObject();
                            InhibitorInfo info;
                            info.name = objInhibitor.value("name").toString();
                            info.icon = objInhibitor.value("icon").toString();
                            info.uid = objInhibitor.value("uid").toInt();
                            logoutLockCheck.append(info);
                        }
                    }
                    m_modelLockDialog->updateLogoutLockcheck(logoutLockCheck);
                }
                if (objRes.contains("GetAppWindows")) {
                    QVector<WindowInfo::WindowInfo> apps;
                    QJsonArray jsonArray = objRes.value("GetAppWindows").toArray();
                    if (!jsonArray.isEmpty()) {
                        for (auto jsonValue : jsonArray) {
                            WindowInfo::WindowInfo info;
                            QJsonObject objUser = jsonValue.toObject();
                            info.wid = objUser.value("wid").toString();
                            info.icon = objUser.value("icon").toString();
                            info.name = objUser.value("name").toString();
                            apps.append(info);
                        }
                    }
                    m_modelLockDialog->updateAppWindows(apps);
                }
                if (objRes.contains("LockTimeout")) {
                    m_modelLockDialog->updateLockTimeout(objRes.value("LockTimeout").toInt());
                }
                if (objRes.contains("IdleDelay")) {
                    m_modelLockDialog->updateIdleDelay(objRes.value("IdleDelay").toInt());
                }
                if (objRes.contains("LockEnable")) {
                    m_modelLockDialog->updateLockEnabled(objRes.value("LockEnable").toBool());
                }
                if (objRes.contains("SleepActivationEnable")) {
                    m_modelLockDialog->updateSleepActivationEnabled(objRes.value("SleepActivationEnable").toBool());
                }
                if (objRes.contains("SystemFontSize")) {
                    m_modelLockDialog->updateSystemFontSize(objRes.value("SystemFontSize").toDouble());
                }
                if (objRes.contains("SystemFont")) {
                    m_modelLockDialog->updateSystemFont(objRes.value("SystemFont").toString());
                }
                if (objRes.contains("TabletMode")) {
                    m_modelLockDialog->updateTabletMode(objRes.value("TabletMode").toBool());
                }
                if (objRes.contains("LogoutMusic")) {
                    m_modelLockDialog->updateSessionLogoutMusic(objRes.value("LogoutMusic").toBool());
                }
                if (objRes.contains("PoweroffMusic")) {
                    m_modelLockDialog->updateSessionPoweroffMusic(objRes.value("PoweroffMusic").toBool());
                }
                if (objRes.contains("LockDatetimeEnable")) {
                    m_modelLockDialog->updateLockDatetimeEnable(objRes.value("LockDatetimeEnable").toBool());
                }
                if (objRes.contains("CanSwitchUser")) {
                    m_modelLockDialog->updateCanSwitchUser(objRes.value("CanSwitchUser").toBool());
                }
            }
        }
    } else {
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
        m_modelLockDialog->updateUsdMediaKeys(
            KEY_SCREEN_SHOT2, m_bdHelper->getUsdMediaKeys(KEY_SCREEN_SHOT2).toString());
        m_modelLockDialog->updateUsdMediaKeys(
            KEY_WINDOW_SCREENSHOT, m_bdHelper->getUsdMediaKeys(KEY_WINDOW_SCREENSHOT).toString());
#ifdef USDFIXED
        m_modelLockDialog->updateUsdMediaStateKeys(
            KEY_RFKILL_STATE, m_bdHelper->getUsdMediaStateKeys(KEY_RFKILL_STATE).toInt());
#endif
        m_modelLockDialog->updatePowerButtonKey(
            KEY_BUTTON_POWER, m_bdHelper->getPowerManagerConf(KEY_BUTTON_POWER).toString());
        m_modelLockDialog->updateScreensaverMode(m_bdHelper->getLockScreenConf(KEY_MODE).toString());
        m_modelLockDialog->updateScreensaverTheme(m_bdHelper->getSaverThemes());
        m_modelLockDialog->updateScreensaverImageTSEffect(
            m_bdHelper->getLockScreenConf(KEY_IMAGE_TRANSITION_EFFECT).toInt());
        m_modelLockDialog->updateScreensaverImageSwitchInterval(
            m_bdHelper->getLockScreenConf(KEY_IMAGE_SWITCH_INTERVAL).toInt());
        m_modelLockDialog->updateBatteryArgs(m_bdHelper->getBatteryArgs());
        m_modelLockDialog->updateBatteryIconName(m_bdHelper->getBatteryIconName());
        m_modelLockDialog->updateIsBattery(m_bdHelper->getIsBattery());
        m_modelLockDialog->updateIsSessionLegacy(m_bdHelper->getIsSessionLegacy());
        m_modelLockDialog->updateLogoutLockcheck(m_bdHelper->getLogoutLockcheck());
        m_modelLockDialog->updateSleepLockcheck(m_bdHelper->getSleepLockcheck());
        m_modelLockDialog->updateShutdownLockcheck(m_bdHelper->getShutdownLockcheck());
        m_modelLockDialog->updateAppWindows(m_bdHelper->getAppWindows());
        m_modelLockDialog->updateLoggedInUsersCount();
        m_modelLockDialog->updateLockTimeout(m_bdHelper->getLockScreenConf(KEY_LOCK_TIMEOUT).toInt());
        m_modelLockDialog->updateIdleDelay(m_bdHelper->getLockScreenConf(KEY_IDLE_DELAY).toInt());
        m_modelLockDialog->updateLockEnabled(m_bdHelper->getLockScreenConf(KEY_LOCK_ENABLED).toBool());
        m_modelLockDialog->updateSleepActivationEnabled(
            m_bdHelper->getLockScreenConf(KEY_SLEEP_ACTIVATION_ENABLED).toBool());
        m_modelLockDialog->updateSystemFontSize(m_bdHelper->getThemeStyleConf(KEY_SYSTEM_FONT_SIZE).toDouble());
        m_modelLockDialog->updateSystemFont(m_bdHelper->getThemeStyleConf(KEY_SYSTEM_FONT).toString());
        m_modelLockDialog->updateTabletMode(m_bdHelper->getCurTabletMode());
        m_modelLockDialog->updateSessionLogoutMusic(m_bdHelper->getSessionConf(KEY_SESSION_LOGOUT_MUSIC).toBool());
        m_modelLockDialog->updateSessionPoweroffMusic(m_bdHelper->getSessionConf(KEY_SESSION_POWEROFF_MUSIC).toBool());
        m_modelLockDialog->updateLockDatetimeEnable(m_bdHelper->getLockDatetimeEnable());
    }
}

void LockDialogPerformer::initExtraData()
{
    //    if (m_modelLockDialog->isUseWayland()) {
    //        connect(
    //            PlasmaShellManager::getInstance(),
    //            &PlasmaShellManager::capslockStateChanged,
    //            m_modelLockDialog,
    //            &LockDialogModel::onCapslockStateChanged);
    //    }
}

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
#include "lockdialogmodel.h"
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <QScreen>
#include "../../src/common/utils.h"
#include "../../src/common/definetypes.h"
#include <pwd.h>

LockDialogModel::LockDialogModel(QObject *parent)
    : QObject(parent), m_isUseWayland(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland")
{
}

bool LockDialogModel::parseCmdArguments(QStringList args, CommandLineArgs &cmdArgs)
{
    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription(QGuiApplication::translate("main", "Dialog for the ukui ScreenSaver."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    QCommandLineOption lockOption(
        QStringLiteral("lock"), QGuiApplication::translate("main", "lock the screen immediately"));
    QCommandLineOption lstOption(
        QStringLiteral("lock-startup"), QGuiApplication::translate("main", "lock the screen immediately"));
    QCommandLineOption sessionIdleOption(
        QStringLiteral("session-idle"), QGuiApplication::translate("main", "activated by session idle signal"));
    QCommandLineOption lscreensaverOption(
        QStringLiteral("lock-screensaver"),
        QGuiApplication::translate("main", "lock the screen and show screensaver immediately"));
    QCommandLineOption screensaverOption(
        QStringLiteral("screensaver"), QGuiApplication::translate("main", "show screensaver immediately"));
    QCommandLineOption blankOption(
        QStringLiteral("blank"),
        QGuiApplication::translate("main", "lock the screen and show screensaver immediately"));
    QCommandLineOption delayOption(
        QStringLiteral("delay"),
        QGuiApplication::translate("main", "show blank screensaver immediately and delay time to show lock"),
        QGuiApplication::translate("delay", "how long to show lock"),
        "0");
    QCommandLineOption hasLockOption(
        QStringLiteral("has-lock"),
        QGuiApplication::translate("main", "show blank screensaver immediately and if lock"),
        QGuiApplication::translate("has-lock", "if show lock"),
        "1");
    QCommandLineOption sessiontoolsOption(
        { "t", QStringLiteral("session-tools") }, QCoreApplication::translate("main", "show the session tools"));
    QCommandLineOption switchuserOption(
        QStringLiteral("switchuser"), QGuiApplication::translate("main", "show switch user window"));
    QCommandLineOption appBlockOption(
        { "a", QStringLiteral("app-block") },
        QCoreApplication::translate("main", "show the app block window"),
        QGuiApplication::translate("action", "which block type"),
        "type");
    QCommandLineOption multiUsersOption(
        { "m", QStringLiteral("multiusers-block") },
        QCoreApplication::translate("main", "show the multiUsers block window"),
        QGuiApplication::translate("action", "which block type"),
        "type");
    parser.addOptions({ lockOption,
                        lstOption,
                        sessionIdleOption,
                        screensaverOption,
                        blankOption,
                        lscreensaverOption,
                        delayOption,
                        hasLockOption,
                        sessiontoolsOption,
                        switchuserOption,
                        appBlockOption,
                        multiUsersOption });
    parser.process(args);

    QString strHostCloundPlatform = getHostCloudPlatform();
    qDebug() << "HostCloundPlatform:" << strHostCloundPlatform;
    if (strHostCloundPlatform == "ctyun") { // 天翼云不允许锁屏
        if (!parser.isSet(lstOption)) {
            return false;
        }
    } else {
        if (!parser.isSet(sessionIdleOption) && !parser.isSet(lockOption) && !parser.isSet(lstOption)
            && !parser.isSet(screensaverOption) && !parser.isSet(lscreensaverOption) && !parser.isSet(blankOption)
            && !parser.isSet(sessiontoolsOption) && !parser.isSet(switchuserOption) && !parser.isSet(appBlockOption)
            && !parser.isSet(multiUsersOption)) {
            return false;
        }
    }
    cmdArgs.isSessionIdle = parser.isSet(sessionIdleOption);
    cmdArgs.isLock = parser.isSet(lockOption);
    cmdArgs.isLockStartup = parser.isSet(lstOption);
    cmdArgs.isScreenSaver = parser.isSet(screensaverOption);
    cmdArgs.isLockScreensaver = parser.isSet(lscreensaverOption);
    cmdArgs.isBlank = parser.isSet(blankOption);
    cmdArgs.isSessionTools = parser.isSet(sessiontoolsOption);
    cmdArgs.isSwitchUser = parser.isSet(switchuserOption);
    if (parser.isSet(hasLockOption)) {
        int nHasLockValue = parser.value(hasLockOption).toInt();
        if (nHasLockValue == 0) {
            cmdArgs.isBlankHasLock = false;
        } else {
            cmdArgs.isBlankHasLock = true;
        }
    }
    if (parser.isSet(delayOption)) {
        cmdArgs.nBlankDelay = parser.value(delayOption).toInt();
    }
    if (parser.isSet(appBlockOption)) {
        QString opValue = parser.value(appBlockOption);
        if (opValue == "Restart") {
            cmdArgs.isAppBlock = 0;
        } else if (opValue == "Shutdown") {
            cmdArgs.isAppBlock = 1;
        } else if (opValue == "Suspend") {
            cmdArgs.isAppBlock = 2;
        } else if (opValue == "Hibernate") {
            cmdArgs.isAppBlock = 3;
        } else if (opValue == "Logout") {
            cmdArgs.isAppBlock = 4;
        }
    }
    if (parser.isSet(multiUsersOption)) {
        QString opValue = parser.value(multiUsersOption);
        if (opValue == "Restart") {
            cmdArgs.isMultiUserBlock = 0;
        } else if (opValue == "Shutdown") {
            cmdArgs.isMultiUserBlock = 1;
        }
    }
    return true;
}

void LockDialogModel::onSessionActiveChanged(bool isActive)
{
    m_isSessionActive = isActive;
    Q_EMIT sessionActiveChanged(isActive);
}

void LockDialogModel::updateSessionState(bool isActive)
{
    m_isSessionActive = isActive;
}

bool LockDialogModel::sessionActive()
{
    return m_isSessionActive;
}

void LockDialogModel::onRunningMessage(const QString &strMsg)
{
    CommandLineArgs cmdArgs;
    QStringList arguments = strMsg.split(",");
    qInfo() << "onRunningMessage:" << arguments;
    if (!parseCmdArguments(arguments, cmdArgs))
        return;
    if (cmdArgs.isBlank) {
        Q_EMIT showBlankScreensaver(cmdArgs.nBlankDelay, cmdArgs.isBlankHasLock);
    }

    if (cmdArgs.isLock) {
        Q_EMIT showLock(false);
    }

    if (cmdArgs.isLockStartup) {
        Q_EMIT showLock(true);
    }

    if (cmdArgs.isSessionIdle) {
        Q_EMIT showSessionIdle();
    }

    if (cmdArgs.isLockScreensaver) {
        Q_EMIT showLockScreensaver();
    }

    if (cmdArgs.isScreenSaver) {
        Q_EMIT showScreensaver();
    }

    if (cmdArgs.isSessionTools) {
        Q_EMIT showSessionTools();
    }

    if (cmdArgs.isAppBlock != -1) {
        Q_EMIT showAppBlockWindow(cmdArgs.isAppBlock);
    }

    if (cmdArgs.isMultiUserBlock != -1) {
        Q_EMIT showMultiUsersBlockWindow(cmdArgs.isMultiUserBlock);
    }

    if (cmdArgs.isSwitchUser) {
        Q_EMIT showSwitchUserLock();
    }

    KillFocusOfKydroid();
}

UserInfoPtr LockDialogModel::findUserByName(const QString &strName)
{
    for (auto user : m_listUsersInfo) {
        if (user->name() == strName) {
            return user;
        }
    }
    struct passwd *userInfo = getpwnam(strName.toLatin1().data());
    if (userInfo) {
        UserInfoPtr userPtr = std::make_shared<UserInfo>();
        userPtr->updateUid(userInfo->pw_uid);
        userPtr->updateName(strName);
        if (userInfo->pw_gecos && userInfo->pw_gecos[0] != '\0') {
            QString geoces = QString(userInfo->pw_gecos);
            QStringList list = geoces.split(QLatin1Char(','));
            if (list.size() > 0) {
                userPtr->updateFullName(list[0]);
            }
        }
        return userPtr;
    }
    return nullptr;
}

UserInfoPtr LockDialogModel::findUserById(const uid_t &id)
{
    for (auto user : m_listUsersInfo) {
        if (user->uid() == id) {
            return user;
        }
    }
    struct passwd *userInfo = getpwuid(id);
    if (userInfo) {
        UserInfoPtr userPtr = std::make_shared<UserInfo>();
        userPtr->updateUid(userInfo->pw_uid);
        userPtr->updateName(userInfo->pw_name);
        if (userInfo->pw_gecos && userInfo->pw_gecos[0] != '\0') {
            QString geoces = QString(userInfo->pw_gecos);
            QStringList list = geoces.split(QLatin1Char(','));
            if (list.size() > 0) {
                userPtr->updateFullName(list[0]);
            }
        }
        return userPtr;
    }
    return nullptr;
}

void LockDialogModel::updateSessionsInfo(QStringList list)
{
    m_listSessions = list;
}

void LockDialogModel::updateUsersInfo(QList<UserInfoPtr> list)
{
    for (auto userinfo : list) {
        if (m_listUsersInfo.contains(userinfo)) {
            UserInfoPtr changedUserInfo = findUserByName(userinfo->name());
            if (changedUserInfo) {
                changedUserInfo->updateUid(userinfo->uid());
                changedUserInfo->updateFullName(userinfo->fullName());
                changedUserInfo->updateHeadImage(userinfo->headImage());
                changedUserInfo->updateLang(userinfo->lang());
                changedUserInfo->updateLoggedIn(userinfo->isLoggedIn());
                changedUserInfo->updateBackground(userinfo->backGround());
                changedUserInfo->updateGreeterBackground(userinfo->greeterBackGround());
                changedUserInfo->updateBackgroundColor(userinfo->backGroundColor());
                changedUserInfo->updateSystemAccount(userinfo->isSystemAccount());
            }
        } else {
            m_listUsersInfo.append(userinfo);
        }
    }
    foreach (auto userinfo, m_listUsersInfo) {
        if (!list.contains(userinfo)) {
            m_listUsersInfo.removeOne(userinfo);
        }
    }
    Q_EMIT usersInfoChanged();
}

void LockDialogModel::updateAgreementInfo(AgreementInfoPtr agreementInfo)
{
    m_agreementInfo = agreementInfo;
    Q_EMIT agreementInfoChanged();
}

void LockDialogModel::onUsersInfoChanged(QList<UserInfoPtr> list)
{
    for (auto userinfo : list) {
        if (m_listUsersInfo.contains(userinfo)) {
            UserInfoPtr changedUserInfo = findUserByName(userinfo->name());
            if (changedUserInfo) {
                changedUserInfo->updateUid(userinfo->uid());
                changedUserInfo->updateFullName(userinfo->fullName());
                changedUserInfo->updateHeadImage(userinfo->headImage());
                changedUserInfo->updateLang(userinfo->lang());
                changedUserInfo->updateLoggedIn(userinfo->isLoggedIn());
                changedUserInfo->updateBackground(userinfo->backGround());
                changedUserInfo->updateGreeterBackground(userinfo->greeterBackGround());
                changedUserInfo->updateBackgroundColor(userinfo->backGroundColor());
            }
        } else {
            m_listUsersInfo.append(userinfo);
        }
    }
    foreach (auto userinfo, m_listUsersInfo) {
        if (!list.contains(userinfo)) {
            m_listUsersInfo.removeOne(userinfo);
        }
    }
    Q_EMIT usersInfoChanged();
}

void LockDialogModel::onCurUserChanged(const QString &strUserName)
{
    updateCurUserName(strUserName);
    Q_EMIT currentUserChanged(strUserName);
}

void LockDialogModel::onCurSessionChanged(const QString &strSession)
{
    updateCurSession(strSession);
    Q_EMIT currentSessionChanged(strSession);
}

QString LockDialogModel::getAgreementWindowText()
{
    if (!m_agreementInfo->promptText().isEmpty()) {
        return m_agreementInfo->promptText();
    }

    if (m_agreementInfo->promptTextFilePath().isEmpty())
        return "";

    QFile file(m_agreementInfo->promptTextFilePath());
    if (!file.exists())
        return "";
    file.open(QFile::ReadOnly);
    QString str(file.readAll());
    if (str == "")
        return "";
    file.close();
    return str;
}

ScreenSaver *LockDialogModel::getScreensaver()
{
    QStringList modeStr{ "blank-only", "random", "single", "image", "default-ukui", "default-ukui-custom" };
    ScreenSaver *saver = new ScreenSaver;
    int index = modeStr.indexOf(m_saverMode);
    saver->mode = SaverMode(index);
    saver->interval = m_imageSwitchInterval;
    saver->effect = TransitionEffect(m_imageTSEffect);

    switch (index) {
        case SAVER_BLANK_ONLY:
            break;
        case SAVER_RANDOM: {
            if (m_saverTheme.count() == 0) {
                saver->path = "/usr/lib/ukui-screensaver/ukui-screensaver-default";
                break;
            }
            qsrand((unsigned)time(0));
            int index = qrand() % m_saverTheme.count();
            while (QString::compare(m_saverTheme[index], "kyccss-personal-slideshow") == 0) {
                index = qrand() % m_saverTheme.count();
            }
            saver->path = getXScreensaverPath(m_saverTheme[index]);
            break;
        }
        case SAVER_SINGLE:
            if (m_saverTheme.count() == 0) {
                saver->path = "/usr/lib/ukui-screensaver/ukui-screensaver-default";
                break;
            }
            saver->path = getXScreensaverPath(m_saverTheme[0]);
            break;
        case SAVER_IMAGE: {
            QString lang = qgetenv("LANG");
            if (!lang.isEmpty()) {
                qDebug() << "lang = " << lang;
                if (lang.contains("zh_CN")) {
                    saver->path = QDir::homePath() + "/图片";
                    break;
                }
            }
            saver->path = QDir::homePath() + "/" + QStandardPaths::displayName(QStandardPaths::PicturesLocation);
            break;
        }
        case SAVER_DEFAULT:
            saver->path = "/usr/lib/ukui-screensaver/ukui-screensaver-default";
            break;
        case SAVER_DEFAULT_CUSTOM:
            saver->path = "/usr/lib/ukui-screensaver/ukui-screensaver-default";
            break;
        default:
            break;
    }
    return saver;
}

double LockDialogModel::getCurFontSize()
{
    double defaultFontSize = getDefaultFontSize();
    return m_curFontSize - defaultFontSize;
}

double LockDialogModel::getPtToPx()
{
    double m_ptTopx = 1.0;
    if (QApplication::primaryScreen()->logicalDotsPerInch() > 0)
        m_ptTopx = 72 / (QApplication::primaryScreen()->logicalDotsPerInch());
    return m_ptTopx;
}

QString LockDialogModel::getXScreensaverPath(const QString &theme)
{
    /* screensavers-ukui-binaryring => binaryring */
    QStringList strs = theme.split("-");
    QString str = strs.at(strs.size() - 1);
    QString filePath = QString("%1/%2").arg(XSCREENSAVER_DIRNAME, str);
    // 除了判断gsetting值是否为空，还需要判断屏保文件是否存在，不存在就使用默认屏保。
    if (QFile(filePath).exists())
        return filePath;
    else
        return "/usr/lib/ukui-screensaver/ukui-screensaver-default";
}

void LockDialogModel::onLidstateChanged(const QString &lidstate)
{
    Q_EMIT lidstateChanged(lidstate);
}

void LockDialogModel::onBatteryStatusChanged(const QString &iconName)
{
    Q_EMIT batteryStatusChanged(iconName);
}

void LockDialogModel::onBatteryChanged(const QStringList &batteryArgs)
{
    Q_EMIT batteryChanged(batteryArgs);
}

void LockDialogModel::onLockScreenConfChanged(const QString &key, QVariant value)
{
    if (key == KEY_MODE) {
        m_saverMode = value.toString();
    } else if (key == KEY_THEMES) {
        m_saverTheme = value.toStringList();
    } else if (key == KEY_IMAGE_TRANSITION_EFFECT) {
        m_imageTSEffect = value.toInt();
    } else if (key == KEY_IMAGE_SWITCH_INTERVAL) {
        m_imageSwitchInterval = value.toInt();
    } else if (key == KEY_LOCK_TIMEOUT) {
        m_lockTimeout = value.toInt();
    } else if (key == KEY_LOCK_ENABLED) {
        m_lockEnabled = value.toBool();
    } else if (key == KEY_SLEEP_ACTIVATION_ENABLED) {
        m_sleepActivationEnabled = value.toBool();
    }
}

void LockDialogModel::onThemeStyleConfChanged(const QString &key, QVariant value)
{
    if (key == KEY_SYSTEM_FONT_SIZE) {
        m_curFontSize = value.toDouble();
        double defaultFontSize = getDefaultFontSize();
        Q_EMIT fontSizeChanged(m_curFontSize - defaultFontSize);
    } else if (key == KEY_SYSTEM_FONT) {
        m_curFont = value.toString();
        Q_EMIT fontChanged(m_curFont);
    }
}

void LockDialogModel::onCapslockStateChanged(const bool capslockState)
{
    Q_EMIT capslockStateChanged(capslockState);
}

void LockDialogModel::onUsdMediaStateKeysChanged(const QString &key, const int &value)
{
    Q_EMIT usdMediaStateKeysChanged(key, value);
}

void LockDialogModel::onUsdMediaKeysChanged(const QString &key, const QString &value)
{
    Q_EMIT usdMediaKeysChanged(key, value);
}

void LockDialogModel::onTabletModeChanged(bool tabletMode)
{
    Q_EMIT tabletModeChanged(tabletMode);
}

void LockDialogModel::onSessionConfChanged(const QString &key, QVariant value)
{
    if (key == KEY_SESSION_LOGOUT_MUSIC) {
        m_logoutMusic = value.toBool();
    } else if (key == KEY_SESSION_POWEROFF_MUSIC) {
        m_poweroffMusic = value.toBool();
    }
}

void LockDialogModel::updateCapslockState(bool capslockState)
{
    m_capslockState = capslockState;
}

void LockDialogModel::onSecondRunParam(const QString &str)
{
    Q_EMIT SecondRunParam(str);
}

void LockDialogModel::updateUsdMediaKeys(QString keys, QString value)
{
    if (keys == KEY_AREA_SCREENSHOT) {
        m_areaScreenShot = value;
    } else if (keys == KEY_AREA_SCREENSHOT2) {
        m_areaScreenShot2 = value;
    } else if (keys == KEY_SCREEN_SHOT) {
        m_screenShot = value;
    } else if (keys == KEY_SCREEN_SHOT2) {
        m_screenShot2 = value;
    } else if (keys == KEY_WINDOW_SCREENSHOT) {
        m_windowScreenshot = value;
    }
}

void LockDialogModel::updateUsdMediaStateKeys(QString keys, int value)
{
    if (keys == KEY_RFKILL_STATE) {
        m_rfkillState = value;
    }
}

void LockDialogModel::updateLockEnabled(bool value)
{
    m_lockEnabled = value;
}

void LockDialogModel::updateSleepActivationEnabled(bool value)
{
    m_sleepActivationEnabled = value;
}

void LockDialogModel::updateLockTimeout(int value)
{
    m_lockTimeout = value;
}

void LockDialogModel::updateScreensaverMode(QString value)
{
    m_saverMode = value;
}

void LockDialogModel::updateScreensaverTheme(QStringList value)
{
    m_saverTheme = value;
}

void LockDialogModel::updateScreensaverImageTSEffect(int value)
{
    m_imageTSEffect = value;
}

void LockDialogModel::updateScreensaverImageSwitchInterval(int value)
{
    m_imageSwitchInterval = value;
}

void LockDialogModel::updateCanHibernate(bool canflag)
{
    qWarning() << __LINE__ << __FUNCTION__ << canflag;

    m_CanHibernate = canflag;
}
void LockDialogModel::updateCanReboot(bool canflag)
{
    qWarning() << __LINE__ << __FUNCTION__ << canflag;

    m_CanReboot = canflag;
}
void LockDialogModel::updateCanPowerOff(bool canflag)
{
    qWarning() << __LINE__ << __FUNCTION__ << canflag;
    m_CanPowerOff = canflag;
}
void LockDialogModel::updateCanSuspend(bool canflag)
{
    qWarning() << __LINE__ << __FUNCTION__ << canflag;
    m_CanSuspend = canflag;
}

void LockDialogModel::updateCanLockScreen(bool canflag)
{
    qWarning() << __LINE__ << __FUNCTION__ << canflag;
    m_CanLockScreen = canflag;
}

void LockDialogModel::updateCanSwitchUser(bool canflag)
{
    qWarning() << __LINE__ << __FUNCTION__ << canflag;
    m_CanSwitchUser = canflag;
}

void LockDialogModel::updateCanLogout(bool canflag)
{
    qWarning() << __LINE__ << __FUNCTION__ << canflag;
    m_CanLogout = canflag;
}

void LockDialogModel::updataCheckSystemUpgrade(bool canflag)
{
    qWarning() << __LINE__ << __FUNCTION__ << canflag;
    m_sysUpgradeStatus = canflag;
}

void LockDialogModel::updateBatteryArgs(QStringList batteryArgs)
{
    m_batteryArgs = batteryArgs;
}

void LockDialogModel::updateBatteryIconName(QString iconName)
{
    m_batteryIconName = iconName;
}

void LockDialogModel::updateIsBattery(bool isBattery)
{
    m_isBattery = isBattery;
}

void LockDialogModel::updateSleepLockcheck(QStringList sleepLockcheck)
{
    m_sleepLockcheck = sleepLockcheck;
}

void LockDialogModel::updateShutdownLockcheck(QStringList shutdownLockcheck)
{
    m_shutdownLockcheck = shutdownLockcheck;
}

void LockDialogModel::updateLoggedInUsersCount()
{
    m_loggedinUsersCount = 0;
    if (!m_listUsersInfo.isEmpty()) {
        for (auto user : m_listUsersInfo) {
            if (user->isLoggedIn()) {
                m_loggedinUsersCount++;
            }
        }
    }
}

void LockDialogModel::updateTabletMode(bool tabletMode)
{
    m_tabletMode = tabletMode;
}

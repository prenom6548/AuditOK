/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#ifndef GSETTINGSHELPER_H
#define GSETTINGSHELPER_H

#include <QObject>
#include <qgsettings.h>
#include <QVariant>

class GSettingsHelper : public QObject
{
    Q_OBJECT
public:
    explicit GSettingsHelper(QObject *parent = nullptr);
    virtual ~GSettingsHelper();

public:
    QVariant GetLockScreenConf(QString strKey);
    bool SetLockScreenConf(QString strKey, QVariant value);
    QVariant ResetLockScreenConf(QString strKey);
    QVariant GetScreenSaverConf(QString strKey);
    bool SetScreenSaverConf(QString strKey, QVariant value);
    QVariant ResetScreenSaverConf(QString strKey);
    QVariant GetPowerManagerConf(QString strKey);
    bool SetPowerManagerConf(QString strKey, QVariant value);
    QVariant GetMateBgConf(QString strKey);
    bool SetMateBgConf(QString strKey, QVariant value);
    QVariant GetUkccPluginsConf(QString strKey);
    bool SetUkccPluginsConf(QString strKey, QVariant value);
    QVariant GetThemeStyleConf(QString strKey);
    bool SetThemeStyleConf(QString strKey, QVariant value);
    QVariant GetSessionConf(QString strKey);
    bool SetSessionConf(QString strKey, QVariant value);
    QVariant GetKeyboardConf(QString strKey);
    bool SetKeyboardConf(QString strKey, QVariant value);
    QVariant GetUsdMediaKeys(QString strKey);
    QVariant GetUsdMediaStateKeys(QString strKey);
    bool SetUsdMediaStateKeys(QString strKey, QVariant value);
    QVariant GetWeatherKeys(QString strKey);

public Q_SLOTS:
    void onLockScreenConfigChanged(QString strKey);
    void onScreenSaverConfigChanged(QString strKey);
    void onPowerManagerConfigChanged(QString strKey);
    void onMateBgConfigChanged(QString strKey);
    void onUkccPluginsConfigChanged(QString strKey);
    void onThemeStyleConfigChanged(QString strKey);
    void onSessionConfigChanged(QString strKey);
    void onKeyboardConfigChanged(QString strKey);
    void onUsdMediaKeysConfigChanged(QString strKey);
    void onUsdMediaStateKeysConfigChanged(QString strKey);
    void onWeatherKeysConfigChanged(QString strKey);

Q_SIGNALS:
    void lockScreenConfigChanged(QString strKey, QVariant value);
    void screenSaverConfigChanged(QString strKey, QVariant value);
    void powerManagerConfigChanged(QString strKey, QVariant value);
    void mateBgConfigChanged(QString strKey, QVariant value);
    void ukccPluginsConfigChanged(QString strKey, QVariant value);
    void themeStyleConfigChanged(QString strKey, QVariant value);
    void sessionConfigChanged(QString strKey, QVariant value);
    void keyboardConfigChanged(QString strKey, QVariant value);
    void usdMediaKeysConfigChanged(QString strKey, QVariant value);
    void usdMediaStateKeysConfigChanged(QString strKey, QVariant value);
    void idleLockConfigChanged(int idleLock);

private:
    bool initLockScreen();
    bool initScreenSaver();
    bool initPowerManager();
    bool initMateBg();
    bool initUkccPlugins();
    bool initThemeStyle();
    bool initSession();
    bool initKeyboard();
    bool initUsdMediaKeys();
    bool initUsdMediaStateKeys();
    bool initWeatherKeys();

private:
    QGSettings *m_gsLockScreen = nullptr;
    QGSettings *m_gsScreenSaver = nullptr;
    QGSettings *m_gsPowerManager = nullptr;
    QGSettings *m_gsMateBg = nullptr;
    QGSettings *m_gsUkccPlugins = nullptr;
    QGSettings *m_gsThemeStyle = nullptr;
    QGSettings *m_gsSession = nullptr;
    QGSettings *m_gsKeyboard = nullptr;
    QGSettings *m_gsUsdMediaKeys = nullptr;
    QGSettings *m_gsUsdMediaStateKeys = nullptr;
    QGSettings *m_gsWeatherKeys = nullptr;

    bool m_bShowRestTime = false;
    bool m_bShowCustomRestTime = false;
    bool m_bShowUkuiRestTime = false;
    int m_nCycleTime = 1;
    bool m_bAutomaticSwitchingEnable = false;
    QString m_strBackgroundPath;
    QString m_strMyText;
    bool m_bTextIsCenter = true;
    bool m_bShowMessageEnable = false;
    int m_nMessageNum = 0;
    QString m_strVideoFormat;
    QString m_strVideoPath;
    int m_nVideoSize = 0;
    int m_nVideoWidth = 0;
    int m_nVideoHeight = 0;
    int m_nSubMode = -1;
    int m_nWeatherPosition = 0;
    QString m_sMusicPath;
    int m_nPhotoCycleMode = -1;
    int m_nPhotoCycleInterval = -1;
    QStringList m_sDefaultPhotoPath;
    QStringList m_sCustomPhptoPath;
    bool m_bMusicFollowSystem;

    int m_nIdleDelay = -1;
    int m_nIdleLock = -1;
    bool m_bIdleActivationEnable = false;
    bool m_bIdleLockEnable = false;
    int m_nLockTimeout = -1;
    bool m_bCloseActivationEnable = false;
    bool m_bSleepActivationEnable = false;
    bool m_bLockEnable = false;
    QString m_strBackground;

    bool m_bLockSuspend = false;
    bool m_bLockHibernate = false;
    bool m_bLockBlankScreen = false;
    int m_nSleepComputeAc = -1;
    int m_nSleepDisplayAc = -1;
    QString m_strButtonLidAc;

    QString m_strPicFileName;
    QString m_strPicOptions;
    QString m_strPrimaryColor;

    int m_nHourSystem = 0;
    QString m_strDateType;

    double m_lfFontSize = 0.0;
    QString m_strThemeColor;
    QString m_font;
    int m_menuTransparency = 0;
    QString m_styleName;

    int m_nSessionIdle = -1;
    bool m_nSessionLogout = false;
    bool m_nSessionPoweroff = false;

    bool m_capsState = false;

    QString m_areaScreenShot;
    QString m_areaScreenShot2;
    QString m_screenShot;
    QString m_screenShot2;
    QString m_windowScreenshot;

    QString m_saverMode;
    QStringList m_saverTheme;
    int m_imageTSEffect;
    int m_imageSwitchInterval;

    int m_rfkillState = -1;

    QString m_sWeatherData;
};

#endif // GSETTINGSHELPER_H

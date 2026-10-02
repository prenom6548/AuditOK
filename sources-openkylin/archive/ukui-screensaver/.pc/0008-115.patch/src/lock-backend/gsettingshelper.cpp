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
#include "gsettingshelper.h"
#include <QVariant>
#include <QDebug>
#include "../common/definetypes.h"

GSettingsHelper::GSettingsHelper(QObject *parent) : QObject(parent) {}

GSettingsHelper::~GSettingsHelper() {}

bool GSettingsHelper::initLockScreen()
{
    if (!m_gsLockScreen) {
        if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_SCREENSAVER)) {
            m_gsLockScreen = new QGSettings(GSETTINGS_SCHEMA_SCREENSAVER, "", this);
            connect(m_gsLockScreen, &QGSettings::changed, this, &GSettingsHelper::onLockScreenConfigChanged);
            QStringList keys = m_gsLockScreen->keys();
            if (keys.contains(KEY_IDLE_DELAY)) {
                m_nIdleDelay = m_gsLockScreen->get(KEY_IDLE_DELAY).toInt();
            }
            if (keys.contains(KEY_IDLE_LOCK)) {
                m_nIdleLock = m_gsLockScreen->get(KEY_IDLE_LOCK).toInt();
            }
            if (keys.contains(KEY_IDLE_ACTIVATION_ENABLED)) {
                m_bIdleActivationEnable = m_gsLockScreen->get(KEY_IDLE_ACTIVATION_ENABLED).toBool();
            }
            if (keys.contains(KEY_IDLE_LOCK_ENABLED)) {
                m_bIdleLockEnable = m_gsLockScreen->get(KEY_IDLE_LOCK_ENABLED).toBool();
            }
            if (keys.contains(KEY_LOCK_TIMEOUT)) {
                m_nLockTimeout = m_gsLockScreen->get(KEY_LOCK_TIMEOUT).toInt();
            }
            if (keys.contains(KEY_CLOSE_ACTIVATION_ENABLED)) {
                m_bCloseActivationEnable = m_gsLockScreen->get(KEY_CLOSE_ACTIVATION_ENABLED).toBool();
            }
            if (keys.contains(KEY_SLEEP_ACTIVATION_ENABLED)) {
                m_bSleepActivationEnable = m_gsLockScreen->get(KEY_SLEEP_ACTIVATION_ENABLED).toBool();
            }
            if (keys.contains(KEY_LOCK_ENABLED)) {
                m_bLockEnable = m_gsLockScreen->get(KEY_LOCK_ENABLED).toBool();
            }
            if (keys.contains(KEY_BACKGROUND)) {
                m_strBackground = m_gsLockScreen->get(KEY_BACKGROUND).toString();
            }
            if (keys.contains(KEY_MODE)) {
                m_saverMode = m_gsLockScreen->get(KEY_MODE).toString();
            }
            if (keys.contains(KEY_THEMES)) {
                m_saverTheme = m_gsLockScreen->get(KEY_THEMES).toStringList();
            }
            if (keys.contains(KEY_IMAGE_TRANSITION_EFFECT)) {
                m_imageTSEffect = m_gsLockScreen->get(KEY_IMAGE_TRANSITION_EFFECT).toInt();
            }
            if (keys.contains(KEY_IMAGE_SWITCH_INTERVAL)) {
                m_imageSwitchInterval = m_gsLockScreen->get(KEY_IMAGE_SWITCH_INTERVAL).toInt();
            }
        }
    }
    return (bool)(m_gsLockScreen);
}

bool GSettingsHelper::initScreenSaver()
{
    if (!m_gsScreenSaver) {
        if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_SCREENSAVER_DEFAULT)) {
            m_gsScreenSaver = new QGSettings(GSETTINGS_SCHEMA_SCREENSAVER_DEFAULT, "", this);
            connect(m_gsScreenSaver, &QGSettings::changed, this, &GSettingsHelper::onScreenSaverConfigChanged);
            QStringList keys = m_gsScreenSaver->keys();
            if (keys.contains(KEY_SHOW_REST_TIME)) {
                m_bShowRestTime = m_gsScreenSaver->get(KEY_SHOW_REST_TIME).toBool();
            }
            if (keys.contains(KEY_SHOW_CUSTOM_REST_TIME)) {
                m_bShowCustomRestTime = m_gsScreenSaver->get(KEY_SHOW_CUSTOM_REST_TIME).toBool();
            }
            if (keys.contains(KEY_SHOW_UKUI_REST_TIME)) {
                m_bShowUkuiRestTime = m_gsScreenSaver->get(KEY_SHOW_UKUI_REST_TIME).toBool();
            }
            if (keys.contains(KEY_CYCLE_TIME)) {
                m_nCycleTime = m_gsScreenSaver->get(KEY_CYCLE_TIME).toInt();
            }
            if (keys.contains(KEY_AUTOMATIC_SWITCHING_ENABLE)) {
                m_bAutomaticSwitchingEnable = m_gsScreenSaver->get(KEY_AUTOMATIC_SWITCHING_ENABLE).toBool();
            }
            if (keys.contains(KEY_BACKGROUND_PATH)) {
                m_strBackgroundPath = m_gsScreenSaver->get(KEY_BACKGROUND_PATH).toString();
            }
            if (keys.contains(KEY_MYTEXT)) {
                m_strMyText = m_gsScreenSaver->get(KEY_MYTEXT).toString();
            }
            if (keys.contains(KEY_TEXT_IS_CENTER)) {
                m_bTextIsCenter = m_gsScreenSaver->get(KEY_TEXT_IS_CENTER).toBool();
            }
            if (keys.contains(KEY_SHOW_MESSAGE_ENABLED)) {
                m_bShowMessageEnable = m_gsScreenSaver->get(KEY_SHOW_MESSAGE_ENABLED).toBool();
            }
            if (keys.contains(KEY_MESSAGE_NUMBER)) {
                m_nMessageNum = m_gsScreenSaver->get(KEY_MESSAGE_NUMBER).toInt();
            }
            if (keys.contains(KEY_VIDEO_FORMAT)) {
                m_strVideoFormat = m_gsScreenSaver->get(KEY_VIDEO_FORMAT).toString();
            }
            if (keys.contains(KEY_VIDEO_PATH)) {
                m_strVideoPath = m_gsScreenSaver->get(KEY_VIDEO_PATH).toString();
            }
            if (keys.contains(KEY_VIDEO_SIZE)) {
                m_nVideoSize = m_gsScreenSaver->get(KEY_VIDEO_SIZE).toInt();
            }
            if (keys.contains(KEY_VIDEO_WIDTH)) {
                m_nVideoWidth = m_gsScreenSaver->get(KEY_VIDEO_WIDTH).toInt();
            }
            if (keys.contains(KEY_VIDEO_HEIGHT)) {
                m_nVideoHeight = m_gsScreenSaver->get(KEY_VIDEO_HEIGHT).toInt();
            }
        }
    }
    return (bool)(m_gsScreenSaver);
}

bool GSettingsHelper::initPowerManager()
{
    if (!m_gsPowerManager) {
        if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_POWER_MANAGER)) {
            m_gsPowerManager = new QGSettings(GSETTINGS_SCHEMA_POWER_MANAGER, "", this);
            connect(m_gsPowerManager, &QGSettings::changed, this, &GSettingsHelper::onPowerManagerConfigChanged);
            QStringList keys = m_gsPowerManager->keys();
            if (keys.contains(KEY_LOCK_SUSPEND)) {
                m_bLockSuspend = m_gsPowerManager->get(KEY_LOCK_SUSPEND).toBool();
            }
            if (keys.contains(KEY_LOCK_HIBERNATE)) {
                m_bLockHibernate = m_gsPowerManager->get(KEY_LOCK_HIBERNATE).toBool();
            }
            if (keys.contains(KEY_LOCK_BLANKSCREEN)) {
                m_bLockBlankScreen = m_gsPowerManager->get(KEY_LOCK_BLANKSCREEN).toBool();
            }
            if (keys.contains(KEY_SLEEP_COMPUTER_AC)) {
                m_nSleepComputeAc = m_gsPowerManager->get(KEY_SLEEP_COMPUTER_AC).toInt();
            }
            if (keys.contains(KEY_SLEEP_DISPLAY_AC)) {
                m_nSleepDisplayAc = m_gsPowerManager->get(KEY_SLEEP_DISPLAY_AC).toInt();
            }
            if (keys.contains(KEY_BUTTON_LID_AC)) {
                m_strButtonLidAc = m_gsPowerManager->get(KEY_BUTTON_LID_AC).toString();
            }
        }
    }
    return (bool)(m_gsPowerManager);
}

bool GSettingsHelper::initMateBg()
{
    if (!m_gsMateBg) {
        if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_MATE_BG)) {
            m_gsMateBg = new QGSettings(GSETTINGS_SCHEMA_MATE_BG, "", this);
            connect(m_gsMateBg, &QGSettings::changed, this, &GSettingsHelper::onMateBgConfigChanged);
            QStringList keys = m_gsMateBg->keys();
            if (keys.contains(KEY_PICTURE_FILENAME)) {
                m_strPicFileName = m_gsMateBg->get(KEY_PICTURE_FILENAME).toBool();
            }
            if (keys.contains(KEY_PICTURE_OPTIONS)) {
                m_strPicOptions = m_gsMateBg->get(KEY_PICTURE_OPTIONS).toBool();
            }
            if (keys.contains(KEY_PRIMARY_COLOR)) {
                m_strPrimaryColor = m_gsMateBg->get(KEY_PRIMARY_COLOR).toBool();
            }
        }
    }
    return (bool)(m_gsMateBg);
}

bool GSettingsHelper::initUkccPlugins()
{
    if (!m_gsUkccPlugins) {
        if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_TIME_TYPE)) {
            m_gsUkccPlugins = new QGSettings(GSETTINGS_SCHEMA_TIME_TYPE, "", this);
            connect(m_gsUkccPlugins, &QGSettings::changed, this, &GSettingsHelper::onUkccPluginsConfigChanged);
            QStringList keys = m_gsUkccPlugins->keys();
            if (keys.contains(KEY_HOUR_SYSTEM)) {
                m_nHourSystem = m_gsUkccPlugins->get(KEY_HOUR_SYSTEM).toInt();
            }
            if (keys.contains(KEY_DATE)) {
                m_strDateType = m_gsUkccPlugins->get(KEY_DATE).toString();
            }
        }
    }
    return (bool)(m_gsUkccPlugins);
}

bool GSettingsHelper::initThemeStyle()
{
    if (!m_gsThemeStyle) {
        if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_STYLE)) {
            m_gsThemeStyle = new QGSettings(GSETTINGS_SCHEMA_STYLE, "", this);
            connect(m_gsThemeStyle, &QGSettings::changed, this, &GSettingsHelper::onThemeStyleConfigChanged);
            QStringList keys = m_gsThemeStyle->keys();
            if (keys.contains(KEY_SYSTEM_FONT_SIZE)) {
                m_lfFontSize = m_gsThemeStyle->get(KEY_SYSTEM_FONT_SIZE).toDouble();
            }
            if (keys.contains(KEY_THEME_COLOR)) {
                m_strThemeColor = m_gsThemeStyle->get(KEY_THEME_COLOR).toString();
            }
            if (keys.contains(KEY_MENU_TRANSPARENCY)) {
                m_menuTransparency = m_gsThemeStyle->get(KEY_MENU_TRANSPARENCY).toInt();
            }
            if (keys.contains(KEY_STYLE_NAME)) {
                m_styleName = m_gsThemeStyle->get(KEY_STYLE_NAME).toString();
            }
            if (keys.contains(KEY_SYSTEM_FONT)) {
                m_font = m_gsThemeStyle->get(KEY_SYSTEM_FONT).toString();
            }
        }
    }
    return (bool)(m_gsThemeStyle);
}

bool GSettingsHelper::initSession()
{
    if (!m_gsSession) {
        if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_SESSION)) {
            m_gsSession = new QGSettings(GSETTINGS_SCHEMA_SESSION, "", this);
            connect(m_gsSession, &QGSettings::changed, this, &GSettingsHelper::onSessionConfigChanged);
            QStringList keys = m_gsSession->keys();
            if (keys.contains(KEY_SESSION_IDLE)) {
                m_nSessionIdle = m_gsSession->get(KEY_SESSION_IDLE).toInt();
            }
            if (keys.contains(KEY_SESSION_LOGOUT_MUSIC)) {
                m_nSessionLogout = m_gsSession->get(KEY_SESSION_LOGOUT_MUSIC).toBool();
            }
            if (keys.contains(KEY_SESSION_POWEROFF_MUSIC)) {
                m_nSessionPoweroff = m_gsSession->get(KEY_SESSION_POWEROFF_MUSIC).toBool();
            }
        }
    }
    return (bool)(m_gsSession);
}

bool GSettingsHelper::initKeyboard()
{
    if (!m_gsKeyboard) {
        if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_PERIPHERALS_KEYBOARD)) {
            m_gsKeyboard = new QGSettings(GSETTINGS_SCHEMA_PERIPHERALS_KEYBOARD, "", this);
            connect(m_gsKeyboard, &QGSettings::changed, this, &GSettingsHelper::onKeyboardConfigChanged);
            QStringList keys = m_gsKeyboard->keys();
            if (keys.contains(KEY_CAPSLOCK_STATUS)) {
                m_capsState = m_gsKeyboard->get(KEY_CAPSLOCK_STATUS).toBool();
            }
        }
    }
    return (bool)(m_gsKeyboard);
}

bool GSettingsHelper::initUsdMediaKeys()
{
    if (!m_gsUsdMediaKeys) {
        if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_MEDIAKEY_SCHEMA)) {
            m_gsUsdMediaKeys = new QGSettings(GSETTINGS_SCHEMA_MEDIAKEY_SCHEMA, "", this);
            connect(m_gsUsdMediaKeys, &QGSettings::changed, this, &GSettingsHelper::onUsdMediaKeysConfigChanged);
            QStringList keys = m_gsUsdMediaKeys->keys();
            if (keys.contains(KEY_AREA_SCREENSHOT)) {
                m_areaScreenShot = m_gsUsdMediaKeys->get(KEY_AREA_SCREENSHOT).toString();
            }
            if (keys.contains(KEY_AREA_SCREENSHOT2)) {
                m_areaScreenShot2 = m_gsUsdMediaKeys->get(KEY_AREA_SCREENSHOT2).toString();
            }
            if (keys.contains(KEY_SCREEN_SHOT)) {
                m_screenShot = m_gsUsdMediaKeys->get(KEY_SCREEN_SHOT).toString();
            }
            if (keys.contains(KEY_SCREEN_SHOT2)) {
                m_screenShot2 = m_gsUsdMediaKeys->get(KEY_SCREEN_SHOT2).toString();
            }
            if (keys.contains(KEY_WINDOW_SCREENSHOT)) {
                m_windowScreenshot = m_gsUsdMediaKeys->get(KEY_WINDOW_SCREENSHOT).toString();
            }
        }
    }
    return (bool)(m_gsUsdMediaKeys);
}

bool GSettingsHelper::initUsdMediaStateKeys()
{
    if (!m_gsUsdMediaStateKeys) {
        if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_MEDIAKEY_STATE_SCHEMA)) {
            m_gsUsdMediaStateKeys = new QGSettings(GSETTINGS_SCHEMA_MEDIAKEY_STATE_SCHEMA, "", this);
            connect(
                m_gsUsdMediaStateKeys, &QGSettings::changed, this, &GSettingsHelper::onUsdMediaStateKeysConfigChanged);
            QStringList keys = m_gsUsdMediaStateKeys->keys();
            if (keys.contains(KEY_RFKILL_STATE)) {
                m_rfkillState = m_gsUsdMediaStateKeys->get(KEY_RFKILL_STATE).toInt();
            }
        }
    }
    return (bool)(m_gsUsdMediaKeys);
}

void GSettingsHelper::onLockScreenConfigChanged(QString strKey)
{
    QVariant varValue;
    if (strKey == KEY_IDLE_DELAY) {
        varValue = m_nIdleDelay = m_gsLockScreen->get(KEY_IDLE_DELAY).toInt();
        Q_EMIT idleLockConfigChanged(m_nIdleDelay);
    } else if (strKey == KEY_IDLE_LOCK) {
        varValue = m_nIdleLock = m_gsLockScreen->get(KEY_IDLE_LOCK).toInt();
    } else if (strKey == KEY_IDLE_ACTIVATION_ENABLED) {
        varValue = m_bIdleActivationEnable = m_gsLockScreen->get(KEY_IDLE_ACTIVATION_ENABLED).toBool();
    } else if (strKey == KEY_IDLE_LOCK_ENABLED) {
        varValue = m_bIdleLockEnable = m_gsLockScreen->get(KEY_IDLE_LOCK_ENABLED).toBool();
    } else if (strKey == KEY_LOCK_TIMEOUT) {
        varValue = m_nLockTimeout = m_gsLockScreen->get(KEY_LOCK_TIMEOUT).toInt();
    } else if (strKey == KEY_CLOSE_ACTIVATION_ENABLED) {
        varValue = m_bCloseActivationEnable = m_gsLockScreen->get(KEY_CLOSE_ACTIVATION_ENABLED).toBool();
    } else if (strKey == KEY_SLEEP_ACTIVATION_ENABLED) {
        varValue = m_bSleepActivationEnable = m_gsLockScreen->get(KEY_SLEEP_ACTIVATION_ENABLED).toBool();
    } else if (strKey == KEY_LOCK_ENABLED) {
        varValue = m_bLockEnable = m_gsLockScreen->get(KEY_LOCK_ENABLED).toBool();
    } else if (strKey == KEY_BACKGROUND) {
        varValue = m_strBackground = m_gsLockScreen->get(KEY_BACKGROUND).toString();
    } else if (strKey == KEY_MODE) {
        varValue = m_saverMode = m_gsLockScreen->get(KEY_MODE).toString();
    } else if (strKey == KEY_THEMES) {
        varValue = m_saverTheme = m_gsLockScreen->get(KEY_THEMES).toStringList();
    } else if (strKey == KEY_IMAGE_TRANSITION_EFFECT) {
        varValue = m_imageTSEffect = m_gsLockScreen->get(KEY_IMAGE_TRANSITION_EFFECT).toInt();
    } else if (strKey == KEY_IMAGE_SWITCH_INTERVAL) {
        varValue = m_imageSwitchInterval = m_gsLockScreen->get(KEY_IMAGE_SWITCH_INTERVAL).toInt();
    }
    Q_EMIT lockScreenConfigChanged(strKey, varValue);
}

void GSettingsHelper::onScreenSaverConfigChanged(QString strKey)
{
    QVariant varValue;
    if (strKey == KEY_SHOW_REST_TIME) {
        varValue = m_bShowRestTime = m_gsScreenSaver->get(KEY_SHOW_REST_TIME).toBool();
    } else if (strKey == KEY_SHOW_CUSTOM_REST_TIME) {
        varValue = m_bShowCustomRestTime = m_gsScreenSaver->get(KEY_SHOW_CUSTOM_REST_TIME).toBool();
    } else if (strKey == KEY_SHOW_UKUI_REST_TIME) {
        varValue = m_bShowUkuiRestTime = m_gsScreenSaver->get(KEY_SHOW_UKUI_REST_TIME).toBool();
    } else if (strKey == KEY_CYCLE_TIME) {
        varValue = m_nCycleTime = m_gsScreenSaver->get(KEY_CYCLE_TIME).toInt();
    } else if (strKey == KEY_AUTOMATIC_SWITCHING_ENABLE) {
        varValue = m_bAutomaticSwitchingEnable = m_gsScreenSaver->get(KEY_AUTOMATIC_SWITCHING_ENABLE).toBool();
    } else if (strKey == KEY_BACKGROUND_PATH) {
        varValue = m_strBackgroundPath = m_gsScreenSaver->get(KEY_BACKGROUND_PATH).toString();
    } else if (strKey == KEY_MYTEXT) {
        varValue = m_strMyText = m_gsScreenSaver->get(KEY_MYTEXT).toString();
    } else if (strKey == KEY_TEXT_IS_CENTER) {
        varValue = m_bTextIsCenter = m_gsScreenSaver->get(KEY_TEXT_IS_CENTER).toBool();
    } else if (strKey == KEY_SHOW_MESSAGE_ENABLED) {
        varValue = m_bShowMessageEnable = m_gsScreenSaver->get(KEY_SHOW_MESSAGE_ENABLED).toBool();
    } else if (strKey == KEY_MESSAGE_NUMBER) {
        varValue = m_nMessageNum = m_gsScreenSaver->get(KEY_MESSAGE_NUMBER).toInt();
    } else if (strKey == KEY_VIDEO_FORMAT) {
        varValue = m_strVideoFormat = m_gsScreenSaver->get(KEY_VIDEO_FORMAT).toString();
    } else if (strKey == KEY_VIDEO_PATH) {
        varValue = m_strVideoPath = m_gsScreenSaver->get(KEY_VIDEO_PATH).toString();
    } else if (strKey == KEY_VIDEO_SIZE) {
        varValue = m_nVideoSize = m_gsScreenSaver->get(KEY_VIDEO_SIZE).toInt();
    } else if (strKey == KEY_VIDEO_WIDTH) {
        varValue = m_nVideoWidth = m_gsScreenSaver->get(KEY_VIDEO_WIDTH).toInt();
    } else if (strKey == KEY_VIDEO_HEIGHT) {
        varValue = m_nVideoHeight = m_gsScreenSaver->get(KEY_VIDEO_HEIGHT).toInt();
    }
    Q_EMIT screenSaverConfigChanged(strKey, varValue);
}

void GSettingsHelper::onPowerManagerConfigChanged(QString strKey)
{
    QVariant varValue;
    if (strKey == KEY_LOCK_SUSPEND) {
        varValue = m_bLockSuspend = m_gsPowerManager->get(KEY_LOCK_SUSPEND).toInt();
    } else if (strKey == KEY_LOCK_HIBERNATE) {
        varValue = m_bLockHibernate = m_gsPowerManager->get(KEY_LOCK_HIBERNATE).toInt();
    } else if (strKey == KEY_LOCK_BLANKSCREEN) {
        varValue = m_bLockBlankScreen = m_gsPowerManager->get(KEY_LOCK_BLANKSCREEN).toBool();
    } else if (strKey == KEY_SLEEP_COMPUTER_AC) {
        varValue = m_nSleepComputeAc = m_gsPowerManager->get(KEY_SLEEP_COMPUTER_AC).toBool();
    } else if (strKey == KEY_SLEEP_DISPLAY_AC) {
        varValue = m_nSleepDisplayAc = m_gsPowerManager->get(KEY_SLEEP_DISPLAY_AC).toInt();
    } else if (strKey == KEY_BUTTON_LID_AC) {
        varValue = m_strButtonLidAc = m_gsPowerManager->get(KEY_BUTTON_LID_AC).toBool();
    }
    Q_EMIT powerManagerConfigChanged(strKey, varValue);
}

void GSettingsHelper::onMateBgConfigChanged(QString strKey)
{
    QVariant varValue;
    if (strKey == KEY_PICTURE_FILENAME) {
        varValue = m_strPicFileName = m_gsPowerManager->get(KEY_PICTURE_FILENAME).toInt();
    } else if (strKey == KEY_PICTURE_OPTIONS) {
        varValue = m_strPicOptions = m_gsPowerManager->get(KEY_PICTURE_OPTIONS).toInt();
    } else if (strKey == KEY_PRIMARY_COLOR) {
        varValue = m_strPrimaryColor = m_gsPowerManager->get(KEY_PRIMARY_COLOR).toBool();
    }
    Q_EMIT mateBgConfigChanged(strKey, varValue);
}

void GSettingsHelper::onUkccPluginsConfigChanged(QString strKey)
{
    QVariant varValue;
    if (strKey == KEY_HOUR_SYSTEM) {
        varValue = m_nHourSystem = m_gsUkccPlugins->get(KEY_HOUR_SYSTEM).toInt();
    } else if (strKey == KEY_DATE) {
        varValue = m_strDateType = m_gsUkccPlugins->get(KEY_DATE).toString();
    }
    Q_EMIT ukccPluginsConfigChanged(strKey, varValue);
}

void GSettingsHelper::onThemeStyleConfigChanged(QString strKey)
{
    QVariant varValue;
    if (strKey == KEY_SYSTEM_FONT_SIZE) {
        varValue = m_lfFontSize = m_gsThemeStyle->get(KEY_SYSTEM_FONT_SIZE).toDouble();
    } else if (strKey == KEY_THEME_COLOR) {
        varValue = m_strThemeColor = m_gsThemeStyle->get(KEY_THEME_COLOR).toString();
    } else if (strKey == KEY_SYSTEM_FONT) {
        varValue = m_font = m_gsThemeStyle->get(KEY_SYSTEM_FONT).toString();
    } else if (strKey == KEY_MENU_TRANSPARENCY) {
        varValue = m_menuTransparency = m_gsThemeStyle->get(KEY_MENU_TRANSPARENCY).toInt();
    } else if (strKey == KEY_STYLE_NAME) {
        varValue = m_styleName = m_gsThemeStyle->get(KEY_STYLE_NAME).toString();
    }
    Q_EMIT themeStyleConfigChanged(strKey, varValue);
}

void GSettingsHelper::onSessionConfigChanged(QString strKey)
{
    QVariant varValue;
    if (strKey == KEY_SESSION_IDLE) {
        varValue = m_nSessionIdle = m_gsSession->get(KEY_SESSION_IDLE).toInt();
    } else if (strKey == KEY_SESSION_LOGOUT_MUSIC) {
        m_nSessionLogout = m_gsSession->get(KEY_SESSION_LOGOUT_MUSIC).toBool();
    } else if (strKey == KEY_SESSION_POWEROFF_MUSIC) {
        m_nSessionPoweroff = m_gsSession->get(KEY_SESSION_POWEROFF_MUSIC).toBool();
    }
    Q_EMIT sessionConfigChanged(strKey, varValue);
}

void GSettingsHelper::onKeyboardConfigChanged(QString strKey)
{
    QVariant varValue;
    if (strKey == KEY_CAPSLOCK_STATUS) {
        varValue = m_capsState = m_gsKeyboard->get(KEY_CAPSLOCK_STATUS).toBool();
    }
    Q_EMIT keyboardConfigChanged(strKey, varValue);
}

void GSettingsHelper::onUsdMediaKeysConfigChanged(QString strKey)
{
    QVariant varValue;
    if (strKey == KEY_AREA_SCREENSHOT) {
        varValue = m_areaScreenShot = m_gsUsdMediaKeys->get(KEY_AREA_SCREENSHOT).toString();
    } else if (strKey == KEY_AREA_SCREENSHOT2) {
        varValue = m_areaScreenShot2 = m_gsUsdMediaKeys->get(KEY_AREA_SCREENSHOT2).toString();
    } else if (strKey == KEY_SCREEN_SHOT) {
        varValue = m_screenShot = m_gsUsdMediaKeys->get(KEY_SCREEN_SHOT).toString();
    } else if (strKey == KEY_SCREEN_SHOT2) {
        varValue = m_screenShot2 = m_gsUsdMediaKeys->get(KEY_SCREEN_SHOT2).toString();
    } else if (strKey == KEY_WINDOW_SCREENSHOT) {
        varValue = m_windowScreenshot = m_gsUsdMediaKeys->get(KEY_WINDOW_SCREENSHOT).toString();
    }
    Q_EMIT usdMediaKeysConfigChanged(strKey, varValue);
}

void GSettingsHelper::onUsdMediaStateKeysConfigChanged(QString strKey)
{
    QVariant varValue;
    if (strKey == KEY_RFKILL_STATE) {
        varValue = m_rfkillState = m_gsUsdMediaStateKeys->get(KEY_RFKILL_STATE).toInt();
    }
    Q_EMIT usdMediaStateKeysConfigChanged(strKey, varValue);
}

QVariant GSettingsHelper::GetScreenSaverConf(QString strKey)
{
    QVariant varValue;
    if (initScreenSaver()) {
        if (strKey == KEY_SHOW_REST_TIME) {
            varValue = m_bShowRestTime;
        } else if (strKey == KEY_SHOW_CUSTOM_REST_TIME) {
            varValue = m_bShowCustomRestTime;
        } else if (strKey == KEY_SHOW_UKUI_REST_TIME) {
            varValue = m_bShowUkuiRestTime;
        } else if (strKey == KEY_CYCLE_TIME) {
            varValue = m_nCycleTime;
        } else if (strKey == KEY_AUTOMATIC_SWITCHING_ENABLE) {
            varValue = m_bAutomaticSwitchingEnable;
        } else if (strKey == KEY_BACKGROUND_PATH) {
            varValue = m_strBackgroundPath;
        } else if (strKey == KEY_MYTEXT) {
            varValue = m_strMyText;
        } else if (strKey == KEY_TEXT_IS_CENTER) {
            varValue = m_bTextIsCenter;
        } else if (strKey == KEY_SHOW_MESSAGE_ENABLED) {
            varValue = m_bShowMessageEnable;
        } else if (strKey == KEY_MESSAGE_NUMBER) {
            varValue = m_nMessageNum;
        } else if (strKey == KEY_VIDEO_FORMAT) {
            varValue = m_strVideoFormat;
        } else if (strKey == KEY_VIDEO_PATH) {
            varValue = m_strVideoPath;
        } else if (strKey == KEY_VIDEO_SIZE) {
            varValue = m_nVideoSize;
        } else if (strKey == KEY_VIDEO_WIDTH) {
            varValue = m_nVideoWidth;
        } else if (strKey == KEY_VIDEO_HEIGHT) {
            varValue = m_nVideoHeight;
        }
    }
    return varValue;
}

bool GSettingsHelper::SetScreenSaverConf(QString strKey, QVariant value)
{
    if (initScreenSaver()) {
        if (value.isValid()) {
            m_gsScreenSaver->set(strKey, value);
            return true;
        }
    }
    return false;
}

QVariant GSettingsHelper::ResetScreenSaverConf(QString strKey)
{
    QVariant defValue;
    if (initLockScreen()) {
        if (m_gsLockScreen->keys().contains(strKey)) {
            m_gsLockScreen->reset(strKey);
            defValue = m_gsLockScreen->get(strKey);
        }
    }
    return defValue;
}

QVariant GSettingsHelper::GetLockScreenConf(QString strKey)
{
    QVariant varValue;
    if (initLockScreen()) {
        if (strKey == KEY_IDLE_DELAY) {
            varValue = m_nIdleDelay;
        } else if (strKey == KEY_IDLE_LOCK) {
            varValue = m_nIdleLock;
        } else if (strKey == KEY_IDLE_ACTIVATION_ENABLED) {
            varValue = m_bIdleActivationEnable;
        } else if (strKey == KEY_IDLE_LOCK_ENABLED) {
            varValue = m_bIdleLockEnable;
        } else if (strKey == KEY_LOCK_TIMEOUT) {
            varValue = m_nLockTimeout;
        } else if (strKey == KEY_CLOSE_ACTIVATION_ENABLED) {
            varValue = m_bCloseActivationEnable;
        } else if (strKey == KEY_SLEEP_ACTIVATION_ENABLED) {
            varValue = m_bSleepActivationEnable;
        } else if (strKey == KEY_LOCK_ENABLED) {
            varValue = m_bLockEnable;
        } else if (strKey == KEY_BACKGROUND) {
            varValue = m_strBackground;
        } else if (strKey == KEY_MODE) {
            varValue = m_saverMode;
        } else if (strKey == KEY_THEMES) {
            varValue = m_saverTheme;
        } else if (strKey == KEY_IMAGE_TRANSITION_EFFECT) {
            varValue = m_imageTSEffect;
        } else if (strKey == KEY_IMAGE_SWITCH_INTERVAL) {
            varValue = m_imageSwitchInterval;
        }
    }
    return varValue;
}

bool GSettingsHelper::SetLockScreenConf(QString strKey, QVariant value)
{
    if (initLockScreen()) {
        if (value.isValid()) {
            m_gsLockScreen->set(strKey, value);
            return true;
        }
    }
    return false;
}

QVariant GSettingsHelper::ResetLockScreenConf(QString strKey)
{
    QVariant defValue;
    if (initLockScreen()) {
        if (m_gsLockScreen->keys().contains(strKey)) {
            m_gsLockScreen->reset(strKey);
            defValue = m_gsLockScreen->get(strKey);
        }
    }
    return defValue;
}

QVariant GSettingsHelper::GetPowerManagerConf(QString strKey)
{
    QVariant varValue;
    if (initPowerManager()) {
        if (strKey == KEY_LOCK_SUSPEND) {
            varValue = m_bLockSuspend;
        } else if (strKey == KEY_LOCK_HIBERNATE) {
            varValue = m_bLockHibernate;
        } else if (strKey == KEY_LOCK_BLANKSCREEN) {
            varValue = m_bLockBlankScreen;
        } else if (strKey == KEY_SLEEP_COMPUTER_AC) {
            varValue = m_nSleepComputeAc;
        } else if (strKey == KEY_SLEEP_DISPLAY_AC) {
            varValue = m_nSleepDisplayAc;
        } else if (strKey == KEY_BUTTON_LID_AC) {
            varValue = m_strButtonLidAc;
        }
    }
    return varValue;
}

bool GSettingsHelper::SetPowerManagerConf(QString strKey, QVariant value)
{
    if (initPowerManager()) {
        if (value.isValid()) {
            m_gsPowerManager->set(strKey, value);
            return true;
        }
    }
    return false;
}

QVariant GSettingsHelper::GetMateBgConf(QString strKey)
{
    QVariant varValue;
    if (initMateBg()) {
        if (strKey == KEY_PICTURE_FILENAME) {
            varValue = m_strPicFileName;
        } else if (strKey == KEY_PICTURE_OPTIONS) {
            varValue = m_strPicOptions;
        } else if (strKey == KEY_PRIMARY_COLOR) {
            varValue = m_strPrimaryColor;
        }
    }
    return varValue;
}

bool GSettingsHelper::SetMateBgConf(QString strKey, QVariant value)
{
    if (initMateBg()) {
        if (value.isValid()) {
            m_gsMateBg->set(strKey, value);
            return true;
        }
    }
    return false;
}

QVariant GSettingsHelper::GetUkccPluginsConf(QString strKey)
{
    QVariant varValue;
    if (initUkccPlugins()) {
        if (strKey == KEY_HOUR_SYSTEM) {
            varValue = m_nHourSystem;
        } else if (strKey == KEY_DATE) {
            varValue = m_strDateType;
        }
    }
    return varValue;
}

bool GSettingsHelper::SetUkccPluginsConf(QString strKey, QVariant value)
{
    if (initUkccPlugins()) {
        if (value.isValid()) {
            m_gsUkccPlugins->set(strKey, value);
            return true;
        }
    }
    return false;
}

QVariant GSettingsHelper::GetThemeStyleConf(QString strKey)
{
    QVariant varValue;
    if (initThemeStyle()) {
        if (strKey == KEY_SYSTEM_FONT_SIZE) {
            varValue = m_lfFontSize;
        } else if (strKey == KEY_THEME_COLOR) {
            varValue = m_strThemeColor;
        } else if (strKey == KEY_MENU_TRANSPARENCY) {
            varValue = m_menuTransparency;
        } else if (strKey == KEY_STYLE_NAME) {
            varValue = m_styleName;
        } else if (strKey == KEY_SYSTEM_FONT) {
            varValue = m_font;
        }
    }
    return varValue;
}

bool GSettingsHelper::SetThemeStyleConf(QString strKey, QVariant value)
{
    if (initThemeStyle()) {
        if (value.isValid()) {
            m_gsThemeStyle->set(strKey, value);
            return true;
        }
    }
    return false;
}

QVariant GSettingsHelper::GetSessionConf(QString strKey)
{
    QVariant varValue;
    if (initSession()) {
        if (strKey == KEY_SESSION_IDLE) {
            varValue = m_nSessionIdle;
        } else if (strKey == KEY_SESSION_LOGOUT_MUSIC) {
            varValue = m_nSessionLogout;
        } else if (strKey == KEY_SESSION_POWEROFF_MUSIC) {
            varValue = m_nSessionPoweroff;
        }
    }
    return varValue;
}

bool GSettingsHelper::SetSessionConf(QString strKey, QVariant value)
{
    if (initSession()) {
        if (value.isValid()) {
            m_gsSession->set(strKey, value);
            return true;
        }
    }
    return false;
}

QVariant GSettingsHelper::GetKeyboardConf(QString strKey)
{
    QVariant varValue;
    if (initKeyboard()) {
        if (strKey == KEY_CAPSLOCK_STATUS) {
            varValue = m_capsState;
        }
    }
    return varValue;
}

bool GSettingsHelper::SetKeyboardConf(QString strKey, QVariant value)
{
    if (initKeyboard()) {
        if (value.isValid()) {
            m_gsKeyboard->set(strKey, value);
            return true;
        }
    }
    return false;
}

QVariant GSettingsHelper::GetUsdMediaKeys(QString strKey)
{
    QVariant varValue;
    if (initUsdMediaKeys()) {
        if (strKey == KEY_AREA_SCREENSHOT) {
            varValue = m_areaScreenShot;
        } else if (strKey == KEY_AREA_SCREENSHOT2) {
            varValue = m_areaScreenShot2;
        } else if (strKey == KEY_SCREEN_SHOT) {
            varValue = m_screenShot;
        } else if (strKey == KEY_SCREEN_SHOT2) {
            varValue = m_screenShot2;
        } else if (strKey == KEY_WINDOW_SCREENSHOT) {
            varValue = m_windowScreenshot;
        }
    }
    return varValue;
}

QVariant GSettingsHelper::GetUsdMediaStateKeys(QString strKey)
{
    QVariant varValue;
    if (initUsdMediaStateKeys()) {
        if (strKey == KEY_RFKILL_STATE) {
            varValue = m_rfkillState;
        }
    }
    return varValue;
}

bool GSettingsHelper::SetUsdMediaStateKeys(QString strKey, QVariant value)
{
    if (initUsdMediaStateKeys()) {
        if (value.isValid()) {
            m_gsUsdMediaStateKeys->set(strKey, value);
            return true;
        }
    }
    return false;
}

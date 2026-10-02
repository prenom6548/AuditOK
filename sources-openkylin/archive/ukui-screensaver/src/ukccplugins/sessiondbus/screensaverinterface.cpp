/*
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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
#include "screensaverinterface.h"
#include "../../lock-backend/gsettingshelper.h"
#include "../../src/dbusifs/greeterconfig.h"


ScreensaverInterface::ScreensaverInterface(DbusUpperInterface *upperInterface, QObject *parent)
    : QObject(parent), m_upperInterface(upperInterface)
{
}

bool ScreensaverInterface::initData()
{
    if (!m_isInited) {
        mScreensaverType.clear();
        QString mode = m_upperInterface->getGSettingsInstance()->GetLockScreenConf(KEY_MODE).toString();
        if (mode == "default-ukui") {
            mScreensaverType = "ukui";
        } else if (mode == "default-ukui-custom") {
            mScreensaverType = "custom";
        } else {
            mScreensaverType = "other";
        }
        connect(
            m_upperInterface->getGSettingsInstance().get(),
            &GSettingsHelper::lockScreenConfigChanged,
            this, &ScreensaverInterface::onLockScreenConfigChanged);
        connect(m_upperInterface->getGSettingsInstance().get(),
                &GSettingsHelper::weatherKeysConfigChanged,
                this,
                [=](QString strKey, QVariant value) {
                    if (strKey == KEY_WEATHER) {
                        Q_EMIT changed("weatherData");
                    }
                });
        connect(m_upperInterface->getGSettingsInstance().get(),
                &GSettingsHelper::screenSaverConfigChanged,
            this, &ScreensaverInterface::onScreenSaverConfigChanged);
        connect(
                GreeterConfig::instance(),
                &GreeterConfig::screensaverDatetimeChanged,
                this,
                    [=](bool isEnable, QString strUserName) {
                        if (strUserName == QString(getenv("USER"))) {
                            Q_EMIT changed("saverDatetimeEnable");
                        }
                    });
        m_isInited = true;
    }
    return m_isInited;
}

ScreensaverInterface::~ScreensaverInterface() {}

void ScreensaverInterface::onScreenSaverConfigChanged(QString strKey, QVariant value)
{
    if (strKey == KEY_MODE) {
        Q_EMIT changed("screensaverType");
    } else if (strKey == KEY_SUB_MODE) {
        Q_EMIT changed("subMode");
    } else if (strKey == KEY_BACKGROUND_PATH) {
        Q_EMIT changed("customPath");
    } else if (strKey == KEY_AUTOMATIC_SWITCHING_ENABLE) {
        Q_EMIT changed("switchRandom");
    } else if (strKey == KEY_CYCLE_TIME) {
        Q_EMIT changed("cycleTime");
    } else if (strKey == KEY_MYTEXT) {
        Q_EMIT changed("customText");
    } else if (strKey == KEY_TEXT_IS_CENTER) {
        Q_EMIT changed("customTextCentered");
    } else if (strKey == KEY_MUSIC_FOLLOW_SYSTEM) {
        Q_EMIT changed("followSystem");
    } else if (strKey == KEY_MUSIC_PATH) {
        Q_EMIT changed("musicPath");
    } else if (strKey == KEY_PHOTO_CUSTOM_PATH) {
        Q_EMIT changed("photoPath");
    } else if (strKey == KEY_PHOTO_SELECT_PATH) {
        Q_EMIT changed("photoSelectPath");
    } else if (strKey == KEY_PHOTO_CYCLE_MODE) {
        Q_EMIT changed("photoCycleMode");
    } else if (strKey == KEY_PHOTO_CYCLE_INTERVAL) {
        Q_EMIT changed("photoCycleInterval");
    }
}

void ScreensaverInterface::onLockScreenConfigChanged(QString strKey, QVariant value)
{
    if (strKey == KEY_MODE) {
        QString mode = value.toString();
        if (mode == "default-ukui") {
            mScreensaverType = "ukui";
        } else if (mode == "default-ukui-custom") {
            mScreensaverType = "custom";
        } else {
            mScreensaverType = "other";
        }
    } else if (strKey == KEY_BACKGROUND) {
        Q_EMIT changed("background");
    } else if (strKey == KEY_LOCK_ENABLED) {
        Q_EMIT changed("screenLockEnabled");
    } else if (strKey == KEY_IDLE_DELAY) {
        Q_EMIT changed("preentryTime");
    }
}

int ScreensaverInterface::getPreentryTime()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetLockScreenConf(KEY_IDLE_DELAY).toInt();
}

void ScreensaverInterface::setPreentryTime(int time)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetLockScreenConf(KEY_IDLE_DELAY, time);
    Q_EMIT changed("preentryTime");
}

QString ScreensaverInterface::getScreensaverType()
{
    initData();
    return mScreensaverType;
}

void ScreensaverInterface::setScreensaverType(QString type)
{
    initData();
    if (type == "ukui") {
        m_upperInterface->getGSettingsInstance()->SetLockScreenConf(KEY_MODE, "default-ukui");
    } else if (type == "custom") {
        m_upperInterface->getGSettingsInstance()->SetLockScreenConf(KEY_MODE, "default-ukui-custom");
    }
    Q_EMIT changed("screensaverType");
}

bool ScreensaverInterface::getShowBreakTimeUkui()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_SHOW_UKUI_REST_TIME).toBool();
}

void ScreensaverInterface::setShowBreakTimeUkui(bool b)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_SHOW_UKUI_REST_TIME, b);
    Q_EMIT changed("showBreakTimeUkui");
}

bool ScreensaverInterface::getShowBreakTimeCustom()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_SHOW_CUSTOM_REST_TIME).toBool();
}

void ScreensaverInterface::setShowBreakTimeCustom(bool b)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_SHOW_CUSTOM_REST_TIME, b);
    Q_EMIT changed("showBreakTimeCustom");
}

bool ScreensaverInterface::getScreenLockEnabled()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetLockScreenConf(KEY_LOCK_ENABLED).toBool();
}

void ScreensaverInterface::setScreenLockEnabled(bool b)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetLockScreenConf(KEY_LOCK_ENABLED, b);
    Q_EMIT changed("screenLockEnabled");
}

QString ScreensaverInterface::getcustomPath()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_BACKGROUND_PATH).toString();
}

void ScreensaverInterface::setCustomPath(QString path)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_BACKGROUND_PATH, path);
    Q_EMIT changed("customPath");
}

bool ScreensaverInterface::getSwitchRandom()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_AUTOMATIC_SWITCHING_ENABLE).toBool();
}

void ScreensaverInterface::setSwitchRandom(bool b)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_AUTOMATIC_SWITCHING_ENABLE, b);
    Q_EMIT changed("switchRandom");
}

int ScreensaverInterface::getCycleTime()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_CYCLE_TIME).toInt();
}

void ScreensaverInterface::setCycleTime(int time)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_CYCLE_TIME, time);
    Q_EMIT changed("cycleTime");
}

QString ScreensaverInterface::getCustomText()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_MYTEXT).toString();
}

void ScreensaverInterface::setCustomText(QString text)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_MYTEXT, text);
    Q_EMIT changed("customText");
}

bool ScreensaverInterface::getCustomTextCentered()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_TEXT_IS_CENTER).toBool();
}

void ScreensaverInterface::setCustomTextCentered(bool b)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_TEXT_IS_CENTER, b);
    Q_EMIT changed("customTextCentered");
}

QString ScreensaverInterface::getBackground()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetLockScreenConf(KEY_BACKGROUND).toString();
}

int ScreensaverInterface::getSubMode()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_SUB_MODE).toInt();
}

bool ScreensaverInterface::getMusicFollowSystem()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_MUSIC_FOLLOW_SYSTEM).toBool();
}

QString ScreensaverInterface::getMusicPath()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_MUSIC_PATH).toString();
}

QStringList ScreensaverInterface::getPhotoPath()
{
    initData();
    QStringList defaultPath = m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_PHOTO_DEFAULT_PATH).toStringList();
    QStringList customPath = m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_PHOTO_CUSTOM_PATH).toStringList();
    QSet<QString> set;
    for (const QString &str : customPath) {
        set.insert(str);
    }
    for (const QString &str : defaultPath) {
        set.insert(str);
    }
    QStringList allPhotoPath = set.values();
    qDebug() << "allList = " << allPhotoPath;
    return allPhotoPath;
}

QString ScreensaverInterface::getPhotoSelectPath()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_PHOTO_SELECT_PATH).toString();
}

int ScreensaverInterface::getPhotoCycleMode()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_PHOTO_CYCLE_MODE).toInt();
}

int ScreensaverInterface::getPhotoCycleInterval()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_PHOTO_CYCLE_INTERVAL).toInt();
}

QString ScreensaverInterface::getWeatherDate()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetWeatherKeys(KEY_WEATHER).toString();
}

QStringList ScreensaverInterface::getPreviewWallpapers()
{
    initData();
    QDBusInterface interface(
        "org.ukui.ukcc.session", "/Wallpaper", "org.ukui.ukcc.session.Wallpaper", QDBusConnection::sessionBus(), this);
    if (interface.isValid()) {
        return interface.property("previewWallpapers").toStringList();
    }
    return QStringList();
}

QStringList ScreensaverInterface::getSourceWallpapers()
{
    QDBusInterface interface(
        "org.ukui.ukcc.session", "/Wallpaper", "org.ukui.ukcc.session.Wallpaper", QDBusConnection::sessionBus(), this);
    if (interface.isValid()) {
        return interface.property("sourceWallpapers").toStringList();
    }
    return QStringList();
}

bool ScreensaverInterface::getSaverDatetimeEnable()
{
    initData();
    return m_upperInterface->GetSaverDatetimeEnable();
}

QString ScreensaverInterface::getSaverVideoFormat()
{
    initData();
    return m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_VIDEO_FORMAT).toString();
}

void ScreensaverInterface::setSubMode(int mode)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_SUB_MODE, mode);
    Q_EMIT changed("subMode");
}

void ScreensaverInterface::setMusicPath(QString path)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_MUSIC_PATH, path);
    Q_EMIT changed("musicPath");
}

void ScreensaverInterface::setMusicFollowSystem(bool b)
{
    initData();
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_MUSIC_FOLLOW_SYSTEM, b);
    Q_EMIT changed("musicFollowSystem");
}

void ScreensaverInterface::setPhotoCustomPath(QString path)
{
    initData();
    QStringList customPath = m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_PHOTO_CUSTOM_PATH).toStringList();
    QSet<QString> set;
    for (const QString &str : customPath) {
        set.insert(str);
    }
    set.insert(path);
    QStringList allPhotoPath = set.values();
    qDebug() << "allList = " << allPhotoPath;
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_PHOTO_CUSTOM_PATH, allPhotoPath);
    Q_EMIT changed("photoCustomPath");
}

void ScreensaverInterface::removePhotoPath(QString path)
{
    initData();
    QString selectPath = getPhotoSelectPath();
    if (selectPath == path) {
        setSelectPhotoPath(DEFAULT_PHOTO_PATH);
    }

    QStringList customPath = m_upperInterface->getGSettingsInstance()->GetScreenSaverConf(KEY_PHOTO_CUSTOM_PATH).toStringList();
    QSet<QString> set;
    for (const QString &str : customPath) {
        set.insert(str);
    }
    set.remove(path);
    QStringList allPhotoPath = set.values();
    qDebug() << "allList = " << allPhotoPath;
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_PHOTO_CUSTOM_PATH, allPhotoPath);
    Q_EMIT changed("photoCustomPath");
}

void ScreensaverInterface::setPhotoCycleMode(int mode)
{
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_PHOTO_CYCLE_MODE, mode);
    Q_EMIT changed("photoCycleMode");
}

void ScreensaverInterface::setPhotoCycleInterval(int time)
{
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_PHOTO_CYCLE_INTERVAL, time);
    Q_EMIT changed("photoCycleInterval");
}

void ScreensaverInterface::setSelectPhotoPath(QString path)
{
    m_upperInterface->getGSettingsInstance()->SetScreenSaverConf(KEY_PHOTO_SELECT_PATH, path);
    Q_EMIT changed("photoSelectPath");
}

void ScreensaverInterface::openFileFolder(QString path)
{
    QStringList paths;
    paths.append(path);
    m_upperInterface->ShowFolder(paths);
    Q_EMIT changed("photoSelectPath");
}

void ScreensaverInterface::setSaverDatetimeEnable(bool enable)
{
    m_upperInterface->SetSaverDatetimeEnable(enable);
    Q_EMIT changed("saverDatetimeEnable");
}

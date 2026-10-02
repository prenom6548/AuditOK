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
            this,
            [=](QString strKey, QVariant value) {
                if (strKey == KEY_MODE) {
                    QString mode = value.toString();
                    if (mode == "default-ukui") {
                        mScreensaverType = "ukui";
                    } else if (mode == "default-ukui-custom") {
                        mScreensaverType = "custom";
                    } else {
                        mScreensaverType = "other";
                    }
                }
            });
        m_isInited = true;
    }
    return m_isInited;
}

ScreensaverInterface::~ScreensaverInterface() {}

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



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
#ifndef SCREENSAVERINTERFACE_H
#define SCREENSAVERINTERFACE_H

#include <QObject>
#include <QtDBus>
#include "../../lock-backend/dbusupperinterface.h"

class ScreensaverInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ScreenSaver.session.Screensaver")
    Q_PROPERTY(int preentryTime READ getPreentryTime)
    Q_PROPERTY(QString screensaverType READ getScreensaverType)
    Q_PROPERTY(bool showBreakTimeUkui READ getShowBreakTimeUkui)
    Q_PROPERTY(bool showBreakTimeCustom READ getShowBreakTimeCustom)
    Q_PROPERTY(bool screenLockEnabled READ getScreenLockEnabled)
    Q_PROPERTY(QString customPath READ getcustomPath)
    Q_PROPERTY(bool switchRandom READ getSwitchRandom)
    Q_PROPERTY(int cycleTime READ getCycleTime)
    Q_PROPERTY(QString customText READ getCustomText)
    Q_PROPERTY(bool customTextCentered READ getCustomTextCentered)
    Q_PROPERTY(QString background READ getBackground)
    Q_PROPERTY(int subMode READ getSubMode)
    Q_PROPERTY(bool followSystem READ getMusicFollowSystem)
    Q_PROPERTY(QString musicPath READ getMusicPath)
    Q_PROPERTY(QStringList photoPath READ getPhotoPath)
    Q_PROPERTY(QString photoSelectPath READ getPhotoSelectPath)
    Q_PROPERTY(int photoCycleMode READ getPhotoCycleMode)
    Q_PROPERTY(int photoCycleInterval READ getPhotoCycleInterval)
    Q_PROPERTY(QString weatherData READ getWeatherDate)
    Q_PROPERTY(QStringList previewWallpapers READ getPreviewWallpapers)
    Q_PROPERTY(QStringList sourceWallpapers READ getSourceWallpapers)
    Q_PROPERTY(bool saverDatetimeEnable READ getSaverDatetimeEnable)
    Q_PROPERTY(QString videoFormat READ getSaverVideoFormat)

public:
    ScreensaverInterface(DbusUpperInterface *upperInterface, QObject *parent = nullptr);
    ~ScreensaverInterface();
    int getPreentryTime();
    QString getScreensaverType();
    bool getShowBreakTimeUkui();
    bool getShowBreakTimeCustom();
    bool getScreenLockEnabled();
    QString getcustomPath();
    bool getSwitchRandom();
    int getCycleTime();
    QString getCustomText();
    bool getCustomTextCentered();
    QString getBackground();
    int getSubMode();
    bool getMusicFollowSystem();
    QString getMusicPath();
    QStringList getPhotoPath();
    QString getPhotoSelectPath();
    int getPhotoCycleMode();
    int getPhotoCycleInterval();
    QString getWeatherDate();
    QStringList getPreviewWallpapers();
    QStringList getSourceWallpapers();
    bool getSaverDatetimeEnable();
    QString getSaverVideoFormat();

private:
    bool initData();

private:
    QString mScreensaverType;
    DbusUpperInterface *m_upperInterface = nullptr;
    bool m_isInited = false;

public Q_SLOTS:
    void setPreentryTime(int time);
    void setScreensaverType(QString type);
    void setShowBreakTimeUkui(bool b);
    void setShowBreakTimeCustom(bool b);
    void setScreenLockEnabled(bool b);
    void setCustomPath(QString path);
    void setSwitchRandom(bool b);
    void setCycleTime(int time);
    void setCustomText(QString Text);
    void setCustomTextCentered(bool b);
    void setSubMode(int mode);
    void setMusicFollowSystem(bool b);
    void setMusicPath(QString path);
    void setPhotoCustomPath(QString path);
    void setPhotoCycleMode(int mode);
    void setPhotoCycleInterval(int time);
    void setSelectPhotoPath(QString path);
    void removePhotoPath(QString path);
    void openFileFolder(QString path);
    void setSaverDatetimeEnable(bool enable);

private Q_SLOTS:
    void onLockScreenConfigChanged(QString strKey, QVariant value);
    void onScreenSaverConfigChanged(QString strKey, QVariant value);
    
Q_SIGNALS:
    void changed(QString key);
};

#endif // SCREENSAVERINTERFACE_H

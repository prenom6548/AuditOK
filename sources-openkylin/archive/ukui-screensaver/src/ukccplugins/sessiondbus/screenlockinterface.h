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
#ifndef SCREENLOCKINTERFACE_H
#define SCREENLOCKINTERFACE_H

#include <QObject>
#include <QtDBus>
#include "../../lock-backend/dbusupperinterface.h"

class ScreenlockInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ScreenSaver.session.Screenlock")
    Q_PROPERTY(bool showOnLogin READ getShowOnLogin)
    Q_PROPERTY(QString wallpaper READ getWallpaper)
    Q_PROPERTY(QStringList previewWallpapers READ getPreviewWallpapers)
    Q_PROPERTY(QStringList sourceWallpapers READ getSourceWallpapers)
    Q_PROPERTY(bool lockDatetimeEnable READ getLockDatetimeEnable)

public:
    ScreenlockInterface(DbusUpperInterface *upperInterface, QObject *parent = nullptr);
    ~ScreenlockInterface();
    bool getShowOnLogin();
    QString getWallpaper();
    QStringList getPreviewWallpapers();
    QStringList getSourceWallpapers();
    bool getLockDatetimeEnable();

public Q_SLOTS:
    void setShowOnLogin(bool b);
    void setWallpaper(QString f);
    void resetDefault();
    void setLockDatetimeEnable(bool enable);

private:
    QString copyLoginFile(QString fileName);
    bool initData();

private:
    QStringList previewWallpaperList;
    QStringList sourceWallpaperList;
    QString     mUKCConfig     = "";
    QSettings   *lockSetting   = nullptr;
    DbusUpperInterface *m_upperInterface = nullptr;
    bool m_isInited = false;

Q_SIGNALS:
    void changed(QString key);
};

#endif // SCREENLOCKINTERFACE_H

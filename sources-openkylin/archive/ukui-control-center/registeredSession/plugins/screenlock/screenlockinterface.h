/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SCREENLOCKINTERFACE_H
#define SCREENLOCKINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QtDBus>
#include <QGSettings>

class ScreenlockInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Screenlock")
    Q_PROPERTY(bool showOnLogin READ getShowOnLogin)
    Q_PROPERTY(QString wallpaper READ getWallpaper)
    Q_PROPERTY(QStringList previewWallpapers READ getPreviewWallpapers)
    Q_PROPERTY(QStringList sourceWallpapers READ getSourceWallpapers)

public:
    ScreenlockInterface();
    ~ScreenlockInterface();
    bool getShowOnLogin();
    QString getWallpaper();
    QStringList getPreviewWallpapers();
    QStringList getSourceWallpapers();

public Q_SLOTS:
    void setShowOnLogin(bool b);
    void setWallpaper(QString f);
    void resetDefault();

private:
    QString copyLoginFile(QString fileName);

private:
    QStringList previewWallpaperList;
    QStringList sourceWallpaperList;
    QString     mUKCConfig     = "";
    QSettings   *lockSetting   = nullptr;
    QGSettings  *lockGSettings = nullptr;

Q_SIGNALS:
    void changed(QString key);
};

#endif // SCREENLOCKINTERFACE_H

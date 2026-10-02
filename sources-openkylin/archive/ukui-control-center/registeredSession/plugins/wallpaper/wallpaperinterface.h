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
#ifndef WALLPAPERINTERFACE_H
#define WALLPAPERINTERFACE_H

#include "../../universalinterface.h"
#include "workerobject.h"
#include <ukui-service-interface/serviceobject.h>
#include <QtDBus>

class WallpaperInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Wallpaper")
    Q_PROPERTY(QString wallpaperMode READ getWallpaperMode)
    Q_PROPERTY(QString displayType READ getDisplayType)
    Q_PROPERTY(QString wallpaper READ getWallpaper)
    Q_PROPERTY(QStringList previewWallpapers READ getPreviewWallpapers)
    Q_PROPERTY(QStringList sourceWallpapers READ getSourceWallpapers)

public:
    WallpaperInterface();
    ~WallpaperInterface();
    QString getWallpaperMode(); // 背景
    QString getDisplayType();  // 显示方式
    QString getWallpaper();
    QStringList getPreviewWallpapers();
    QStringList getSourceWallpapers();

public Q_SLOTS:
    void setDisplayType(QString type);
    void setWallpaper(QString mode, QString data);
    void resetDefault();
    void setNeedChangeCustomTheme(bool status);

private:
    void changeGlobalTheme();
    bool validateDisplayType(const QString &type);
    bool validateWallpaperPicturePath(const QString &path, QString *canonicalPath);
    bool validateWallpaperColor(const QString &color);
    QStringList allowedWallpaperDirPrefixes() const;
    bool isSpecialFsPath(const QString &path) const;
    bool isPathUnderPrefix(const QString &canonicalPath, const QString &prefix) const;

private:
    QStringList previewWallpaperList;
    QStringList sourceWallpaperList;
    QString mWallpaperMode;
    QString mDisplayType;
    QString mWallpaper;
    bool m_needChangeCustomTheme = true;

Q_SIGNALS:
    void changed(QString key);
};

#endif // WALLPAPERINTERFACE_H

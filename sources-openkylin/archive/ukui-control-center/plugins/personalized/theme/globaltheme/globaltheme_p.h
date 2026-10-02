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
#ifndef GLOBALTHEME_P_H
#define GLOBALTHEME_P_H

#include <QObject>
#include <QMap>
#include <QPixmap>

class QSettings;

class GlobalThemePrivate
{
public:
    GlobalThemePrivate() {}

    QString themeName;
    QString localeThemeName;

    bool supportLightDarkMode = true;
    QString defaultLightDarkMode = "light";

    QString widgetThemeName = "default";
    QString qtLightThemeName = "ukui-light";
    QString qtDarkThemeName = "ukui-dark";
    QString qtMixedThemeName = "ukui-default";
    QString qtAccentRole = "daybreakBlue";

    QString gtkThemeName = "ukui-white";
    QString gtkLightThemeName = "ukui-white";
    QString gtkDarkThemeName = "ukui-black";
    QString gtkAccent = "#3790FA";

    QString iconThemeName = "ukui-icon-theme-default";
    QString cursorThemeName = "DMZ-White";
    QString wallPaperPath = "/usr/share/backgrounds/ubuntukylin-default-settings.jpg";

    QString soundThemeName;

    bool supportBlur = true;
    bool blurEnabled = true;

    bool supportTransparency = true;
    int transparencyBlur = 50;
    int transparencyNoBlur = 90;

    bool supportAnimation = true;
    int animationDuration = 150;

    bool supportRadius = true;
    QString  radiusValue = "12,8";

    QPixmap thumbnail;

    QString realPath;
    QSettings *settings = nullptr;
};

#endif // GLOBALTHEME_P_H

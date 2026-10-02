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
#ifndef GLOBALTHEMEHELPER_P_H
#define GLOBALTHEMEHELPER_P_H

#include <QObject>
#include <QMap>

class GlobalTheme;
class QGSettings;

class GlobalThemeHelperPrivate
{
public:
    GlobalThemeHelperPrivate() {}
    void initThemes();

    // info
    QString getCurrentThemeName();
    bool setCurrentTheme(const QString &theme);

    bool getCurrentThemeModified();

    // color schemes
    bool getThemeSupportLightDarkMode(const QString &theme);

    // qt
    QString getThemeWidgetStyleName(const QString &theme);

    // gtk
    QString getThemeGtkStyleName(const QString &theme);

    // xdg
    QString getThemeIconThemeName(const QString &theme);
    QString getThemeCursorThemeName(const QString &theme);

    // wallpaper
    QString getThemeWallPaper(const QString &theme);

    // effect
    bool getThemeSupportBlur(const QString &theme);
    bool getThemeSupportTransparency(const QString &theme);

    // animation
    bool getThemeSupportAnimation(const QString &theme);

    QMap<QString, GlobalTheme *> globalThemes;
    QGSettings *settings = nullptr;
};

class Thumbnailer : public QObject
{
    Q_OBJECT
public:
    explicit Thumbnailer(GlobalTheme *theme);

Q_SIGNALS:
    void finished(const QPixmap &pixmap);

public Q_SLOTS:
    void doThumbnail();

private:
    QString picturePath;
};

#endif // GLOBALTHEMEHELPER_P_H

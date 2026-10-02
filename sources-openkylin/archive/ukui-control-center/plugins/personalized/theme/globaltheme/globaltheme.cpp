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
#include "globaltheme.h"
#include "globaltheme_p.h"

#include "globalthemehelper.h"

#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QPixmap>
#include <QLocale>
#include <QTimer>
#include <QTextCodec>

#include <QFileSystemWatcher>

#include <QDebug>
#include "libkypackages.h"

#define SYSTEM_THEME_DIR QString("/usr/share/config/globaltheme/")
#define USER_THEME_DIR QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).arg("globaltheme/")

GlobalTheme::GlobalTheme(const QString &theme, QObject *parent) : QObject(parent)
{
    d_ptr = new GlobalThemePrivate;

    d_ptr->themeName = theme;
    if (theme.isEmpty()) {
        // get current theme
        d_ptr->themeName = "Light-Seeking";
    }

    /// 如果存在重名，优先使用用户目录下的主题，其次使用系统目录下的主题
    QString userPath = USER_THEME_DIR + d_ptr->themeName + "/theme.conf";
    QString systemPath = SYSTEM_THEME_DIR + d_ptr->themeName + "/theme.conf";
    char** themePaths = kdk_package_get_standard_path("THEME_CONF_PATH");
    bool hitPathEnv = false;
    if (themePaths != nullptr) {
        for (int idx = 0; themePaths[idx] != nullptr; idx++) {
            qDebug() << "kdk_package_get_standard_path path index" << idx << "theme path" << themePaths[idx];
            QString globalThemePath = QString(themePaths[idx]);
            QDir globalThemeDir = QDir(globalThemePath);
            if (!globalThemeDir.exists()) {
                qDebug() << Q_FUNC_INFO << "global theme path not exist, skip" << globalThemeDir;
                continue;
            }
            QString themePath = globalThemePath + "/" + d_ptr->themeName + "/theme.conf";
            if (QFile::exists(themePath)) {
                hitPathEnv = true;
                d_ptr->settings = new QSettings(themePath, QSettings::NativeFormat, this);
                d_ptr->realPath = themePath;
                d_ptr->realPath.remove("theme.conf");
                qDebug() << Q_FUNC_INFO << "kdk_package_get_standard_path theme.conf" << theme << themePath;
                break;
            }
        }
    }
    if (!hitPathEnv) {
        qDebug() << Q_FUNC_INFO << "kdk_package_get_standard_path not get theme.conf" << theme;
        if (QFile::exists(userPath)) {
            d_ptr->settings = new QSettings(userPath, QSettings::NativeFormat, this);
            d_ptr->realPath = userPath;
            d_ptr->realPath.remove("theme.conf");
        } else if (QFile::exists(systemPath)) {
            d_ptr->settings = new QSettings(systemPath, QSettings::NativeFormat, this);
            d_ptr->realPath = systemPath;
            d_ptr->realPath.remove("theme.conf");
        } else {
            qWarning()<<"can not find global theme"<<d_ptr->themeName;
            d_ptr->settings = new QSettings(userPath, QSettings::NativeFormat, this);
        }
    }
    d_ptr->settings->setIniCodec(QTextCodec::codecForName("utf-8"));

    QVariant var;

    // generic
    d_ptr->settings->beginGroup("generic");
    var = d_ptr->settings->value(QString("Name[%1]").arg(QLocale::system().name()));
    if (var.isValid()) {
        d_ptr->localeThemeName = var.toString();
    } else {
        d_ptr->localeThemeName = d_ptr->themeName;
    }
    d_ptr->settings->endGroup();

    // color-schemes
    d_ptr->settings->beginGroup("color-schemes");
    var = d_ptr->settings->value("supportLightDarkMode");
    if (var.isValid()) {
        d_ptr->supportLightDarkMode = var.toBool();
    }
    var = d_ptr->settings->value("defaultLightDarkMode");
    if (var.isValid()) {
        d_ptr->defaultLightDarkMode = var.toString();
    }
    d_ptr->settings->endGroup();

    // qt
    d_ptr->settings->beginGroup("qt");
    var = d_ptr->settings->value("widgetThemeName");
    if (var.isValid()) {
        d_ptr->widgetThemeName = var.toString();
    }
    var = d_ptr->settings->value("darkThemeName");
    if (var.isValid()) {
        d_ptr->qtDarkThemeName = var.toString();
    }
    var = d_ptr->settings->value("lightThemeName");
    if (var.isValid()) {
        d_ptr->qtLightThemeName = var.toString();
    }
    var = d_ptr->settings->value("accentRole");
    if (var.isValid()) {
        d_ptr->qtAccentRole = var.toString();
    }
    var = d_ptr->settings->value("mixedThemeName");
    if (var.isValid()) {
        d_ptr->qtMixedThemeName = var.toString();
    }
    d_ptr->settings->endGroup();

    // gtk
    d_ptr->settings->beginGroup("gtk");
    var = d_ptr->settings->value("themeName");
    if (var.isValid()) {
        d_ptr->gtkThemeName = var.toString();
    }
    var = d_ptr->settings->value("lightThemeName");
    if (var.isValid()) {
        d_ptr->gtkLightThemeName = var.toString();
    }
    var = d_ptr->settings->value("darkThemeName");
    if (var.isValid()) {
        d_ptr->gtkDarkThemeName = var.toString();
    }
    var = d_ptr->settings->value("accent");
    if (var.isValid()) {
        d_ptr->gtkAccent = var.toString();
    }
    d_ptr->settings->endGroup();

    // xdg
    d_ptr->settings->beginGroup("xdg");
    var = d_ptr->settings->value("iconThemeName");
    if (var.isValid()) {
        d_ptr->iconThemeName = var.toString();
    }
    var = d_ptr->settings->value("cursorThemeName");
    if (var.isValid()) {
        d_ptr->cursorThemeName = var.toString();
    }
    var = d_ptr->settings->value("soundThemeName");
    if (var.isValid()) {
        d_ptr->soundThemeName = var.toString();
    }
    d_ptr->settings->endGroup();

    // wallpaper
    d_ptr->settings->beginGroup("wallpaper");
    var = d_ptr->settings->value("wallPaperPath");
    if (var.isValid()) {
        d_ptr->wallPaperPath = var.toString();
    }
    d_ptr->settings->endGroup();

    // effect
    d_ptr->settings->beginGroup("effect");
    var = d_ptr->settings->value("supportBlur");
    if (var.isValid()) {
        d_ptr->supportBlur = var.toBool();
    }
    var = d_ptr->settings->value("blurEnabled");
    if (var.isValid()) {
        d_ptr->blurEnabled = var.toBool();
    }
    var = d_ptr->settings->value("supportTransparency");
    if (var.isValid()) {
        d_ptr->supportTransparency = var.toBool();
    }
    var = d_ptr->settings->value("transparencyBlur");
    if (var.isValid()) {
        d_ptr->transparencyBlur = var.toInt();
    }
    var = d_ptr->settings->value("transparencyNoBlur");
    if (var.isValid()) {
        d_ptr->transparencyNoBlur = var.toInt();
    }
    var = d_ptr->settings->value("supportAnimation");
    if (var.isValid()) {
        d_ptr->supportAnimation = var.toBool();
    }
    var = d_ptr->settings->value("animationDuration");
    if (var.isValid()) {
        d_ptr->animationDuration = var.toInt();
    }
    var = d_ptr->settings->value("windowRadius");
    if (var.isValid()) {
        d_ptr->radiusValue = var.toString();
    }
    d_ptr->settings->endGroup();

    if (realPath().startsWith("/usr")) {
        auto fsWatcher = new QFileSystemWatcher(this);
        fsWatcher->addPath(SYSTEM_THEME_DIR);
        connect(fsWatcher, &QFileSystemWatcher::directoryChanged, this, [=]{
                bool existed = QFile::exists(realPath());
                if (!existed) {
                    GlobalThemeHelper::getInstance()->d_ptr->globalThemes.remove(this->getThemeName());
                    Q_EMIT GlobalThemeHelper::getInstance()->globalThemesChanged();
                }
        });
    }
}

GlobalTheme::GlobalTheme(QObject *parent)
{
    d_ptr = new GlobalThemePrivate;
}

QString GlobalTheme::getThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->themeName;
}

QString GlobalTheme::getLocaleThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->localeThemeName;
}

bool GlobalTheme::getSupportLightDarkMode()
{
    Q_D(GlobalTheme);
    return d_func()->supportLightDarkMode;
}

QString GlobalTheme::getDefaultLightDarkMode()
{
    Q_D(GlobalTheme);
    return d_func()->defaultLightDarkMode;
}

QString GlobalTheme::getWidgetStyleName()
{
    Q_D(GlobalTheme);
    return d_func()->widgetThemeName;
}

QString GlobalTheme::getQtLightThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->qtLightThemeName;
}

QString GlobalTheme::getQtDarkThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->qtDarkThemeName;
}

QString GlobalTheme::getQtMixedThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->qtMixedThemeName;
}

QString GlobalTheme::getQtAccentRole()
{
    Q_D(GlobalTheme);
    return d_func()->qtAccentRole;
}

QString GlobalTheme::getGtkThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->gtkThemeName;
}

QString GlobalTheme::getGtkLightThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->gtkLightThemeName;
}

QString GlobalTheme::getGtkDarkThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->gtkDarkThemeName;
}

QString GlobalTheme::getGtkAccent()
{
    Q_D(GlobalTheme);
    return d_func()->gtkAccent;
}

QString GlobalTheme::getIconThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->iconThemeName;
}

QString GlobalTheme::getCursorThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->cursorThemeName;
}

QString GlobalTheme::getSoundThemeName()
{
    Q_D(GlobalTheme);
    return d_func()->soundThemeName;
}

QString GlobalTheme::getWallPaperPath()
{
    Q_D(GlobalTheme);
    return d_func()->wallPaperPath;
}

bool GlobalTheme::getSupportBlur()
{
    Q_D(GlobalTheme);
    return d_func()->supportBlur;
}

bool GlobalTheme::getBlurEnabled()
{
    Q_D(GlobalTheme);
    return d_func()->blurEnabled;
}

bool GlobalTheme::getSupportTransparency()
{
    Q_D(GlobalTheme);
    return d_func()->supportTransparency;
}

int GlobalTheme::getTransparencyBlur()
{
    Q_D(GlobalTheme);
    return d_func()->transparencyBlur;
}

int GlobalTheme::getTransparencyNoBlur()
{
    Q_D(GlobalTheme);
    return d_func()->transparencyNoBlur;
}

bool GlobalTheme::getSupportAnimation()
{
    Q_D(GlobalTheme);
    return d_func()->supportAnimation;
}

int GlobalTheme::getAnimationDuration()
{
    Q_D(GlobalTheme);
    return d_func()->animationDuration;
}

bool GlobalTheme::getSupportRadius()
{
    Q_D(GlobalTheme);
    return d_func()->supportRadius;
}

QString GlobalTheme::getRadiusValue()
{
    Q_D(GlobalTheme);
    return d_func()->radiusValue;
}

void GlobalTheme::loadThumbnail()
{
    GlobalThemeHelper::getInstance()->loadThumbnail(this);
}

QPixmap GlobalTheme::getThumbnail()
{
    Q_D(GlobalTheme);
    return d_func()->thumbnail;
}

QString GlobalTheme::realPath()
{
    Q_D(GlobalTheme);
    return d_func()->realPath;
}

void GlobalTheme::updateThumbnail(const QPixmap &pixmap)
{
    Q_D(GlobalTheme);
    d_func()->thumbnail = pixmap;
    qInfo()<<"thumbnail updated"<<d_func()->themeName;
    Q_EMIT thumbnailLoaded();
}

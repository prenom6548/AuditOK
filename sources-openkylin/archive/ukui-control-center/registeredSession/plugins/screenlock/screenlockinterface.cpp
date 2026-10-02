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
#include "screenlockinterface.h"
#include <QFile>
#include <QFileInfo>
#include <sys/stat.h>

#define SCREENLOCK_SCHEMA "org.ukui.screensaver"
#define SCREENLOCK_BG_KEY "background"

ScreenlockInterface::ScreenlockInterface():UKUI::ServiceObject(nullptr)
{
    mUKCConfig;
    const QByteArray id(SCREENLOCK_SCHEMA);
    mUKCConfig    = QDir::homePath() + "/.config/ukui/ukui-control-center.conf";
    lockSetting   = new QSettings(mUKCConfig, QSettings::IniFormat, this);
    lockGSettings = new QGSettings(id, QByteArray(), this);
}

ScreenlockInterface::~ScreenlockInterface()
{

}

void ScreenlockInterface::setShowOnLogin(bool b)
{
    QString bgStr;
    struct stat fileStat {};
    if (b && lockGSettings->keys().contains(SCREENLOCK_BG_KEY)) {
        bgStr = lockGSettings->get(SCREENLOCK_BG_KEY).toString();
        if (stat(bgStr.toUtf8().constData(), &fileStat) == 0 && fileStat.st_uid != 0) {  //在普通用户下
            bgStr = copyLoginFile(bgStr);
        }
    } else if (!b) {
        bgStr = "";
    }
    lockSetting->beginGroup("ScreenLock");
    lockSetting->setValue("lockStatus", b);
    lockSetting->endGroup();

    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }

    QString lockfilename = "/var/lib/lightdm-data/" + name + "/ukui-greeter.conf";
    QSettings lockLoginSettings(lockfilename, QSettings::IniFormat, this);

    lockLoginSettings.beginGroup("greeter");
    lockLoginSettings.setValue("backgroundPath", bgStr);
    lockLoginSettings.endGroup();
    Q_EMIT changed("showOnLogin");
}

void ScreenlockInterface::setWallpaper(QString f)
{
    lockGSettings->set(SCREENLOCK_BG_KEY, f);
    setShowOnLogin(getShowOnLogin());
    Q_EMIT changed("wallpaper");
}

bool ScreenlockInterface::getShowOnLogin()
{
    if (!QFile::exists(mUKCConfig)) {
        setShowOnLogin(true);
    }

    lockSetting->beginGroup("ScreenLock");
    lockSetting->sync();
    bool status = lockSetting->value("lockStatus").toBool();
    lockSetting->endGroup();
    return status;
}

QString ScreenlockInterface::getWallpaper()
{
    QString initBgStr = "";
    if (lockGSettings->keys().contains(SCREENLOCK_BG_KEY)) {
        initBgStr = lockGSettings->get(SCREENLOCK_BG_KEY).toString();
    }

    // 锁屏壁纸不存在，则展示默认壁纸
    if (!QFile::exists(initBgStr)) {
        initBgStr = "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
    }
    return initBgStr;
}

QStringList ScreenlockInterface::getPreviewWallpapers()
{
    QDBusInterface interface("org.ukui.ukcc.session",
                              "/Wallpaper",
                              "org.ukui.ukcc.session.Wallpaper",
                              QDBusConnection::sessionBus(), this);
    if (interface.isValid()) {
        return interface.property("previewWallpapers").toStringList();
    }
    return QStringList();
}

QStringList ScreenlockInterface::getSourceWallpapers()
{
    QDBusInterface interface("org.ukui.ukcc.session",
                              "/Wallpaper",
                              "org.ukui.ukcc.session.Wallpaper",
                              QDBusConnection::sessionBus(), this);
    if (interface.isValid()) {
        return interface.property("sourceWallpapers").toStringList();
    }
    return QStringList();
}

void ScreenlockInterface::resetDefault()
{
    lockGSettings->reset(SCREENLOCK_BG_KEY);
    setWallpaper(lockGSettings->get(SCREENLOCK_BG_KEY).toString());
}

QString ScreenlockInterface::copyLoginFile(QString fileName)
{
    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }
    QString loginFilename = "/var/lib/lightdm-data/" + name + "/" + "loginBackground";
    const QFileInfo sourceInfo(fileName);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        qWarning() << "Lock screen background does not exist or is not a file:" << fileName;
        return QString();
    }
    QFile target(loginFilename);
    if (target.exists() && !target.remove()) {
        qWarning() << "Failed to remove existing login background:" << loginFilename;
        return QString();
    }
    if (!QFile::copy(fileName, loginFilename)) {
        qWarning() << "Failed to copy lock screen background to:" << loginFilename;
        return QString();
    }
    return loginFilename;
}

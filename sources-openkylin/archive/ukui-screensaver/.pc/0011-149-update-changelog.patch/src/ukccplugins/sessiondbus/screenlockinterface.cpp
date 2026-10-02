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
#include "screenlockinterface.h"
#include <sys/stat.h>
#include "../../lock-backend/gsettingshelper.h"

ScreenlockInterface::ScreenlockInterface(DbusUpperInterface *upperInterface, QObject *parent)
    : QObject(parent), m_upperInterface(upperInterface)
{
}

bool ScreenlockInterface::initData()
{
    if (!m_isInited) {
        mUKCConfig = QDir::homePath() + "/.config/ukui/ukui-control-center.conf";
        lockSetting = new QSettings(mUKCConfig, QSettings::IniFormat, this);
        m_isInited = true;
    }
    return m_isInited;
}

ScreenlockInterface::~ScreenlockInterface() {}

void ScreenlockInterface::setShowOnLogin(bool b)
{
    initData();
    QString bgStr;
    struct stat fileStat;
    if (b && m_upperInterface) {
        bgStr = m_upperInterface->getGSettingsInstance()->GetLockScreenConf(KEY_BACKGROUND).toString();
        stat(bgStr.toStdString().c_str(), &fileStat);
        if (fileStat.st_uid != 0) { // 在普通用户下
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
    initData();
    if (m_upperInterface) {
        m_upperInterface->getGSettingsInstance()->SetLockScreenConf(KEY_BACKGROUND, f);
    }
    setShowOnLogin(getShowOnLogin());
    Q_EMIT changed("wallpaper");
}

bool ScreenlockInterface::getShowOnLogin()
{
    initData();
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
    initData();
    QString initBgStr = "";
    if (m_upperInterface) {
        initBgStr = m_upperInterface->getGSettingsInstance()->GetLockScreenConf(KEY_BACKGROUND).toString();
    }

    // 锁屏壁纸不存在，则展示默认壁纸
    if (!QFile::exists(initBgStr)) {
        initBgStr = "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
        if (!QFile::exists(initBgStr)) {
            initBgStr = "/usr/share/backgrounds/1-openkylin.jpg";
        }
    }
    return initBgStr;
}

QStringList ScreenlockInterface::getPreviewWallpapers()
{
    initData();
    QDBusInterface interface(
        "org.ukui.ukcc.session", "/Wallpaper", "org.ukui.ukcc.session.Wallpaper", QDBusConnection::sessionBus(), this);
    if (interface.isValid()) {
        return interface.property("previewWallpapers").toStringList();
    }
    return QStringList();
}

QStringList ScreenlockInterface::getSourceWallpapers()
{
    QDBusInterface interface(
        "org.ukui.ukcc.session", "/Wallpaper", "org.ukui.ukcc.session.Wallpaper", QDBusConnection::sessionBus(), this);
    if (interface.isValid()) {
        return interface.property("sourceWallpapers").toStringList();
    }
    return QStringList();
}

void ScreenlockInterface::resetDefault()
{
    initData();
    if (m_upperInterface) {
        QString strDefBg = m_upperInterface->getGSettingsInstance()->ResetLockScreenConf(KEY_BACKGROUND).toString();
        setWallpaper(strDefBg);
    }
}

QString ScreenlockInterface::copyLoginFile(QString fileName)
{
    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }
    QString loginFilename = "/var/lib/lightdm-data/" + name + "/" + "loginBackground";
    QString loginCmd = QString("cp '%1' %2").arg(fileName).arg(loginFilename);
    system(loginCmd.toUtf8().data());
    return loginFilename;
}

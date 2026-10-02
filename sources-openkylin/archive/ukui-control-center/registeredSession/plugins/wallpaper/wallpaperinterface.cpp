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
#include "wallpaperinterface.h"

#include "libkypackages.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>

WallpaperInterface::WallpaperInterface():UKUI::ServiceObject(nullptr)
{
    WorkerObject *workThread = new WorkerObject(this);
    workThread->start();
    connect(workThread, &WorkerObject::wallpaperReaded, this, [=](QStringList previewFiles, QStringList sourceFiles) {
        previewWallpaperList = previewFiles;
        sourceWallpaperList  = sourceFiles;
    });
    connect(workThread, &QThread::finished, workThread, &QThread::deleteLater);
    mWallpaperMode.clear();
    QString filename = UniversalInterface::self()->mateBackgroundGsettings()->get("picture-filename").toString();
    if (filename.isEmpty()) {
        mWallpaperMode = "color";
    } else {
        mWallpaperMode = "picture";
    }

    mDisplayType.clear();
    mDisplayType = UniversalInterface::self()->mateBackgroundGsettings()->get("picture-options").toString();

    mWallpaper.clear();
    if (mWallpaperMode == "color") {
        mWallpaper = UniversalInterface::self()->mateBackgroundGsettings()->get("primary-color").toString();
    } else {
        mWallpaper = UniversalInterface::self()->accountUserDbus()->property("BackgroundFile").toString();
        if (mWallpaper.isEmpty() || !QFile::exists(mWallpaper) ||
            mWallpaper == "/usr/share/backgrounds/ubuntukylin-default-settings.jpg") {
            mWallpaper = "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
        }
    }
    connect(UniversalInterface::self()->mateBackgroundGsettings(), &QGSettings::changed, this, [=](QString key){
        QString filename = UniversalInterface::self()->mateBackgroundGsettings()->get("picture-filename").toString();
        if (filename.isEmpty()) {
            mWallpaperMode = "color";
        } else {
            mWallpaperMode = "picture";
        }
        if (key == "pictureOptions") {
            mDisplayType = UniversalInterface::self()->mateBackgroundGsettings()->get("picture-options").toString();
            Q_EMIT changed("displayType");
        } else if(key == "pictureFilename") {
            if (mWallpaperMode == "color") {
                mWallpaper = UniversalInterface::self()->mateBackgroundGsettings()->get("primary-color").toString();
            } else {
                mWallpaper = filename;
                qDebug() << Q_FUNC_INFO << "picture-filename" << mWallpaper << "needChangeCustomTheme" << m_needChangeCustomTheme;
                if (mWallpaper.isEmpty() || !QFile::exists(mWallpaper) ||
                    mWallpaper == "/usr/share/backgrounds/ubuntukylin-default-settings.jpg") {
                    mWallpaper = "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
                }
                if (m_needChangeCustomTheme) {
                    changeGlobalTheme();
                } else {
                    m_needChangeCustomTheme = true;
                    qDebug() << Q_FUNC_INFO << "no needChangeCustomTheme";
                }
            }
            Q_EMIT changed("wallpaper");
        }
    });
    workThread->wait();
}

WallpaperInterface::~WallpaperInterface()
{

}

QString WallpaperInterface::getWallpaperMode()
{
    return mWallpaperMode;
}

QString WallpaperInterface::getDisplayType()
{
    return mDisplayType;
}

QString WallpaperInterface::getWallpaper()
{
    return mWallpaper;
}

QStringList WallpaperInterface::getPreviewWallpapers()
{
    return previewWallpaperList;
}

QStringList WallpaperInterface::getSourceWallpapers()
{
    return sourceWallpaperList;
}

QStringList WallpaperInterface::allowedWallpaperDirPrefixes() const
{
    QStringList prefixes;
    prefixes << QStringLiteral("/usr/share/backgrounds/")
             << QStringLiteral("/usr/share/ukui-background-properties/");

    char **wallpaperPaths = kdk_package_get_standard_path("UKUI_BACKGROUND_PROPERTIES_PATH");
    if (wallpaperPaths != nullptr) {
        for (int idx = 0; wallpaperPaths[idx] != nullptr; ++idx) {
            QString path = QString::fromUtf8(wallpaperPaths[idx]);
            if (!path.endsWith(QLatin1Char('/'))) {
                path += QLatin1Char('/');
            }
            prefixes << path;
        }
    }

    const QString home = QDir::homePath();
    if (!home.isEmpty()) {
        prefixes << home + QLatin1Char('/');
    }

    const QString pictures = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (!pictures.isEmpty()) {
        QString picPath = pictures;
        if (!picPath.endsWith(QLatin1Char('/'))) {
            picPath += QLatin1Char('/');
        }
        prefixes << picPath;
    }

    QString user = qgetenv("USER");
    if (user.isEmpty()) {
        user = qgetenv("USERNAME");
    }
    if (!user.isEmpty()) {
        prefixes << QStringLiteral("/media/") + user + QLatin1Char('/');
        prefixes << QStringLiteral("/run/media/") + user + QLatin1Char('/');
    }

    return prefixes;
}

bool WallpaperInterface::isSpecialFsPath(const QString &path) const
{
    const QString normalized = path.startsWith(QLatin1Char('/')) ? path : QLatin1Char('/') + path;
    return normalized.startsWith(QStringLiteral("/dev/"))
        || normalized.startsWith(QStringLiteral("/proc/"))
        || normalized.startsWith(QStringLiteral("/sys/"));
}

bool WallpaperInterface::isPathUnderPrefix(const QString &canonicalPath, const QString &prefix) const
{
    QString normalizedPrefix = QDir(prefix).canonicalPath();
    if (normalizedPrefix.isEmpty()) {
        normalizedPrefix = prefix;
    }
    if (!normalizedPrefix.endsWith(QLatin1Char('/'))) {
        normalizedPrefix += QLatin1Char('/');
    }
    return canonicalPath.startsWith(normalizedPrefix);
}

bool WallpaperInterface::validateDisplayType(const QString &type)
{
    const QStringList allowedDisplayTypes = {
        QStringLiteral("scaled"),
        QStringLiteral("wallpaper"),
        QStringLiteral("centered"),
        QStringLiteral("stretched"),
        QStringLiteral("zoom"),
        QStringLiteral("spanned"),
    };
    if (allowedDisplayTypes.contains(type)) {
        return true;
    }
    qDebug() << Q_FUNC_INFO << "reject display type" << type;
    return false;
}

bool WallpaperInterface::validateWallpaperColor(const QString &color)
{
    static const QRegularExpression hexColor(QStringLiteral("^#[0-9A-Fa-f]{6}$"));
    if (hexColor.match(color).hasMatch()) {
        return true;
    }
    qDebug() << Q_FUNC_INFO << "reject wallpaper color" << color;
    return false;
}

bool WallpaperInterface::validateWallpaperPicturePath(const QString &path, QString *canonicalPath)
{
    if (path.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "reject wallpaper path, path is empty";
        return false;
    }
    if (path.contains(QChar::Null)) {
        qDebug() << Q_FUNC_INFO << "reject wallpaper path, contains invalid characters" << path;
        return false;
    }
    if (isSpecialFsPath(path)) {
        qDebug() << Q_FUNC_INFO << "reject wallpaper path, special fs path" << path;
        return false;
    }

    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        qDebug() << Q_FUNC_INFO << "reject wallpaper path, file does not exist" << path;
        return false;
    }

    const QString resolvedPath = fileInfo.canonicalFilePath();
    if (resolvedPath.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "reject wallpaper path, failed to resolve" << path;
        return false;
    }
    if (isSpecialFsPath(resolvedPath)) {
        qDebug() << Q_FUNC_INFO << "reject wallpaper path, resolved special fs path" << resolvedPath;
        return false;
    }

    const QStringList allowedImageSuffixes = {
        QStringLiteral("jpg"),
        QStringLiteral("jpeg"),
        QStringLiteral("bmp"),
        QStringLiteral("dib"),
        QStringLiteral("png"),
        QStringLiteral("jfif"),
        QStringLiteral("jpe"),
        QStringLiteral("gif"),
        QStringLiteral("tif"),
        QStringLiteral("tiff"),
        QStringLiteral("wdp"),
    };
    const QString suffix = QFileInfo(resolvedPath).suffix().toLower();
    if (!allowedImageSuffixes.contains(suffix)) {
        qDebug() << Q_FUNC_INFO << "reject wallpaper path, unsupported format" << resolvedPath << suffix;
        return false;
    }

    bool allowed = sourceWallpaperList.contains(resolvedPath);
    if (!allowed) {
        const QStringList prefixes = allowedWallpaperDirPrefixes();
        for (const QString &prefix : prefixes) {
            if (isPathUnderPrefix(resolvedPath, prefix)) {
                allowed = true;
                break;
            }
        }
    }
    if (!allowed) {
        qDebug() << Q_FUNC_INFO << "reject wallpaper path, outside allowed directories" << resolvedPath;
        return false;
    }

    if (canonicalPath != nullptr) {
        *canonicalPath = resolvedPath;
    }
    return true;
}

void WallpaperInterface::setDisplayType(QString type)
{
    if (!validateDisplayType(type)) {
        qDebug() << Q_FUNC_INFO << "setDisplayType rejected" << type;
        return;
    }
    UniversalInterface::self()->mateBackgroundGsettings()->set("picture-options", type);
    Q_EMIT changed("displayType");
}

void WallpaperInterface::setWallpaper(QString mode, QString data)
{
    QString wallpaperPath = data;
    if (mode == QStringLiteral("color")) {
        if (!validateWallpaperColor(data)) {
            qDebug() << Q_FUNC_INFO << "setWallpaper rejected, invalid color" << data;
            return;
        }
    } else if (mode == QStringLiteral("picture")) {
        if (!validateWallpaperPicturePath(data, &wallpaperPath)) {
            qDebug() << Q_FUNC_INFO << "setWallpaper rejected, invalid picture path" << data;
            return;
        }
    } else {
        qDebug() << Q_FUNC_INFO << "setWallpaper rejected, invalid mode" << mode;
        return;
    }

    changeGlobalTheme();
    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }
    QString confFilename = "/var/lib/lightdm-data/" + name + "/ukui-greeter.conf";
    QSettings qsetting(confFilename, QSettings::IniFormat);
    qsetting.beginGroup("greeter");
    if (mode == "color") {
        qsetting.setValue("color", data);
        UniversalInterface::self()->mateBackgroundGsettings()->set("picture-filename", "");
        UniversalInterface::self()->mateBackgroundGsettings()->set("primary-color", QVariant(data));
        UniversalInterface::self()->mateBackgroundGsettings()->set("secondary-color", QVariant(data));
    } else {
        qsetting.setValue("color", "");
        UniversalInterface::self()->accountUserDbus()->call("SetBackgroundFile", wallpaperPath);
        UniversalInterface::self()->mateBackgroundGsettings()->set("picture-filename", wallpaperPath);
    }
    qsetting.endGroup();
    Q_EMIT changed("wallpaper");
}

void WallpaperInterface::setNeedChangeCustomTheme(bool status) {
    m_needChangeCustomTheme = status;
    qDebug() << Q_FUNC_INFO << "setNeedChangeCustomTheme status" << m_needChangeCustomTheme;
}


void WallpaperInterface::resetDefault()
{
    UniversalInterface::self()->mateBackgroundGsettings()->reset("picture-filename");
    setWallpaper("picture", UniversalInterface::self()->mateBackgroundGsettings()->get("picture-filename").toString());
    setDisplayType("scaled");
}

void WallpaperInterface::changeGlobalTheme()
{
    if (QGSettings::isSchemaInstalled(QByteArray("org.ukui.globaltheme.settings"))) {
        QGSettings globalTheme("org.ukui.globaltheme.settings");
        globalTheme.set("global-theme-name", "custom");
    }
}

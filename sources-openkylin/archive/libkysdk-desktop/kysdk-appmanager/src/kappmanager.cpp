/*
 * libkysdk-appmanager's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "kappmanager.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QSettings>
#include <unistd.h>

#include <QDebug>
#include <glib.h>

QList<QStringList> mimeTypes{
    {"audio/mpeg",
        "audio/x-mpegurl",
        "audio/x-scpls",
        "audio/x-vorbis+ogg","audio/x-wav",
        "audio/x-ms-wma",
        "audio/x-flac",
        "audio/ac3",
        "audio/acc",
        "audio/aac",
        "audio/AMR",
        "audio/x-m4r",
        "audio/midi",
        "audio/mp2",
        "audio/x-wavpack",
        "audio/x-ape",
        "audio/x-mmf",
        "audio/mp3",
        "audio/flac",
        "audio/wma",
        "audio/amr",
        "audio/vnd.dts",
        "audio/basic",
        "audio/x-aiff",
        "audio/x-matroska",
        "audio/x-voc",
        "application/x-smaf",
        "audio/m4a"
    },
    {"video/mp4",
        "video/mpeg",
        "video/mp2t",
        "video/msvideo",
        "video/quicktime",
        "video/webm",
        "video/x-avi",
        "video/x-flv",
        "video/x-matroska",
        "video/x-mpeg",
        "video/x-ogm+ogg",
        "video/rm",
        "video/3pg",
        "video/asf",
        "video/3gp",
        "video/3gpp",
        "video/3gpp2",
        "video/x-ms-afs",
        "video/x-ms-asf",
        "video/x-mpeg2",
        "video/x-mpeg3",
        "video/mp4v-es",
        "video/x-m4v",
        "video/divx",
        "video/vnd.divx",
        "video/x-msvideo",
        "video/ogg",
        "video/vnd.rn-realvideo",
        "video/x-ms-wmv",
        "video/x-ms-wmx",
        "video/x-ms-wvxvideo",
        "video/avi",
        "video/x-flic",
        "video/fli",
        "video/x-flc",
        "video/flv",
        "video/x-theora",
        "video/x-theora+ogg",
        "video/mkv",
        "video/x-ogm",
        "video/vnd.mpegurl",
        "video/dv",
        "application/vnd.rn-realmedia",
        "application/vnd.rn-realmedia-vbr"
    },
    {"image/bmp",
        "image/gif",
        "image/jpeg",
        "image/png",
        "image/tiff",
        "image/webp",
        "image/x-xbitmap",
        "image/x-xpixmap",
        "image/x-icon"
    },
    {"text/plain"},
    {"x-scheme-handler/http","x-scheme-handler/https","text/html"},
    {"x-scheme-handler/mailto","application/x-extension-eml","message/rfc822"}
};

kdk::KAppManager::KAppManager()
{
}

QList<kdk::AppInfo *> kdk::KAppManager::getApplications()
{
    QList<kdk::AppInfo*> result;

    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    for (QString &path : locations)
    {
        QDir dir(path);
        if (!dir.exists())
            continue;

        for (auto &entry : dir.entryList())
        {
            if (entry.endsWith(".desktop"))
            {
                kdk::AppInfo *info = new kdk::AppInfo(QString("%1/%2").arg(path).arg(entry));
                result.append(info);
            }
        }
    }

    return result;
}

QList<kdk::AppInfo *> kdk::KAppManager::getApplicationByMime(const QString &mimeType)
{
    QList<kdk::AppInfo*> result;

    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    for (QString &path : locations)
    {
        QDir dir(path);
        if (!dir.exists())
            continue;

        for (auto &entry : dir.entryList())
        {
            if (entry.endsWith(".desktop"))
            {
                kdk::AppInfo *info = new kdk::AppInfo(QString("%1/%2").arg(path).arg(entry));
                if (info->mimeTypes().contains(mimeType))
                    result.append(info);
                else
                    delete info;
            }
        }
    }

    return result;
}

QList<kdk::AppInfo*> kdk::KAppManager::getAutoStartApplications()
{
    QList<kdk::AppInfo*> result;

    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);

    for (QString &path : locations)
    {
        QDir dir(path + "/autostart");
        if (!dir.exists())
            continue;

        for (auto &entry : dir.entryList())
        {
            if (entry.endsWith(".desktop"))
            {
                kdk::AppInfo *info = new kdk::AppInfo(QString("%1/autostart/%2").arg(path).arg(entry));
                result.append(info);
            }
        }
    }

    return result;
}

bool kdk::KAppManager::addAutoStartApplication(const QString &desktopFullPath)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart";

    QDir().mkpath(path);

    QFileInfo sourceInfo(desktopFullPath);
    QString file = path + "/" + sourceInfo.fileName();

    // 强制覆盖已存在文件
    if (QFile::exists(file))
    {
        QFile::remove(file);
    }

    return QFile::copy(desktopFullPath, file);
}

bool kdk::KAppManager::removeAutoStartApplication(const QString &desktop)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart";

    QFileInfo sourceInfo(desktop);
    QString file = path + "/" + sourceInfo.fileName();

    if (!QFile::exists(file))
    {
        return true;
    }

    return QFile::remove(file);
}

kdk::AppInfo *kdk::KAppManager::getDefaultApplicationByMime(const QString &mimeType)
{
    kdk::AppInfo *result = nullptr;
    QString defaultApplication;

    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    QString userFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/mimeapps.list";
    if (QFile::exists(userFile))
    {
        QSettings setting(userFile, QSettings::IniFormat);
        setting.setIniCodec("UTF-8");
        setting.beginGroup("Default Applications");
        defaultApplication = setting.value(mimeType, "").toString();
        setting.endGroup();
        if (!defaultApplication.isEmpty())
        {
            result = new kdk::AppInfo(findDesktopFile(locations, defaultApplication));
            return result;
        }
    }

    for (QString &path : locations)
    {
        QString file = path + "/applications/ukui-mimeapps.list";
        if (QFile::exists(file))
        {
            QSettings setting(file, QSettings::IniFormat);
            setting.setIniCodec("UTF-8");
            setting.beginGroup("Default Applications");
            defaultApplication = setting.value(mimeType, "").toString();
            setting.endGroup();
            if (!defaultApplication.isEmpty())
            {
                result = new kdk::AppInfo(findDesktopFile(locations, defaultApplication));
                break;
            }
        }

        file.replace("ukui-mimeapps", "mimeapps");
        if (QFile::exists(file))
        {

            QSettings setting(file, QSettings::IniFormat);
            setting.setIniCodec("UTF-8");
            setting.beginGroup("Default Applications");
            defaultApplication = setting.value(mimeType, "").toString();
            setting.endGroup();
            if (!defaultApplication.isEmpty())
            {
                result = new kdk::AppInfo(findDesktopFile(locations, defaultApplication));
                break;
            }
        }
    }
    return result;
}

kdk::AppInfo *kdk::KAppManager::getDefaultApplicationByType(int type)
{
    kdk::AppInfo *result = nullptr;
    QString defaultApplication;
    QString mimeType = mimeTypes[type][0];

    return getDefaultApplicationByMime(mimeType);
}

bool kdk::KAppManager::setDefaultApplicationByMime(const QString &desktopFullPath, const QString &mimeType)
{
    kdk::AppInfo app(desktopFullPath);
    if (!app.isVaild())
        return false;

    QFileInfo info(desktopFullPath);
    QString desktopName = info.fileName();

    QString file = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/mimeapps.list";

    GError *error = NULL;
    GKeyFile *keyfile = g_key_file_new();

    const gchar *group = "Default Applications";
    /* 尝试加载现有文件 */
    if (!g_key_file_load_from_file(keyfile, file.toLatin1().data(), G_KEY_FILE_NONE, &error))
    {
        /* 忽略文件不存在的错误 */
        if (!g_error_matches(error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
        {
            qDebug() << "加载文件错误: %s\n" << error->message;
            g_error_free(error);
            g_key_file_free(keyfile);
            return false;
        }
        g_clear_error(&error); // 清除错误
    }

    /* 添加/修改键值 */
    g_key_file_set_string(keyfile, group, mimeType.toLatin1().data(), desktopName.toLatin1().data());

    /* 写入文件 */
    if (!g_key_file_save_to_file(keyfile, file.toLatin1().data(), &error))
    {
        qDebug() << "保存文件错误: %s\n" << error->message;
        g_error_free(error);
        g_key_file_free(keyfile);
        return 1;
    }

    /* 清理资源 */
    g_key_file_free(keyfile);

    return true;
}

bool kdk::KAppManager::setDefaultApplicationByType(const QString &desktopFullPath, int type)
{
    kdk::AppInfo app(desktopFullPath);
    if (!app.isVaild())
        return false;

    QFileInfo info(desktopFullPath);
    QString desktopName = info.fileName();

    QString file = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/mimeapps.list";

    GError *error = NULL;
    GKeyFile *keyfile = g_key_file_new();
    /* 尝试加载现有文件 */
    if (!g_key_file_load_from_file(keyfile, file.toLatin1().data(), G_KEY_FILE_NONE, &error))
    {
        /* 忽略文件不存在的错误 */
        if (!g_error_matches(error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
        {
            qDebug() << "加载文件错误: %s\n" << error->message;
            g_error_free(error);
            g_key_file_free(keyfile);
            return false;
        }
        g_clear_error(&error); // 清除错误
    }

    for (QString &key : mimeTypes[type])
    {
        const gchar *group = "Default Applications";

        /* 添加/修改键值 */
        g_key_file_set_string(keyfile, group, key.toLatin1().data(), desktopName.toLatin1().data());
    }

    /* 写入文件 */
    if (!g_key_file_save_to_file(keyfile, file.toLatin1().data(), &error))
    {
        qDebug() << "保存文件错误: %s\n" << error->message;
        g_error_free(error);
        g_key_file_free(keyfile);
        return 1;
    }

    /* 清理资源 */
    g_key_file_free(keyfile);

    return true;
}

QString kdk::KAppManager::findDesktopFile(const QStringList &locations, const QString &desktop)
{
    for (const QString &path : locations)
    {
        QString f = path + "/applications/" + desktop;
        if (QFile::exists(f))
            return f;
    }
    return QString();
}

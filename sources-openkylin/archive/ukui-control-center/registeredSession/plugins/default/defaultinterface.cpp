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
#include "defaultinterface.h"
#include <QDebug>
#include <QStandardPaths>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QMimeDatabase>

#define BROWSERTYPE "x-scheme-handler/http"
#define MAILTYPE    "x-scheme-handler/mailto"
#define IMAGETYPE   "image/png"
#define AUDIOTYPE   "audio/x-vorbis+ogg"
#define VIDEOTYPE   "video/mp4"
#define TEXTTYPE    "text/plain"

#define LOCAL_CONFIG_DIR "/.config/"

QMap <QString , QVariant> DefaultInterface::mSysDefaultList;

static bool mimeTypeLessThan(const QMimeType &m1, const QMimeType &m2)
{
    return m1.name() < m2.name();
}

DefaultInterface::DefaultInterface():UKUI::ServiceObject(nullptr)
{
    mLocalMimefile = QDir::homePath() + LOCAL_CONFIG_DIR + "mimeapps.list";
    getSystemDefaultAppList();
    mBrowserMimeTypeList << "x-scheme-handler/http" << "x-scheme-handler/https" << "x-scheme-handler/about" << "text/html";
    mMailMimeTypeList << "x-scheme-handler/mailto" << "application/x-extension-eml" << "message/rfc822";
    mTextMimeTypeList << "text/plain";
}

QMap<QString, QVariant> DefaultInterface::getSystemDefaultAppList()
{
    if (!mSysDefaultList.isEmpty()) {
        return mSysDefaultList;
    }
    QStringList mimeappsFileNames;
    const QString desktops = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP"));
    const auto list = desktops.split(QLatin1Char(':'));
    for (const QString &desktop : list) {
        mimeappsFileNames.append(desktop.toLower() + QLatin1String("-mimeapps.list"));
    }
    const QStringList mimeappsDirs = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation) + QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    for (const QString &dir : mimeappsDirs) {
        for (QString file : mimeappsFileNames) {
             // 系统目录配置文件
            QString filePath = dir + QLatin1Char('/') + file;
            if (QFile::exists(filePath)) {
                QSettings* mimeappFile = new QSettings(filePath, QSettings::IniFormat);
                mimeappFile->setIniCodec("utf-8");
                mSysDefaultList[BROWSERTYPE] = mimeappFile->value(QString("Default Applications/%1").arg(BROWSERTYPE)).toString();
                mSysDefaultList[AUDIOTYPE] = mimeappFile->value(QString("Default Applications/%1").arg(AUDIOTYPE)).toString();
                mSysDefaultList[VIDEOTYPE] = mimeappFile->value(QString("Default Applications/%1").arg(VIDEOTYPE)).toString();
                mSysDefaultList[IMAGETYPE] = mimeappFile->value(QString("Default Applications/%1").arg(IMAGETYPE)).toString();
                mSysDefaultList[TEXTTYPE] = mimeappFile->value(QString("Default Applications/%1").arg(TEXTTYPE)).toString();
                mSysDefaultList[MAILTYPE] = mimeappFile->value(QString("Default Applications/%1").arg(MAILTYPE)).toString();
                delete mimeappFile;
                mimeappFile = nullptr;
                return mSysDefaultList;
            }
        }
    }
    return mSysDefaultList;
}

void DefaultInterface::resetDefault() {
    if (!QFile(mLocalMimefile).exists()) {
        qDebug() << "local mime file not exist";
        return;
    }
    QFile(mLocalMimefile).remove();
    qDebug() << Q_FUNC_INFO << "reset default suc";
    emit defaultChanged();
}

QVariantList DefaultInterface::getDefaultApp(const QString &mimeType)
{
    qDBusRegisterMetaType<Service>();
    QVariantList list;
    GAppInfo* info = g_app_info_get_default_for_type(mimeType.toUtf8().data(), FALSE);
    if (!info) {
        return list;
    }

    const char* name = g_app_info_get_name(info);
    const char* storageId = g_app_info_get_id(info);
    const char* icon = nullptr;
    GIcon* gIcon = g_app_info_get_icon(info);
    if (gIcon) {
        gchar* iconString = g_icon_to_string(gIcon);
        if (iconString) {
            icon = iconString;
        }
    }
    Service service;
    service.icon = QString(icon);
    service.name = QString(name);
    service.storageId = QString(storageId);
    qDebug() << Q_FUNC_INFO << "getDefaultApp" << mimeType << name << name << icon;
    list.append(QVariant::fromValue(service));
    return list;
}

QVariantList DefaultInterface::getAppList(const QString &mimeType)
{
    qDBusRegisterMetaType<Service>();
    QVariantList list;
    QStringList nameList;
    QVector<Appinfo> appInfoList = _getAppList(mimeType.toUtf8().constData());
    for (const auto& app : appInfoList) {
        GAppInfo* info = app.item;
        if (!info) {
            continue;
        }
        const char* appName = g_app_info_get_name(info);
        const char* storageId = g_app_info_get_id(info);
        if (!appName || !storageId) {
            continue;
        }
        // 视频播放器屏蔽音乐
        if (mimeType == VIDEOTYPE && QString(storageId).contains("kylin-music.desktop")) {
            continue;
        }

        QString appNameStr = QString::fromUtf8(appName);
        if (nameList.contains(appNameStr)) {
            continue;
        }
        nameList.append(appNameStr);

        Service service;
        service.name = appNameStr;
        service.storageId = QString::fromUtf8(storageId);
        GIcon* gIcon = g_app_info_get_icon(info);
        if (gIcon) {
            gchar* iconString = g_icon_to_string(gIcon);
            if (iconString) {
                service.icon = QString::fromUtf8(iconString);
                g_free(iconString);
            }
        }
        list.append(QVariant::fromValue(service));
    }
    return list;
}

/* Set Default Browser */
bool DefaultInterface::setWebBrowsersDefaultProgram(char * appid) {
    qDebug() << __func__ << __LINE__;

    const char * content_type = "x-scheme-handler/http";
    QVector<Appinfo> appinfo = _getAppList(content_type);
    bool judge = false;
    if (!appinfo.isEmpty()) {
        for(int i = 0; i < appinfo.size(); i++) {
            const char *id = g_app_info_get_id(appinfo[i].item);
            int result = strcmp(id,appid);
            if (0 == result) {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/http", NULL);
                gboolean ret2=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/https", NULL);
                gboolean ret3=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/about", NULL);
                gboolean ret4=g_app_info_set_as_default_for_type(appitem, "text/html", NULL);
                if(ret1 && ret2 && ret3 && ret4)
                    judge=true;
                break;
            }
        }
    }
    qDebug() << __func__ << __LINE__;
    return judge;
}

/* Set Default Mail */
bool DefaultInterface::setMailReadersDefaultProgram(char *appid) {
    const char *content_type="x-scheme-handler/mailto";
    QVector<Appinfo> appinfo = _getAppList(content_type);

    bool judge = false;
    if (!appinfo.isEmpty()) {
        for (int i = 0; i < appinfo.size(); i++) {
            const char * id = g_app_info_get_id(appinfo[i].item);
            int result=strcmp(id,appid);
            if (0 == result) {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/mailto", NULL);
                gboolean ret2=g_app_info_set_as_default_for_type(appitem, "application/x-extension-eml", NULL);
                gboolean ret3=g_app_info_set_as_default_for_type(appitem, "message/rfc822", NULL);
                if(ret1 && ret2 && ret3)
                    judge=true;
                break;
            }
        }
    }
    return judge;
}

/* Set Default Image */
bool DefaultInterface::setImageViewersDefaultProgram(char *appid) {
    const char *content_type = "image/png";
    QVector<Appinfo> appinfo = _getAppList(content_type);

    bool judge = false;
    if (!appinfo.isEmpty()) {
        for (int i = 0; i < appinfo.size(); i++) {
            const char *id = g_app_info_get_id(appinfo[i].item);
            int result = strcmp(id, appid);
            if (0 == result) {
                GAppInfo *appitem = appinfo[i].item;
                gboolean ret1 = g_app_info_set_as_default_for_type(appitem, "image/bmp", NULL);
                gboolean ret2 = g_app_info_set_as_default_for_type(appitem, "image/gif", NULL);
                gboolean ret3 = g_app_info_set_as_default_for_type(appitem, "image/jpeg", NULL);
                gboolean ret4 = g_app_info_set_as_default_for_type(appitem, "image/png", NULL);
                gboolean ret5 = g_app_info_set_as_default_for_type(appitem, "image/tiff", NULL);
                gboolean ret6 = g_app_info_set_as_default_for_type(appitem, "image/webp", NULL);
                gboolean ret7 = g_app_info_set_as_default_for_type(appitem, "image/x-xbitmap", NULL);
                gboolean ret8 = g_app_info_set_as_default_for_type(appitem, "image/x-xpixmap", NULL);
                gboolean ret9 = g_app_info_set_as_default_for_type(appitem, "image/x-icon", NULL);
                gboolean ret10 = g_app_info_set_as_default_for_type(appitem, "image/x-win-bitmap", NULL);
                gboolean ret11 = g_app_info_set_as_default_for_type(appitem, "image/x-portable-graymap", NULL);
                gboolean ret12 = g_app_info_set_as_default_for_type(appitem, "image/x-portable-anymap", NULL);
                gboolean ret13 = g_app_info_set_as_default_for_type(appitem, "image/x-portable-bitmap", NULL);
                gboolean ret14 = g_app_info_set_as_default_for_type(appitem, "image/x-portable-pixmap", NULL);
                gboolean ret15 = g_app_info_set_as_default_for_type(appitem, "image/x-tga", NULL);
                gboolean ret16 = g_app_info_set_as_default_for_type(appitem, "image/wmf", NULL);
                gboolean ret17 = g_app_info_set_as_default_for_type(appitem, "image/x-wmf", NULL);
                if (ret1 && ret2 && ret3 && ret4 && ret5 && ret6 && ret7 && ret8 && ret9
                        && ret10 && ret11 && ret12 && ret13 && ret14 && ret15 && ret16 && ret17)
                    judge = true;
                break;
            }
        }
    }

    return judge;
}

/* Set Default Video */
bool DefaultInterface::setVideoPlayersDefaultProgram(char *appid) {
    const char *content_type = "video/x-ogm+ogg";
    QVector<Appinfo> appinfo = _getAppList(content_type);

    bool judge = false;
    if (!appinfo.isEmpty()) {
        for(int i = 0; i < appinfo.size(); i++) {
            const char *id = g_app_info_get_id(appinfo[i].item);
            int result = strcmp(id,appid);
            if (0 == result) {
                GAppInfo *appitem = appinfo[i].item;
                gboolean ret1 = g_app_info_set_as_default_for_type(appitem, "video/mp4", NULL);
                gboolean ret2 = g_app_info_set_as_default_for_type(appitem, "video/mpeg", NULL);
                gboolean ret3 = g_app_info_set_as_default_for_type(appitem, "video/mp2t", NULL);
                gboolean ret4 = g_app_info_set_as_default_for_type(appitem, "video/msvideo", NULL);
                gboolean ret5 = g_app_info_set_as_default_for_type(appitem, "video/quicktime", NULL);
                gboolean ret6 = g_app_info_set_as_default_for_type(appitem, "video/webm", NULL);
                gboolean ret7 = g_app_info_set_as_default_for_type(appitem, "video/x-avi", NULL);
                gboolean ret8 = g_app_info_set_as_default_for_type(appitem, "video/x-flv", NULL);
                gboolean ret9 = g_app_info_set_as_default_for_type(appitem, "video/x-matroska", NULL);
                gboolean ret10 = g_app_info_set_as_default_for_type(appitem, "video/x-mpeg", NULL);
                gboolean ret11 = g_app_info_set_as_default_for_type(appitem, "video/x-ogm+ogg", NULL);
                gboolean ret12 = g_app_info_set_as_default_for_type(appitem, "video/rm", NULL);
                gboolean ret13 = g_app_info_set_as_default_for_type(appitem, "video/3pg", NULL);
                gboolean ret14 = g_app_info_set_as_default_for_type(appitem, "video/asf", NULL);
                gboolean ret15 = g_app_info_set_as_default_for_type(appitem, "video/3gp", NULL);
                gboolean ret16 = g_app_info_set_as_default_for_type(appitem, "video/3gpp", NULL);
                gboolean ret17 = g_app_info_set_as_default_for_type(appitem, "video/3gpp2", NULL);
                gboolean ret18 = g_app_info_set_as_default_for_type(appitem, "video/x-ms-afs", NULL);
                gboolean ret19 = g_app_info_set_as_default_for_type(appitem, "video/x-ms-asf", NULL);
                gboolean ret20 = g_app_info_set_as_default_for_type(appitem, "video/x-mpeg2", NULL);
                gboolean ret21 = g_app_info_set_as_default_for_type(appitem, "video/x-mpeg3", NULL);
                gboolean ret22 = g_app_info_set_as_default_for_type(appitem, "video/mp4v-es", NULL);
                gboolean ret23 = g_app_info_set_as_default_for_type(appitem, "video/x-m4v", NULL);
                gboolean ret24 = g_app_info_set_as_default_for_type(appitem, "video/divx", NULL);
                gboolean ret25 = g_app_info_set_as_default_for_type(appitem, "video/vnd.divx", NULL);
                gboolean ret26 = g_app_info_set_as_default_for_type(appitem, "video/x-msvideo", NULL);
                gboolean ret27 = g_app_info_set_as_default_for_type(appitem, "video/ogg", NULL);
                gboolean ret28 = g_app_info_set_as_default_for_type(appitem, "video/vnd.rn-realvideo", NULL);
                gboolean ret29 = g_app_info_set_as_default_for_type(appitem, "video/x-ms-wmv", NULL);
                gboolean ret30 = g_app_info_set_as_default_for_type(appitem, "video/x-ms-wmx", NULL);
                gboolean ret31 = g_app_info_set_as_default_for_type(appitem, "video/x-ms-wvxvideo", NULL);
                gboolean ret32 = g_app_info_set_as_default_for_type(appitem, "video/avi", NULL);
                gboolean ret33 = g_app_info_set_as_default_for_type(appitem, "video/x-flic", NULL);
                gboolean ret34 = g_app_info_set_as_default_for_type(appitem, "video/fli", NULL);
                gboolean ret35 = g_app_info_set_as_default_for_type(appitem, "video/x-flc", NULL);
                gboolean ret36 = g_app_info_set_as_default_for_type(appitem, "video/flv", NULL);
                gboolean ret37 = g_app_info_set_as_default_for_type(appitem, "video/x-theora", NULL);
                gboolean ret38 = g_app_info_set_as_default_for_type(appitem, "video/x-theora+ogg", NULL);
                gboolean ret39 = g_app_info_set_as_default_for_type(appitem, "video/mkv", NULL);
                gboolean ret40 = g_app_info_set_as_default_for_type(appitem, "video/x-ogm", NULL);
                gboolean ret41 = g_app_info_set_as_default_for_type(appitem, "video/vnd.mpegurl", NULL);
                gboolean ret42 = g_app_info_set_as_default_for_type(appitem, "video/dv", NULL);
                gboolean ret43 = g_app_info_set_as_default_for_type(appitem, "application/vnd.rn-realmedia", NULL);
                gboolean ret44 = g_app_info_set_as_default_for_type(appitem, "application/vnd.rn-realmedia-vbr", NULL);
                if (ret1 && ret2 && ret3 && ret4 && ret5 && ret6 && ret7 && ret8 && ret9 && ret10 && ret11 && \
                        ret12 && ret13 && ret14 && ret15 && ret16 && ret17 && ret18 && ret19 && ret20 && ret21 && \
                        ret22 && ret23 && ret24 && ret25 && ret26 && ret27 && ret28 && ret29 && ret30 && \
                        ret31 && ret32 && ret33 && ret34 && ret35 && ret36 && ret37 && ret38 && ret39 && \
                        ret40 && ret41 && ret42 && ret43 && ret44)
                    judge=true;
                break;
            }
        }
    }
    return judge;
}

/* Set Default Audio */
bool DefaultInterface::setAudioPlayersDefaultProgram(char *appid) {
    const char *content_type = "audio/x-vorbis+ogg";
    QVector<Appinfo> appinfo = _getAppList(content_type);

    bool judge = false;
    if (!appinfo.isEmpty()) {
        for(int i = 0; i < appinfo.size() ;i++) {
            const char *id = g_app_info_get_id(appinfo[i].item);
            int result = strcmp(id,appid);
            if (0 == result) {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1 = g_app_info_set_as_default_for_type(appitem, "audio/mpeg", NULL);
                gboolean ret2 = g_app_info_set_as_default_for_type(appitem, "audio/x-mpegurl", NULL);
                gboolean ret3 = g_app_info_set_as_default_for_type(appitem, "audio/x-scpls", NULL);
                gboolean ret4 = g_app_info_set_as_default_for_type(appitem, "audio/x-vorbis+ogg", NULL);
                gboolean ret5 = g_app_info_set_as_default_for_type(appitem, "audio/x-wav", NULL);
                gboolean ret6 = g_app_info_set_as_default_for_type(appitem, "audio/x-ms-wma", NULL);
                gboolean ret7 = g_app_info_set_as_default_for_type(appitem, "audio/x-flac", NULL);
                gboolean ret8 = g_app_info_set_as_default_for_type(appitem, "audio/ac3", NULL);
                gboolean ret9 = g_app_info_set_as_default_for_type(appitem, "audio/acc", NULL);
                gboolean ret10 = g_app_info_set_as_default_for_type(appitem, "audio/aac", NULL);
                gboolean ret11 = g_app_info_set_as_default_for_type(appitem, "audio/AMR", NULL);
                gboolean ret12 = g_app_info_set_as_default_for_type(appitem, "audio/x-m4r", NULL);
                gboolean ret13 = g_app_info_set_as_default_for_type(appitem, "audio/midi", NULL);
                gboolean ret14 = g_app_info_set_as_default_for_type(appitem, "audio/mp2", NULL);
                gboolean ret15 = g_app_info_set_as_default_for_type(appitem, "audio/x-wavpack", NULL);
                gboolean ret16 = g_app_info_set_as_default_for_type(appitem, "audio/x-ape", NULL);
                gboolean ret17 = g_app_info_set_as_default_for_type(appitem, "audio/x-mmf", NULL);
                gboolean ret18 = g_app_info_set_as_default_for_type(appitem, "audio/mp3", NULL);
                gboolean ret19 = g_app_info_set_as_default_for_type(appitem, "audio/flac", NULL);
                gboolean ret20 = g_app_info_set_as_default_for_type(appitem, "audio/wma", NULL);
                gboolean ret21 = g_app_info_set_as_default_for_type(appitem, "audio/amr", NULL);
                gboolean ret22 = g_app_info_set_as_default_for_type(appitem, "audio/vnd.dts", NULL);
                gboolean ret23 = g_app_info_set_as_default_for_type(appitem, "audio/basic", NULL);
                gboolean ret24 = g_app_info_set_as_default_for_type(appitem, "audio/x-aiff", NULL);
                gboolean ret25 = g_app_info_set_as_default_for_type(appitem, "audio/x-matroska", NULL);
                gboolean ret26 = g_app_info_set_as_default_for_type(appitem, "audio/x-voc", NULL);
                gboolean ret27 = g_app_info_set_as_default_for_type(appitem, "application/x-smaf", NULL);
                gboolean ret28 = g_app_info_set_as_default_for_type(appitem, "audio/m4a", NULL);
                if(ret1 && ret2 && ret3 && ret4 && ret5 && ret6 && ret7 && ret8 && ret9 && ret10 && ret11 && ret12 && ret13 && ret14
                        && ret15 && ret16 && ret17 && ret18 && ret19 && ret20 && ret21 && ret22 && ret23 && ret24 && ret25 && ret26
                        && ret27 && ret28) {
                    judge=true;
                }
                break;
            }
        }
    }
    return judge;
}

/* Set Default Text */
bool DefaultInterface::setTextEditorsDefautlProgram(char *appid) {
    const char * content_type = "text/plain";
    QVector<Appinfo> appinfo = _getAppList(content_type);

    bool judge = false;
    if (!appinfo.isEmpty()) {
        for (int i = 0;i < appinfo.size(); i++) {
            const char * id = g_app_info_get_id(appinfo[i].item);
            int result = strcmp(id,appid);
            if (0 == result) {
                GAppInfo *appitem = appinfo[i].item;
                gboolean ret1 = g_app_info_set_as_default_for_type(appitem, "text/plain", NULL);
                if(ret1)
                    judge=true;
                break;
            }
        }
    }

    return judge;
}

/* 获取可用应用信息列表 */
QVector<Appinfo> DefaultInterface::_getAppList(const char *contentType) {
    GList *applist;
    applist = g_app_info_get_all_for_type(contentType);
    GAppInfo * item;
    QVector<Appinfo> appinfo;
    appinfo.clear();

    if (applist != NULL) {
        int len = g_list_length(applist);

        //获取应用列表
        for (int index=0; index < len; index++) {
            item = (GAppInfo*) g_list_nth_data(applist, index);
            Appinfo ai;
            ai.item = item;
            appinfo.append(ai);
        }
    }
    return appinfo;
}

bool DefaultInterface::setSelfDefaultApp(const QString &desktopFile, const QString &mimeType) {
    qDebug() << Q_FUNC_INFO << "setSelfDefaultApp desktopFile" << desktopFile << "mimeType" << mimeType;
    GList* applist = g_app_info_get_all_for_type(mimeType.toUtf8().data());
    bool hitDesktopFile = false;
    if (applist != nullptr) {
        int len = g_list_length(applist);
        for (int index = 0; index < len; index++) {
            GAppInfo* appInfo = (GAppInfo*) g_list_nth_data(applist, index);
            if (!appInfo) {
                qDebug() << Q_FUNC_INFO << "index convert to GAppInfo failed" << index << mimeType;
                continue;
            }
            const char* storageId = g_app_info_get_id(appInfo);
            if (!storageId) {
                qDebug() << Q_FUNC_INFO << "g_app_info_get_id get storageId null" << index << mimeType;
                continue;
            }
            GDesktopAppInfo* desktopInfo = g_desktop_app_info_new(storageId);
            if (!desktopInfo) {
                qDebug() << Q_FUNC_INFO << "g_desktop_app_info_new get GDesktopAppInfo null" << index << mimeType;
                continue;
            }
            const char* filename = g_desktop_app_info_get_filename(desktopInfo);
            if (!filename) {
                qDebug() << Q_FUNC_INFO << "g_desktop_app_info_get_filename get absolute path null" << storageId << mimeType;
                continue;
            }
            if (QString(storageId) == desktopFile) {
                hitDesktopFile = true;
                qDebug() << Q_FUNC_INFO << "mimeType" << mimeType << "hit storageId" << storageId << "desktop absolute path" << filename;
                break;
            }
            qDebug() << Q_FUNC_INFO << "mimeType" << mimeType << "storageId" << storageId << "desktop absolute path" << filename;
        }
    }
    if (!hitDesktopFile) {
        qDebug() << Q_FUNC_INFO << "mimeType" << mimeType << "not find desktop" << desktopFile;
        return false;
    }

    GAppInfo* info = g_app_info_get_default_for_type(mimeType.toUtf8().data(), FALSE);
    if (info) {
        const char *defaultStorageId = g_app_info_get_id(info);
        if (QString(defaultStorageId) == desktopFile) {
            qDebug() << Q_FUNC_INFO << "g_app_info_get_default_for_type is already default app" << desktopFile << mimeType;
            return true;
        }
        qDebug() << Q_FUNC_INFO << "current default app" << defaultStorageId << mimeType;
    }
    qDebug() << Q_FUNC_INFO << "setSelfDefaultApp mimeType" << mimeType << "storageId" << desktopFile;
    bool ret = false;
    if (mimeType == BROWSERTYPE) {
        ret = setWebBrowsersDefaultProgram(desktopFile.toUtf8().data());
    } else if (mimeType == TEXTTYPE) {
        ret = setTextEditorsDefautlProgram(desktopFile.toUtf8().data());
    } else if (mimeType == MAILTYPE) {
        ret = setMailReadersDefaultProgram(desktopFile.toUtf8().data());
    } else if (mimeType == IMAGETYPE) {
        ret = setImageViewersDefaultProgram(desktopFile.toUtf8().data());
    } else if (mimeType == AUDIOTYPE) {
        ret = setAudioPlayersDefaultProgram(desktopFile.toUtf8().data());
    } else if (mimeType == VIDEOTYPE) {
        ret = setVideoPlayersDefaultProgram(desktopFile.toUtf8().data());
    }
    qDebug() << Q_FUNC_INFO << "setSelfDefaultApp status" << ret << "desktopFile" << desktopFile << "mimeType" << mimeType;
    if (ret) {
        emit defaultMimeTypeChanged(mimeType, desktopFile);
    }
    return true;
}

bool DefaultInterface::setDefaultApp(const QString &storageId, const QString &mimeType)
{
    qDebug() << Q_FUNC_INFO << "setDefaultApp mimeType" << mimeType << "storageId" << storageId;
    if (mimeType == BROWSERTYPE) {
        setWebBrowsersDefaultProgram(storageId.toUtf8().data());
    } else if (mimeType == TEXTTYPE) {
        setTextEditorsDefautlProgram(storageId.toUtf8().data());
    } else if (mimeType == MAILTYPE) {
        setMailReadersDefaultProgram(storageId.toUtf8().data());
    } else if (mimeType == IMAGETYPE) {
        setImageViewersDefaultProgram(storageId.toUtf8().data());
    } else if (mimeType == AUDIOTYPE) {
        setAudioPlayersDefaultProgram(storageId.toUtf8().data());
    } else if (mimeType == VIDEOTYPE) {
        setVideoPlayersDefaultProgram(storageId.toUtf8().data());
    }
    return true;
}

void DefaultInterface::saveMimeTypeAssociation(const QString &mime, const QString &storageId)
{
    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);
    if (profile->isConfigWritable(true)) {
        KConfigGroup defaultApp(profile, "Default Applications");
        defaultApp.writeEntry(mime, QStringList(storageId));

        KConfigGroup addedApps(profile, QStringLiteral("Added Associations"));
        QStringList apps = addedApps.readXdgListEntry(mime);
        apps.removeAll(storageId);
        apps.prepend(storageId);
        addedApps.writeXdgListEntry(mime, apps);
        profile->sync();
    }
}

const QDBusArgument &operator<<(QDBusArgument &argument, const Service &service)
{
    argument.beginStructure();
    argument << service.icon;
    argument << service.name;
    argument << service.storageId;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Service &service)
{
    argument.beginStructure();
    argument >> service.icon;
    argument >> service.name;
    argument >> service.storageId;
    argument.endStructure();
    return argument;
}

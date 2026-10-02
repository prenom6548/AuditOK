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
#ifndef DEFAULTINTERFACE_H
#define DEFAULTINTERFACE_H
#include <ukui-service-interface/serviceobject.h>
#include <QVariant>
#include <QVector>
#include <KSycoca>
#include <KApplicationTrader>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDBusArgument>
#include <QDBusMetaType>

#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

struct Service
{
    QString icon;
    QString name;
    QString storageId;
};

typedef struct _Applist
{
    QString strAppid;
    bool operator<(const _Applist &other) const {
        return strAppid < other.strAppid;
    }
}AppList;// 用于存放应用列表

typedef struct _AppInfo
{
    GAppInfo *item;
}Appinfo;// 用于存放应用列表信息

const QDBusArgument &operator<<(QDBusArgument &argument, const Service &service);
const QDBusArgument &operator>>(const QDBusArgument &argument, Service &service);
Q_DECLARE_METATYPE(Service)

class DefaultInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Default")
    Q_PROPERTY(QMap <QString , QVariant> sysDefaultList READ getSystemDefaultAppList)
public:
    DefaultInterface();

private:
    bool setWebBrowsersDefaultProgram(char *appid);
    bool setMailReadersDefaultProgram(char *appid);
    bool setImageViewersDefaultProgram(char *appid);
    bool setAudioPlayersDefaultProgram(char *appid);
    bool setVideoPlayersDefaultProgram(char *appid);
    bool setTextEditorsDefautlProgram(char *appid);
    static QVector<Appinfo> _getAppList(const char *contentType);

private:
    QString mLocalMimefile;
    static QMap <QString , QVariant> mSysDefaultList;
    QStringList mBrowserMimeTypeList;
    QStringList mMailMimeTypeList;
    QStringList mTextMimeTypeList;

protected:
    QMap <QString , QVariant> getSystemDefaultAppList();

Q_SIGNALS:
    void defaultChanged();
    void defaultMimeTypeChanged(const QString& content_type, const QString& appid);

public Q_SLOTS:
    QVariantList getDefaultApp(const QString &mimeType);
    QVariantList getAppList(const QString &mimeType);
    bool setDefaultApp(const QString &storageId, const QString &mimeType);
    void saveMimeTypeAssociation(const QString &mime, const QString &storageId);
    void resetDefault();
    bool setSelfDefaultApp(const QString &desktopFile, const QString &mimeType);
};

#endif // DEFAULTINTERFACE_H

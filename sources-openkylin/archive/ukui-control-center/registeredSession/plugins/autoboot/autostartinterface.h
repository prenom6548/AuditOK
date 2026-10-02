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
#ifndef AUTOSTARTINTERFACE_H
#define AUTOSTARTINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QDBusArgument>
#include <QVariant>
#include <QDBusMetaType>
#include <QGSettings>

#ifdef signals
#undef signals
#endif
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

enum Pos
{
    LOCALPOS,
    ALLPOS,
    SYSTEMPOS
};

struct AutoApp
{
    QString bname;
    QString icon;
    QString name;
    bool hidden;
    int position;

    QString path;
    bool no_display;
    bool shown;
    QString comment;
    QString exec;
    QString description;
};
const QDBusArgument &operator<<(QDBusArgument &argument, const AutoApp &app);
const QDBusArgument &operator>>(const QDBusArgument &argument, AutoApp &app);
Q_DECLARE_METATYPE(AutoApp)

class AutoStartInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Autoboot")
    Q_PROPERTY(QMap <QString , QVariant> statusMap READ getStatusMap)
    Q_PROPERTY(QStringList appList READ getAppList)
public:
    explicit AutoStartInterface(QObject *parent = nullptr);
    AutoApp getAppInfo(const QString &filepath);
    AutoApp setInformation(QString filepath);

protected:
    QStringList getAppList();
    QMap <QString, QVariant> getStatusMap();

private:
    QMap <QString , AutoApp> getLocalAppMap();
    QMap <QString , AutoApp> getServiceMap();
    QMap <QString , AutoApp> getAllAppMap();
    gboolean keyFileToTile(GKeyFile *keyfile, const gchar *path);
    // 仅允许 autostart 目录下的纯文件名，拒绝路径穿越
    static QString sanitizeAutostartFilename(const QString &filename);
    static bool isPathInsideDir(const QString &filePath, const QString &dirPath);
    bool copyFiletoLocal(const QString &filename);
    bool setAppHiddenInfo(const QString &filename, bool hidden);

public Q_SLOTS:
    bool deleteLocalFile(const QString &filename);
    bool addAutobootApp(const QString &filename);
    bool saveAppStatus(const QString &filename, bool status);
    bool setApplist(const QStringList &list);
    bool getDisplayStatus(const QString &filename);
    bool setAutobootAllBlack();
    bool setAutobootBlacklist(QStringList blacklist);
    bool setAutobootWhitelist(QStringList whitelist);

Q_SIGNALS:
    void changed(const QString &key);

private:
    QMap <QString , AutoApp> mSysServiceMap;
    QMap <QString , AutoApp> mAllAppMap;

    QGSettings *mUkccGsettings = nullptr;
    QGSettings *mStyleGsettings = nullptr;
    QStringList m_blackList;
};

#endif // AUTOSTARTINTERFACE_H

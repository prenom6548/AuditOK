/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MEDIAKEYBLOCKSHUTCUT_H
#define MEDIAKEYBLOCKSHUTCUT_H


#include <QDBusConnectionInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusServiceWatcher>
#include <QDBusPendingCall>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QMutexLocker>
#include <QDBusReply>
#include <QMutex>
#include <QDebug>

#include "clib-syslog.h"
#include "usd_base_class.h"
#include "usd_global_define.h"

#define USD_BLOCK_SETTINGS_DAEMON_STARTWITH  "org.ukui.settingsDaemon.shortcut."
#define USD_BLOCK_SETTINGS_DAEMON_SHORTCUT  "org.ukui.settingsDaemon.shortcut*"
#define USD_BLOCK_SETTINGS_DAEMON_PATH      "/org/ukui/settingsDaemon/shortcut"
#define USD_BLOCK_SETTINGS_DAEMON_INTERFEACE  "org.ukui.settingsDaemon.shortcut"

#define USD_BLOCK_SETTINGS_DAEMON_METHOD  "blockShortcuts"
#define USD_UNBLOCK_SETTINGS_DAEMON_METHOD  "unblockShortcuts"

class MediaKeyBlockShortcutPrivate;

class MediaKeyBlockShutcut:public QObject
{
    Q_OBJECT
public:
    explicit MediaKeyBlockShutcut(QObject *parent = nullptr);
    ~MediaKeyBlockShutcut();

    QMap<QString, QStringList> getBlockList();

Q_SIGNALS:
    void blockChanged(QMap<QString, QStringList> blockName);
    void unblockChanged(QMap<QString, QStringList> blockName);
private:
    MediaKeyBlockShortcutPrivate *d = nullptr;
};

#endif // MEDIAKEYBLOCKSHUTCUT_H

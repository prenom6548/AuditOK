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
#include "getshortcutworker.h"
#include <QDebug>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include "../../universalinterface.h"
#include "type.h"

static const char *const kWlcomService = "com.kylin.Wlcom";
static const char *const kWlcomShortcutsManagerPath = "/com/kylin/Wlcom/GlobalShortcuts";
static const char *const kWlcomShortcutsManagerIface = "com.kylin.Wlcom.GlobalShortcuts.Manager";
static const char *const kWlcomShortcutsSessionPath = "/com/kylin/Wlcom/GlobalShortcuts/Session/kylin_wlcom";
static const char *const kWlcomShortcutsSessionIface = "com.kylin.Wlcom.GlobalShortcuts.Session";
static const char *const kWlcomSessionId = "kylin_wlcom";
// 与 ShortcutInterface 一致：清除占位键，读回时映射为 Null
static const char *const kWlcomClearedKey = "win+f24";

// 窗口应用区只展示「显示或隐藏桌面」（Win+D / desktop_toggle_visibility）
static bool isShowDesktopWindowShortcut(const QString &actionId, const QString &name)
{
    const QString id = actionId.toLower();
    if (id == QLatin1String("desktop_toggle_visibility")) {
        return true;
    }
    if (name.contains(QStringLiteral("显示或隐藏桌面"))) {
        return true;
    }
    const QString n = name.toLower();
    return n.contains(QLatin1String("show or hide desktop"));
}

static QVariantMap variantToMap(const QVariant &var)
{
    if (var.canConvert<QVariantMap>()) {
        return var.toMap();
    }
    if (var.canConvert<QDBusArgument>()) {
        return qdbus_cast<QVariantMap>(var);
    }
    const QDBusArgument arg = var.value<QDBusArgument>();
    if (arg.currentType() != QDBusArgument::UnknownType) {
        return qdbus_cast<QVariantMap>(arg);
    }
    return QVariantMap();
}

static bool ensureWlcomShortcutSession()
{
    QDBusInterface session(kWlcomService,
                           kWlcomShortcutsSessionPath,
                           kWlcomShortcutsSessionIface,
                           QDBusConnection::sessionBus());
    if (session.isValid()) {
        return true;
    }

    QDBusInterface manager(kWlcomService,
                           kWlcomShortcutsManagerPath,
                           kWlcomShortcutsManagerIface,
                           QDBusConnection::sessionBus());
    if (!manager.isValid()) {
        qWarning() << Q_FUNC_INFO << "wlcom GlobalShortcuts manager invalid"
                   << manager.lastError().message();
        return false;
    }

    QDBusMessage reply = manager.call(QStringLiteral("CreateSession"),
                                      QString::fromLatin1(kWlcomSessionId));
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << Q_FUNC_INFO << "CreateSession failed" << reply.errorMessage();
        return false;
    }
    return true;
}

GetShortcutWorker::GetShortcutWorker(QObject *parent)
    : QThread(parent) {

}

GetShortcutWorker::~GetShortcutWorker() {

}

bool GetShortcutWorker::getKwinWindowShortcut()
{
    QDBusInterface *kwinInterface = UniversalInterface::self()->kwinDbusInterface();
    if (!kwinInterface || !kwinInterface->isValid()) {
        qDebug() << Q_FUNC_INFO << "kwin interface failed";
        return false;
    }

    QDBusReply<QVariantList> res = kwinInterface->call("getShortcutInfos");
    if (!res.isValid()) {
        qDebug() << Q_FUNC_INFO << "getShortcutInfos failed" << res.error();
        return false;
    }
    if (res.value().isEmpty()) {
        qDebug() << Q_FUNC_INFO << "getShortcutInfos empty";
        return false;
    }

    const static int WIN_SHORTCUT_MIN_LEN = 3;
    const static int WIN_SHORTCUT_MAX_LEN = 4;
    qDebug() << Q_FUNC_INFO << "window shortcut size" << res.value().size();
    int emitted = 0;
    for (auto &shortcutInfo : res.value()) {
        QStringList kwinStringList = shortcutInfo.toStringList();
        int size = kwinStringList.size();

        if (size < WIN_SHORTCUT_MIN_LEN) {
            continue;
        }
        QString key = kwinStringList.at(0);
        QString name = kwinStringList.at(1);
        if (!isShowDesktopWindowShortcut(key, name)) {
            continue;
        }
        for (int index = 2; index < size && index < WIN_SHORTCUT_MAX_LEN; ++index) {
            QString value = kwinStringList.at(index);
            if (index == WIN_SHORTCUT_MIN_LEN) {
                key = key + "2";
            }
            Q_EMIT windowShortcutGenerate(name, key, value);
            ++emitted;
        }
    }
    // kwin 有接口但无「显示桌面」时回落 wlcom，避免窗口应用区空白
    return emitted > 0;
}

bool GetShortcutWorker::getWlcomWindowShortcut()
{
    if (!ensureWlcomShortcutSession()) {
        return false;
    }

    QDBusInterface session(kWlcomService,
                           kWlcomShortcutsSessionPath,
                           kWlcomShortcutsSessionIface,
                           QDBusConnection::sessionBus());
    if (!session.isValid()) {
        qWarning() << Q_FUNC_INFO << "wlcom shortcut session invalid"
                   << session.lastError().message();
        return false;
    }

    QDBusReply<QVariantMap> reply = session.call(QStringLiteral("ListShortcuts"));
    if (!reply.isValid()) {
        qWarning() << Q_FUNC_INFO << "ListShortcuts failed" << reply.error();
        return false;
    }

    const QVariantMap shortcuts = reply.value();
    qDebug() << Q_FUNC_INFO << "wlcom window shortcut size" << shortcuts.size();
    if (shortcuts.isEmpty()) {
        return false;
    }

    for (auto it = shortcuts.constBegin(); it != shortcuts.constEnd(); ++it) {
        const QString actionId = it.key();
        const QVariantMap info = variantToMap(it.value());
        if (info.isEmpty()) {
            continue;
        }

        QString name = info.value(QStringLiteral("desc_l10n")).toString();
        if (name.isEmpty()) {
            name = info.value(QStringLiteral("desc")).toString();
        }
        if (name.isEmpty()) {
            name = actionId;
        }
        if (!isShowDesktopWindowShortcut(actionId, name)) {
            continue;
        }

        QString value = info.value(QStringLiteral("current_key")).toString().trimmed();
        // 清除占位键显示为「无」，勿回落到 default_key
        if (value.compare(QString::fromLatin1(kWlcomClearedKey), Qt::CaseInsensitive) == 0) {
            value = QStringLiteral("Null");
        } else if (value.isEmpty()) {
            value = info.value(QStringLiteral("default_key")).toString().trimmed();
        }
        if (value.isEmpty()) {
            value = QStringLiteral("Null");
        }

        Q_EMIT windowShortcutGenerate(name, actionId, value);
    }
    return true;
}

void GetShortcutWorker::getWindowShortcut() {
    if (getKwinWindowShortcut()) {
        return;
    }
    qDebug() << Q_FUNC_INFO << "fallback to wlcom GlobalShortcuts";
    if (!getWlcomWindowShortcut()) {
        qWarning() << Q_FUNC_INFO << "failed to load window shortcuts from kwin and wlcom";
    }
}

void GetShortcutWorker::getSystemShortcut() {
    GSettings *desktopsettings = nullptr;
    if (QGSettings::isSchemaInstalled(KEYBINDINGS_DESKTOP_SCHEMA)) {
        desktopsettings = g_settings_new(KEYBINDINGS_DESKTOP_SCHEMA);
        char **dkeys = g_settings_list_keys(desktopsettings);
        for (int i = 0; dkeys[i] != nullptr; i++) {
            // 跳过非快捷键
            if (!g_strcmp0(dkeys[i], "active") || !g_strcmp0(dkeys[i], "volume-step")
                || !g_strcmp0(dkeys[i], "priority") || !g_strcmp0(dkeys[i], "enable-osd")) {
                continue;
            }

            GVariant *variant = g_settings_get_value(desktopsettings, dkeys[i]);
            gsize size = g_variant_get_size(variant);
            char *str   = const_cast<char *>(g_variant_get_string(variant, &size));

            // 保存桌面快捷键
            QString key = QString(dkeys[i]);
            QString value = QString(str);
            if (value.contains("KP_Delete")) {
                value = "<Ctrl><Alt>Del";
                Q_EMIT generalShortcutGenerate(KEYBINDINGS_DESKTOP_SCHEMA, key, value);
            }
            if (value != "" && !value.contains("XF86") && !value.contains("KP_")) {
                Q_EMIT generalShortcutGenerate(KEYBINDINGS_DESKTOP_SCHEMA, key, value);
            }
        }
        g_strfreev(dkeys);
        g_object_unref(desktopsettings);
    }
}

void GetShortcutWorker::getCustomShortcut() {
    QList<char *> existsPath = listExistsCustomShortcutPath();
    for (char *path : existsPath) {
        QString strFullPath = QString(KEYBINDINGS_CUSTOM_DIR);
        strFullPath.append(path);

        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(strFullPath.toLatin1().data());
        QGSettings settings(ba, bba);
        QString pathStr = strFullPath;
        QString actionStr = settings.get(ACTION_KEY).toString();
        QString bindingStr = settings.get(BINDING_KEY).toString();
        QString nameStr = settings.get(NAME_KEY).toString();
        Q_EMIT customShortcutGenerate(pathStr, nameStr, bindingStr, actionStr);
    }
}

void GetShortcutWorker::run() {
    // get window shortcut
    getWindowShortcut();

    // get desktop shortcut
    getSystemShortcut();

    // get custdom shortcut
    getCustomShortcut();

    Q_EMIT workerComplete();
}

QList<char *> GetShortcutWorker::listExistsCustomShortcutPath() {
    char ** childs;
    int len;

    DConfClient * client = dconf_client_new();
    childs = dconf_client_list (client, KEYBINDINGS_CUSTOM_DIR, &len);
    g_object_unref (client);

    QList<char *> vals;

    for (int i = 0; childs[i] != nullptr; i++){
        if (dconf_is_rel_dir (childs[i], nullptr)){
            char * val = g_strdup (childs[i]);

            vals.append(val);
        }
    }
    g_strfreev (childs);
    return vals;
}

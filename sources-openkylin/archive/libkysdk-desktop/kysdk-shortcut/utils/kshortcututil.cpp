/*
 * libkysdk-shortcut's Library
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

#include "kshortcututil.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QGSettings>

KShortcutUtil::KShortcutUtil()
{
}

QStringList KShortcutUtil::listExistsShortcutPath(const char *dir)
{
    int len;
    DConfClient *client = dconf_client_new();
    char **childs = dconf_client_list(client, dir, &len);
    g_object_unref(client);

    QStringList vals;
    for (int i = 0; childs[i] != nullptr; i++)
    {
        if (dconf_is_rel_dir(childs[i], nullptr))
        {
            char *val = g_strdup(childs[i]);

            vals.append(val);
        }
    }
    g_strfreev(childs);
    return vals;
}

QString KShortcutUtil::findFreePath(const char *dir)
{
    int i = 0;
    QString d;
    QStringList existsdirs;

    existsdirs = listExistsShortcutPath(dir);

    for (; i < MAX_CUSTOM_SHORTCUTS; i++)
    {
        d = QString("custom%1/").arg(i);
        if (!existsdirs.contains(d))
        {
            break;
        }
    }

    if (i == MAX_CUSTOM_SHORTCUTS)
    {
        return "";
    }

    return QString("%1%2").arg(KYSDK_KEYBINDINGS_DIR).arg(QString(d));
}

QString KShortcutUtil::findTargetPath(QString name, const char *dir)
{
    QString result;
    QStringList existsdirs;
    existsdirs = listExistsShortcutPath(dir);

    DConfClient *client = dconf_client_new();
    for (QString dir : existsdirs)
    {
        QString gsettingsPath = QString("%1%2name").arg(KYSDK_KEYBINDINGS_DIR).arg(dir);
        GVariant *variant = dconf_client_read(client, gsettingsPath.toLatin1().constData());
        if (NULL != variant)
        {
            gsize size = g_variant_get_size(variant);
            const char *value = g_variant_get_string(variant, &size);
            if (0 == name.compare(value))
            {
                g_variant_unref(variant);
                result = QString("%1%2").arg(KYSDK_KEYBINDINGS_DIR).arg(dir);
                break;
            }
        }
    }
    g_object_unref(client);
    return result;
}

QList<KeyEntry> KShortcutUtil::getKysdkShortcuts()
{
    QList<KeyEntry> result;
    QStringList existsPath = listExistsShortcutPath(KYSDK_KEYBINDINGS_DIR);
    for (QString dir : existsPath)
    {
        QString strFullPath = QString("%1%2").arg(KYSDK_KEYBINDINGS_DIR).arg(dir);

        const QByteArray ba(KYSDK_KEYBINDING_SCHEMA);
        const QByteArray bba(strFullPath.toLatin1().data());
        QGSettings settings(ba, bba);
        QString pathStr = strFullPath;
        QString actionStr = settings.get(ACTION_KEY).toString();
        QString bindingStr = settings.get(BINDING_KEY).toString();
        QString nameStr = settings.get(NAME_KEY).toString();

        KeyEntry customKeyEntry;
        customKeyEntry.gsSchema = KYSDK_KEYBINDING_SCHEMA;
        customKeyEntry.gsPath = pathStr;
        customKeyEntry.name = nameStr;
        customKeyEntry.binding = bindingStr;
        customKeyEntry.action = actionStr;
        result.append(customKeyEntry);
    }
    return result;
}

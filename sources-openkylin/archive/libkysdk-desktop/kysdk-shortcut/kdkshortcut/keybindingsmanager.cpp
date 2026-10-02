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

#include "keybindingsmanager.h"
#include <QGSettings/QGSettings>
#include <QDir>
#include <QSettings>

#define KYSDK_COMPONET_NAME "kysdk-keybindings"

KeybindingsManager *KeybindingsManager::mKeybindingsManager = nullptr;
KeybindingsManager::KeybindingsManager(QObject *parent)
    : QObject(parent),
      mDconfClient(nullptr)
{
    start();
}

KeybindingsManager::~KeybindingsManager()
{
    if (mDconfClient)
    {
        dconf_client_unwatch_fast(mDconfClient, KYSDK_KEYBINDINGS_DIR);
        dconf_client_unwatch_sync(mDconfClient, KYSDK_KEYBINDINGS_DIR);
        g_object_unref(mDconfClient);
        mDconfClient = nullptr;
    }
    clearShortcutList();
}

KeybindingsManager *KeybindingsManager::getKeybindingsManager()
{
    if (mKeybindingsManager == nullptr)
        mKeybindingsManager = new KeybindingsManager();
    return mKeybindingsManager;
}

bool KeybindingsManager::start()
{
    clearKglobalShortcutAll();
    registerShortcutAll();
    if (!mDconfClient)
    {
        mDconfClient = dconf_client_new();
        dconf_client_watch_fast(mDconfClient, KYSDK_KEYBINDINGS_DIR);
        dconf_client_watch_sync(mDconfClient, KYSDK_KEYBINDINGS_DIR);
        g_signal_connect(mDconfClient, "changed", G_CALLBACK(bindingsCallback), this);
    }

    return true;
}

void KeybindingsManager::stop()
{
    if (mDconfClient)
    {
        dconf_client_unwatch_fast(mDconfClient, KYSDK_KEYBINDINGS_DIR);
        dconf_client_unwatch_sync(mDconfClient, KYSDK_KEYBINDINGS_DIR);
        g_object_unref(mDconfClient);
        mDconfClient = nullptr;
    }
    clearShortcutList();
}

void KeybindingsManager::bindingsCallback(DConfClient *client, gchar *prefix, const gchar **changes, gchar *tag, KeybindingsManager *manager)
{
    Q_UNUSED(client)
    Q_UNUSED(changes)
    Q_UNUSED(tag)

    if (strncmp(KYSDK_KEYBINDINGS_DIR, prefix, strlen(KYSDK_KEYBINDINGS_DIR)))
    {
        return;
    }

    // 卸载所有自定义快捷键
    manager->unRegisterShortcutAll();
    // 重新注册已定义的热键
    manager->registerShortcutAll();
    qDebug() << prefix;
}

void KeybindingsManager::registerShortcutAll()
{
    KShortcutUtil util;
    QStringList existsDirs = util.listExistsShortcutPath(KYSDK_KEYBINDINGS_DIR);
    for (QString dir : existsDirs)
    {
        QString gsettingsPath = QString("%1%2").arg(KYSDK_KEYBINDINGS_DIR).arg(dir);
        QGSettings setting(KYSDK_KEYBINDING_SCHEMA, gsettingsPath.toLatin1());
        QString name = setting.get(NAME_KEY).toString();
        QString binding = setting.get(BINDING_KEY).toString();
        QString action = setting.get(ACTION_KEY).toString();
        Keybind *key = new Keybind(name, binding, action, gsettingsPath, KYSDK_COMPONET_NAME, this);
        mKeybinds.append(key);
    }
}

void KeybindingsManager::unRegisterShortcutAll()
{
    if (mKeybinds.isEmpty())
        return;
    for (Keybind *key : mKeybinds)
    {
        KGlobalAccel::self()->removeAllShortcuts(key->action());
    }
    qDeleteAll(mKeybinds);
    mKeybinds.clear();
}

void KeybindingsManager::clearShortcutList()
{
    if (mKeybinds.isEmpty())
    {
        return;
    }
    qDeleteAll(mKeybinds);
    mKeybinds.clear();
}

void KeybindingsManager::clearKglobalShortcutAll()
{
    QString path = QDir::homePath() + QStringLiteral("/.config/kglobalshortcutsrc");
    qDebug() << path;
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(KYSDK_COMPONET_NAME);
    QStringList strList = settings.allKeys();
    for (QString str : strList)
    {
        QStringList list = settings.value(str).toStringList();
        if (list.contains("kdkshortcut"))
        {
            continue;
        }
        QAction action;
        action.setObjectName(str.toLatin1().data());
        action.setProperty("componentName", KYSDK_COMPONET_NAME);
        QList<QKeySequence> seq = QKeySequence::listFromString(list.at(0));
        KGlobalAccel::self()->setDefaultShortcut(&action, seq);
        KGlobalAccel::self()->setShortcut(&action, seq);
        KGlobalAccel::self()->removeAllShortcuts(&action);
    }
    settings.endGroup();
}

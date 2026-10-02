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

#include "libkyshortcut.h"
#include "kglobalshortcut.h"

#define KEYBINDINGS_DESKTOP_SCHEMA "org.ukui.SettingsDaemon.plugins.media-keys"
#define KEYBINDINGS_SYSTEM_SCHEMA "org.gnome.desktop.wm.keybindings"
#define KEYBINDINGS_CUSTOM_SCHEMA "org.ukui.control-center.keybinding"
#define KEYBINDINGS_CUSTOM_DIR "/org/ukui/desktop/keybindings/"
#define ACTION_KEY "action"
#define BINDING_KEY "binding"
#define NAME_KEY "name"
#define DEFAULT_BINDING "disable"
#define DEFAULT_SHORTCUT "defaultShortcut"
#define DEFAULT_SHORTCUT_SCHEMA "org.ukui.control-center"
#define MAX_CUSTOM_SHORTCUTS 1000

/*******************************KDK FUNC*****************************************/

int kdk_shortcut_create_global_shortcut(const char *name, const char *key, const char *action)
{
    kdk::KGlobalShortcut kGlobalShortcut;
    return kGlobalShortcut.creatGlobalShortcut(name, key, action);
}

int kdk_shortcut_delete_global_shortcut(const char *name)
{
    kdk::KGlobalShortcut kGlobalShortcut;
    return kGlobalShortcut.deleteGlobalShortcut(name);
}

int kdk_shortcut_global_shortcut_is_available(const char *key)
{
    kdk::KGlobalShortcut kGlobalShortcut;
    return kGlobalShortcut.globalShortcutIsAvailable(key);
}

int kdk_shortcut_set_global_shortcut(const char *name, const char *key, const char *action)
{
    kdk::KGlobalShortcut kGlobalShortcut;
    return kGlobalShortcut.setGlobalShortcut(name, key, action);
}

KYSDKGlobalShortcutInfoList *kdk_shortcut_get_global_shortcuts()
{
    kdk::KGlobalShortcut kGlobalShortcut;
    return kGlobalShortcut.getGlobalShortcuts();
}

KYSDKGlobalShortcutInfoList *kdk_shortcut_get_global_shortcuts_by_key(const char *key)
{
    kdk::KGlobalShortcut kGlobalShortcut;
    return kGlobalShortcut.getGlobalShortcutByKey(key);
}

void kdk_shortcut_destroy_info_list(KYSDKGlobalShortcutInfoList *list)
{
    if (NULL == list)
        return;

    KYSDKGlobalShortcutInfoList *tmp = nullptr;
    while (list)
    {
        tmp = list;
        list = list->next;

        if (tmp->data->name)
            free(tmp->data->name);
        if (tmp->data->action)
            free(tmp->data->action);
        free(tmp->data);
        free(tmp);
    }
}

const char *kdk_shortcut_key_combination_to_string(unsigned int key)
{
    static QByteArray result;
    result.clear();
    kdk::KGlobalShortcut kGlobalShortcut;
    result = kGlobalShortcut.numberToKey(key).toLatin1();
    return result.constData();
}

unsigned int kdk_shortcut_string_to_key_combination(const char *key)
{
    kdk::KGlobalShortcut kGlobalShortcut;
    return kGlobalShortcut.keyToNumber(key);
}

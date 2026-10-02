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

#ifndef DCONFUTIL_H
#define DCONFUTIL_H

#include <QObject>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C"
{
#include <gio/gio.h>
#include <dconf/dconf.h>
#include <dconf/client/dconf-client.h>
#include <dconf/common/dconf-paths.h>
}

#define MAX_CUSTOM_SHORTCUTS 1000
#define KYSDK_KEYBINDINGS_DIR "/com/kylin/kysdk/keybindings/"
#define KYSDK_KEYBINDING_SCHEMA "com.kylin.kysdk.keybinding"
#define ACTION_KEY "action"
#define BINDING_KEY "binding"
#define NAME_KEY "name"

typedef struct _KeyEntry
{
    QString gsSchema;
    QString gsPath;
    QString name;
    QString binding;
    QString action;
} KeyEntry;

class KShortcutUtil
{
public:
    explicit KShortcutUtil();

public:
    QStringList listExistsShortcutPath(const char *dir);
    QString findFreePath(const char *dir);
    QString findTargetPath(QString name, const char *dir);
    QList<KeyEntry> getKysdkShortcuts();
};

#endif // DCONFUTIL_H

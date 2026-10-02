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

#ifndef KeybindingsManager_H
#define KeybindingsManager_H

#include <QObject>
#include "keybind.h"
#include "../utils/kshortcututil.h"

class KeybindingsManager : public QObject
{
    Q_OBJECT

private:
    KeybindingsManager(QObject *parent = nullptr);
    KeybindingsManager(KeybindingsManager &) = delete;

public:
    ~KeybindingsManager();
    static KeybindingsManager *getKeybindingsManager();
    bool start();
    void stop();

public:
    static void bindingsCallback(DConfClient *client,
                                  gchar *prefix,
                                  const gchar **changes,
                                  gchar *tag,
                                  KeybindingsManager *manager);

private:
    void registerShortcutAll();
    void unRegisterShortcutAll();
    void clearShortcutList();

    void clearKglobalShortcutAll();

private:
    static KeybindingsManager *mKeybindingsManager;
    DConfClient *mDconfClient;
    QList<Keybind *> mKeybinds;
};

#endif // KeybindingsManager_H

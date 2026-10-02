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
#ifndef GETSHORTCUTWORKER_H
#define GETSHORTCUTWORKER_H

#include <QObject>
#include <QList>
#include <QGSettings>
#include <QSettings>
#include <QThread>

#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <dconf/dconf.h>
#include <dconf/common/dconf-paths.h>
}

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

class GetShortcutWorker : public QThread
{
    Q_OBJECT

public:
    explicit GetShortcutWorker(QObject *parent = nullptr);
    ~GetShortcutWorker();

public:
    void run();
    QList<char *> listExistsCustomShortcutPath();

private:
    void getWindowShortcut();
    bool getKwinWindowShortcut();
    bool getWlcomWindowShortcut();
    void getSystemShortcut();
    void getCustomShortcut();

Q_SIGNALS:
    void generalShortcutGenerate(QString schema, QString key, QString value);
    void windowShortcutGenerate(QString name, QString key, QString value);
    void customShortcutGenerate(QString path, QString name, QString bindingkey, QString action);
    void workerComplete();
};

#endif // GETSHORTCUTWORKER_H

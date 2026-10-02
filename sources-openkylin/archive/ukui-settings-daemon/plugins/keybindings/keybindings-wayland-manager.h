/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2022 KylinSoft Co., Ltd.
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
#ifndef KEYBINDINGSWAYLANDMANAGER_H
#define KEYBINDINGSWAYLANDsMANAGER_H

#include <QApplication>
#include <QObject>
#include <QTimer>
#include <QGSettings>
#include <QSettings>
#include <QAction>
#include <QKeySequence>
#include <KF5/KGlobalAccel/KGlobalAccel>

extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <dconf/dconf.h>
#include <gio/gdesktopappinfo.h>
#include <dconf/common/dconf-paths.h>
}

class ShortCutKeyBind;

class KeybindingsWaylandManager
{
private:
    KeybindingsWaylandManager();
    KeybindingsWaylandManager(KeybindingsWaylandManager&)=delete;

public:
    ~KeybindingsWaylandManager();
    static KeybindingsWaylandManager *KeybindingsWaylandManagerNew();
    bool start();
    void stop();
public:
    static void bindings_callback (DConfClient  *client,
                                   gchar        *prefix,
                                   const gchar **changes,
                                   gchar        *tag,
                                   KeybindingsWaylandManager *manager);
private:
    QStringList getCustomShortcutPath();
    void registerShortcutAll();
    void unRegisterShortcutAll();
    void clearShortcutList();

    void clearKglobalShortcutAll();

private:
    static KeybindingsWaylandManager *m_keybinding;
    DConfClient                      *m_dconfClient = nullptr;
    QList<ShortCutKeyBind*>          m_shortcutList;

};
/*************ShortCutKeyBind**************/
class ShortCutKeyBind : public QObject
{
    Q_OBJECT
public:
    ShortCutKeyBind(QString settingsPath, QString actionName, QString bindKey, QString execName,QString componetName ,QObject *parent = nullptr);
    ~ShortCutKeyBind();
    QString settingPath(){return m_settingsPath;}
    QAction* action(){return m_action;}
private:
    void setUp();
    void setShortcut();
    static void parsingDesktop(QString);
    static void parsingDesktopInGIO(QString);
    QList<QKeySequence> listFromString();

private:

    QString m_settingsPath;
    QString m_actionName;//名称唯一
    QString m_bindKey;//绑定组合键
    QString m_execName;//组合键动作
    QString m_componentName;//组名称
    QAction *m_action;
};

#endif // KEYBINDINGSWAYLANDMANAGER_H

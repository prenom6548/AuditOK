/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2020 KylinSoft Co., Ltd.
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
#include <QDebug>
#include <QDir>
#include <QProcess>
#include "keybindings-wayland-manager.h"
#include "clib-syslog.h"

#define GSETTINGS_KEYBINDINGS_DIR "/org/ukui/desktop/keybindings/"
#define CUSTOM_KEYBINDING_SCHEMA  "org.ukui.control-center.keybinding"
#define ACTION "action"
#define BINDING "binding"
#define NAME "name"

#define CUSTOM_COMPONET_NAME    "usd_keybindings"

KeybindingsWaylandManager *KeybindingsWaylandManager::m_keybinding = nullptr;

KeybindingsWaylandManager::KeybindingsWaylandManager()
{

}

KeybindingsWaylandManager::~KeybindingsWaylandManager()
{
    if (m_dconfClient) {
        dconf_client_unwatch_fast(m_dconfClient, GSETTINGS_KEYBINDINGS_DIR);
        dconf_client_unwatch_sync(m_dconfClient, GSETTINGS_KEYBINDINGS_DIR);
        g_object_unref (m_dconfClient);
        m_dconfClient = nullptr;
    }
    clearShortcutList();
}

KeybindingsWaylandManager *KeybindingsWaylandManager::KeybindingsWaylandManagerNew()
{
    if(nullptr == m_keybinding)
        m_keybinding = new KeybindingsWaylandManager();
    return m_keybinding;
}

void KeybindingsWaylandManager::bindings_callback (DConfClient  *client,
                                            gchar        *prefix,
                                            const gchar **changes,
                                            gchar        *tag,
                                            KeybindingsWaylandManager *manager)
{
    Q_UNUSED(client)
    Q_UNUSED(changes)
    Q_UNUSED(tag)

    if (strncmp(GSETTINGS_KEYBINDINGS_DIR,prefix,strlen(GSETTINGS_KEYBINDINGS_DIR))) {
        return;
    }

    //卸载所有自定义快捷键
    manager->unRegisterShortcutAll();
    //重新注册已定义的热键
    manager->registerShortcutAll();
    qDebug()<<prefix;
}

bool KeybindingsWaylandManager::start()
{
    USD_LOG(LOG_DEBUG,"-- Keybindings Wayland Manager Start --");
    //清除kglobalaceel 的配置
    clearKglobalShortcutAll();
    registerShortcutAll();
    if(!m_dconfClient){
        m_dconfClient = dconf_client_new ();
        dconf_client_watch_fast (m_dconfClient, GSETTINGS_KEYBINDINGS_DIR);
        dconf_client_watch_sync (m_dconfClient, GSETTINGS_KEYBINDINGS_DIR);
        g_signal_connect (m_dconfClient, "changed", G_CALLBACK (bindings_callback), this);
    }

    return true;
}

QStringList KeybindingsWaylandManager::getCustomShortcutPath()
{
    int len;
    gchar ** childs;
    QStringList path;
    DConfClient * client = dconf_client_new();
    childs = dconf_client_list (client, GSETTINGS_KEYBINDINGS_DIR, &len);
    g_object_unref (client);

    for (int i = 0; childs[i] != NULL; i++){
        if (dconf_is_rel_dir (childs[i], NULL)){
            gchar * val = g_strdup (childs[i]);
            path<<val;
        }
    }
    g_strfreev (childs);
    return path;
}

void KeybindingsWaylandManager::registerShortcutAll()
{
    QStringList customPaths = getCustomShortcutPath();
    for (QString path : customPaths) {
        QString customPath = QString(GSETTINGS_KEYBINDINGS_DIR) + path;
        GSettings* settings;
        settings = g_settings_new_with_path (CUSTOM_KEYBINDING_SCHEMA, customPath.toLatin1().data());
        if(settings){
            QString path = customPath;
            QString action = QString(g_settings_get_string (settings, ACTION));
            QString binding = QString(g_settings_get_string (settings, BINDING));
            QString name = QString(g_settings_get_string (settings, NAME));
            USD_LOG(LOG_DEBUG,"keybindings name : %s   key : %s  action : %s",
                    name.toLocal8Bit().data(),binding.toLatin1().data(),action.toLatin1().data());
            ShortCutKeyBind* bind = new ShortCutKeyBind(path,name,binding,action,CUSTOM_COMPONET_NAME);
            m_shortcutList.append(bind);
            g_object_unref (settings);
        }
    }
}
void KeybindingsWaylandManager::clearKglobalShortcutAll()
{
    QString path = QDir::homePath() + QStringLiteral("/.config/kglobalshortcutsrc");
    qDebug()<<path;
    QSettings settings(path,QSettings::IniFormat);
    settings.beginGroup(CUSTOM_COMPONET_NAME);
    QStringList strList = settings.allKeys();
    for(QString str : strList){
        QStringList list = settings.value(str).toStringList();
        if(list.contains("ukui-settings-daemon")){
            continue;
        }
        QAction action;
        action.setObjectName(str.toLatin1().data());
        action.setProperty("componentName",CUSTOM_COMPONET_NAME);
        QList<QKeySequence> seq = QKeySequence::listFromString(list.at(0));
        KGlobalAccel::self()->setDefaultShortcut(&action, seq);
        KGlobalAccel::self()->setShortcut(&action, seq);
        KGlobalAccel::self()->removeAllShortcuts(&action);

    }
    settings.endGroup();
}

void KeybindingsWaylandManager::unRegisterShortcutAll()
{
    if(m_shortcutList.isEmpty()){
        return;
    }
    for(ShortCutKeyBind* bind : m_shortcutList){
        KGlobalAccel::self()->removeAllShortcuts(bind->action());
    }
    qDeleteAll(m_shortcutList);
    m_shortcutList.clear();
}


void KeybindingsWaylandManager::clearShortcutList()
{
    if(m_shortcutList.isEmpty()){
        return;
    }
    qDeleteAll(m_shortcutList);
    m_shortcutList.clear();
}

void KeybindingsWaylandManager::stop()
{
    USD_LOG(LOG_DEBUG,"Stopping keybindings manager");
    if (m_dconfClient) {
        dconf_client_unwatch_fast(m_dconfClient, GSETTINGS_KEYBINDINGS_DIR);
        dconf_client_unwatch_sync(m_dconfClient, GSETTINGS_KEYBINDINGS_DIR);
        g_object_unref (m_dconfClient);
        m_dconfClient = nullptr;
    }
    clearShortcutList();
}

/*********************ShortCutKeyBind***********************/

ShortCutKeyBind::ShortCutKeyBind(QString settingsPath, QString actionName, QString bindKey, QString execName,QString componetName,QObject* parent)
    :m_settingsPath(settingsPath),
     m_actionName(actionName),
     m_bindKey(bindKey),
     m_execName(execName),
     m_componentName(componetName),
     QObject(parent)
{
    m_action = new QAction(this);
    m_action->setObjectName(m_actionName);
    m_action->setProperty("componentName", m_componentName);
    setUp();
}

ShortCutKeyBind::~ShortCutKeyBind()
{

}

void ShortCutKeyBind::setUp()
{
    setShortcut();
}

void ShortCutKeyBind::setShortcut()
{
    QList<QKeySequence> seq = listFromString();
    KGlobalAccel::self()->setDefaultShortcut(m_action, seq);
    KGlobalAccel::self()->setShortcut(m_action, seq);
    connect(m_action, &QAction::triggered,this,[this]() {
        USD_LOG(LOG_DEBUG,"shortcut action name %s",m_execName.toLatin1().data());
        parsingDesktopInGIO(m_execName);

    });
}

QList<QKeySequence> ShortCutKeyBind::listFromString()
{
    m_bindKey.replace("<", "");
    m_bindKey.replace(">", "+");
    m_bindKey.replace("Win", "Meta", Qt::CaseInsensitive);
    m_bindKey.replace("Super", "Meta", Qt::CaseInsensitive);
    m_bindKey.replace("Start", "Meta", Qt::CaseInsensitive);
    return QKeySequence::listFromString(m_bindKey);
}

void ShortCutKeyBind::parsingDesktop(QString exec)
{
    if(exec.contains("desktop")){
        QSettings setting(exec,QSettings::IniFormat);
        setting.beginGroup("Desktop Entry");
        QString name = setting.value("Name").toString();
        QString type = setting.value("Type").toString();
        QString exec = setting.value("Exec").toString();
        setting.endGroup();
        Q_UNUSED(name)
        Q_UNUSED(type)
        QProcess process;
        process.startDetached(exec);
    }
}

void ShortCutKeyBind::parsingDesktopInGIO(QString exec)
{
    if(exec.contains("desktop")){
        GDesktopAppInfo *info = g_desktop_app_info_new_from_filename(exec.toLatin1().constData());
        if (info == nullptr) {
            USD_LOG(LOG_ERR,"can't open %s",exec.toLatin1().data());
            return;
        }

        if (!g_app_info_launch_uris((GAppInfo *)info, NULL, NULL, NULL))
        {
            USD_LOG(LOG_ERR,"g_app_info_launch_uris %s error!",exec.toLatin1().data());
        }
        g_object_unref(info);
    }
}


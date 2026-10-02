/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Nicole <buxiaoqing@kylinos.cn>
 *
 */

#ifndef CLIPBOARDSHORTCUT_H
#define CLIPBOARDSHORTCUT_H

#include "ukui-shortcut-plugin.h"
#include <application-info.h>

namespace UkuiShortcut {

class ClipboardShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "clipboard-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit ClipboardShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("ClipboardShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class ClipboardShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit ClipboardShortcut(QObject *parent = nullptr);
    ~ClipboardShortcut();

    QString pluginId() override { return QStringLiteral("ClipboardShortcut"); };
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
    void setAddedState(bool added) override;

private:
    inline void initMetaData();
    void openClipboard();
    void onAppUninstalled(const QStringList &desktopFiles);
    void onAppInstalled(const QStringList &desktopFiles);

private:
    bool m_isEnable = false;
    bool m_isAdded = false;
    StatusInfo m_currentStatus;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
    UkuiSearch::ApplicationInfo *m_applicationInfo;
};
}

#endif // CLIPBOARDSHORTCUT_H

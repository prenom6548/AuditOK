/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#ifndef MULTISCREENSHORTCUT_H
#define MULTISCREENSHORTCUT_H
#include "ukui-shortcut-plugin.h"

namespace UkuiShortcut {

class MultiScreenShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "multi-screen-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit MultiScreenShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("MultiScreenShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class  MultiScreenShortcut : public Shortcut
{
    Q_OBJECT
public:
    MultiScreenShortcut();

    QString pluginId() override {return QStringLiteral("MultiScreenShortcut");}
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;

private:
    inline void initMetaData();

private:
    StatusInfo m_statusInfo;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};
}
#endif // MULTISCREENSHORTCUT_H

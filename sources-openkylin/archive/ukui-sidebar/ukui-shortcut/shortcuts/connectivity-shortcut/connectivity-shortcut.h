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

#ifndef CONNECTIVITYSHORTCUT_H
#define CONNECTIVITYSHORTCUT_H

#include "ukui-shortcut-plugin.h"
#include <application-info.h>


#define KYLIN_APP_MANAGER_NAME         "com.kylin.ProcessManager"
#define KYLIN_APP_MANAGER_PATH         "/com/kylin/ProcessManager/AppLauncher"
#define KYLIN_APP_MANAGER_INTERFACE    "com.kylin.ProcessManager.AppLauncher"

namespace UkuiShortcut {

class ConnectivityShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "connectivity-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit ConnectivityShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("ConnectivityShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class ConnectivityShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit ConnectivityShortcut(QObject *parent = nullptr);
    ~ConnectivityShortcut();

    QString pluginId() override { return QStringLiteral("ConnectivityShortcut"); };
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
    void setAddedState(bool added) override;

private:
    inline void initMetaData();
    void openConnectivity();
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

#endif // CONNECTIVITYSHORTCUT_H

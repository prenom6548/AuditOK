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

#ifndef BRIGHTNESSSHORTCUT_H
#define BRIGHTNESSSHORTCUT_H
#include "ukui-shortcut-plugin.h"

#define KYLIN_APP_MANAGER_NAME         "com.kylin.ProcessManager"
#define KYLIN_APP_MANAGER_PATH         "/com/kylin/ProcessManager/AppLauncher"
#define KYLIN_APP_MANAGER_INTERFACE    "com.kylin.ProcessManager.AppLauncher"

namespace UkuiShortcut {

class SystemSettingShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "system-setting-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit SystemSettingShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("SystemSetting"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class SystemSettingShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit SystemSettingShortcut(QObject *parent = nullptr);
    ~SystemSettingShortcut() override;

    QString pluginId() override;
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;

private:
    bool LaunchApp(QString desktopFile);
    void trigger();
    inline void initMetaData();

    bool m_isEnable = false;
    StatusInfo m_currentStatusInfo;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};
}

#endif // BRIGHTNESSSHORTCUT_H

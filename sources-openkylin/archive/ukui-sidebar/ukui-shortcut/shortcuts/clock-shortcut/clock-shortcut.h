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
 * Authors: an-yongtai <anyongtai@kylinos.cn>
 *
 */

#ifndef CLOCKSHORTCUT_H
#define CLOCKSHORTCUT_H
#include "ukui-shortcut-plugin.h"
#include "application-info.h"

using namespace UkuiSearch;
namespace UkuiShortcut {
class ClockShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "clock-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit ClockShortcutPlugin(QObject *parent = nullptr);
    QString pluginId() override { return QStringLiteral("Clock"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class ClockShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit ClockShortcut(QObject *parent = nullptr);
    ~ClockShortcut() override;
    QString pluginId() override;
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
    void setAddedState(bool added) override;

private:
    void LaunchApp();
    inline void initMetaData();
    void onAppUninstalled(const QStringList &desktopFiles);
    void onAppInstalled(const QStringList &desktopFiles);
    QString getDesktop(QString id);
    void checkAppInstallState();

    bool m_isEnable = false;
    bool m_isAdded = false;
    QString m_appPath;
    StatusInfo m_currentStatusInfo;
    ApplicationInfo *m_applicationInfo = nullptr;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};
}

#endif // CLOCKSHORTCUT_H

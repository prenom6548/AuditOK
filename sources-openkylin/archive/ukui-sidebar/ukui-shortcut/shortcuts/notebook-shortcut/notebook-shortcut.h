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

#ifndef NOTEBOOKSHORTCUT_H
#define NOTEBOOKSHORTCUT_H
#include "ukui-shortcut-plugin.h"
#include "application-info.h"

using namespace UkuiSearch;
namespace UkuiShortcut {
class NotebookShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "notebook-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit NotebookShortcutPlugin(QObject *parent = nullptr);
    QString pluginId() override { return QStringLiteral("Notebook"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class NotebookShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit NotebookShortcut(QObject *parent = nullptr);
    ~NotebookShortcut() override;
    QString pluginId() override;
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
    void setAddedState(bool added) override;
    QString getDesktopPath(QString id);

private:
    void LaunchApp();
    inline void initMetaData();
    void onAppUninstalled(const QStringList &desktopFiles);
    void onAppInstalled(const QStringList &desktopFiles);
    void checkAppInstallState();

private:
    bool m_isEnable = false;
    bool m_isAdded = false;
    ApplicationInfo *m_applicationInfo = nullptr;
    StatusInfo m_currentStatusInfo;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
    QString m_appPath;
};
}

#endif // NOTEBOOKSHORTCUT_H

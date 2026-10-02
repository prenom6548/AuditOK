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

#ifndef SCREENSHOTSHORTCUT_H
#define SCREENSHOTSHORTCUT_H

#define KYLIN_APP_MANAGER_NAME         "com.kylin.ProcessManager"
#define KYLIN_APP_MANAGER_PATH         "/com/kylin/ProcessManager/AppLauncher"
#define KYLIN_APP_MANAGER_INTERFACE    "com.kylin.ProcessManager.AppLauncher"
#define KYLIN_SCREENSHOT               "kylin-screenshot"

#include <QObject>
#include "ukui-shortcut-plugin.h"
#include <QtDBus/QDBusInterface>
#include "application-info.h"
namespace UkuiShortcut {

class ScreenShotShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "screenshot-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit ScreenShotShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("ScreenShot"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class ScreenShotShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit ScreenShotShortcut(QObject *parent = nullptr);
    ~ScreenShotShortcut() override;
    void kyscreenshotbegin();
    QString pluginId() override;
    bool isEnable() override;
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;

private:
    inline void initMetaData();
    void checkAppInstallState();

private Q_SLOT:
    void onAppInstallStatusChanged(const QStringList &desktopFiles);

private:
    bool m_isEnable = false;
    StatusInfo m_currentStatus;
    QString m_appPath;
    UkuiSearch::ApplicationInfo *m_applicationInfo = nullptr;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};


}
#endif // NIGHTMODESHORTCUT_H

/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef APPLAUNCHMANAGER_H
#define APPLAUNCHMANAGER_H

#include "appchooser.h"
#include "applauncher.h"
#include "desktopfilemanager.h"
#include "processinfomanager.h"
#include <QDBusContext>
#include <QObject>
#include <memory>

class AppLaunchManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    AppLaunchManager(
        ProcessInfoManager &processInfoManager, QObject *parent = nullptr);

    void LaunchApp(const QString &desktopFile);
    void LaunchAppWithGeometry(const QString &desktopFile, int x, int y, uint width, uint height);
    void LaunchAppWithArguments(const QString &desktopFile, const QStringList &args);
    void LaunchAppWithArgumentsAndGeometry(const QString &desktopFile, const QStringList &args, int x, int y, uint width, uint height);
    void LaunchDefaultAppWithUrl(const QString &url);
    void RunCommand(const QString &command);
    QStringList GetAvailableAppListForFile(const QString &fileName);
    QString GetDesktopFileByPid(int pid);

    void setDeviceMode(sched_policy::DeviceMode deviceMode);

Q_SIGNALS:
    void AppLaunched(const QString &appId);

private:
    void initAppLauncher();
    void sendAppLauncherErrorReply();

private:
    std::unique_ptr<AppLauncher> m_appLauncher;
    ProcessInfoManager &m_processInfoManager;
    AppChooser m_appChooser;
    DesktopFileManager &m_desktopFileManager = DesktopFileManager::instance();
};

#endif // APPLAUNCHMANAGER_H

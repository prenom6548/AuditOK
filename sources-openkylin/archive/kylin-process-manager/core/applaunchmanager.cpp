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

#include "applaunchmanager.h"
#include "applauncher.h"
#include "utils/appinfohelper.h"
#include "utils/misc.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QElapsedTimer>
#include "loghelper.h"

namespace
{

const char *launcher_interface_error_name_prefix = "com.kylin.ProcessManager.AppLauncher.";

}

AppLaunchManager::AppLaunchManager(
    ProcessInfoManager &processInfoManager, QObject *parent)
    : m_processInfoManager{processInfoManager}
    , QObject{parent}
{
    initAppLauncher();
}

void AppLaunchManager::LaunchApp(const QString &desktopFile)
{
    QElapsedTimer timer;
    timer.start();
    //qDebug() << "==========process manager service starting==========";
    PRINT_DBG("==========AppLaunchManager::LaunchApp starting desktop: %s==========\n", desktopFile.toStdString().c_str());
    if (m_appLauncher->launchApp(desktopFile.toStdString())) {
        Q_EMIT AppLaunched(desktopFile);
        PRINT_DBG("==========AppLaunchManager::LaunchApp end in %lld ms==========\n", timer.elapsed());
        return;
    }
    
    sendAppLauncherErrorReply();
}

void AppLaunchManager::LaunchAppWithGeometry(const QString &desktopFile, int x, int y, uint width, uint height)
{
    QElapsedTimer timer;
    timer.start();
    //qDebug() << "==========process manager service starting==========";
    PRINT_DBG("==========AppLaunchManager::LaunchAppWithGeometry starting desktop: %s==========\n", desktopFile.toStdString().c_str());
    if (m_appLauncher->launchApp(desktopFile.toStdString(), x, y, width, height)) {
        Q_EMIT AppLaunched(desktopFile);
        PRINT_DBG("==========AppLaunchManager::LaunchAppWithGeometry end in %lld ms==========\n", timer.elapsed());
        return;
    }
    
    sendAppLauncherErrorReply();
}

void AppLaunchManager::LaunchAppWithArguments(const QString &desktopFile, const QStringList &args)
{
    QElapsedTimer timer;
    timer.start();
    //qDebug() << "==========process manager service starting==========";
    PRINT_DBG("==========AppLaunchManager::LaunchAppWithArguments starting desktop: %s==========\n", desktopFile.toStdString().c_str());
    if (m_appLauncher->launchAppWithArguments(
            desktopFile.toStdString(), misc::qt2stl::qstringList2StdVectorString(args))) {
        Q_EMIT AppLaunched(desktopFile);
        PRINT_DBG("==========AppLaunchManager::LaunchAppWithArguments end in %lld ms==========\n", timer.elapsed());
        return;
    }
    
    sendAppLauncherErrorReply();
}

void AppLaunchManager::LaunchAppWithArgumentsAndGeometry(const QString &desktopFile, const QStringList &args, int x, int y, uint width, uint height)
{
    QElapsedTimer timer;
    timer.start();
    //qDebug() << "==========process manager service starting==========";
    PRINT_DBG("==========AppLaunchManager::LaunchAppWithArgumentsAndGeometry starting desktop: %s==========\n", desktopFile.toStdString().c_str());
    if (m_appLauncher->launchAppWithArguments(
            desktopFile.toStdString(), misc::qt2stl::qstringList2StdVectorString(args), x, y, width, height)) {
        Q_EMIT AppLaunched(desktopFile);
        PRINT_DBG("==========AppLaunchManager::LaunchAppWithArgumentsAndGeometry end in %lld ms==========\n", timer.elapsed());
        return;
    }
    
    sendAppLauncherErrorReply();
}

void AppLaunchManager::LaunchDefaultAppWithUrl(const QString &url)
{
    QElapsedTimer timer;
    timer.start();
    //qDebug() << "==========process manager service starting==========";
    PRINT_DBG("==========AppLaunchManager::LaunchDefaultAppWithUrl starting url: %s==========\n", url.toStdString().c_str());
    const std::string desktopFile = m_appChooser.getDefaultAppForUrl(url.toStdString());
    if (m_appLauncher->launchAppWithArguments(desktopFile, {url.toStdString()})) {
        Q_EMIT AppLaunched(QString::fromStdString(desktopFile));
        PRINT_DBG("==========AppLaunchManager::LaunchDefaultAppWithUrl end in %lld ms==========\n", timer.elapsed());
        return;
    }
    
    sendAppLauncherErrorReply();
}

void AppLaunchManager::RunCommand(const QString &command)
{
    QElapsedTimer timer;
    timer.start();
    //qDebug() << "==========process manager service starting==========";
    PRINT_DBG("==========AppLaunchManager::RunCommand starting command: %s==========\n", command.toStdString().c_str());
    if (m_appLauncher->runCommand(command.toStdString())) {
        auto appDesktopFile = appinfo_helper::findDesktopFileFromCmdline(command.toStdString());
        const QString appId = appDesktopFile.empty() ? command : QString::fromStdString(appDesktopFile);
        Q_EMIT AppLaunched(appId);
        PRINT_DBG("==========AppLaunchManager::RunCommand end in %lld ms==========\n", timer.elapsed());
        return;
    }
    
    sendAppLauncherErrorReply();
}

QStringList AppLaunchManager::GetAvailableAppListForFile(const QString &fileName)
{
    const auto appList = m_appChooser.getAvailableAppListForFile(fileName.toStdString());
    return misc::stl2qt::vectorString2QStringList(appList);
}

QString AppLaunchManager::GetDesktopFileByPid(int pid)
{
    // 优先匹配appLauncher缓存
    if (m_appLauncher->m_pidDesktopFileMap.contains(pid)) {
        return m_appLauncher->m_pidDesktopFileMap.value(pid);
    }

    std::string desktopFile = m_processInfoManager.getDesktopFileByPid(pid);
    return QString::fromStdString(desktopFile);
}

void AppLaunchManager::setDeviceMode(sched_policy::DeviceMode deviceMode)
{
    bool keepAppSingleInstance = (deviceMode == sched_policy::DeviceMode::Tablet);
    m_appLauncher->setKeepAppSingleInstance(keepAppSingleInstance);
}

void AppLaunchManager::initAppLauncher()
{
    m_appLauncher.reset(
        new AppLauncher(
            [this](const std::string &desktopFile, const std::vector<std::string> &args, int pid) {
        m_processInfoManager.createAppInstance(desktopFile, args, pid);
    },
            AppInfoGetter([this](const std::string &desktopFile, const std::vector<std::string> &args) {
        return m_processInfoManager.getLatestAppInfoByDesktopFile(desktopFile, args);
    },
                          [this](const std::string &cmdline) {
        return m_processInfoManager.getLatestAppInfoByCmdline(cmdline);
    })));
}

void AppLaunchManager::sendAppLauncherErrorReply()
{
    const QString errorName = QString::fromStdString(
        launcher_interface_error_name_prefix + m_appLauncher->errorMessage().errorName);

    const QString errorMessage = QString::fromStdString(
        m_appLauncher->errorMessage().errorMessage);

    sendErrorReply(errorName, errorMessage);
}

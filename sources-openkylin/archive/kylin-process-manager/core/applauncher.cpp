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

#include "applauncher.h"
#include "appinfohelper.h"
#include "loghelper.h"
#include "misc.h"
#include "priorityschedulingmanager.h"
#include "waylanddisplay.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QString>

namespace
{

struct LaunchGeometry
{
    int x;
    int y;
    uint32_t width;
    uint32_t height;
};

struct ChildProcessCallbackUserData
{
    AppLauncher *appLauncher;
    std::string appId;
    std::vector<std::string> args;
    LaunchGeometry geometry;
};

PrioritySchedulingManager priority_scheduling_manager;

}

AppInfoGetter::AppInfoGetter(
    LatestAppInfoByDesktopFileCallback byDesktopFileCallback,
    LatestAppInfoByCmdlineCallback byCmdlineCallback)
    : m_latestAppInfoByDesktopFileCallback(std::move(byDesktopFileCallback))
    , m_latestAppInfoByCmdlineCallback(std::move(byCmdlineCallback))
{
}

AppInfo AppInfoGetter::getLatestAppInfoByDesktopFile(
    const std::string &desktopFile, const std::vector<std::string> &args) const
{
    return m_latestAppInfoByDesktopFileCallback(desktopFile, args);
}

AppInfo AppInfoGetter::getLatestAppInfoByCmdline(const std::string &cmdline) const
{
    return m_latestAppInfoByCmdlineCallback(cmdline);
}

void childProcessCallback(GDesktopAppInfo *appInfo, GPid pid, gpointer userData)
{
    priority_scheduling_manager.insertTopTask(pid, 1, 3, 10, 1);
    ChildProcessCallbackUserData *callbackUserData =
        static_cast<ChildProcessCallbackUserData *>(userData);
    if (!callbackUserData || !appInfo || !callbackUserData->appLauncher->m_launcherProcessCallback) {
        return;
    }

    callbackUserData->appLauncher->m_launcherProcessCallback(
        g_desktop_app_info_get_filename(appInfo), callbackUserData->args, pid);

    priority_scheduling_manager.eraseTopTask(pid);
}

void childProcessCallbackWithGeometry(GDesktopAppInfo *appInfo, GPid pid, gpointer userData)
{
    priority_scheduling_manager.insertTopTask(pid, 1, 3, 10, 1);
    ChildProcessCallbackUserData *data = static_cast<ChildProcessCallbackUserData *>(userData);
    if (!data || !appInfo || !data->appLauncher->m_launcherProcessCallback) {
        return;
    }

    std::string desktopfile = g_desktop_app_info_get_filename(appInfo);

    auto appid = data->appId;

    data->appLauncher->m_launcherProcessCallback(desktopfile, data->args, pid);

    // 兼容旧协议，后续可能会被移除
    waylandDisplay()->setStartupGeometryForApp(pid, data->geometry.x, data->geometry.y, data->geometry.width, data->geometry.height);

    // 使用新协议向已有启动信息中增加 pid信息
    waylandDisplay()->setPidForStartupInfoV2(appid, pid);
    // 结束启动信息设置
    waylandDisplay()->finishSetupInfoV2(appid);

    delete data;
    
    priority_scheduling_manager.eraseTopTask(pid);
}

AppLauncher::AppLauncher(
    LauncherProcessCallback launcherProcessCallback,
    AppInfoGetter appInfoGetter)
    : m_keepAppSingleInstance(false)
    , m_launcherProcessCallback(std::move(launcherProcessCallback))
    , m_appInfoGetter(std::move(appInfoGetter))
{
}

void AppLauncher::setKeepAppSingleInstance(bool keepAppSingleInstance)
{
    m_keepAppSingleInstance = keepAppSingleInstance;
}

bool AppLauncher::launchApp(const std::string &desktopFile)
{
    return launchDesktopApp(desktopFile);
}

bool AppLauncher::launchApp(const std::string &desktopFile, int x, int y, uint32_t width, uint32_t height)
{
    return launchDesktopAppWithGeometry(desktopFile, {}, x, y, width, height);
}

bool AppLauncher::launchAppWithArguments(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    return launchDesktopApp(desktopFile, args);
}

bool AppLauncher::launchAppWithArguments(
    const std::string &desktopFile,
    const std::vector<std::string> &args,
    int x, int y, uint32_t width, uint32_t height)
{
    return launchDesktopAppWithGeometry(desktopFile, args, x, y, width, height);
}

bool AppLauncher::runCommand(const std::string &command)
{
    if (m_keepAppSingleInstance) {
        AppInfo appInfo = m_appInfoGetter.getLatestAppInfoByCmdline(command);
        if (appWindowHasCreated(appInfo)) {
            return activeAppWindow(appInfo);
        }
    }

    return runNewCommandInstance(command);
}

AppLauncher::LaunchError AppLauncher::error() const
{
    return m_launchError;
}

AppLauncher::LaunchErrorMessage AppLauncher::errorMessage() const
{
    switch (m_launchError) {
    case LaunchError::FailedToFindDesktopFile:
        return {m_launchError,
                "FailedToFindDesktopFile", "Failed to find the desktop file."};

    case LaunchError::FailedToCreateDesktopAppInfo:
        return {m_launchError,
                "FailedToCreateDesktopAppInfo", "Failed to create DesktopAppInfo."};

    case LaunchError::FailedToActiveWindow:
        return {m_launchError,
                "FailedToActiveWindow", "Failed to active application window."};

    case LaunchError::FailedToLaunchApp:
        return {m_launchError,
                "FailedToLaunchApp", "Failed to launch the application."};

    case LaunchError::FailedToParseCommand:
        return {m_launchError,
                "FailedToParseCommand", "Failed to create app info from command."};

    case LaunchError::FailedToRunCommand:
        return {m_launchError,
                "FailedToRunCommand", "Failed to run command."};

    default:
        return {m_launchError,
                "NoError", "No error occurred."};
    }
}

bool AppLauncher::launchDesktopApp(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    initLaunchError();

    if (!misc::file::fileExists(desktopFile)) {
        setLaunchError(LaunchError::FailedToFindDesktopFile);
        return false;
    }

    if (shouldLaunchNewInstance(desktopFile, args)) {
        return doLaunchApp(desktopFile, args);
    }

    return activeAppWindow(desktopFile, args);
}

bool AppLauncher::launchDesktopAppWithGeometry(
    const std::string &desktopFile,
    const std::vector<std::string> &args,
    int x, int y, uint32_t width, uint32_t height)
{
    initLaunchError();

    if (!misc::file::fileExists(desktopFile)) {
        setLaunchError(LaunchError::FailedToFindDesktopFile);
        return false;
    }

    if (shouldLaunchNewInstance(desktopFile, args)) {
        return doLaunchAppWithGeometry(desktopFile, args, x, y, width, height);
    }

    return activeAppWindow(desktopFile, args);
}

bool AppLauncher::shouldLaunchNewInstance(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    if (!m_keepAppSingleInstance) {
        return true;
    }

    if (appinfo_helper::isKmreApp(desktopFile)) {
        return true;
    }

    AppInfo appInfo = m_appInfoGetter.getLatestAppInfoByDesktopFile(desktopFile, args);

    if (appWindowHasCreated(appInfo)) {
        return false;
    }

    if (appWindowIsCreating(appInfo)) {
        return false;
    }

    return true;
}

bool AppLauncher::appWindowHasCreated(const AppInfo &appInfo)
{
    return !appInfo.wids().empty();
}

bool AppLauncher::appWindowIsCreating(const AppInfo &appInfo)
{
    return appInfo.wids().empty() && QDateTime::currentSecsSinceEpoch() - appInfo.launchTimestamp() <= 5;
}

bool AppLauncher::doLaunchApp(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopFile.c_str());
    if (appInfo == nullptr) {
        setLaunchError(LaunchError::FailedToCreateDesktopAppInfo);
        PRINT_DBG("Create DesktopAppInfo failed from desktop %s.\n", desktopFile.c_str());
        return false;
    }

    GList *uris = createUrisFromStringList(args);

    GError *error = nullptr;
    ChildProcessCallbackUserData userData{
        .appLauncher = this,
        .args = args,
    };

    GAppLaunchContext *appLaunchContext = g_app_launch_context_new();
    bool launched = g_desktop_app_info_launch_uris_as_manager(
        appInfo, uris, appLaunchContext, G_SPAWN_DEFAULT, nullptr, nullptr, childProcessCallback, &userData, &error);

    if (!launched) {
        setLaunchError(LaunchError::FailedToLaunchApp);
        PRINT_NOTICE("Failed to launch %s, %s.\n", desktopFile.c_str(), error->message);
        g_clear_error(&error);
    }

    g_list_free(uris);
    g_object_unref(appInfo);

    return launched;
}

bool AppLauncher::doLaunchAppWithGeometry(
    const std::string &desktopFile,
    const std::vector<std::string> &args,
    int x, int y, uint32_t width, uint32_t height)
{
    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopFile.c_str());
    if (appInfo == nullptr) {
        setLaunchError(LaunchError::FailedToCreateDesktopAppInfo);
        PRINT_DBG("Create DesktopAppInfo failed from desktop %s.\n", desktopFile.c_str());
        return false;
    }

    GList *uris = createUrisFromStringList(args);

    GError *error = nullptr;

    // Set the startup geometry for the app
    std::string appId = QFileInfo(QString::fromStdString(desktopFile)).baseName().toStdString();
    bool hasWMClass = g_desktop_app_info_has_key(appInfo, "StartupWMClass");
    if (hasWMClass) {
        appId = g_desktop_app_info_get_string(appInfo, "StartupWMClass");
    }
    waylandDisplay()->createStartupInfoForAppId(appId, x, y, width, height);

    ChildProcessCallbackUserData *userData = new ChildProcessCallbackUserData{
        .appLauncher = this,
        .appId = appId,
        .args = args,
        .geometry = {x, y, width, height}};

    GAppLaunchContext *appLaunchContext = g_app_launch_context_new();

    bool launched = g_desktop_app_info_launch_uris_as_manager(
        appInfo, uris, appLaunchContext, G_SPAWN_DEFAULT,
        nullptr, nullptr, childProcessCallbackWithGeometry, userData, &error);

    if (!launched) {
        setLaunchError(LaunchError::FailedToLaunchApp);
        g_clear_error(&error);
    }

    g_list_free(uris);
    g_object_unref(appInfo);

    return launched;
}

bool AppLauncher::activeAppWindow(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    AppInfo appInfo = m_appInfoGetter.getLatestAppInfoByDesktopFile(desktopFile, args);
    return activeAppWindow(appInfo);
}

bool AppLauncher::activeAppWindow(const AppInfo &appInfo)
{
    if (appInfo.wids().empty()) {
        setLaunchError(LaunchError::FailedToActiveWindow);
        return false;
    }

    const std::string latestWid = appInfo.wids().back();
    waylandDisplay()->avtivateWindow(latestWid);
    return true;
}

GList *AppLauncher::createUrisFromStringList(const std::vector<std::string> &args)
{
    GList *uris = nullptr;
    for (const auto &arg : args) {
        uris = g_list_append(uris, const_cast<char *>(arg.c_str()));
    }

    return uris;
}

bool AppLauncher::runNewCommandInstance(const std::string &command)
{
    initLaunchError();

    //QStringList arguments = QString::fromStdString(command).split(' ', Qt::SkipEmptyParts);
    QStringList arguments = QProcess::splitCommand(QString::fromStdString(command));
    if (arguments.isEmpty()) {
        setLaunchError(LaunchError::FailedToParseCommand);
        PRINT_DBG("Failed to parse command: %s.\n", command.c_str());
        return false;
    }

    bool ret = QProcess::startDetached(arguments[0], arguments.mid(1));

    if (!ret) {
        setLaunchError(LaunchError::FailedToRunCommand);
        PRINT_DBG("Run new commmand '%s' instance failed.\n", command.c_str());
    }

    return ret;
}

void AppLauncher::initLaunchError()
{
    setLaunchError(AppLauncher::LaunchError::NoError);
}

void AppLauncher::setLaunchError(LaunchError launchError)
{
    m_launchError = std::move(launchError);
}

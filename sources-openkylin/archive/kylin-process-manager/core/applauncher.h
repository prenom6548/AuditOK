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

#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include "base/appinfo.h"

#include <functional>
#include <string>

#include <gio/gdesktopappinfo.h> // It must be included before gio/gappinfo.h

#include <gio/gappinfo.h>

#include <QHash>

class AppInfoGetter
{
public:
    using LatestAppInfoByDesktopFileCallback =
        std::function<AppInfo(const std::string &, const std::vector<std::string> &)>;

    using LatestAppInfoByCmdlineCallback =
        std::function<AppInfo(const std::string &)>;

    AppInfoGetter(
        LatestAppInfoByDesktopFileCallback byDesktopFileCallback,
        LatestAppInfoByCmdlineCallback byCmdlineCallback);

    AppInfo getLatestAppInfoByDesktopFile(
        const std::string &desktopFile, const std::vector<std::string> &args) const;

    AppInfo getLatestAppInfoByCmdline(const std::string &cmdline) const;

private:
    LatestAppInfoByDesktopFileCallback m_latestAppInfoByDesktopFileCallback;
    LatestAppInfoByCmdlineCallback m_latestAppInfoByCmdlineCallback;
};

class AppLauncher
{
public:
    // desktop file, args, .pid
    using LauncherProcessCallback =
        std::function<void(const std::string &, const std::vector<std::string> &args, int)>;

    enum class LaunchError {
        NoError,
        FailedToFindDesktopFile,
        FailedToCreateDesktopAppInfo,
        FailedToActiveWindow,
        FailedToLaunchApp,
        FailedToParseCommand,
        FailedToRunCommand,
    };

    struct LaunchErrorMessage
    {
        LaunchError error;
        std::string errorName;
        std::string errorMessage;
    };

    AppLauncher(
        LauncherProcessCallback launcherProcessCallback,
        AppInfoGetter appInfoGetter);

    void setKeepAppSingleInstance(bool keepAppSingleInstance);

    /**
     * @brief 启动desktop文件对应的应用程序。
     * @param desktopFile desktop文件路径。
     * @return 成功返回 true，失败返回 false。
     */
    bool launchApp(const std::string &desktopFile);

    /**
     * @brief 带几何参数启动desktop文件对应的应用程序。
     * @param desktopFile desktop文件路径。
     * @param x 图标左上角的 x 坐标。
     * @param y 图标左上角的 y 坐标。
     * @param width 图标宽度（对应 taskbar/desktop entry）。
     * @param height 图标高度（对应 taskbar/desktop entry）。
     * @return 成功返回 true，失败返回 false。
     */
    bool launchApp(const std::string &desktopFile, int x, int y, uint32_t width, uint32_t height);

    /**
     * @brief 带参数启动指定的desktop文件对应的应用程序。
     * @param desktopFile desktop文件路径。
     * @param args 启动参数。
     * @return 成功返回 true，失败返回 false。
     */
    bool launchAppWithArguments(
        const std::string &desktopFile, const std::vector<std::string> &args);

    /**
     * @brief 带参数和几何参数启动指定的desktop文件对应的应用程序。
     * @param desktopFile desktop文件路径。
     * @param args 启动参数。
     * @param x 图标左上角的 x 坐标。
     * @param y 图标左上角的 y 坐标。
     * @param width 图标宽度（对应 taskbar/desktop entry）。
     * @param height 图标高度（对应 taskbar/desktop entry）。
     * @return 成功返回 true，失败返回 false。
     */
    bool launchAppWithArguments(
        const std::string &desktopFile,
        const std::vector<std::string> &args,
        int x, int y, uint32_t width, uint32_t height);

    /**
     * @brief 运行指定的命令。
     * @param command 命令字符串。
     * @return 成功返回 true，失败返回 false。
     */
    bool runCommand(const std::string &command);

    LaunchError error() const;
    LaunchErrorMessage errorMessage() const;

    QHash<uint64_t, QString> m_pidDesktopFileMap; // 保存 pid 和 desktopFile 的映射关系

private:
    friend void childProcessCallback(GDesktopAppInfo *appInfo, GPid pid, gpointer userData);
    friend void childProcessCallbackWithGeometry(GDesktopAppInfo *appInfo, GPid pid, gpointer userData);

    bool launchDesktopApp(
        const std::string &desktopFile, const std::vector<std::string> &args = {});

    bool launchDesktopAppWithGeometry(
        const std::string &desktopFile,
        const std::vector<std::string> &args = {},
        int x = 0, int y = 0, uint32_t width = 0, uint32_t height = 0);

    bool shouldLaunchNewInstance(
        const std::string &desktopFile, const std::vector<std::string> &args = {});

    bool appWindowHasCreated(const AppInfo &appInfo);
    bool appWindowIsCreating(const AppInfo &appInfo);

    bool doLaunchApp(
        const std::string &desktopFile, const std::vector<std::string> &args = {});

    bool doLaunchAppWithGeometry(
        const std::string &desktopFile,
        const std::vector<std::string> &args = {},
        int x = 0, int y = 0, uint32_t width = 0, uint32_t height = 0);

    bool activeAppWindow(
        const std::string &desktopFile, const std::vector<std::string> &args = {});
    bool activeAppWindow(const AppInfo &appInfo);

    GList *createUrisFromStringList(const std::vector<std::string> &args);

    bool runNewCommandInstance(const std::string &command);

    void initLaunchError();
    void setLaunchError(LaunchError launchError);

private:
    LauncherProcessCallback m_launcherProcessCallback;
    AppInfoGetter m_appInfoGetter;
    bool m_keepAppSingleInstance;
    LaunchError m_launchError;
};

#endif // APPLAUNCHER_H

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

#ifndef APPINFO_H
#define APPINFO_H

#include "schedpolicy.h"
#include "appinfohelper.h"
#include <functional>
#include <unordered_map>
#include <memory>
#include <vector>

using AppChangeHash = std::unordered_map<sched_policy::AppChangeType, bool>;

struct WinInfo
{
    WinInfo(std::string wid);

    std::string wid_;
    std::string title_;
    bool is_added;
    bool is_removed;
    bool is_active;
    bool is_keepabove;
    bool is_minimized;
    bool is_maximized;
    bool is_fullscreen;
};

class AppInfo
{
public:
    using AppStateChangedCallback = std::function<void(AppInfo &)>;
    using AppWinChangedCallback = std::function<void(AppInfo &, const std::string &)>;

    AppInfo();
    AppInfo(const std::string &desktopFile,
            std::string appId, int launcherPid, int64_t launchTimestap,
            sched_policy::AppType type,
            sched_policy::AppState appState,
            AppStateChangedCallback stateChangedCallback);
    AppInfo(const AppInfo &rhs);
    AppInfo &operator=(const AppInfo &rhs);
    bool operator==(const AppInfo &rhs);
    ~AppInfo();

    std::string appId() const;
    std::string name() const;
    std::string desktopFileId() const;
    std::string desktopFile() const;
    std::string desktopFilePid() const;
    std::string cmdline() const;
    std::string mprisDbusService() const;
    int64_t launchTimestamp() const;
    std::string groupName() const;
    const std::vector<std::string> &args() const;
    int launcherPid() const;
    std::vector<int> pids() const;
    std::vector<std::string> wids() const;
    const std::unordered_map<std::string, WinInfo> &winInfos() const;
    sched_policy::AppState appState() const;
    sched_policy::AppType appType() const;
    bool isAvailable() const;
    bool isSystemTrayIconApp() const;
    bool isAllWinsMinimize() const;
    bool isHasActiveWindow() const;

    void setAppId(const std::string &appId);
    void setMrprisDbusService(const std::string &service);
    void clearWinsInstantState();

    bool containsPid(int pid) const;
    bool containsWid(const std::string &wid) const;

    void appendWid(const std::string &wid);
    void removeWid(const std::string &wid);
    void setAppState(sched_policy::AppState state);
    void setGroupName(const std::string &groupName);
    void setCmdline(const std::string &cmdline);
    void setIsSystemTrayIconApp(bool isSystemTrayIconApp);
    void setKeepAbove(const std::string &wid, bool isKeepAbove);
    void setMaximized(const std::string &wid, bool isMaximized);
    void setMinimized(const std::string &wid, bool isMinimized);
    void setFullscreen(const std::string &wid, bool isFullscreen);
    void setActive(const std::string &wid, bool isActive);
    void setArgs(const std::vector<std::string> &args);

    void setAppStateChangedCallback(AppStateChangedCallback callback);
    void setAppWinsChangedCallback(AppWinChangedCallback callback);
private:
    struct AppInfoPrivate;
    std::unique_ptr<AppInfoPrivate> pImpl;
};

#endif // APPINFO_H

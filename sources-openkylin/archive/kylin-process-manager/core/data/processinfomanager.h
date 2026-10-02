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

#ifndef PROCESSINFOMANAGER_H
#define PROCESSINFOMANAGER_H

#include "registrinfo.h"
#include "appinfomanager.h"
#include "configmanager.h"

class ProcessInfoManager
{
public:
    using AppChangedCallback = AppInfoManager::AppChangedCallback;

    ProcessInfoManager(const std::shared_ptr<ConfigManager> &configManager, const std::shared_ptr<StateManager> &stateManager);
    ~ProcessInfoManager();

    void handleWindowAdded(const std::string &wid);
    void handleWindowRemoved(const std::string &wid);
    void handleWindowActive(const std::string &wid);
    void setWindowMinimized(const std::string &wid, bool isMinimized);
    void setWindowKeepAbove(const std::string &wid, bool isKeepAbove);
    void setWindowMaximized(const std::string &wid, bool isMaximized);
    void setWindowFullscreen(const std::string &wid, bool isFullscreen);
    void handleExceptionGroupSessionApp(const std::string &desktopFile, int pid);
    void handleMprisDbusAdded(int pid, const std::string &service);
    void handleStatusNotifierItemRegistered(int pid);

    void createAppInstance(
        const std::string &desktopFile,
        const std::vector<std::string> &args, int pid);
    void setAppGroupName(const std::string &desktopFilePid, const std::string &groupName);

    void forceChangCachedStateTo(sched_policy::AppState state);
    bool forceChangeAppStateByPid(int pid, sched_policy::AppState state);

    void setBackgroundToCachedStateInterval(int interval);

    sched_policy::AppType getAppTypeByPid(int pid);
    std::string syncGetDesktopFileByPid(int pid, bool needUpDateDesktopTable = true);

    std::vector<AppInfo> getAllAppInfos();
    std::vector<AppInfo> getAppInfosWithState(sched_policy::AppState appState);

    AppInfo getLatestAppInfoByDesktopFile(
        const std::string &desktopFile, const std::vector<std::string> &args);
    AppInfo getLatestAppInfoByCmdline(const std::string &cmdline);
    AppInfo getAppInfo(const std::string &appId);

    std::string getDesktopFileByPid(int pid);

    void registerAppWinChangedSpellWatcher(const std::string &uuid, const RegistrAppInfo &process, int spell);
    void unregisterAppWinChangedSpellWatcher(const std::string &uuid);

    void registerAppFocusSpellWatcher(const std::string &uuid, const RegistrProcessInfo &process, int spell);
    void unregisterAppFocusSpellWatcher(const std::string &uuid);

public:
    std::unordered_map<std::string, AppChangeHash> app_change_type;
    std::unordered_map<std::string, AppInfo> all_app_infos;
    std::unordered_set<std::string> app_win_changed_spell;
    std::unordered_set<std::string> app_focus_spell;
    std::string focus_app_id;

private:
    void initAppInfoChanged();
    void handleAppInfoChanged(const AppInfo &appInfo);
    void handleAppStateChanged(const AppInfo &appInfo);
    void handleWindowChanged(const AppInfo &appInfo, const std::string &wid);
    void setAppChangeType(std::string appId, sched_policy::AppChangeType changeType);

private:
    AppInfoManager m_appInfoManager;
    std::shared_ptr<StateManager> m_stateManager;

    std::unordered_map<std::string, QTimer> m_timers;

    std::vector<QMetaObject::Connection> m_conns;
    std::string m_currFoucsUuid;
};

#endif // PROCESSINFOMANAGER_H

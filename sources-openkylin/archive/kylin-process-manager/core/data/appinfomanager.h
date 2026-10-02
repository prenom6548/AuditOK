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

#ifndef APPINFOMANAGER_H
#define APPINFOMANAGER_H

#include "appinfo.h"
#include "configmanager.h"
#include "desktopfilemanager.h"
#include "selection/selectionchecker.h"
#include <QTimer>
#include <map>
#include <string>
#include <vector>

#include <QObject>

class AppInfoManager : public QObject
{
    Q_OBJECT
public:
    using AppChangedCallback = std::function<void(const AppInfo &)>;
    using AppWinChangedCallback = std::function<void(const AppInfo &, const std::string&)>;

    explicit AppInfoManager(const std::shared_ptr<ConfigManager> &configManager);
    ~AppInfoManager();

    /**
     * @brief 处理窗口添加事件
     * @param wid 窗口wid
     */
    void handleWindowAdded(const std::string &wid);
    /**
     * @brief 处理窗口移除事件
     * @param wid 窗口wid
     */
    void handleWindowRemoved(const std::string &wid);
    /**
     * @brief 处理活动窗口变更事件
     * @param wid 当前活动窗口的wid
     */
    void handleWindowActive(const std::string &wid);
    /**
     * @brief 处理窗口最小化事件
     * @param wid 被最小化的窗口wid
     */
    void handleWindowMinimized(const std::string &wid, bool isMinimized);
    /**
     * @brief 处理窗口最上端事件
     * @param wid 被最上端的窗口wid
     */
    void handleWindowKeepAbove(const std::string &wid, bool isKeepAbove);
    /**
     * @brief 处理窗口最大化事件
     * @param wid 被最大化的窗口wid
     */
    void handleWindowMaximized(const std::string &wid, bool isMaximized);
    /**
     * @brief 处理窗口全屏事件
     * @param wid 被全屏的窗口wid
     */
    void handleWindowFullscreen(const std::string &wid, bool isFullscreen);

    /**
     * @brief 处理异常组会话应用事件
     * @param desktopFile desktop文件路径
     * @param pid 进程ID
     */
    void handleExceptionGroupSessionApp(const std::string &desktopFile, int pid);

    void handleMprisDbusAdded(int pid, const std::string &service);
    void handleStatusItemNotifierItemRegistered(int pid);

    /**
     * @brief 创建应用程序实例
     * @param desktopFile desktop文件路径
     * @param args 启动参数列表
     * @param pid 进程ID
     */
    void createAppInstance(
        const std::string &desktopFile,
        const std::vector<std::string> &args, int pid);
    void setAppGroupName(const std::string &desktopFilePid, const std::string &groupName);

    void setAppStartedCallback(AppChangedCallback callback);
    void setAppStateChangedCallback(AppChangedCallback callback);
    void setAppFinishedCallback(AppChangedCallback callback);
    void setAppChildPidsUpdatedCallback(AppChangedCallback callback);
    void setAppWinsChangedCallback(AppWinChangedCallback callback);

    bool forceChangeAppStateByPid(int pid, sched_policy::AppState state);
    void forceChangCachedStateTo(sched_policy::AppState state);

    sched_policy::AppType getAppTypeByDesktopFile(const std::string &desktopFile);
    sched_policy::AppType getAppTypeByPid(int pid);

    std::string syncGetDesktopFileByPid(int pid, bool needUpDateDesktopTable = true);

    std::vector<AppInfo> getAllAppInfos();
    std::vector<AppInfo> getAppInfosWithState(sched_policy::AppState appState);

    AppInfo getLatestAppInfoByDesktopFile(
        const std::string &desktopFile, const std::vector<std::string> &args);
    AppInfo getLatestAppInfoByCmdline(const std::string &cmdline);

    /**
     * @brief 获取所有应用程序信息
     * @return 应用程序信息列表
     */
    AppInfo getAppInfo(const std::string &appId);
    /**
     * @brief 获取具有指定状态的应用程序信息
     * @param appState 目标状态
     * @return 符合条件的应用程序信息列表
     */
    std::string getDesktopFileByPid(int pid);

    void setBackgroundToCachedStateInterval(int interval);

private:
    using AppInfoIterator = std::vector<AppInfo>::iterator;

    void doHandleActiveWindowChanged();
    void addWidToAppInfo(AppInfoIterator appInfoIt, const std::string &wid);
    void updateAppCmdlineByWid(AppInfoIterator appInfoIt, const std::string &wid);
    void createAppInstanceWithWid(const std::string &wid);
    void appBecomeActiveWithWid(const std::string &wid);
    void appBecomeInactiveWithWid(const std::string &wid);

    void updateAppStateWithoutEmptyWindows(AppInfoIterator appInfoIt);
    void updateAppInfoWithEmptyWindows(AppInfoIterator appInfoIt);
    void setAppState(sched_policy::AppState appState, AppInfoIterator appInfoIt);
    unsigned long long setBackgroundStateTimer(const std::string &appId);
    bool canBeCachedState(AppInfoIterator appInfoIt) const;
    void resetBackgroundStateTimers();
    std::vector<std::string> appIdsWithTimer();

    AppInfo generateAppInfoByWid(const std::string &wid);
    AppInfo generateAppInfo(const std::string &desktopFile, int pid);

    using FindAppInfoPredicateCallback = std::function<bool(const AppInfo &)>;
    std::tuple<bool, AppInfoIterator> findAppInfo(const FindAppInfoPredicateCallback &predicateCallback);
    bool hasAppInfoContainsPid(const FindAppInfoPredicateCallback &predicateCallback);
    std::tuple<bool, AppInfoIterator> findAppInfoWithWid(const std::string &wid);
    std::tuple<bool, AppInfoIterator> findAppInfoWithPid(int pid);
    std::tuple<bool, AppInfoIterator> findAppInfoWithAppId(const std::string &appId);
    std::tuple<bool, AppInfoIterator> findAppInfoWithDesktopFilePid(const std::string &desktopFilePid);

    void addStateChangedTimerId(const std::string &appId, unsigned long long timerId);
    void deleteStateChangeTimerId(const std::string &appId);

    void saveMprisServiceToCache(int pid, const std::string &service);
    std::string takeMprisServiceFromCache(int pid);

    void saveSystemTrayIconAppToCache(int pid);
    int takeSystemTrayIconAppFromCache(int);

    bool canChangeState(const AppInfo &appInfo);

private:
    // <desktopfile id, appinfo list>
    std::map<std::string, std::vector<AppInfo>> m_appInfos;
    //<appId, timerId>
    std::map<std::string, unsigned long long> m_appStateChangeTimerIds;
    std::map<int, std::string> m_mprisServices;
    std::vector<int> m_systemTrayIconApps;
    DesktopFileManager &m_desktopFileManager = DesktopFileManager::instance();
    std::shared_ptr<ConfigManager> m_configManager;

    std::string m_currentActiveWid;
    std::string m_preActiveWid;

    int m_backgroundToCachedStateInterval;

    SelectionChecker m_selectionChecker;

    AppChangedCallback m_appStartedCallback;
    AppChangedCallback m_appStateChangedCallback;
    AppChangedCallback m_appFinishedCallback;
    AppChangedCallback m_appChildPidsUpdatedCallback;
    AppWinChangedCallback m_appWinsChangedCallback;
};

#endif // APPINFOMANAGER_H

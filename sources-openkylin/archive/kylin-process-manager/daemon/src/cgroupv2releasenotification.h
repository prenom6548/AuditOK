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

#ifndef CGROUPV2RELEASENOTIFICATION_H
#define CGROUPV2RELEASENOTIFICATION_H

#include <atomic>
#include <functional>
#include <future>
#include <map>
#include <string>
#include <sys/epoll.h>
#include <sys/inotify.h>

class CGroupV2ReleaseNotification
{
public:
    using ReleaseHandler = std::function<void(std::string groupPath)>;

    CGroupV2ReleaseNotification();
    ~CGroupV2ReleaseNotification();
    void addWatchGroup(const std::string &groupPath, ReleaseHandler releaseHandler);

private:
    void startWatch();
    int initEpoll();
    void doStartWatch();
    void handleEpollEvents(epoll_event *events, int eventNumber);
    void handleInotifyEvent();
    void handleCGroupReleaseEvent(inotify_event *event);
    void checkAndTrigger(const std::string &groupPath, ReleaseHandler handler);

private:
    int m_inotifyFd;
    int m_epollFd;
    std::future<void> m_watcherFuture;
    std::mutex m_mutex;
    // <watched fd<watched path, release handler>>
    std::map<int, std::pair<std::string, ReleaseHandler>> m_watchedPathInfo;
    bool m_started;
    std::atomic<bool> m_stop;
};

#endif // CGROUPV2RELEASENOTIFICATION_H

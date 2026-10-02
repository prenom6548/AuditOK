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

#include "cgroupv2releasenotification.h"
#include "loghelper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_EVENTS 5
#define INOTIFY_BUF_LEN 4096

CGroupV2ReleaseNotification::CGroupV2ReleaseNotification()
    : m_started(false)
{
}

CGroupV2ReleaseNotification::~CGroupV2ReleaseNotification()
{
    m_stop = true;
    if (m_watcherFuture.valid()) {
        m_watcherFuture.wait();
    }
}

void CGroupV2ReleaseNotification::addWatchGroup(
    const std::string &groupPath, ReleaseHandler releaseHandler)
{
    if (groupPath.empty())
        return;
    if (!m_started)
        startWatch();

    std::string filePath = groupPath + "/cgroup.events";
    int wd = inotify_add_watch(m_inotifyFd, filePath.c_str(), IN_MODIFY);
    if (wd < 0) {
        perror("inotify_add_watch");
        return;
    }

    {
        std::lock_guard<std::mutex> locker(m_mutex);
        m_watchedPathInfo[wd] = {groupPath, std::move(releaseHandler)};
    }
}

void CGroupV2ReleaseNotification::startWatch()
{
    m_inotifyFd = inotify_init1(IN_NONBLOCK);
    if (m_inotifyFd < 0) {
        perror("inotify_init");
        return;
    }

    m_epollFd = epoll_create1(0);
    if (m_epollFd < 0) {
        perror("epoll_create1");
        close(m_inotifyFd);
        return;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = m_inotifyFd;
    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_inotifyFd, &ev) == -1) {
        perror("epoll_ctl");
        close(m_epollFd);
        close(m_inotifyFd);
        return;
    }

    m_stop = false;
    m_started = true;
    doStartWatch();
}

void CGroupV2ReleaseNotification::doStartWatch()
{
    m_watcherFuture = std::async(std::launch::async, [this]() {
        struct epoll_event events[MAX_EVENTS];
        while (!m_stop) {
            int eventNumber = epoll_wait(m_epollFd, events, MAX_EVENTS, 100);
            if (eventNumber < 0) {
                if (errno == EINTR)
                    continue;
                perror("epoll_wait");
                break;
            }
            handleEpollEvents(events, eventNumber);
        }
        close(m_epollFd);
        close(m_inotifyFd);
        m_epollFd = -1;
        m_inotifyFd = -1;
        m_started = false;
    });
}

void CGroupV2ReleaseNotification::handleEpollEvents(epoll_event *events, int eventNumber)
{
    for (int i = 0; i < eventNumber; i++) {
        if (events[i].data.fd != m_inotifyFd) {
            continue;
        }
        handleInotifyEvent();
    }
}

void CGroupV2ReleaseNotification::handleInotifyEvent()
{
    char buffer[INOTIFY_BUF_LEN];
    ssize_t len = read(m_inotifyFd, buffer, sizeof(buffer));
    if (len < 0) {
        if (errno != EAGAIN)
            perror("read");
        return;
    }

    for (char *ptr = buffer; ptr < buffer + len;) {
        struct inotify_event *event = (inotify_event *)ptr;
        ptr += sizeof(inotify_event) + event->len;

        if (event->mask & IN_Q_OVERFLOW) {
            std::map<int, std::pair<std::string, ReleaseHandler>> snapshot;
            {
                std::lock_guard<std::mutex> locker(m_mutex);
                snapshot = m_watchedPathInfo; // 拷贝一份
            }
            for (auto &item : snapshot) {
                checkAndTrigger(item.second.first, item.second.second);
            }
            continue;
        }

        if (event->mask & IN_MODIFY) {
            handleCGroupReleaseEvent(event);
        }
    }
}

void CGroupV2ReleaseNotification::handleCGroupReleaseEvent(inotify_event *event)
{
    std::string groupPath;
    ReleaseHandler handler;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        auto it = m_watchedPathInfo.find(event->wd);
        if (it == m_watchedPathInfo.end())
            return;
        groupPath = it->second.first;
        handler = it->second.second;
    }

    checkAndTrigger(groupPath, handler);
}

void CGroupV2ReleaseNotification::checkAndTrigger(const std::string &groupPath, ReleaseHandler handler)
{
    std::string filePath = groupPath + "/cgroup.events";
    FILE *file = fopen(filePath.c_str(), "r");
    if (!file)
        return;

    char line[256];
    int populated = 1;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "populated", 9) == 0) {
            sscanf(line, "populated %d", &populated);
            break;
        }
    }
    PRINT_DBG("group path : %s, populated : %d", groupPath.c_str(), populated);
    fclose(file);

    if (populated == 0) {
        handler(groupPath);
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            for (auto it = m_watchedPathInfo.begin(); it != m_watchedPathInfo.end(); ++it) {
                if (it->second.first == groupPath) {
                    inotify_rm_watch(m_inotifyFd, it->first);
                    m_watchedPathInfo.erase(it);
                    break;
                }
            }
        }
    }
}
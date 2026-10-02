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

#ifndef EXCEPTIONGROUPPROCESSWATCHER_H
#define EXCEPTIONGROUPPROCESSWATCHER_H

#include "processinfomanager.h"
#include "schedpolicy.h"
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>
#include <atomic>
#include <unordered_map>
#include <vector>

class ExceptionGroupProcessWatcher;
class WatcherThread : public QThread
{
    Q_OBJECT
public:
    explicit WatcherThread(ExceptionGroupProcessWatcher *watcher, QObject *parent = nullptr)
        : QThread(parent)
        , m_watcher(watcher)
        , m_running(false)
    {
    }

    void run() override;
    void stop();

private:
    ExceptionGroupProcessWatcher *m_watcher;
    std::atomic<bool> m_running;
    QMutex m_mutex;
    QWaitCondition m_condition;
};

class ExceptionGroupProcessWatcher : public QObject
{
    Q_OBJECT
public:
    explicit ExceptionGroupProcessWatcher(
        const std::shared_ptr<ProcessInfoManager> &processInfoManager, QObject *parent = nullptr);
    ~ExceptionGroupProcessWatcher();

public Q_SLOTS:
    void startWatcher();
    void stopWatcher();

Q_SIGNALS:
    void exceptionSessionAppCaught(const std::string &desktopFile, int pid);

private:
    std::vector<int> readFileToPids(const std::string &file) const;
    std::vector<int> findExceptionPids(const std::vector<int> &oldPids, const std::vector<int> &newPids) const;
    void handleExceptionPids(const std::vector<int> &pids);
    void addPath(std::string path);
    void initPaths();

private:
    struct WatchedGroupInfo
    {
        std::string path;
        std::vector<int> pids;
    };

private:
    std::map<std::string, WatchedGroupInfo> m_watchedGroupPaths;
    std::atomic_bool m_watcherRunning;
    std::shared_ptr<ProcessInfoManager> m_processInfoManager;
    WatcherThread *m_watcherThread = nullptr;

    friend class WatcherThread;
};

#endif // EXCEPTIONGROUPPROCESSWATCHER_H

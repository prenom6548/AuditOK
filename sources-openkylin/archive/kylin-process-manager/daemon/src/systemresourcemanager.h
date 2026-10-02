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

#ifndef SYSTEMRESOURCEMANAGER_H
#define SYSTEMRESOURCEMANAGER_H

#include <QObject>

#include <map>
#include <vector>

#include "resource.h"

class QThread;

struct ResourceThresholdConfig {
    std::map<resource::Resource, resource::ResourceUrgencyThreshold> resourceThresholds;
    /// Memory: SwapFree must be below this many MiB (meminfo kB) for PSI or 1s poll path.
    int memoryGateSwapFreeMb{0};
    /// Memory: (MemAvailable - MemFree) must be below this many MiB for PSI or 1s poll path.
    int memoryGateReclaimHeadroomMb{0};
    /// Memory: MemFree must be below (zone high watermark + this many MiB).
    int memoryGateMemFreeHighMarginMb{0};
};

class ResourceWatcher;
class SystemResourceManager : public QObject
{
    Q_OBJECT
public:
    explicit SystemResourceManager(QObject *parent = nullptr);
    void startWatchers();
    QString reclaimProcesses(const QList<int> &pids) const;

Q_SIGNALS:
    void ResourceThresholdWarning(QString resource, int level);

private:
    void startWatcherThread(ResourceWatcher *watcher, QThread *thread) const;
    std::vector<ResourceWatcher *> createWatchers(const ResourceThresholdConfig &config) const;
    ResourceThresholdConfig parserResourceThreshold() const;
    std::string reclaimProcess(int pid) const;
};

#endif // SYSTEMRESOURCEMANAGER_H

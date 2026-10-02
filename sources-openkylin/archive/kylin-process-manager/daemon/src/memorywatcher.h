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

#ifndef MEMORYWATCHER_H
#define MEMORYWATCHER_H

#include "resource.h"
#include "resourcewatcher.h"

#include <string>
#include <vector>

class QTimer;
class MemoryWatcher : public ResourceWatcher
{
    Q_OBJECT
public:
    explicit MemoryWatcher(const std::map<resource::Resource, resource::ResourceUrgencyThreshold> &resourceThresholds,
                           int meminfoGateSwapFreeMb,
                           int meminfoGateReclaimHeadroomMb,
                           int memFreeHighWatermarkMarginMb,
                           QObject *parent = nullptr);

public Q_SLOTS:
    void start() override;
    void stop() override;

private Q_SLOTS:
    void checkMemoryRemaining();

private:
    struct MemoryPsiTrigger {
        std::string trigger;
        int fd = -1;
    };

    /// Whether this kernel exposes usable /proc/pressure/memory (PSI memory).
    bool isMemoryPsiSupported() const;
    /// SwapFree and (MemAvailable - MemFree) vs configured MiB gates (libproc2 / meminfo).
    bool meminfoGatesSatisfied() const;
    /// Parse /proc/zoneinfo once and cache system high watermark in kB.
    unsigned long loadSystemHighWatermarkKb() const;
    int registerMemoryPsiTriggers();
    bool addMemoryPsiFdToEpoll(int epfd, int fd) const;
    MemoryPsiTrigger findMemoryPsiTriggerByFd(int fd) const;
    void receiveMemoryPsiEvents(int epfd);
    void closeMemoryPsiTriggers();

    QTimer *m_timer;
    bool m_stop;
    std::vector<MemoryPsiTrigger> m_memoryPsiTriggers;
    int m_meminfoGateSwapFreeMb;
    int m_meminfoGateReclaimHeadroomMb;
    int m_memFreeHighWatermarkMarginMb;
    unsigned long m_systemHighWatermarkKb;
};

#endif // MEMORYWATCHER_H

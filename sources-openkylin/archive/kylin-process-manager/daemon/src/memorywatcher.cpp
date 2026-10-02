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

#include "memorywatcher.h"
#include "loghelper.h"

#include <libproc2/meminfo.h>

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>

#include <QDebug>
#include <QTimer>

using namespace resource;

namespace {

// Fixed PSI memory trigger: 100ms aggregate stall within a 1s window (not configurable).
constexpr int kPsiStallUs = 100000;
constexpr int kPsiWindowUs = 1000000;

std::string makeFixedMemoryPsiTriggerString()
{
    return std::string("some ") + std::to_string(kPsiStallUs) + " " + std::to_string(kPsiWindowUs);
}

} // namespace

MemoryWatcher::MemoryWatcher(const std::map<Resource, ResourceUrgencyThreshold> &resourceThresholds,
                             int meminfoGateSwapFreeMb,
                             int meminfoGateReclaimHeadroomMb,
                             int memFreeHighWatermarkMarginMb,
                             QObject *parent)
    : ResourceWatcher{parent}
    , m_timer(new QTimer(this))
    , m_stop(false)
    , m_meminfoGateSwapFreeMb(meminfoGateSwapFreeMb)
    , m_meminfoGateReclaimHeadroomMb(meminfoGateReclaimHeadroomMb)
    , m_memFreeHighWatermarkMarginMb(memFreeHighWatermarkMarginMb)
    , m_systemHighWatermarkKb(loadSystemHighWatermarkKb())
{
    Q_UNUSED(resourceThresholds);
    PRINT_NOTICE("Init memory meminfo gates: SwapFree < %d MiB, (MemAvailable-MemFree) < %d MiB, "
                 "MemFree < (zone-high + %d MiB), zone-high cached=%lu MiB\n",
                 m_meminfoGateSwapFreeMb, m_meminfoGateReclaimHeadroomMb, m_memFreeHighWatermarkMarginMb,
                 m_systemHighWatermarkKb / 1024UL);

    connect(m_timer, &QTimer::timeout, this, &MemoryWatcher::checkMemoryRemaining);
}

bool MemoryWatcher::isMemoryPsiSupported() const
{
    if (access("/proc/pressure/memory", F_OK) != 0)
        return false;
    int fd = open("/proc/pressure/memory", O_RDWR | O_NONBLOCK);
    if (fd < 0)
        return false;
    close(fd);
    return true;
}

bool MemoryWatcher::meminfoGatesSatisfied() const
{
    struct meminfo_info *info = nullptr;
    if (procps_meminfo_new(&info) < 0) {
        PRINT_NOTICE("Memory meminfo gate: procps_meminfo_new failed\n");
        return false;
    }

    const unsigned long swapFreeKb = MEMINFO_GET(info, MEMINFO_SWAP_FREE, ul_int);
    const unsigned long memAvailKb = MEMINFO_GET(info, MEMINFO_MEM_AVAILABLE, ul_int);
    const unsigned long memFreeKb = MEMINFO_GET(info, MEMINFO_MEM_FREE, ul_int);
    procps_meminfo_unref(&info);

    const unsigned long swapThKb = static_cast<unsigned long>(m_meminfoGateSwapFreeMb) * 1024UL;
    const unsigned long headThKb = static_cast<unsigned long>(m_meminfoGateReclaimHeadroomMb) * 1024UL;
    const unsigned long freeHighMarginKb = static_cast<unsigned long>(m_memFreeHighWatermarkMarginMb) * 1024UL;
    const unsigned long memFreeHighGateKb = m_systemHighWatermarkKb + freeHighMarginKb;

    if (swapFreeKb >= swapThKb)
        return false;

    const long long diffKb = static_cast<long long>(memAvailKb) - static_cast<long long>(memFreeKb);
    const unsigned long diffKbPos = diffKb > 0 ? static_cast<unsigned long>(diffKb) : 0UL;
    if (diffKbPos >= headThKb)
        return false;

    if (memFreeKb >= memFreeHighGateKb)
        return false;

    return true;
}

unsigned long MemoryWatcher::loadSystemHighWatermarkKb() const
{
    std::ifstream zoneinfo("/proc/zoneinfo");
    if (!zoneinfo.is_open()) {
        PRINT_NOTICE("Memory meminfo gate: failed to open /proc/zoneinfo, fallback zone-high=0\n");
        return 0UL;
    }

    const long pageSize = sysconf(_SC_PAGESIZE);
    const unsigned long pageKb = (pageSize > 0) ? static_cast<unsigned long>(pageSize / 1024L) : 4UL;
    if (pageSize <= 0)
        PRINT_NOTICE("Memory meminfo gate: sysconf(_SC_PAGESIZE) failed, fallback page size to 4 KiB\n");

    unsigned long totalHighPages = 0UL;
    std::string line;
    while (std::getline(zoneinfo, line)) {
        std::istringstream iss(line);
        std::string key;
        unsigned long value = 0UL;
        if (!(iss >> key >> value))
            continue;
        if (key == "high")
            totalHighPages += value;
    }

    return totalHighPages * pageKb;
}

bool MemoryWatcher::addMemoryPsiFdToEpoll(int epfd, int fd) const
{
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = EPOLLPRI | EPOLLET;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        PRINT_NOTICE("Memory PSI epoll_ctl failed, %s\n", strerror(errno));
        close(fd);
        return false;
    }
    return true;
}

MemoryWatcher::MemoryPsiTrigger MemoryWatcher::findMemoryPsiTriggerByFd(int fd) const
{
    for (const MemoryPsiTrigger &t : m_memoryPsiTriggers) {
        if (t.fd == fd)
            return t;
    }
    return MemoryPsiTrigger{};
}

void MemoryWatcher::closeMemoryPsiTriggers()
{
    for (MemoryPsiTrigger &t : m_memoryPsiTriggers) {
        if (t.fd >= 0) {
            close(t.fd);
            t.fd = -1;
        }
    }
    m_memoryPsiTriggers.clear();
}

int MemoryWatcher::registerMemoryPsiTriggers()
{
    closeMemoryPsiTriggers();

    int epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd < 0)
        epfd = epoll_create(1);
    if (epfd < 0) {
        PRINT_NOTICE("Memory PSI epoll_create failed, %s\n", strerror(errno));
        return -1;
    }

    MemoryPsiTrigger entry;
    entry.trigger = makeFixedMemoryPsiTriggerString();

    int fd = open("/proc/pressure/memory", O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        PRINT_NOTICE("Open /proc/pressure/memory failed, %s\n", strerror(errno));
        close(epfd);
        return -1;
    }

    const std::string &tr = entry.trigger;
    if (write(fd, tr.c_str(), tr.length() + 1) < 0) {
        PRINT_NOTICE("Memory PSI write failed (%s), %s\n", tr.c_str(), strerror(errno));
        close(fd);
        close(epfd);
        return -1;
    }

    if (!addMemoryPsiFdToEpoll(epfd, fd)) {
        close(epfd);
        return -1;
    }

    entry.fd = fd;
    m_memoryPsiTriggers.push_back(std::move(entry));

    return epfd;
}

void MemoryWatcher::receiveMemoryPsiEvents(int epfd)
{
    while (!m_stop) {
        epoll_event events[8];
        int n = epoll_wait(epfd, events, 8, 1000);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            PRINT_NOTICE("Memory PSI epoll_wait failed, %s\n", strerror(errno));
            break;
        }
        if (n == 0)
            continue;

        for (int i = 0; i < n; ++i) {
            if (!(events[i].events & EPOLLPRI))
                continue;
            MemoryPsiTrigger t = findMemoryPsiTriggerByFd(events[i].data.fd);
            if (t.fd < 0)
                continue;
            if (!meminfoGatesSatisfied())
                continue;
            constexpr int kMemoryWarningLevel = 2;
            PRINT_DBG("Emit Memory resource warning level: %d (PSI + meminfo gates)\n", kMemoryWarningLevel);
            Q_EMIT ResourceThresholdWarning("Memory", kMemoryWarningLevel);
        }
    }
}

void MemoryWatcher::checkMemoryRemaining()
{
    if (!meminfoGatesSatisfied())
        return;

    constexpr int kMemoryWarningLevel = 2;
    PRINT_DBG("Emit Memory resource warning level: %d (1s poll + meminfo gates)\n", kMemoryWarningLevel);
    Q_EMIT ResourceThresholdWarning("Memory", kMemoryWarningLevel);
}

void MemoryWatcher::stop()
{
    m_stop = true;
    m_timer->stop();
}

void MemoryWatcher::start()
{
    m_stop = false;

    if (!isMemoryPsiSupported()) {
        PRINT_NOTICE("Memory monitor: PSI not supported, using 1s meminfo polling ");
        m_timer->start(1000);
        return;
    }

    int epfd = registerMemoryPsiTriggers();
    if (epfd < 0) {
        PRINT_NOTICE("Memory monitor: PSI present but registration failed, using 1s meminfo polling\n");
        closeMemoryPsiTriggers();
        m_timer->start(1000);
        return;
    }

    PRINT_NOTICE("Memory monitor: PSI supported (some %dus stall / %dus window); meminfo gates SwapFree<%d MiB, "
                 "(MemAvailable-MemFree)<%d MiB, MemFree<(zone-high+%d MiB)\n",
                 kPsiStallUs, kPsiWindowUs, m_meminfoGateSwapFreeMb, m_meminfoGateReclaimHeadroomMb,
                 m_memFreeHighWatermarkMarginMb);
    receiveMemoryPsiEvents(epfd);
    closeMemoryPsiTriggers();
    close(epfd);
}

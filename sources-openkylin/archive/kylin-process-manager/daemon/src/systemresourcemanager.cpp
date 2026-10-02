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

#include "systemresourcemanager.h"

#include <fcntl.h>
#include <fstream>
#include <unistd.h>

#include <QDebug>
#include <QThread>
#include <jsoncpp/json/json.h>

#include "loghelper.h"
#include "memorywatcher.h"
#include "pressurewatcher.h"

static const char *RESOURCE_CONFIG_FILE = "/etc/kylin-process-manager/kylin-process-manager.json";
static const int RESOURCE_URGENCY_COUNT = 3;

using namespace resource;

SystemResourceManager::SystemResourceManager(QObject *parent)
    : QObject{parent}
{
}

QString SystemResourceManager::reclaimProcesses(const QList<int> &pids) const
{
    QString pidsStr;
    for (size_t i = 0; i < pids.size(); ++i) {
        pidsStr += QString::number(pids[i]);
        if (i != pids.size() - 1) {
            pidsStr += ", "; // 元素间用逗号分隔
        }
    }
    PRINT_DBG("Reclaim processes: %s", pidsStr.toUtf8().constData());

    if (pids.isEmpty()) {
        QString errorMessage = "Pids is empty.";
        return errorMessage;
    }
    if (access("/proc/1/reclaim", F_OK) == -1) {
        QString errorMessage = "Current system doesn't support memory reclamation.";
        return errorMessage;
    }

    for (const int &pid : qAsConst(pids)) {
        QString errorMessage = QString::fromStdString(reclaimProcess(pid));
        if (!errorMessage.isEmpty())
            return errorMessage;
    }

    return QString();
}

std::string SystemResourceManager::reclaimProcess(int pid) const
{
    const char *content = "all";
    std::string api = "/proc/" + std::to_string(pid) + "/reclaim";

    int fd = open(api.c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        std::string errorMessage = "Open api file " + api + " failed, " + strerror(errno);
        return errorMessage;
    }
    std::string contentCstr(content);
    if (write(fd, content, contentCstr.length() + 1) < 0) {
        close(fd);
        std::string errorMessage = "Write api file " + api + " failed, " + strerror(errno);
        return errorMessage;
    }

    return std::string();
}

std::vector<ResourceWatcher *> SystemResourceManager::createWatchers(const ResourceThresholdConfig &config) const
{
    std::vector<ResourceWatcher *> watchers;

    watchers.push_back(new PressureWatcher(config.resourceThresholds));
    watchers.push_back(new MemoryWatcher(config.resourceThresholds, config.memoryGateSwapFreeMb,
                                         config.memoryGateReclaimHeadroomMb,
                                         config.memoryGateMemFreeHighMarginMb));

    return watchers;
}

void SystemResourceManager::startWatchers()
{
    ResourceThresholdConfig config = parserResourceThreshold();
    std::vector<ResourceWatcher *> watchers = createWatchers(config);
    for (ResourceWatcher *watcher : watchers) {
        QThread *watcherThread = new QThread(this);
        startWatcherThread(watcher, watcherThread);
    }
}

void SystemResourceManager::startWatcherThread(ResourceWatcher *watcher, QThread *thread) const
{
    if (!watcher || !thread) {
        return;
    }

    watcher->moveToThread(thread);

    connect(thread, &QThread::started, watcher, &ResourceWatcher::start);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, watcher, &ResourceWatcher::deleteLater);
    connect(watcher, &ResourceWatcher::finished, thread, &QThread::quit);
    connect(watcher, &ResourceWatcher::ResourceThresholdWarning, this, &SystemResourceManager::ResourceThresholdWarning);

    thread->start();
}

ResourceThresholdConfig SystemResourceManager::parserResourceThreshold() const
{
    ResourceThresholdConfig result;
    std::ifstream file(RESOURCE_CONFIG_FILE);
    Json::Value resourceConfig;

    if (!file.is_open()) {
        PRINT_NOTICE("Failed to open config file: %s\n", RESOURCE_CONFIG_FILE);
        return result;
    }
    file >> resourceConfig;

    resourceConfig = resourceConfig["ResourceThreshold"];
    for (Json::Value::ArrayIndex i = 0; i < resourceConfig.size(); ++i) {
        auto resourceThresholdInfo = resourceConfig[i];
        std::string resource = resourceThresholdInfo["resource"].asString();
        const Resource resEnum = stringToResourceEnum(resource);

        if (resEnum == Resource::Memory) {
            if (!resourceThresholdInfo.isMember("gateSwapFreeMb")) {
                PRINT_NOTICE("Memory resource requires gateSwapFreeMb (integer, MiB)\n");
                return ResourceThresholdConfig{};
            }
            if (!resourceThresholdInfo.isMember("gateReclaimHeadroomMb")) {
                PRINT_NOTICE("Memory resource requires gateReclaimHeadroomMb (integer, MiB)\n");
                return ResourceThresholdConfig{};
            }
            if (!resourceThresholdInfo.isMember("gateMemFreeHighMarginMb")) {
                PRINT_NOTICE("Memory resource requires gateMemFreeHighMarginMb (integer, MiB)\n");
                return ResourceThresholdConfig{};
            }
            const Json::Value &swapGate = resourceThresholdInfo["gateSwapFreeMb"];
            const Json::Value &headGate = resourceThresholdInfo["gateReclaimHeadroomMb"];
            const Json::Value &highMarginGate = resourceThresholdInfo["gateMemFreeHighMarginMb"];
            if ((!swapGate.isInt() && !swapGate.isUInt()) || (!headGate.isInt() && !headGate.isUInt())
                || (!highMarginGate.isInt() && !highMarginGate.isUInt())) {
                PRINT_NOTICE("Memory gate*Mb fields must be integers\n");
                return ResourceThresholdConfig{};
            }
            result.memoryGateSwapFreeMb = swapGate.asInt();
            result.memoryGateReclaimHeadroomMb = headGate.asInt();
            result.memoryGateMemFreeHighMarginMb = highMarginGate.asInt();
            if (result.memoryGateSwapFreeMb <= 0 || result.memoryGateReclaimHeadroomMb <= 0
                || result.memoryGateMemFreeHighMarginMb <= 0) {
                PRINT_NOTICE("Memory gate*Mb values must be positive\n");
                return ResourceThresholdConfig{};
            }
            continue;
        }

        auto thresholds = resourceThresholdInfo["threshold"];
        if (RESOURCE_URGENCY_COUNT != thresholds.size()) {
            PRINT_NOTICE("Config file thresholds error, threshold count need: %d but current config threshold count is: %d\n", RESOURCE_URGENCY_COUNT, thresholds.size());
            return ResourceThresholdConfig{};
        }

        ResourceUrgencyThreshold urgencyThreshold;
        for (Json::Value::ArrayIndex j = 0; j < thresholds.size(); ++j) {
            urgencyThreshold[intToResourceUrgencyEnum(j)] = thresholds[j].asInt();
        }

        result.resourceThresholds[resEnum] = urgencyThreshold;
    }

    return result;
}

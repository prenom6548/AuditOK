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

#include "pressurewatcher.h"
#include "loghelper.h"
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <QDebug>

using namespace resource;

PressureWatcher::PressureWatcher(const std::map<Resource, ResourceUrgencyThreshold> &resourceThresholds,
                                 QObject *parent)
    : ResourceWatcher{parent}
    , m_stop(false)
{
    initPsiTriggers(resourceThresholds);
}

void PressureWatcher::initPsiTriggers(const std::map<Resource, ResourceUrgencyThreshold> &resourceThresholds)
{
    using ResourceThreshold = std::pair<Resource, ResourceUrgencyThreshold>;
    for (const ResourceThreshold &resourceThreshold : resourceThresholds) {
        Resource resource = resourceThreshold.first;
        if (resource != Resource::CPU && resource != Resource::IO) {
            continue;
        }
        const ResourceUrgencyThreshold &urgencyThresholds = resourceThreshold.second;
        for (const std::pair<ResourceUrgency, int> urgencyThreshold : urgencyThresholds) {
            ResourceUrgency urgency = urgencyThreshold.first;
            std::string trigger = mapPressureValueToPsiTrigger(urgencyThreshold.second);
            std::string resourceStr = resourceEnumToString(resource);
            m_triggers.push_back(PsiTrigger(resourceStr, urgency, trigger, -1));
        }
    }

    for (const PsiTrigger &trigger : m_triggers) {
        PRINT_DBG("Init trigger: %s %d %s\n", trigger.resource.c_str(), resourceUrgencyEnumToInt(trigger.urgency), trigger.trigger.c_str());
    }
}

std::string PressureWatcher::mapPressureValueToPsiTrigger(int pressure) const
{
    const int coefficient = 1;
    const int windowTime = 1000000 * coefficient;
    const int triggerTime = (pressure / (double)100) * 1000000 * coefficient;
    std::string trigger = "some " + std::to_string(triggerTime) + " " + std::to_string(windowTime);
    return trigger;
}

int PressureWatcher::registerPsiTriggers()
{

    int epfd = epoll_create(1);
    if (epfd < 0) {
        PRINT_NOTICE("Call epoll_create failed, %s\n", strerror(errno));
        return epfd;
    }

    for (auto &trigger : m_triggers) {
        std::string resource = trigger.resource;
        std::transform(resource.begin(), resource.end(), resource.begin(), ::tolower);
        std::string interfaceFile = "/proc/pressure/" + resource;

        int fd = open(interfaceFile.c_str(), O_RDWR | O_NONBLOCK);
        if (-1 == fd) {
            PRINT_NOTICE("Open interface file %s failed, %s\n", interfaceFile.c_str(), strerror(errno));
            continue;
        }
        std::string triggerStr(trigger.trigger);
        if (write(fd, trigger.trigger.c_str(), triggerStr.length() + 1) < 0) {
            PRINT_NOTICE("Write failed, %s\n", strerror(errno));
            close(fd);
            continue;
        }
        if (addToEpoll(epfd, fd))
            trigger.fd = fd;
    }

    return epfd;
}

bool PressureWatcher::addToEpoll(int epfd, int fd) const
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLPRI;
    ev.events |= EPOLLET;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        PRINT_NOTICE("Call epoll_ctl failed, %s\n", strerror(errno));
        close(fd);
        return false;
    }

    return true;
}

PsiTrigger PressureWatcher::findTriggerByFd(int fd) const
{
    PsiTrigger result;
    for (const PsiTrigger &trigger : m_triggers) {
        if (trigger.fd == fd)
            result = trigger;
    }
    return result;
}

void PressureWatcher::closeTriggers()
{
    for (auto &trigger : m_triggers) {
        if (trigger.fd != -1) {
            close(trigger.fd);
            trigger.fd = -1;
        }
    }
}

void PressureWatcher::start()
{
    PRINT_NOTICE("Start watch system pressure\n");

    int epfd = registerPsiTriggers();

    receiveEpollMessage(epfd);

    closeTriggers();
    close(epfd);
}

int PressureWatcher::containResource(std::vector<PsiTrigger> triggers, const std::string &resource) const
{
    for (int i = 0; i < triggers.size(); ++i) {
        if (triggers[i].resource == resource)
            return i;
    }
    return -1;
}

void PressureWatcher::receiveEpollMessage(int epfd)
{
    while (!m_stop) {
        struct epoll_event events[1024];
        int eventNum = epoll_wait(epfd, events, 1024, -1);

        std::vector<PsiTrigger> receivedTriggers = handleEpollMessage(events, eventNum);

        handlePsiMessage(receivedTriggers);

        sleep(3);
    }
}

std::vector<PsiTrigger> PressureWatcher::handleEpollMessage(const epoll_event *events, int eventNum) const
{
    std::vector<PsiTrigger> receivedTriggers;
    // An epoll message may contain multiple events, for events on the
    // same resource, we only keep the one with the higher stress level
    for (int i = 0; i < eventNum; ++i) {
        if (!(events[i].events & EPOLLPRI)) {
            continue;
        }
        PsiTrigger receivedTirgger = findTriggerByFd(events[i].data.fd);
        int index = containResource(receivedTriggers, receivedTirgger.resource);
        if (index == -1) {
            receivedTriggers.push_back(std::move(receivedTirgger));
            continue;
        }

        if (receivedTriggers[index].urgency < receivedTirgger.urgency) {
            receivedTriggers[index] = receivedTirgger;
        }
    }
    return receivedTriggers;
}

void PressureWatcher::handlePsiMessage(const std::vector<PsiTrigger> &triggers)
{
    for (const PsiTrigger &trigger : triggers) {
        int level = resourceUrgencyEnumToInt(trigger.urgency);
        if (-1 == level) {
            PRINT_NOTICE("Get urgency index error\n");
            return;
        }
        PRINT_DBG("Emit: %s, resource warning level : %d\n", trigger.resource.c_str(), level);
        Q_EMIT ResourceThresholdWarning(QString::fromStdString(trigger.resource), level);
    }
}

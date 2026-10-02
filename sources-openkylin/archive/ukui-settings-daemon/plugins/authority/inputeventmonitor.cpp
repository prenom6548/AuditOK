/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "inputeventmonitor.h"
#include <QThread>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <libevdev/libevdev.h>
#include <linux/input.h>
#include <algorithm>
#include <cstring>

InputMonitor::InputMonitor(QObject *parent) : QObject(parent) {}

InputMonitor::~InputMonitor()
{
    stopMonitoring();
    cleanupAllDevices();

    if (m_watchFd >= 0) {
        inotify_rm_watch(m_inotifyFd, m_watchFd);
        m_watchFd = -1;
    }
    if (m_inotifyFd >= 0) {
        close(m_inotifyFd);
        m_inotifyFd = -1;
    }
}

void InputMonitor::stopMonitoring()
{
    m_running.store(false, std::memory_order_release);
}

void InputMonitor::cleanupAllDevices()
{
    for (auto &ctx : m_devices) {
        if (ctx.dev) {
            libevdev_free(ctx.dev);
            ctx.dev = nullptr;
        }
        if (ctx.fd >= 0) {
            close(ctx.fd);
            ctx.fd = -1;
        }
    }
    m_devices.clear();
}

void InputMonitor::initInotify()
{
    m_inotifyFd = inotify_init1(IN_NONBLOCK);
    if (m_inotifyFd == -1) {
        SYS_LOG(LOG_WARNING, "[InputWorker] inotify_init1 失败：%s", strerror(errno));
        return;
    }

    uint32_t mask = IN_CREATE | IN_DELETE;
    m_watchFd = inotify_add_watch(m_inotifyFd, "/dev/input", mask);
    if (m_watchFd == -1) {
        SYS_LOG(LOG_WARNING, "[InputWorker] inotify_add_watch 失败：%s", strerror(errno));
        close(m_inotifyFd);
        m_inotifyFd = -1;
        return;
    }

    SYS_LOG(LOG_DEBUG, "[InputWorker] inotify 监控已建立：/dev/input");
}

void InputMonitor::handleInotifyEvents()
{
    char buf[4096] __attribute__((aligned(8)));
    ssize_t len;

    while ((len = read(m_inotifyFd, buf, sizeof(buf))) > 0) {
        const char *ptr = buf;
        while (ptr < buf + len) {
            struct inotify_event *event = (struct inotify_event *)ptr;

            if (event->len > 0 && (event->mask & (IN_CREATE | IN_DELETE))) {
                std::string name(event->name);
                if (name.find("event") == 0) {
                    std::string fullPath = "/dev/input/" + name;

                    if (event->mask & IN_CREATE) {
                        SYS_LOG(LOG_INFO, "[InputWorker] 检测到设备插入：%s", fullPath.c_str());
                        addDevice(fullPath);
                    } else if (event->mask & IN_DELETE) {
                        SYS_LOG(LOG_INFO, "[InputWorker] 检测到设备移除：%s", fullPath.c_str());
                        removeDeviceByPath(fullPath);
                    }
                }
            }
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    if (len == -1 && errno != EAGAIN && errno != EINTR) {
        SYS_LOG(LOG_WARNING, "[InputWorker] 读取 inotify 事件失败：%s", strerror(errno));
    }
}

void InputMonitor::addDevice(const std::string& path)
{
    // 检查是否已存在
    for (const auto &ctx : m_devices) {
        if (ctx.path == path) {
            return;
        }
    }

    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        if (errno != EACCES) {
            SYS_LOG(LOG_DEBUG, "[InputWorker] 打开设备失败：%s, error: %s", path.c_str(), strerror(errno));
        }
        return;
    }

    struct libevdev *dev = nullptr;
    if (libevdev_new_from_fd(fd, &dev) != 0 || !dev) {
        SYS_LOG(LOG_WARNING, "[InputWorker] libevdev 初始化失败：%s", path.c_str());
        close(fd);
        return;
    }

    bool isRel = libevdev_has_event_type(dev, EV_REL) &&
                 libevdev_has_event_code(dev, EV_REL, REL_X) &&
                 libevdev_has_event_code(dev, EV_REL, REL_Y);

    bool isAbs = libevdev_has_event_type(dev, EV_ABS) &&
                 libevdev_has_event_code(dev, EV_ABS, ABS_X) &&
                 libevdev_has_event_code(dev, EV_ABS, ABS_Y);

    if (isRel || isAbs) {
        DeviceContext ctx;
        ctx.dev = dev;
        ctx.fd = fd;
        ctx.path = path;
        ctx.isTouch = isAbs;
        ctx.lastAbsX = -1;
        ctx.lastAbsY = -1;

        m_devices.emplace_back(std::move(ctx));
        SYS_LOG(LOG_INFO, "[InputWorker] 成功添加设备：%s (%s), Touch: %d",
                libevdev_get_name(dev), path.c_str(), isAbs);
    } else {
        libevdev_free(dev);
        close(fd);
    }
}

void InputMonitor::removeDeviceByPath(const std::string& path)
{
    for (size_t i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i].path == path) {
            SYS_LOG(LOG_INFO, "[InputWorker] 移除设备上下文：%s", path.c_str());
            removeDeviceByIndex(i);
            return;
        }
    }
    SYS_LOG(LOG_DEBUG, "[InputWorker] 尝试移除不存在的设备路径：%s", path.c_str());
}

void InputMonitor::removeDeviceByIndex(size_t index)
{
    if (index >= m_devices.size()) {
        return;
    }

    auto &ctx = m_devices[index];
    if (ctx.dev) {
        libevdev_free(ctx.dev);
        ctx.dev = nullptr;
    }
    if (ctx.fd >= 0) {
        close(ctx.fd);
        ctx.fd = -1;
    }

    m_devices.erase(m_devices.begin() + index);
}

void InputMonitor::scanInitialDevices()
{
    SYS_LOG(LOG_DEBUG, "[InputWorker] 扫描初始设备列表...");

    DIR *dir = opendir("/dev/input");
    if (!dir) {
        SYS_LOG(LOG_WARNING, "[InputWorker] 无法打开 /dev/input 进行初始扫描");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            std::string path = std::string("/dev/input/") + entry->d_name;
            addDevice(path);
        }
    }
    closedir(dir);

    SYS_LOG(LOG_DEBUG, "[InputWorker] 初始扫描完成，当前设备数：%zu", m_devices.size());
}

void InputMonitor::startMonitoring()
{
    m_running.store(true, std::memory_order_release);
    SYS_LOG(LOG_DEBUG, "[InputWorker] 线程启动 (inotify 模式)...");

    initInotify();
    scanInitialDevices();

    if (m_inotifyFd == -1 && m_devices.empty()) {
        SYS_LOG(LOG_WARNING, "[InputWorker] inotify 失败且无设备，等待重试...");
    }

    while (m_running.load(std::memory_order_acquire)) {
        fd_set readfds;
        FD_ZERO(&readfds);
        int max_fd = -1;

        // 添加 inotify fd
        if (m_inotifyFd >= 0) {
            FD_SET(m_inotifyFd, &readfds);
            if (m_inotifyFd > max_fd) {
                max_fd = m_inotifyFd;
            }
        }

        // 添加所有设备 fd
        for (const auto &ctx : m_devices) {
            if (ctx.fd >= 0) {
                FD_SET(ctx.fd, &readfds);
                if (ctx.fd > max_fd) {
                    max_fd = ctx.fd;
                }
            }
        }

        if (max_fd == -1) {
            QThread::msleep(500);
            continue;
        }

        // 阻塞等待 (超时 5 秒)
        struct timeval timeout = {5, 0};
        int ret = select(max_fd + 1, &readfds, nullptr, nullptr, &timeout);

        if (!m_running.load(std::memory_order_acquire)) {
            break;
        }

        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            SYS_LOG(LOG_WARNING, "[InputWorker] select 错误：%s", strerror(errno));
            QThread::msleep(100);
            continue;
        }

        if (ret == 0) {
            continue;
        }

        // 1. 处理 inotify 事件 (热插拔)
        if (m_inotifyFd >= 0 && FD_ISSET(m_inotifyFd, &readfds)) {
            handleInotifyEvents();
        }

        // 2. 处理设备输入事件
        for (size_t i = 0; i < m_devices.size(); ) {
            if (!m_running.load(std::memory_order_acquire)) {
                break;
            }

            auto &ctx = m_devices[i];
            if (ctx.fd >= 0 && FD_ISSET(ctx.fd, &readfds)) {
                struct input_event ev;
                int rc;
                int acc_dx = 0;
                int acc_dy = 0;

                while ((rc = libevdev_next_event(ctx.dev, LIBEVDEV_READ_FLAG_NORMAL, &ev)) == 0) {
                    if (ev.type == EV_REL) {
                        if (ev.code == REL_X) acc_dx += ev.value;
                        if (ev.code == REL_Y) acc_dy += ev.value;
                    } else if (ctx.isTouch && ev.type == EV_ABS) {
                        if (ev.code == ABS_X) {
                            if (ctx.lastAbsX != -1) {
                                acc_dx += (ev.value - ctx.lastAbsX);
                            }
                            ctx.lastAbsX = ev.value;
                        }
                        if (ev.code == ABS_Y) {
                            if (ctx.lastAbsY != -1) {
                                acc_dy += (ev.value - ctx.lastAbsY);
                            }
                            ctx.lastAbsY = ev.value;
                        }
                    } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                        if (acc_dx != 0 || acc_dy != 0) {
                            Q_EMIT mouseMoved(acc_dx, acc_dy);
                            acc_dx = 0;
                            acc_dy = 0;
                        }
                    }
                }

                if (rc != -EAGAIN) {
                    SYS_LOG(LOG_WARNING, "[InputWorker] 设备读取错误：%s (Code: %d)",
                            ctx.path.c_str(), rc);
                    removeDeviceByIndex(i);
                    continue;
                }
            }
            ++i;
        }
    }

    SYS_LOG(LOG_DEBUG, "[InputWorker] 监控线程已停止");
    cleanupAllDevices();

    if (m_watchFd >= 0) {
        inotify_rm_watch(m_inotifyFd, m_watchFd);
        m_watchFd = -1;
    }
    if (m_inotifyFd >= 0) {
        close(m_inotifyFd);
        m_inotifyFd = -1;
    }
}

InputEventMonitor::InputEventMonitor(QObject *parent) : QObject(parent)
{
    m_workerThread = new QThread(this);
    m_monitor = new InputMonitor();

    m_monitor->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_monitor, &InputMonitor::startMonitoring);

    connect(m_monitor, &InputMonitor::mouseMoved, this, &InputEventMonitor::MouseMoved);

    connect(this, &QObject::destroyed, this, [this]() {
        if (m_monitor) {
            QMetaObject::invokeMethod(m_monitor, "stopMonitoring", Qt::QueuedConnection);
        }

        if (m_workerThread && m_workerThread->isRunning()) {
            m_workerThread->quit();
            if (!m_workerThread->wait(3000)) {
                SYS_LOG(LOG_WARNING, "[InputEventMonitor] 工作线程未能及时退出");
            }
        }
    });

    m_workerThread->start();
    SYS_LOG(LOG_DEBUG, "[InputEventMonitor] 服务已启动");
}

InputEventMonitor::~InputEventMonitor()
{
    // 析构由 QObject 树和 destroyed 信号处理
}
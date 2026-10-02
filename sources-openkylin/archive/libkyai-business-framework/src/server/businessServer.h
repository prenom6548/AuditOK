/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#pragma once

#include <gio/gio.h>

#include <map>
#include <memory>
#include <string>

#include "baseProcessor.h"
#include "handler.h"

namespace scene {
class BusinessServer {
    friend gboolean onNewConnection(GDBusServer*, GDBusConnection*, gpointer);
    friend void onConnectionClosed(GDBusConnection*, gboolean, GError*, gpointer);

public:
    BusinessServer();
    void initServer();
    void notifyAllConnections(const std::string& signalName, const std::string& input);
    ~BusinessServer();

private:
    std::string serviceName;
    std::string address;
    GDBusServer* dbusServer;
    std::map<GDBusConnection*, std::shared_ptr<BaseProcessor>>
        connectionProcessorMap;  // 存放connection和processor的映射，连接断开时方便释放processor资源
};
}  // namespace scene

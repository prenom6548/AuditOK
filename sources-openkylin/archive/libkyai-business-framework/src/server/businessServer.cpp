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

#include "businessServer.h"

#include "conf.h"
#include "constant.h"
#include "error/exception.h"
#include "handler.h"
#include "log.h"

using namespace scene;

BusinessServer::BusinessServer() : dbusServer(nullptr) {
    serviceName = Configuration::getServiceName();
    if (serviceName.empty()) {
        PRINT_ERROR("get service name from config is empty.\n");
        throw SystemException(INVALID_CONFIG);
    }

    address = SERVER_ADDRESS_PRE + std::to_string(geteuid()) + "/" + serviceName +
              ".sock";  // GDBUS 监听地址默认前缀 拼接 UID + 服务名，是实际监听地址
}

void BusinessServer::initServer() {
    gchar* guid = g_dbus_generate_guid();
    GError* error = nullptr;

    auto server = g_dbus_server_new_sync(address.c_str(), G_DBUS_SERVER_FLAGS_NONE, guid, nullptr, nullptr, &error);
    if (server == nullptr) {
        PRINT_ERROR("creating server failed at address %s: %s\n", address.c_str(), error->message);
        g_error_free(error);
        g_free(guid);
        throw SystemException(INIT_DBUS_SERVER_FAIL);
    }

    g_dbus_server_start(server);
    g_free(guid);
    dbusServer = server;

    // 监听 new-connection 连接信号
    g_signal_connect(dbusServer, "new-connection", G_CALLBACK(onNewConnection), this);
    PRINT_INFO("Server is listening at: %s\n", g_dbus_server_get_client_address(server));
}

void BusinessServer::notifyAllConnections(const std::string& signalName, const std::string& input) {
    // 通知当前已有的连接
    std::string busName = "com.kylin.AiBusiness." + serviceName;
    std::string objectPath = "/com/kylin/AiBusiness/" + serviceName;
    GVariant* result;
    gboolean success;
    for (const auto& item : connectionProcessorMap) {
        result = g_variant_new("(s)", input.c_str());
        success = g_dbus_connection_emit_signal(item.first, busName.c_str(), objectPath.c_str(), busName.c_str(),
                                                signalName.c_str(), result, nullptr);
        if (!success) {
            PRINT_ERROR("notifyAllConnections emit signal failed, %s, %s, %s.\n", busName.c_str(), objectPath.c_str(),
                        signalName.c_str());
        }
    }
    PRINT_INFO("notifyAllConnections %zu connections, input: %s.\n", connectionProcessorMap.size(), input.c_str());
}

BusinessServer::~BusinessServer() {
    if (!connectionProcessorMap.empty()) {
        connectionProcessorMap.clear();
    }

    if (dbusServer == nullptr) {
        return;
    }
    g_dbus_server_stop(dbusServer);
    g_object_unref(dbusServer);
}

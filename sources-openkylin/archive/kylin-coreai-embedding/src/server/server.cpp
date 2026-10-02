/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "server.h"

#include <unistd.h>

#include <chrono>
#include <thread>

#include "logger.h"

namespace kyai::coreai::embedding {

Server::Server(const std::string &serverPath, const std::string &objectPath)
    : serverPath_(serverPath),
      objectPath_(objectPath),
      connection_(nullptr),
      proxy_(nullptr) {}

Server::~Server() { destroy(); }

GDBusConnection *Server::getConnection() {
    std::lock_guard<std::mutex> lock(connectionMutex_);
    if (isConnectionValid()) {
        return connection_;
    }

    // 获取连接 或获取连接有问题时，尝试重连
    if (ensureConnectionWithRetry(10, 1000) == AI_COMMON_SUCCESS) {
        return connection_;
    }

    return nullptr;
}

bool Server::isConnectionValid() const {
    if (connection_ == nullptr) {
        return false;
    }

    // 检查连接是否仍然有效
    if (g_dbus_connection_is_closed(connection_)) {
        return false;
    }

    return true;
}

int Server::reconnect() {
    std::lock_guard<std::mutex> lock(connectionMutex_);
    return doConnect();
}

int Server::doConnect() {
    // 清理旧连接 重新获取可用连接
    destroy();

    GError *error = nullptr;
    connection_ = g_dbus_connection_new_for_address_sync(
        serverPath_.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
        nullptr, /* GDBusAuthObserver */
        nullptr, /* GCancellable */
        &error);

    if (connection_ == nullptr) {
        if (error) {
            LOGE("Error: connecting to D-Bus address ", serverPath_,
                 " failed, code: ", error->code, ", msg: ", error->message);
            g_error_free(error);
            error = nullptr;
        } else {
            LOGE("Error: connecting to D-Bus address ", serverPath_,
                 " failed.");
        }
        return COREAI_EMBEDDING_CONNECTION_ERROR;
    }

    error = nullptr;
    proxy_ = createProxy(connection_, &error);
    if (proxy_ == nullptr) {
        if (error) {
            LOGE("Error: creating proxy ", objectPath_,
                 " failed, code: ", error->code, ", msg: ", error->message);
            g_error_free(error);
            error = nullptr;
        } else {
            LOGE("Error: creating proxy ", objectPath_, " failed.");
        }
        destroy();
        return COREAI_EMBEDDING_CONNECTION_ERROR;
    }

    // 刷新协议版本
    refreshProtocolVersion(proxy_);
    LOGI("Connection established successfully to ", serverPath_);
    return AI_COMMON_SUCCESS;
}

int Server::ensureConnectionWithRetry(int maxRetries, int retryIntervalMs) {
    for (int i = 0; i < maxRetries; ++i) {
        int result = doConnect();
        if (result == AI_COMMON_SUCCESS) {
            // 连接成功 return
            return AI_COMMON_SUCCESS;
        }

        // 如果不是最后一次尝试，等待后重试
        if (i < maxRetries - 1) {
            LOGI("Connection attempt ", i + 1, " failed, retrying in ",
                 retryIntervalMs, "ms...");
            std::this_thread::sleep_for(
                std::chrono::milliseconds(retryIntervalMs));
        }
    }

    LOGE("Failed to establish connection after ", maxRetries, " attempts");
    return COREAI_EMBEDDING_CONNECTION_ERROR;
}

void Server::destroy() {
    if (proxy_) {
        g_object_unref(proxy_);
        proxy_ = nullptr;
    }

    if (connection_) {
        g_object_unref(connection_);
        connection_ = nullptr;
    }
}

GDBusProxy *Server::getProxy() { return proxy_; }

}  // namespace kyai::coreai::embedding

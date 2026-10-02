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

#include "abstractserver.h"

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <mutex>

#include "utils/logger.h"

#define UNIX_PATH_MODE 0777

const std::string AbstractServer::socketFilePath_ = "/tmp/.kylin-ai-runtime-unix/" + std::to_string(getuid()) + "/";

AbstractServer::AbstractServer(const std::string &filePath) : filePath_(filePath) { checkAndCreateSocketFilePath(); }

AbstractServer::~AbstractServer() { stopServer(); }
void AbstractServer::startServer() {
    GMainContext *context = g_main_context_new();
    g_main_context_push_thread_default(context);

    auto *guid = g_dbus_generate_guid();
    GError *error = nullptr;

    std::string unixPath = "unix:path=" + filePath_;
    server_ = g_dbus_server_new_sync(unixPath.c_str(),         /* address */
                                     G_DBUS_SERVER_FLAGS_NONE, /* flags */
                                     guid,                     /* guid */
                                     nullptr,                  /* GDBusAuthObserver */
                                     nullptr,                  /* GCancellable */
                                     &error);
    chmod(socketFilePath_.c_str(), UNIX_PATH_MODE);

    if (error) {
        g_printerr("Error creating server at address %s: %s\n", unixPath.c_str(), error->message);
        LOGE("Error creating server at address {}: {}.", unixPath, error->message);
        g_error_free(error);
        g_main_context_pop_thread_default(context);
        g_main_context_unref(context);
        return;
    }

    g_signal_connect(server_, "new-connection", G_CALLBACK(onNewConnection), this);

    g_dbus_server_start(server_);

    GMainLoop *loop = g_main_loop_new(context, FALSE);
    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    g_object_unref(server_);
    g_free(guid);
    g_main_context_pop_thread_default(context);
    g_main_context_unref(context);
}

void AbstractServer::stopServer() {
    if (server_ == nullptr) {
        return;
    }

    disconnectSignalHandlers();

    g_dbus_server_stop(server_);
}

void AbstractServer::checkAndCreateSocketFilePath() {
    // Always try to remove stale socket for this specific server path
    unlink(filePath_.c_str());

    static std::once_flag baseDirOnce;
    std::call_once(baseDirOnce, [&]() { doCheckAndCreateSocketFilePath(); });
}

void AbstractServer::doCheckAndCreateSocketFilePath() {
    const char *socketPath = socketFilePath_.c_str();
    int len = strlen(socketPath);

    for (int i = 0; i < len; ++i) {
        if (socketPath[i] != '/') {
            continue;
        }

        std::string subdir(socketPath, i + 1);
        if (pathExist(subdir)) {
            continue;
        }

        int attempts = 0;
        while (attempts < 3) {
            errno = 0;
            int ret = mkdir(subdir.c_str(), UNIX_PATH_MODE);
            if (ret == 0) {
                break;
            }
            if (errno == EEXIST) {
                struct stat st;
                if (stat(subdir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                    break;
                }
                LOGE("Path exists but is not a directory: {} {} {}.", subdir, errno, strerror(errno));
                return;
            }
            if (errno == ENOENT) {
                usleep(1000 * 5);
                attempts++;
                continue;
            }
            LOGE("Failed to create server directory: {} {} {} {}.", ret, subdir, errno, strerror(errno));
            return;
        }

        if (int chmodRet = chmod(subdir.c_str(), UNIX_PATH_MODE) != 0) {
            LOGE("Failed to chmod server directory: {} {}.", chmodRet, subdir);
        }
    }

    if (!pathExist(socketFilePath_)) {
        LOGE("Socket base directory still missing after creation: {}.", socketFilePath_);
    } else {
        chmod(socketFilePath_.c_str(), UNIX_PATH_MODE);
    }
}

bool AbstractServer::pathExist(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    return false;
}

void AbstractServer::disconnectSignalHandlers() {
    std::lock_guard<std::mutex> lk(connectionsMutex_);
    isShuttingDown_ = true;
    for (auto &entry : connectionToHandlerId_) {
        GDBusConnection *conn = entry.first;
        gulong handlerId = entry.second;
        if (conn) {
            if (handlerId > 0) {
                g_signal_handler_disconnect(conn, handlerId);
            }
            g_object_unref(conn);
        }
    }
    connectionToHandlerId_.clear();
}

gboolean AbstractServer::onNewConnection(GDBusServer * /* server */, GDBusConnection *connection, gpointer userData) {
    auto *server = static_cast<AbstractServer *>(userData);

    server->handleNewConnection(connection);
    gulong handlerId = g_signal_connect(connection, "closed", G_CALLBACK(server->onConnectionClosed), server);
    {
        std::lock_guard<std::mutex> lk(server->connectionsMutex_);
        server->connectionToHandlerId_[connection] = handlerId;
    }
    return true;
}

void AbstractServer::onConnectionClosed(GDBusConnection *connection, gboolean /* remotePeerVanished */,
                                        GError * /* error */, gpointer userData) {
    auto *server = static_cast<AbstractServer *>(userData);

    if (server == nullptr || connection == nullptr) {
        LOGE("AbstractServer::onConnectionClosed: userData or connection is null.");
        return;
    }

    handleConnectionClosed(connection, server);
    g_dbus_connection_close(connection, nullptr, nullptr, nullptr);
    // Drop our strong ref added on new-connection
    g_object_unref(connection);
}

void AbstractServer::handleConnectionClosed(GDBusConnection *connection, AbstractServer *server) {
    gulong handlerId = 0;
    bool shuttingDown = false;
    {
        std::lock_guard<std::mutex> lk(server->connectionsMutex_);
        shuttingDown = server->isShuttingDown_;
        auto it = server->connectionToHandlerId_.find(connection);
        if (it != server->connectionToHandlerId_.end()) {
            handlerId = it->second;
            server->connectionToHandlerId_.erase(it);
        }
    }

    if (handlerId > 0) {
        g_signal_handler_disconnect(connection, handlerId);
    }

    // If shutting down, skip invoking virtuals to avoid touching torn-down objects
    if (!shuttingDown) {
        server->handleDisconnection(connection);
    } else {
        LOGI("Skipping invoking virtuals to avoid touching torn-down objects.");
    }
}

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

#ifndef SERVER_ABSTRACTSERVER_H
#define SERVER_ABSTRACTSERVER_H

#include <gio/gio.h>
#include <gio/giotypes.h>

#include <mutex>
#include <string>
#include <unordered_map>

class AbstractServer {
public:
    explicit AbstractServer(const std::string &filePath);
    virtual ~AbstractServer();

    void startServer();
    void stopServer();

protected:
    virtual void handleNewConnection(GDBusConnection *connection) = 0;
    virtual void handleDisconnection(GDBusConnection *connection) = 0;

private:
    void checkAndCreateSocketFilePath();
    void doCheckAndCreateSocketFilePath();
    bool pathExist(const std::string &path);
    void disconnectSignalHandlers();

    static gboolean onNewConnection(GDBusServer *server, GDBusConnection *connection, gpointer userData);
    static void onConnectionClosed(GDBusConnection *connection, gboolean remotePeerVanished, GError *Error,
                                   gpointer userData);
    static void handleConnectionClosed(GDBusConnection *connection, AbstractServer *server);

private:
    static const std::string socketFilePath_;

    const std::string filePath_;

    // Track connections and handler ids to safely disconnect during shutdown
    std::unordered_map<GDBusConnection *, gulong> connectionToHandlerId_;
    std::mutex connectionsMutex_;
    bool isShuttingDown_ = false;

    gulong closedHandlerId_ = 0;

    GDBusServer *server_ = nullptr;
};

#endif

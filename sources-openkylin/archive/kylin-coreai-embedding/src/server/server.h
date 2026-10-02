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

#ifndef SERVER_H
#define SERVER_H

#include <gio/gio.h>

#include <mutex>
#include <string>

#include "error.h"

namespace kyai::coreai::embedding {

class Server {
public:
    virtual ~Server();

    GDBusConnection *getConnection();

    int reconnect();

protected:
    Server(const std::string &serverPath, const std::string &objectPath);

    virtual GDBusProxy *createProxy(GDBusConnection *connection,
                                    GError **error) = 0;
    virtual void refreshProtocolVersion(GDBusProxy *proxy) = 0;

    GDBusProxy *getProxy();

    const std::string &getObjectPath() const { return objectPath_; }

private:
    bool isConnectionValid() const;
    int doConnect();
    int ensureConnectionWithRetry(int maxRetries, int retryIntervalMs);
    void destroy();

private:
    std::string serverPath_;
    std::string objectPath_;
    GDBusConnection *connection_;
    GDBusProxy *proxy_;  // dbus代理对象的基类
    std::mutex connectionMutex_;
};

}  // namespace kyai::coreai::embedding

#endif

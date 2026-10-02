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

#ifndef SERVER_COREVISIONSERVER_H
#define SERVER_COREVISIONSERVER_H

#include <memory>
#include <string>

#include "abstractserver.h"
#include "services/coreai/vision/corevisionservice.h"

class CoreVisionServer : public AbstractServer {
public:
    CoreVisionServer(EngineSelector &engineSelector);
    ~CoreVisionServer() override;

protected:
    void handleNewConnection(GDBusConnection *connection) override;
    void handleDisconnection(GDBusConnection *connection) override;

private:
    void checkFilePath();

private:
    static const std::string filePath_;
    static const std::string unixPath_;

    EngineSelector &engineSelector_;

    std::map<GDBusConnection *, std::shared_ptr<CoreVisionService> > visionServiceMap_;
};

#endif

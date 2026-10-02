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

#ifndef SERVER_ASSISTANTSERVER_H
#define SERVER_ASSISTANTSERVER_H

#include <string>

#include "abstractserver.h"
#include "assistant/assistantservice.h"
#include "engine/aienginepluginmanager.h"

class AssistantServer : public AbstractServer {
public:
    AssistantServer(ai_engine::AiEnginePluginManager &aiEnginePluginManager, EngineSelector &engineSelector);
    ~AssistantServer() override;

protected:
    void handleNewConnection(GDBusConnection *connection) override;
    void handleDisconnection(GDBusConnection *connection) override;

private:
    void checkFilePath();

private:
    static const std::string filePath_;

    ai_engine::AiEnginePluginManager &aiEnginePluginManager_;
    EngineSelector &engineSelector_;

    std::map<GDBusConnection *, std::shared_ptr<AssistantService> > assistantServiceMap_;
};

#endif

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

#include "assistantserver.h"

#include <sys/stat.h>

#include "utils/logger.h"

const std::string AssistantServer::filePath_ =
    "/tmp/.kylin-ai-runtime-unix/" + std::to_string(getuid()) + "/assistant.sock";

AssistantServer::AssistantServer(ai_engine::AiEnginePluginManager &aiEnginePluginManager,
                                 EngineSelector &engineSelector)
    : AbstractServer(filePath_), aiEnginePluginManager_(aiEnginePluginManager), engineSelector_(engineSelector) {}

AssistantServer::~AssistantServer() {}

void AssistantServer::handleNewConnection(GDBusConnection *connection) {
    LOGI("Assistant new connection");
    assistantServiceMap_.insert(
        {connection, std::make_unique<AssistantService>(*connection, aiEnginePluginManager_, engineSelector_)});
    g_object_ref(connection);
}

void AssistantServer::handleDisconnection(GDBusConnection *connection) {
    if (assistantServiceMap_.find(connection) != assistantServiceMap_.end()) {
        assistantServiceMap_[connection]->stop();
        assistantServiceMap_.erase(connection);
    }
}

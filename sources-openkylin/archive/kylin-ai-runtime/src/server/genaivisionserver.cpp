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

#include "genaivisionserver.h"

#include <sys/stat.h>

#include "utils/logger.h"

const std::string GenAiVisionServer::filePath_ =
    "/tmp/.kylin-ai-runtime-unix/" + std::to_string(getuid()) + "/genai-vision.sock";

GenAiVisionServer::GenAiVisionServer(EngineSelector &engineSelector)
    : AbstractServer(filePath_), engineSelector_(engineSelector) {}

GenAiVisionServer::~GenAiVisionServer() {}

void GenAiVisionServer::handleNewConnection(GDBusConnection *connection) {
    LOGI("Gen vision new connection");
    visionServiceMap_.insert({connection, std::make_unique<GenAiVisionService>(*connection, engineSelector_)});

    g_object_ref(connection);
}

void GenAiVisionServer::handleDisconnection(GDBusConnection *connection) {
    if (visionServiceMap_.find(connection) != visionServiceMap_.end()) {
        visionServiceMap_[connection]->stop();
        visionServiceMap_.erase(connection);
    }
}

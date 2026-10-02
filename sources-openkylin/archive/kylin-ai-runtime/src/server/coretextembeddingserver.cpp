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

#include "coretextembeddingserver.h"

#include <sys/stat.h>

#include "utils/logger.h"

const std::string CoreTextEmbeddingServer::filePath_ =
    "/tmp/.kylin-ai-runtime-unix/" + std::to_string(getuid()) + "/core-textembedding.sock";

CoreTextEmbeddingServer::CoreTextEmbeddingServer(EngineSelector &engineSelector)
    : AbstractServer(filePath_), engineSelector_(engineSelector) {}

CoreTextEmbeddingServer::~CoreTextEmbeddingServer() {}

void CoreTextEmbeddingServer::handleNewConnection(GDBusConnection *connection) {
    LOGI("Text Embedding new connection");
    textEmbeddingServiceMap_.insert(
        {connection, std::make_unique<CoreTextEmbeddingService>(*connection, engineSelector_)});

    g_object_ref(connection);
}

void CoreTextEmbeddingServer::handleDisconnection(GDBusConnection *connection) {
    if (textEmbeddingServiceMap_.find(connection) != textEmbeddingServiceMap_.end()) {
        textEmbeddingServiceMap_[connection]->stop();
        textEmbeddingServiceMap_.erase(connection);
    }
}

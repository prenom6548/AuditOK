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

#include "corevisionserver.h"

#include <sys/stat.h>

#include "utils/logger.h"

const std::string CoreVisionServer::filePath_ =
    "/tmp/.kylin-ai-runtime-unix/" + std ::to_string(getuid()) + "/core-vision.sock";

CoreVisionServer::CoreVisionServer(EngineSelector &engineSelector)
    : AbstractServer(filePath_), engineSelector_(engineSelector) {}

CoreVisionServer::~CoreVisionServer() {}

void CoreVisionServer::handleNewConnection(GDBusConnection *connection) {
    LOGI("Ocr new connection");
    visionServiceMap_.insert({connection, std::make_unique<CoreVisionService>(*connection, engineSelector_)});

    g_object_ref(connection);
}

void CoreVisionServer::handleDisconnection(GDBusConnection *connection) {
    LOGI("OCR handleDisconnection");
    if (visionServiceMap_.find(connection) != visionServiceMap_.end()) {
        visionServiceMap_[connection]->stop();
        visionServiceMap_.erase(connection);
    }
}

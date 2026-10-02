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

#include "configserver.h"

#include <sys/stat.h>

#include "modelconfig/configservice.h"
#include "utils/logger.h"

const std::string ConfigServer::filePath_ = "/tmp/.kylin-ai-runtime-unix/" + std::to_string(getuid()) + "/config.sock";
ConfigServer::ConfigServer(ModelConfigManager &modelConfigManager)
    : AbstractServer(filePath_), modelConfigManager_(modelConfigManager) {}

ConfigServer::~ConfigServer() {}

void ConfigServer::handleNewConnection(GDBusConnection *connection) {
    LOGI("Config new connection");

    configServiceMap_.insert({connection, std::make_unique<ConfigService>(*connection, modelConfigManager_)});

    g_object_ref(connection);
}

void ConfigServer::handleDisconnection(GDBusConnection *connection) {
    if (configServiceMap_.find(connection) != configServiceMap_.end()) {
        configServiceMap_[connection]->stop();
        configServiceMap_.erase(connection);
    }
}

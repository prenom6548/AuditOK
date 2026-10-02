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

#pragma once

#include "assistantserver.h"
#include "configserver.h"
#include "coreaispeechserver.h"
#include "coreimageembeddingserver.h"
#include "coretextembeddingserver.h"
#include "corevisionserver.h"
#include "engine/aienginepluginmanager.h"
#include "engineselector/engineselector.h"
#include "genainlpserver.h"
#include "genaivisionserver.h"
class ServerManager {
public:
    ServerManager(ai_engine::AiEnginePluginManager &pluginManager, EngineSelector &engineSelector,
                  ModelConfigManager &modelConfigManager);
    void startServers();

private:
    ai_engine::AiEnginePluginManager &pluginManager_;
    EngineSelector &engineSelector_;
    ModelConfigManager &modelConfigManager_;
};
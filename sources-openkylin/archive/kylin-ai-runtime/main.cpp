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

#include "assistant/intentdialoguemanager/systemoperation/startuptools.h"
#include "engine/aienginepluginmanager.h"
#include "engineselector/engineselector.h"
#ifdef SUPPORT_LIFE_CYCLE
#include "lifecycle/modelidlesweeper.h"
#include "lifecycle/modellifecyclemanager.h"
#include "lifecycle/powereventmonitor.h"
#endif
#include "server/servermanager.h"
#include "threadpool/async.h"

int main(int argc, char *argv[]) {
    try {
        logger::init();
        cpr::async::startup(8, 36);

        StartUpTools::getInstance().initConfig();
        ai_engine::AiEnginePluginManager &aiEnginePluginManager = ai_engine::AiEnginePluginManager::getInstance();
        ModelConfigManager &modelConfigManager = ModelConfigManager::getInstance();
#ifdef SUPPORT_LIFE_CYCLE
        lifecycle::ModelLifecycleManager &lifecycleManager = lifecycle::ModelLifecycleManager::getInstance();
        lifecycleManager.loadDefaultConfig();
        lifecycleManager.loadModelsOnBoot();
        lifecycleManager.registerModelConfigCallback();
        lifecycle::ModelIdleSweeper idleSweeper(lifecycleManager);
        idleSweeper.start();
        lifecycle::PowerEventMonitor powerMonitor(lifecycleManager);
        powerMonitor.start();
#endif
        EngineSelector engineSelector(aiEnginePluginManager, modelConfigManager);

        ServerManager serverManager(aiEnginePluginManager, engineSelector, modelConfigManager);

        serverManager.startServers();
    } catch (std::exception &exception) {
        LOGE("Exception: {}", exception.what());
    }
    return 0;
}

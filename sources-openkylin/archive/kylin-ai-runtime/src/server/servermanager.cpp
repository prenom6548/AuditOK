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

#include "servermanager.h"

#include <gio/gio.h>

#include <functional>
#include <future>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "utils/logger.h"
#include "utils/util.h"

ServerManager::ServerManager(ai_engine::AiEnginePluginManager &pluginManager, EngineSelector &engineSelector,
                             ModelConfigManager &modelConfigManager)
    : pluginManager_(pluginManager), engineSelector_(engineSelector), modelConfigManager_(modelConfigManager) {}

template <typename ServerType, typename... Args>
std::future<void> startServerAsync(const std::string &serverName, Args &&...args) {
    return std::async(std::launch::async, [serverName, args = std::make_tuple(std::forward<Args>(args)...)]() {
        try {
            LOGI("Starting {} server in thread: {}", serverName, util::thisThreadId());

            ServerType server =
                std::apply([](auto &&...args) { return ServerType(std::forward<decltype(args)>(args)...); }, args);

            server.startServer();
            LOGI("{} server event loop exited", serverName);
        } catch (const std::exception &e) {
            LOGE("{} server error: {}", serverName, e.what());
        } catch (...) {
            LOGE("{} server encountered an unknown error", serverName);
        }
    });
}

void ServerManager::startServers() {
    std::vector<std::future<void>> serverFutures;

    serverFutures.push_back(startServerAsync<ConfigServer>("Config", std::ref(modelConfigManager_)));

    serverFutures.push_back(
        startServerAsync<AssistantServer>("Assistant", std::ref(pluginManager_), std::ref(engineSelector_)));

    serverFutures.push_back(startServerAsync<GenAiNlpServer>("GenAiNlp", std::ref(engineSelector_)));

    serverFutures.push_back(startServerAsync<GenAiVisionServer>("GenAiVision", std::ref(engineSelector_)));

    serverFutures.push_back(startServerAsync<CoreAiSpeechServer>("CoreAiSpeech", std::ref(engineSelector_)));

    serverFutures.push_back(startServerAsync<CoreVisionServer>("CoreVision", std::ref(engineSelector_)));

    serverFutures.push_back(startServerAsync<CoreTextEmbeddingServer>("CoreTextEmbedding", std::ref(engineSelector_)));

    serverFutures.push_back(
        startServerAsync<CoreImageEmbeddingServer>("CoreImageEmbedding", std::ref(engineSelector_)));

    GMainLoop *mainLoop = g_main_loop_new(nullptr, false);
    if (!mainLoop) {
        LOGE("Failed to create main GMainLoop");
        return;
    }

    LOGI("All servers started, running main event loop");
    g_main_loop_run(mainLoop);
    g_main_loop_unref(mainLoop);

    // 等待所有服务器完成（通常不会到达这里）
    for (auto &future : serverFutures) {
        try {
            future.wait();
        } catch (const std::exception &e) {
            LOGE("Server error during shutdown: {}", e.what());
        }
    }

    LOGI("All servers stopped");
}

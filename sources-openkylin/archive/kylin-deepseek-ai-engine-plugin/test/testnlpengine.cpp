/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include <assert.h>
#include <jsoncpp/json/value.h>

#include <filesystem>
#include <iostream>

#include "deepseekengine.h"
#include "kylin-ai/ai-engine/cloudnlpengine.h"

std::string getConfig() {
    return R"(
        {
            "appId" : "",
            "apiKey" : "",
            "secretKey" : ""
        }
    )";
}
void testchatplugin() {
    namespace fs = std::filesystem;

    ai_engine::EngineError error;
    DeepseekAiEngine deepseekAiEngine;
    auto engine = deepseekAiEngine.createNlpEngine();
    if (engine == nullptr) {
        printf("engine is null");
        return;
    }
    static_cast<ai_engine::nlp::AbstractCloudNlpEngine *>(engine.get())
        ->setConfig(getConfig());
    engine->setContextSize(1);
    engine->initChatModule(error);

    engine->setChatResultCallback([](const ai_engine::nlp::ChatResult &result) {
        printf(result.text.c_str(), result.error.getCode(),
               result.error.getMessage());
    });
    const std::string message = R"(
        {
             "messages": [
                {
                    "role": "system",
                    "content": "你是一个小狗，喜欢汪汪汪"
                },
                {
                    "role": "user",
                    "content": "今天天气不错"
                }
            ]
        }
    )";
    engine->chat(message, error);

    while (std::getchar() != '\n') {
        engine->stopChat();
        engine->destroyChatModule(error);
    }
}
int main(int argc, char *argv[]) {
    testchatplugin();
    return 0;
}
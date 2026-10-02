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

#include "customengine.h"

std::string getConfig1() {
    return R"(
        {
            "appId" : "",
            "apiKey" : "",
            "secretKey" : "",
            "apiUrl" : "http://10.41.116.100:11434/api/chat",
            "modelName" : "深度求索",
            "modelVersion" : "deepseek-r1:1.5b"
        }
    )";
}
std::string getConfig2() {
    return R"(
        {
            "appId" : "",
            "apiKey" : "",
            "secretKey" : "",
            "apiUrl" : "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions",
            "modelName" : "千问",
            "modelVersion" : "qwen-plus"
        }
    )";
}
void testChatPlugin() {
    namespace fs = std::filesystem;

    ai_engine::EngineError error;
    CustomAiEngine customAiEngine;
    auto engine = customAiEngine.createTextGenerationEngine();
    if (engine == nullptr) {
        printf("engine is null");
        return;
    }
    static_cast<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine *>(
        engine.get())
        ->setConfig(getConfig1());
    engine->init();

    engine->setTextGenerationResultCallback(
        [](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
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
    engine->chat(message);

    while (std::getchar() != '\n') {
        engine->stopChat();
        engine->destroy();
    }
}
void testGenerateContentPlugin() {
    namespace fs = std::filesystem;

    ai_engine::EngineError error;
    CustomAiEngine customAiEngine;
    auto engine = customAiEngine.createTextGenerationEngine();
    if (engine == nullptr) {
        printf("engine is null");
        return;
    }
    static_cast<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine *>(
        engine.get())
        ->setConfig(getConfig2());

    engine->init();

    engine->setTextGenerationResultCallback(
        [](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
            printf(result.text.c_str(), result.error.getCode(),
                   result.error.getMessage());
        });
    const std::string message = R"(
        {
             "prompt": "你是谁"
    )";
    engine->generateContent(message);

    while (std::getchar() != '\n') {
        engine->stopGenerateContent();
        engine->destroy();
    }
}
int main(int argc, char *argv[]) {
    testChatPlugin();
    testGenerateContentPlugin();
    return 0;
}
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

#include <assert.h>
#include <iostream>
#include "baiduengine.h"

void testNlp() {
    BaiduAiEngine baiduAiEngine;
    auto engine = baiduAiEngine.createTextGenerationEngine();

    const std::string configJson =
        u8R"({"appId":"fakeAppId", "apiKey":"fakeApiKey", "secretKey":"fakeSecretKey"})";
    engine->setConfig(configJson);

    ai_engine::EngineError error;
    auto ret = engine->init();
    std::cout << "init result: " << error.getMessage() << std::endl;
    assert(ret.success == true);

    engine->setTextGenerationResultCallback(
        [](ai_engine::gen_ai::nlp::TextGenerationResult result) -> void {
            std::cout << "chat result text: " << result.text << std::endl;
            std::cout << "char result error message: "
                      << result.error.getMessage() << std::endl;
        });
    const std::string message = R"(
        {
             "messages": [
                {
                    "role": "assistant",
                    "content": "你是一个ai助手"
                },
                {
                    "role": "user",
                    "content": "今天天气怎么样"
                }
            ]
        }
    )";
    ret = engine->chat(message);
    std::cout << "chat result: " << error.getMessage() << std::endl;
    assert(ret.success == true);

    engine->destroy();
}

int main(int argc, char *argv[]) {
    testNlp();
    return 0;
}
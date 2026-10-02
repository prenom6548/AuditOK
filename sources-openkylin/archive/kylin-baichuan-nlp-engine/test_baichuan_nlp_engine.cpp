#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "baichuannlpengine.h"
#include "util.h"
void getModelInfo() {
    BaichuanNlpEngine baichuanNlpEngine;
    printf("engine name is %s", baichuanNlpEngine.engineName().c_str());
    printf("engine current model name is %s",
           baichuanNlpEngine.currentModel().c_str());
    printf("engine modelinfo is %s", baichuanNlpEngine.modelInfo().c_str());
}

std::string getConfig() {
    return R"(
        {
            "appId" : "",
            "apiKey" : "",
            "secretKey" : ""
        }
    )";
}

void testChatEngine() {
    namespace fs = std::filesystem;

    ai_engine::EngineError error;

    BaichuanNlpEngine baichuanNlpEngine;
    if (baichuanNlpEngine.isCloud()) {
        printf("有网络");
    }
    baichuanNlpEngine.setConfig(getConfig());
    auto result = baichuanNlpEngine.init();
    if (!result.success) {
        std::cout << result.error.getCode() << result.error.getMessage()
                  << std::endl;
    }

    baichuanNlpEngine.setTextGenerationResultCallback(
        [](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
            std::cout << result.modelName << std::endl;
            std::cout << result.text << std::endl;
            std::cout << result.error.getCode() << result.error.getMessage()
                      << std::endl;
        });
    const std::string message = R"(
        {
             "messages": [
                {
                    "role": "system",
                    "content": "你是一个AI助手 "
                },
                {
                    "role": "user",
                    "content": "小明出生于2017年8月15日 "
                },
                {
                    "role": "assistant",
                    "content": "好的，已记录小明出生于2017年8月15日 "
                },
                {
                    "role": "user",
                    "content": "小明明年8月18号几岁"
                }
            ]
        }
    )";
    result = baichuanNlpEngine.chat(message);

    if (!result.success) {
        std::cout << result.error.getCode() << result.error.getMessage()
                  << std::endl;
    }

    while (std::getchar() != '\n') {
    }
    const std::string message1 = R"(
        {
             "messages": [
                {
                    "role": "user",
                    "content": "我刚才问的什么 "
                }
            ]
        }
    )";
    baichuanNlpEngine.chat(message1);

    while (std::getchar() != '\n') {
        baichuanNlpEngine.stopChat();
        baichuanNlpEngine.destroy();
    }
}

void testChatGenerateContent() {
    namespace fs = std::filesystem;

    ai_engine::EngineError error;

    BaichuanNlpEngine baichuanNlpEngine;
    if (baichuanNlpEngine.isCloud()) {
        printf("有网络");
    }
    baichuanNlpEngine.setConfig(getConfig());
    baichuanNlpEngine.init();

    baichuanNlpEngine.setTextGenerationResultCallback(
        [](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
            std::cout << result.modelName << std::endl;
            std::cout << result.text << std::endl;
            std::cout << result.error.getCode() << result.error.getMessage()
                      << std::endl;
        });
    const std::string message = R"(
        {
             "prompt1": "你是谁",
             "prompt": "你是谁"
        }
    )";
    baichuanNlpEngine.generateContent(message);

    while (std::getchar() != '\n') {
        baichuanNlpEngine.stopGenerateContent();
        baichuanNlpEngine.destroy();
    }
}
int main() {
    getModelInfo();
    testChatEngine();
    testChatGenerateContent();
    return 0;
}
#include <filesystem>
#include <iostream>
#include <vector>

#include "sensetimenlpengine.h"
#include "util.h"
void getmodelinfo() {
    SensetimeNlpEngine sensetimeNlpEngine;
    printf("engine name is %s", sensetimeNlpEngine.engineName().c_str());
    printf("engine current model name is %s",
           sensetimeNlpEngine.currentModel().c_str());
    printf("engine modelinfo is %s", sensetimeNlpEngine.modelInfo().c_str());
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
    ai_engine::EngineError error;

    SensetimeNlpEngine sensetimeNlpEngine;
    if (sensetimeNlpEngine.isCloud()) {
        printf("有网络");
    }
    sensetimeNlpEngine.setConfig(getConfig());
    auto result = sensetimeNlpEngine.init();
    if (!result.success) {
        std::cout << result.error.getCode() << result.error.getMessage()
                  << std::endl;
    }
    sensetimeNlpEngine.setTextGenerationResultCallback(
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
                    "content": "你是一个作家，善于用优美的语言写文章"
                },
                {
                    "role": "user",
                    "content": "今天天气不错"
                }
            ]
        }
    )";
    result = sensetimeNlpEngine.chat(message);
    if (!result.success) {
        std::cout << result.error.getCode() << result.error.getMessage()
                  << std::endl;
    }

    const std::string message1 = R"(
        {
             "messages": [
                {
                    "role": "user",
                    "content": "我刚才问的什么"
                }
            ]
        }
    )";
    while (std::getchar() != '\n') {
    }
    sensetimeNlpEngine.chat(message1);
    while (std::getchar() != '\n') {
        sensetimeNlpEngine.stopChat();
        sensetimeNlpEngine.destroy();
    }
}
void testGenerateContent() {
    ai_engine::EngineError error;

    SensetimeNlpEngine sensetimeNlpEngine;
    if (sensetimeNlpEngine.isCloud()) {
        printf("有网络");
    }
    sensetimeNlpEngine.setConfig(getConfig());
    sensetimeNlpEngine.init();
    sensetimeNlpEngine.setTextGenerationResultCallback(
        [](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
            std::cout << result.modelName << std::endl;
            std::cout << result.text << std::endl;
            std::cout << result.error.getCode() << result.error.getMessage()
                      << std::endl;
        });
    const std::string prompt = R"(
        {
             "prompt": "你是谁"
        }
    )";
    sensetimeNlpEngine.generateContent(prompt);

    while (std::getchar() != '\n') {
        sensetimeNlpEngine.stopGenerateContent();
        sensetimeNlpEngine.destroy();
    }
}

int main() {
    getmodelinfo();
    testChatEngine();
    testGenerateContent();
    return 0;
}
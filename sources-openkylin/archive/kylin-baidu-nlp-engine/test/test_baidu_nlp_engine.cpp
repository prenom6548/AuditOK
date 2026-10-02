#include <filesystem>
#include <fstream>
#include <vector>

#include "baidunlpengine.h"
#include "logger.h"
#include "util.h"

void getModelInfo() {
    BaiduNlpEngine baiduNlpEngine;
    LOGD("Engine name is", baiduNlpEngine.engineName());
    LOGD("Engine current model name is", baiduNlpEngine.currentModel());
    LOGD("Engine model info is", baiduNlpEngine.modelInfo());
}

std::string getConfig() {
    return R"(
        {
            "apiKey" : "",
            "secretKey" : ""
        }
    )";
}

void testChat() {
    BaiduNlpEngine baiduNlpEngine;
    if (baiduNlpEngine.isCloud()) {
        LOGI("Baidu NLP engine is cloud.");
    }
    baiduNlpEngine.setConfig(getConfig());
    bool setResult = baiduNlpEngine.setCurrentModel("ernie-4.0-8k");
    if (setResult != true) {
        LOGE("Set current error.");
    }
    auto result = baiduNlpEngine.init();
    if (!result.success) {
        LOGE("Baidu NLP engine init error:", result.error.getCode(), result.error.getMessage());
        return;
    }

    baiduNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = baidu_nlp_util::stringToJson(result.text);
        printf("%s\n", json["result"].asString().c_str());
        if (json["is_end"].asBool()) {
            printf("\n");
        }
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
                    "content": "生成3000字的散文"
                }
            ],
            "top_k": 4,
            "stream": false
        }
    )";
    result = baiduNlpEngine.chat(message);
    if (result.success != true) {
        LOGE("Baidu NLP engine chat error:", result.error.getCode(), result.error.getMessage());
    }

    while (std::getchar() != '\n') {
    }

    baiduNlpEngine.stopChat();
    baiduNlpEngine.destroy();
}

void testChatGenerateContent() {
    BaiduNlpEngine baiduNlpEngine;
    if (baiduNlpEngine.isCloud()) {
        LOGI("Baidu NLP engine is cloud.");
    }
    baiduNlpEngine.setConfig(getConfig());
    auto result = baiduNlpEngine.init();
    if (!result.success) {
        LOGE("Baidu NLP engine init error:", result.error.getCode(), result.error.getMessage());
        return;
    }

    baiduNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = baidu_nlp_util::stringToJson(result.text);
        printf("%s\n", json["result"].asCString());
        if (json["is_end"].asBool()) {
            printf("\n");
        }
    });
    const std::string prompt = R"(
        {
             "prompt": "生成2000字文章",
             "stream": false
        }
    )";
    baiduNlpEngine.generateContent(prompt);

    while (std::getchar() != '\n') {
    }

    baiduNlpEngine.stopGenerateContent();
    baiduNlpEngine.destroy();
}

void testChatWithWebSearch() {
    BaiduNlpEngine baiduNlpEngine;
    if (baiduNlpEngine.isCloud()) {
        LOGI("Baidu NLP engine is cloud.");
    }
    baiduNlpEngine.setConfig(getConfig());
    bool setResult = baiduNlpEngine.setCurrentModel("ernie-4.0-8k");
    if (setResult != true) {
        LOGE("Set current error.");
    }
    auto result = baiduNlpEngine.init();
    if (!result.success) {
        LOGE("Baidu NLP engine init error:", result.error.getCode(), result.error.getMessage());
        return;
    }

    baiduNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = baidu_nlp_util::stringToJson(result.text);
        printf("%s\n", json["result"].asString().c_str());
        if (json.isMember("search_info") && json["search_info"].isArray()) {
            for (const auto &item : json["search_info"]) {
                printf("%s\n", baidu_nlp_util::jsonToString(item).c_str());
            }
        }
        if (json["is_end"].asBool()) {
            printf("\n");
        }
    });
    const std::string message = R"(
        {
             "messages":
             [
                {
                    "role": "system",
                    "content": "你是一个助手"
                },
                {
                    "role": "user",
                    "content": "今天天气怎么样？"
                }
            ],
            "top_k": 4,
            "stream": true,
            "web_search": true
        }
    )";
    result = baiduNlpEngine.chat(message);
    if (result.success != true) {
        LOGE("Baidu NLP engine chat error:", result.error.getCode(), result.error.getMessage());
    }

    while (std::getchar() != '\n') {
    }

    baiduNlpEngine.stopChat();
    baiduNlpEngine.destroy();
}

void testGenerateContentWithWebSearch() {
    BaiduNlpEngine baiduNlpEngine;
    if (baiduNlpEngine.isCloud()) {
        LOGI("Baidu NLP engine is cloud.");
    }
    baiduNlpEngine.setConfig(getConfig());
    bool setResult = baiduNlpEngine.setCurrentModel("ernie-4.0-8k");
    if (setResult != true) {
        LOGE("Set current error.");
    }
    auto result = baiduNlpEngine.init();
    if (!result.success) {
        LOGE("Baidu NLP engine init error:", result.error.getCode(), result.error.getMessage());
        return;
    }

    baiduNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = baidu_nlp_util::stringToJson(result.text);
        printf("%s\n", json["result"].asString().c_str());
        if (json.isMember("search_info") && json["search_info"].isArray()) {
            for (const auto &item : json["search_info"]) {
                printf("%s\n", baidu_nlp_util::jsonToString(item).c_str());
            }
        }
        if (json["is_end"].asBool()) {
            printf("\n");
        }
    });
    const std::string message = R"(
        {
            "prompt": "今天天气怎么样？",
            "stream": true,
            "web_search": true
        }
    )";
    result = baiduNlpEngine.generateContent(message);
    if (result.success != true) {
        LOGE("Baidu NLP engine chat error:", result.error.getCode(), result.error.getMessage());
    }

    while (std::getchar() != '\n') {
    }

    baiduNlpEngine.stopGenerateContent();
    baiduNlpEngine.destroy();
}

int main() {
    getModelInfo();
    testChat();
    testChatGenerateContent();
    testChatWithWebSearch();
    testGenerateContentWithWebSearch();
    return 0;
}
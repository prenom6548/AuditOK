#include <cstdio>
#include <filesystem>
#include <fstream>

#include "qwennlpengine.h"
#include "util.h"

void getModelInfo() {
    QwenNlpEngine qwenNlpEngine;
    printf("Engine name is %s\n", qwenNlpEngine.engineName().c_str());
    printf("Engine current model name is %s\n", qwenNlpEngine.currentModel().c_str());
    printf("Engine modelinfo is %s\n", qwenNlpEngine.modelInfo().c_str());
}

std::string getConfig() {
    return R"(
        {
            "apiKey" : ""
        }
    )";
}

void testChatEngine() {
    QwenNlpEngine qwenNlpEngine;
    if (qwenNlpEngine.isCloud()) {
        printf("Engine is cloud\n");
    }
    qwenNlpEngine.setConfig(getConfig());
    qwenNlpEngine.setCurrentModel("qwen-plus");
    auto result = qwenNlpEngine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    qwenNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        printf("%s\n", json["result"].asCString());
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
                    "content": "今天天气不错"
                }
            ],
            "top_k": 4,
            "stream": true
        }
    )";
    result = qwenNlpEngine.chat(message);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }
    while (std::getchar() != '\n') {
    }
    qwenNlpEngine.stopChat();
    qwenNlpEngine.destroy();
}

void testNonStreamChatEngine() {
    QwenNlpEngine qwenNlpEngine;
    if (qwenNlpEngine.isCloud()) {
        printf("Engine is cloud\n");
    }
    qwenNlpEngine.setConfig(getConfig());
    qwenNlpEngine.setCurrentModel("qwen-plus");
    auto result = qwenNlpEngine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    qwenNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        printf("%s\n", json["result"].asCString());
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
                    "content": "今天天气不错"
                }
            ],
            "top_k": 4,
            "stream": false
        }
    )";
    result = qwenNlpEngine.chat(message);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }
    while (std::getchar() != '\n') {
    }
    qwenNlpEngine.stopChat();
    qwenNlpEngine.destroy();
}

void testGenerateContentEngine() {
    QwenNlpEngine qwenNlpEngine;
    if (qwenNlpEngine.isCloud()) {
        printf("Engine is cloud\n");
    }
    qwenNlpEngine.setConfig(getConfig());
    auto result = qwenNlpEngine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    qwenNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        printf("%s\n", json["result"].asCString());
        if (json["is_end"].asBool()) {
            printf("\n");
        }
    });
    const std::string message = R"(
        {
             "prompt": "你是谁",
             "stream": false
        }
    )";
    result = qwenNlpEngine.generateContent(message);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    while (std::getchar() != '\n') {
    }
    qwenNlpEngine.stopGenerateContent();
    qwenNlpEngine.destroy();
}

void testChatWithWebSearch() {
    QwenNlpEngine engine;
    if (engine.isCloud()) {
        printf("NLP engine is cloud.\n");
    }
    engine.setConfig(getConfig());
    bool setResult = engine.setCurrentModel("qwen-plus");

    if (setResult != true) {
        printf("Set current error.\n");
    }
    auto result = engine.init();
    if (!result.success) {
        printf("NLP engine init error: %d, %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }
    engine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        printf("%s\n", json["reasoning_content"].asString().c_str());
        printf("%s\n", json["result"].asString().c_str());
        if (json.isMember("search_info") && json["search_info"].isArray()) {
            for (const auto &item : json["search_info"]) {
                printf("%s\n", qwen_nlp_util::jsonToString(item).c_str());
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
                    "content": "今天天津天气怎么样？"
                }
            ],
            "top_k": 4,
            "stream": true,
            "web_search": true
        }
    )";
    result = engine.chat(message);
    if (result.success != true) {
        printf("NLP engine chat error: %d, %s\n", result.error.getCode(), result.error.getMessage().c_str());
    }
    while (std::getchar() != '\n') {
    }
    engine.stopChat();
    engine.destroy();
}

void testGenerateContentWithWebSearch() {
    QwenNlpEngine engine;
    if (engine.isCloud()) {
        printf("NLP engine is cloud.");
    }
    engine.setConfig(getConfig());
    bool setResult = engine.setCurrentModel("qwen-plus");
    if (setResult != true) {
        printf("Set current error.");
    }
    auto result = engine.init();
    if (!result.success) {
        printf("NLP engine init error: %d, %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    engine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        printf("%s\n", json["result"].asString().c_str());
        if (json.isMember("search_info") && json["search_info"].isArray()) {
            for (const auto &item : json["search_info"]) {
                printf("%s\n", qwen_nlp_util::jsonToString(item).c_str());
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
    result = engine.generateContent(message);
    if (result.success != true) {
        printf("NLP engine chat error: %d, %s\n", result.error.getCode(), result.error.getMessage().c_str());
    }

    while (std::getchar() != '\n') {
    }

    engine.stopGenerateContent();
    engine.destroy();
}

void testChatEngineTopKError() {
    QwenNlpEngine qwenNlpEngine;
    if (qwenNlpEngine.isCloud()) {
        printf("Engine is cloud\n");
    }
    qwenNlpEngine.setConfig(getConfig());
    qwenNlpEngine.setCurrentModel("qwen-plus");
    auto result = qwenNlpEngine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    qwenNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        printf("%s\n", json["result"].asCString());
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
                    "content": "今天天气不错"
                }
            ],
            "top_k": -1,
            "stream": true
        }
    )";
    result = qwenNlpEngine.chat(message);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }
    while (std::getchar() != '\n') {
    }
    qwenNlpEngine.stopChat();
    qwenNlpEngine.destroy();
}

void testChatEngineEnableThinking() {
    QwenNlpEngine engine;
    if (engine.isCloud()) {
        printf("Engine is cloud\n");
    }
    engine.setConfig(getConfig());
    engine.setCurrentModel("qwen-plus");
    auto result = engine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    engine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        std::string reasoningContent;
        if (json.isMember("reasoning_content") && json["reasoning_content"].isString()) {
            reasoningContent = json["reasoning_content"].asString();
        }
        std::string resultContent;
        if (json.isMember("result") && json["result"].isString()) {
            resultContent = json["result"].asString();
        }
        if (!reasoningContent.empty()) {
            printf("深度思考：%s\n", reasoningContent.c_str());
        } else if (!resultContent.empty()) {
            printf("回    答：%s\n", resultContent.c_str());
        } else if (json.isMember("is_end") && json["is_end"].isBool() && json["is_end"].asBool()) {
            printf("\n");
        } else {
            printf("发生错误：%s\n", result.text.c_str());
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
                    "content": "今天天气不错"
                }
            ],
            "top_k": 4,
            "stream": true,
            "enable_thinking": true
        }
    )";
    result = engine.chat(message);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }
    while (std::getchar() != '\n') {
    }
    engine.stopChat();
    engine.destroy();
}

void testChatEngineEnableThinkingNonStream() {
    QwenNlpEngine engine;
    if (engine.isCloud()) {
        printf("Engine is cloud\n");
    }
    engine.setConfig(getConfig());
    engine.setCurrentModel("qwen-plus");
    auto result = engine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    engine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        std::string reasoningContent;
        if (json.isMember("reasoning_content") && json["reasoning_content"].isString()) {
            reasoningContent = json["reasoning_content"].asString();
        }
        std::string resultContent;
        if (json.isMember("result") && json["result"].isString()) {
            resultContent = json["result"].asString();
        }
        if (!reasoningContent.empty()) {
            printf("深度思考：%s\n", reasoningContent.c_str());
        }
        if (!resultContent.empty()) {
            printf("回    答：%s\n", resultContent.c_str());
        }
        if (json.isMember("is_end") && json["is_end"].isBool() && json["is_end"].asBool()) {
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
                    "content": "今天天气不错"
                }
            ],
            "top_k": 4,
            "stream": false,
            "enable_thinking": true
        }
    )";
    result = engine.chat(message);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }
    while (std::getchar() != '\n') {
    }
    engine.stopChat();
    engine.destroy();
}

void testGenerateContentEngineEnableThinking() {
    QwenNlpEngine engine;
    if (engine.isCloud()) {
        printf("Engine is cloud\n");
    }
    engine.setConfig(getConfig());
    engine.setCurrentModel("qwen-plus");
    auto result = engine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    engine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        std::string reasoningContent;
        if (json.isMember("reasoning_content") && json["reasoning_content"].isString()) {
            reasoningContent = json["reasoning_content"].asString();
        }
        std::string resultContent;
        if (json.isMember("result") && json["result"].isString()) {
            resultContent = json["result"].asString();
        }
        if (!reasoningContent.empty()) {
            printf("深度思考：%s\n", reasoningContent.c_str());
        } else if (!resultContent.empty()) {
            printf("回    答：%s\n", resultContent.c_str());
        } else if (json.isMember("is_end") && json["is_end"].isBool() && json["is_end"].asBool()) {
            printf("\n");
        } else {
            printf("发生错误：%s\n", result.text.c_str());
        }
    });
    const std::string message = R"(
        {
            "prompt": "你是一个作家，善于用优美的语言写文章，今天天气不错",
            "top_k": 4,
            "stream": true,
            "enable_thinking": true
        }
    )";
    result = engine.generateContent(message);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }
    while (std::getchar() != '\n') {
    }
    engine.stopGenerateContent();
    engine.destroy();
}

void testGenerateContentEngineEnableThinkingNonStream() {
    QwenNlpEngine engine;
    if (engine.isCloud()) {
        printf("Engine is cloud\n");
    }
    engine.setConfig(getConfig());
    engine.setCurrentModel("qwen-plus");
    auto result = engine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    engine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        std::string reasoningContent;
        if (json.isMember("reasoning_content") && json["reasoning_content"].isString()) {
            reasoningContent = json["reasoning_content"].asString();
        }
        std::string resultContent;
        if (json.isMember("result") && json["result"].isString()) {
            resultContent = json["result"].asString();
        }
        if (!reasoningContent.empty()) {
            printf("深度思考：%s\n", reasoningContent.c_str());
        }
        if (!resultContent.empty()) {
            printf("回    答：%s\n", resultContent.c_str());
        }
        if (json.isMember("is_end") && json["is_end"].isBool() && json["is_end"].asBool()) {
            printf("\n");
        }
    });
    const std::string message = R"(
        {
            "prompt": "你是一个作家，善于用优美的语言写文章，今天天气不错",
            "top_k": 4,
            "stream": false,
            "enable_thinking": true
        }
    )";
    result = engine.generateContent(message);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }
    while (std::getchar() != '\n') {
    }
    engine.stopGenerateContent();
    engine.destroy();
}

void testMultiToolCallScenario() {
    QwenNlpEngine qwenNlpEngine;
    if (qwenNlpEngine.isCloud()) {
        printf("Engine is cloud\n");
    }
    qwenNlpEngine.setConfig(getConfig());
    qwenNlpEngine.setCurrentModel("qwen-plus");
    auto result = qwenNlpEngine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    qwenNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = qwen_nlp_util::stringToJson(result.text).value();
        printf("%s\n", result.text.c_str());
    });
    const std::string message = R"(
        {
             "messages": [
                {
                    "role": "system",
                    "content": "你是一个助手，当用户提问时，请使用工具进行回答"
                },
                {
                    "role": "user",
                    "content": "我想知道我所在位置，以及北京的天气"
                }
            ],
            "top_k": 4,
            "stream": true,
            "tool_choice": {
                "type": "function",
                "function": {"name":"get_current_weather"}
},
            "tools": [
                {
                    "name": "get_current_weather",
                    "description": "Get the current weather",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "location": {
                                "type": "string",
                                "description": "The city and state, e.g. San Francisco, CA"
                            },
                            "unit": {
                                "type": "string",
                                "enum": ["celsius", "fahrenheit"]
                            }
                        },
                        "required": ["location", "unit"],
                        "additionalProperties": false
                    }
                },
                {
                    "name": "get_current_location",
                    "description": "Get the current location",
                    "parameters": {
                        "type": "object",
                        "properties": {
                        },
                        "additionalProperties": false
                    }
                }
            ]
        }
    )";

    result = qwenNlpEngine.chat(message);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    while (std::getchar() != '\n') {
    }

    const std::string message1 = R"(
{
    "messages": [
		{
			"content" : "今天北京的天气怎么样？",
			"role" : "user"
		},
		{
			"role" : "assistant",
            "content": "",
			"tool_calls" :
			[
				{
					"function" :
					{
						"arguments" : "{\"location\": \"北京, 中国\", \"unit\": \"celsius\"}",
						"name" : "get_current_weather"
					},
                    "id" : "call_b42f1eef59aa42728d2a2d",
					"type" : "function"
				}
			]
		},
		{
			"content" : "25°C",
            "id" : "call_b42f1eef59aa42728d2a2d",
			"role" : "tool"
		}
	],
	"stream" : true,
	"tool_choice" : "auto",
	"tools" :
	[
		{
			"description" : "Get the current weather",
			"name" : "get_current_weather",
			"parameters" :
			{
				"additionalProperties" : false,
				"properties" :
				{
					"location" :
					{
						"description" : "The city and state, e.g. San Francisco, CA",
						"type" : "string"
					},
					"unit" :
					{
						"enum" :
						[
							"celsius",
							"fahrenheit"
						],
						"type" : "string"
					}
				},
				"required" :
				[
					"location"
				],
				"type" : "object"
			}
		}
	],
    "top_k": 4,
    "stream": true,
    "tool_choice": "auto"
}
)";

    result = qwenNlpEngine.chat(message1);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    while (std::getchar() != '\n') {
    }

    qwenNlpEngine.stopChat();
    qwenNlpEngine.destroy();
}

int main() {
    getModelInfo();
    testChatEngine();
    testNonStreamChatEngine();
    testGenerateContentEngine();
    testChatWithWebSearch();
    testGenerateContentWithWebSearch();
    testChatEngineTopKError();
    testChatEngineEnableThinking();
    testChatEngineEnableThinkingNonStream();
    testGenerateContentEngineEnableThinking();
    testGenerateContentEngineEnableThinkingNonStream();
    testMultiToolCallScenario();
    return 0;
}
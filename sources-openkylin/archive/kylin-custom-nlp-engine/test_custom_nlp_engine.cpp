#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "customnlpengine.h"
#include "util.h"
void getmodelinfo() {
    CustomNlpEngine customNlpEngine;
    printf("engine name is %s \n", customNlpEngine.engineName().c_str());
    printf("engine current model name is %s\n", customNlpEngine.currentModel().c_str());
    printf("engine modelinfo is %s\n", customNlpEngine.modelInfo().c_str());
}

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
            "apiKey" : "sk-cd05ca96ede445859997408862c80ed6",
            "secretKey" : "",
            "apiUrl" : "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions",
            "modelName" : "千问",
            "modelVersion" : "qwen-plus"
        }
    )";
}

std::string getConfig3() {
    return R"(
        {
           "apiKey" : "",
           "apiUrl" : "https://api.lkeap.cloud.tencent.com/v1/chat/completions",
            "modelName" : "千问",
           "modelVersion" : "deepseek-r1"
        }
    )";
}

std::string getConfig4() {
    return R"(
        {
           "apiKey" : "",
           "apiUrl" : "http://www.baidu.com",
            "modelName" : "",
           "modelVersion" : ""
        }
    )";
}
std::string getConfig5() {
    return R"(
        {
           "apiKey" : "",
           "apiUrl" : "http://10.41.160.151:11434/api/chat",
            "modelName" : "",
           "modelVersion" : "DeepSeek-R1-Distill-Qwen-14B"
        }
    )";
}

void testchatengine() {
    namespace fs = std::filesystem;

    ai_engine::EngineError error;

    CustomNlpEngine customNlpEngine;
    if (customNlpEngine.isCloud()) {
        printf("有网络\n");
    }
    customNlpEngine.setConfig(getConfig2());
    auto result = customNlpEngine.init();
    if (!result.success) {
        std::cout << result.error.getCode() << result.error.getMessage() << std::endl;
    }

    customNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        std::cout << result.modelName << std::endl;
        std::cout << result.text << std::endl;
        std::cout << result.error.getCode() << result.error.getMessage() << std::endl;
    });
    const std::string message = R"(
        {
             "messages": [
                {
                    "role": "system",
                    "content": "你是一个能够理解自然语言并生成智能回复的助手，名为麒麟AI助手。"
                },
                {
                    "role": "user",
                    "content": "你好"
                }
            ]
        }
    )";
    result = customNlpEngine.chat(message);

    if (!result.success) {
        std::cout << result.error.getCode() << result.error.getMessage() << std::endl;
    }

    while (std::getchar() != '\n') {
        customNlpEngine.stopChat();
        customNlpEngine.destroy();
    }
}

void testGenerateContentEngine() {
    ai_engine::EngineError error;

    CustomNlpEngine customNlpEngine;
    if (customNlpEngine.isCloud()) {
        printf("有网络");
    }
    customNlpEngine.setConfig(getConfig1());
    customNlpEngine.init();

    customNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        std::cout << result.modelName << std::endl;
        std::cout << result.text << std::endl;
        std::cout << result.error.getCode() << result.error.getMessage() << std::endl;
    });
    const std::string message = R"(
        {
             "prompt": "你是谁"
    )";
    customNlpEngine.generateContent(message);

    while (std::getchar() != '\n') {
        customNlpEngine.stopGenerateContent();
        customNlpEngine.destroy();
    }
}

void testMultiToolCallScenario() {
    CustomNlpEngine customNlpEngine;
    if (customNlpEngine.isCloud()) {
        printf("Engine is cloud\n");
    }
    customNlpEngine.setConfig(getConfig3());
    auto result = customNlpEngine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    customNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        std::cout << result.modelName << std::endl;
        std::cout << result.text << std::endl;
        std::cout << result.error.getCode() << result.error.getMessage() << std::endl;
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
            "tool_choice": "auto",
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
                        "required": ["location"],
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

    result = customNlpEngine.chat(message);
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
					"type" : "function",
                    "id" : "call_00_V4mgEC7PLdrylx0hXWvYuTRL"
				}
			]
		},
		{
			"content" : "25°C",
			"role" : "tool",
            "id" : "call_00_V4mgEC7PLdrylx0hXWvYuTRL"
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

    result = customNlpEngine.chat(message1);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    while (std::getchar() != '\n') {
    }

    customNlpEngine.stopChat();
    customNlpEngine.destroy();
}

int main() {
    getmodelinfo();
    testMultiToolCallScenario();
    testchatengine();
    testGenerateContentEngine();
    return 0;
}
#include <filesystem>
#include <fstream>

#include "deepseeknlpengine.h"
#include "util.h"

void getModelInfo() {
    DeepseekNlpEngine deepseekNlpEngine;
    printf("Engine name is %s \n", deepseekNlpEngine.engineName().c_str());
    printf("Engine current model name is %s\n", deepseekNlpEngine.currentModel().c_str());
    printf("Engine modelinfo is %s\n", deepseekNlpEngine.modelInfo().c_str());
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
    DeepseekNlpEngine deepseekNlpEngine;
    if (deepseekNlpEngine.isCloud()) {
        printf("云端模型\n");
    }
    deepseekNlpEngine.setConfig(getConfig());
    deepseekNlpEngine.setCurrentModel("deepseek-chat");
    auto result = deepseekNlpEngine.init();
    if (!result.success) {
        printf("Deepseek init error: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    deepseekNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        printf("Result model name: %s\n", result.modelName.c_str());
        printf("Result text: %s\n", result.text.c_str());
        printf("Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
    });
    const std::string message = R"(
        {
             "messages": [
                {
                    "role": "system",
                    "content": " "
                },
                {
                    "role": "user",
                    "content": "你是谁"
                }
            ]
        }
    )";
    result = deepseekNlpEngine.chat(message);

    if (!result.success) {
        printf("Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
    }

    while (std::getchar() != '\n') {
    }

    deepseekNlpEngine.stopChat();
    deepseekNlpEngine.destroy();
}

void testGenerateContentEngine() {
    DeepseekNlpEngine deepseekNlpEngine;
    if (deepseekNlpEngine.isCloud()) {
        printf("云端模型\n");
    }
    deepseekNlpEngine.setConfig(getConfig());
    deepseekNlpEngine.setCurrentModel("deepseek-reasoner");
    auto result = deepseekNlpEngine.init();
    if (!result.success) {
        printf("Deepseek init error: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    deepseekNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        printf("Result model name: %s\n", result.modelName.c_str());
        printf("Result text: %s\n", result.text.c_str());
        printf("Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
    });
    const std::string message = R"(
        {
             "prompt": "你是谁"
        }
    )";
    result = deepseekNlpEngine.generateContent(message);
    if (!result.success) {
        printf("Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    while (std::getchar() != '\n') {
    }
    deepseekNlpEngine.stopGenerateContent();
    deepseekNlpEngine.destroy();
}

void testMultiToolCallScenario() {
    DeepseekNlpEngine deepseekNlpEngine;
    if (deepseekNlpEngine.isCloud()) {
        printf("Engine is cloud\n");
    }
    deepseekNlpEngine.setConfig(getConfig());
    deepseekNlpEngine.setCurrentModel("deepseek-chat");
    auto result = deepseekNlpEngine.init();
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    deepseekNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        printf("Result model name: %s\n", result.modelName.c_str());
        printf("Result text: %s\n", result.text.c_str());
        printf("Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
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
                    "content": "我想知道我所在的位置，以及今天北京的天气怎么样"
                }
            ],
            "top_k": 4,
            "stream": true,
            "tool_choice": {
"type": "function",
"function": {
"name": "get_current_weather"
}
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

    result = deepseekNlpEngine.chat(message);
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
			"content" : "我想知道我所在的位置以及天气？",
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
				        "arguments" : "{}",
				        "name" : "get_current_location"
			        },
			        "id" : "call_00_bYw2OAGio9irYstDUeuNfq2b",
			        "index" : 0,
			        "type" : "function"
		        },
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
			"content" : "北京",
			"role" : "tool",
            "id" : "call_00_bYw2OAGio9irYstDUeuNfq2b"
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

    result = deepseekNlpEngine.chat(message1);
    if (!result.success) {
        fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    while (std::getchar() != '\n') {
    }

    deepseekNlpEngine.stopChat();
    deepseekNlpEngine.destroy();
}

int main() {
    getModelInfo();
    testMultiToolCallScenario();
    testChatEngine();
    testGenerateContentEngine();
    return 0;
}
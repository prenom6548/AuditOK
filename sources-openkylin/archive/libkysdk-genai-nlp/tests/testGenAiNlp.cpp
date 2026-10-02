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

#include <gio/gio.h>
#include <gio/giotypes.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>
#include <toolchoice.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include "chat.h"
#include "config.h"
#include "error.h"
#include "logger.h"
#include "result.h"

using namespace std::chrono_literals;

std::string getMessage(const char *src) {
    if (src == nullptr) {
        return "nullptr";
    }
    return src;
}

void callback(ChatResult *result, void *user_data) {
    auto getBool = [](bool value) { return value ? "true" : "false"; };
    LOGI("reasoning message is ", chat_result_get_reasoning_message(result));
    LOGI("assistant message is ", chat_result_get_assistant_message(result));
    LOGI("finish reason message is ", chat_result_get_finish_reason_message(result));
    if (chat_result_get_error_code(result)) {
        LOGI("error code is ", chat_result_get_error_code(result));
        LOGI("error message is ", chat_result_get_error_message(result));
    }
    LOGI("is_end is ", getBool(chat_result_get_is_end(result)));

    if (user_data != nullptr) {
        int *a = static_cast<int *>(user_data);
        LOGI(*a);
    }
}

void toolCallCallback(const char *toolsJsonString, void *user_data) {
    auto session = static_cast<GenAiTextSession *>(user_data);
    // 创建 JSON 解析器
    Json::CharReaderBuilder readerBuilder;
    Json::Value root;
    std::string errs;

    // 解析 JSON 字符串
    std::istringstream jsonStream(toolsJsonString);
    if (!Json::parseFromStream(readerBuilder, jsonStream, &root, &errs)) {
        LOGE("JSON 解析失败: ", errs);
        return;
    }
    Json::Value toolCallsJson, resultJson;

    for (const auto &toolCall : root) {
        std::string toolName = toolCall["tool_name"].asString();
        std::string callId = toolCall["call_id"].asString();
        std::string argumentsJson = toolCall["arguments_json"].toStyledString();

        // 输出解析结果
        LOGI("tool name is ", toolName);
        LOGI("call id is ", callId);
        LOGI("arguments json is ", argumentsJson);

        // 解析tool_calls
        Json::Value toolCallJson;
        toolCallJson["type"] = "function";
        toolCallJson["function"]["name"] = toolName;
        toolCallJson["function"]["arguments"] = toolCall["arguments_json"];
        toolCallJson["id"] = callId;
        toolCallsJson.append(toolCallJson);

        // 调用工具并添加结果
        Json::Value toolJson;
        if (toolName == "get_current_weather") {
            toolJson["content"] = "25°C";  // 需要替换为工具实际输出
            toolJson["id"] = callId;
            resultJson.append(toolJson);
        }
        if (toolName == "get_current_location") {
            toolJson["content"] = "北京";  // 需要替换为工具实际输出
            toolJson["id"] = callId;
            resultJson.append(toolJson);
        }
    }

    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    // 回传tool_calls结果
    std::string toolCallsString = Json::writeString(builder, toolCallsJson);
    // 回传工具调用后的结果
    std::string resultString = Json::writeString(builder, resultJson);

    genai_text_submit_tool_result(session, toolCallsString.c_str(), resultString.c_str());
}

void test01_Chat() {
    ChatModelConfig *config = chat_model_config_create();
    chat_model_config_set_name(config, "百度-ERNIE-Bot-4");
    chat_model_config_set_top_k(config, 0.5);
    chat_model_config_set_deploy_type(config, ModelDeployType::PublicCloud);

    GenAiTextSession *session = genai_text_create_session();
    genai_text_set_model_config(session, config);

    int errorCode = genai_text_init_session(session);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("初始化错误：错误码", errorCode);
        return;
    }

    int a = 100;
    genai_text_result_set_callback(session, callback, &a);
    genai_text_chat_async(session, "一加一等于几");

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, pMainLoop] {
        while (std::getchar() != '\n') {
        }

        genai_text_chat_async(session, "你说的不对");

        while (std::getchar() != '\n') {
        }
        genai_text_stop_chat(session);
        genai_text_destroy_session(&session);
        chat_model_config_destroy(&config);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test02_ChatHistoryMessages() {
    ChatModelConfig *config = chat_model_config_create();
    chat_model_config_set_name(config, "");
    chat_model_config_set_top_k(config, 1.25);
    chat_model_config_set_deploy_type(config, ModelDeployType::PublicCloud);

    GenAiTextSession *session = genai_text_create_session();
    genai_text_set_model_config(session, config);
    int errorCode = genai_text_init_session(session);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("初始化错误：错误码", errorCode);
        return;
    }
    genai_text_result_set_callback(session, callback, nullptr);

    ChatMessage *chatMessage = chat_message_create();
    chat_message_add_system_message(chatMessage, "");
    chat_message_add_user_message(chatMessage, "一加一等于几");
    chat_message_add_assistant_message(chatMessage,
                                       "这是一个非常基础的数学问题，"
                                       "、涉及到的是加法运算。题目问"
                                       "的是1+1等于几。\n\n在数学中，"
                                       "加法是一种基本的运算方式，表"
                                       "示两个数量的和。当我们把两个1"
                                       "加在一起时，就是在计算这两个"
                                       "数量的总和。\n\n所以，1 + 1 ="
                                       " 2。\n\n因此，答案是2。这个问"
                                       "题非常直接，没有涉及到复杂的"
                                       "数学概念或技巧，只需要理解加"
                                       "法的基本定义即可。");
    chat_message_add_user_message(chatMessage, "你说的不对");

    genai_text_chat_with_history_messages_async(session, chatMessage);

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, &chatMessage, pMainLoop] {
        while (std::getchar() != '\n') {
        }
        chat_message_destroy(&chatMessage);
        genai_text_stop_chat(session);
        genai_text_destroy_session(&session);
        chat_model_config_destroy(&config);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test03_ClearChatMessage() {
    ChatModelConfig *config = chat_model_config_create();
    chat_model_config_set_name(config, "百度-ERNIE-Bot-4");
    chat_model_config_set_top_k(config, 1.25);
    chat_model_config_set_deploy_type(config, ModelDeployType::PublicCloud);

    GenAiTextSession *session = genai_text_create_session();
    genai_text_set_model_config(session, config);
    int errorCode = genai_text_init_session(session);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("初始化错误：错误码", errorCode);
        return;
    }
    genai_text_result_set_callback(session, callback, nullptr);
    genai_text_chat_async(session, "一加一等于几");

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, pMainLoop] {
        while (std::getchar() != '\n') {
        }

        genai_text_clear_chat_history_messages(session);

        while (std::getchar() != '\n') {
        }

        genai_text_chat_async(session, "你说的不对");

        while (std::getchar() != '\n') {
        }

        genai_text_stop_chat(session);
        genai_text_destroy_session(&session);
        chat_model_config_destroy(&config);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test04_SetModelConfig() {
    ChatModelConfig *config = chat_model_config_create();
    chat_model_config_set_name(config, "百度-ERNIE-Bot-4");
    chat_model_config_set_top_k(config, 1.25);
    chat_model_config_set_deploy_type(config, ModelDeployType::PublicCloud);

    GenAiTextSession *session = genai_text_create_session();
    genai_text_set_model_config(session, config);
    int errorCode = genai_text_init_session(session);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("初始化错误：错误码", errorCode);
        return;
    }
    genai_text_result_set_callback(session, callback, nullptr);
    genai_text_chat_async(session, "一加一等于几");

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, pMainLoop] {
        while (std::getchar() != '\n') {
        }
        genai_text_stop_chat(session);
        genai_text_destroy_session(&session);
        chat_model_config_destroy(&config);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test05_SetChatSystemPrompt() {
    GenAiTextSession *session = genai_text_create_session();
    genai_text_set_model_config(session, nullptr);
    int errorCode = genai_text_init_session(session);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("初始化错误：错误码", errorCode);
        return;
    }
    genai_text_result_set_callback(session, callback, nullptr);
    genai_text_set_chat_system_prompt(session, "汉英互译");
    genai_text_chat_async(session, "你是谁");

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, pMainLoop] {
        while (std::getchar() != '\n') {
        }
        genai_text_stop_chat(session);
        genai_text_destroy_session(&session);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test06_GenerateContent() {
    GenAiTextSession *session = genai_text_create_session();
    int errorCode = genai_text_init_session(session);
    genai_text_set_model_config(session, nullptr);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("初始化错误：错误码", errorCode);
        return;
    }
    genai_text_result_set_callback(session, callback, nullptr);
    genai_text_generate_content_async(session, "今天天气不错");

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, pMainLoop] {
        while (std::getchar() != '\n') {
        }
        genai_text_stop_chat(session);
        genai_text_destroy_session(&session);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void ChatSystemPromptId(PromptId promptId, const std::string &question) {
    GenAiTextSession *session = genai_text_create_session();
    genai_text_set_model_config(session, nullptr);
    int errorCode = genai_text_init_session(session);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("初始化错误：错误码", errorCode);
        return;
    }
    genai_text_result_set_callback(session, callback, nullptr);
    genai_text_set_chat_system_prompt_id(session, promptId);
    genai_text_chat_async(session, question.c_str());

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, pMainLoop] {
        while (std::getchar() != '\n') {
        }
        genai_text_stop_chat(session);
        genai_text_destroy_session(&session);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test07_ChatSystemPromptId_SUMMARY() {
    ChatSystemPromptId(SUMMARY,
                       "大模型是人工智能领域的热门研究方向。专家认为，人工智能进"
                       "入产业级大模型时代。大模型将是未来一段时间科技领域里面最重要的事情之一"
                       "。大模型将开启人工智能的“大一统时代”。");
}

void test08_ChatSystemPromptId_TEXT_EXPANSION() { ChatSystemPromptId(TEXT_EXPANSION, "今天天气不错"); }

void test09_ChatSystemPromptId_TRANSLATE_CHINESE_TO_ENGLISH() {
    ChatSystemPromptId(TRANSLATE_CHINESE_TO_ENGLISH, "今天天气不错");
}
void test10_Chat() {
    ChatModelConfig *config = chat_model_config_create();
    chat_model_config_set_name(config, "百度-ERNIE-Bot-4");
    chat_model_config_set_top_k(config, 0.5);
    chat_model_config_set_deploy_type(config, ModelDeployType::PublicCloud);

    GenAiTextSession *session = genai_text_create_session();
    genai_text_set_model_config(session, config);

    int errorCode = genai_text_init_session(session);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("初始化错误：错误码", errorCode);
        return;
    }

    int a = 100;
    genai_text_result_set_callback(session, callback, &a);
    genai_text_chat_async(session, "一加一等于几");

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, pMainLoop] {
        while (std::getchar() != '\n') {
        }

        genai_text_chat_async(session, "你说的不对");

        while (std::getchar() != '\n') {
        }
        genai_text_stop_chat(session);
        genai_text_destroy_session(&session);
        chat_model_config_destroy(&config);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test11_ToolCallHandler() {
    GenAiTextSession *session = genai_text_create_session();
    auto modelConfig = chat_model_config_create();
    chat_model_config_set_name(modelConfig, "通义千问-qwen-plus");
    genai_text_set_model_config(session, modelConfig);
    int errorCode = genai_text_init_session(session);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("初始化错误：错误码", errorCode);
        return;
    }
    genai_text_result_set_callback(session, callback, nullptr);

    std::map<std::string, std::string> tools = {
        {"get_current_weather", R"(
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
    }
    )"},
        {"get_current_location", R"(
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
    )"},
    };

    // 注册工具
    for (auto &[toolName, toolSchema] : tools) {
        int regResult =
            genai_text_register_tool(session, toolName.c_str(), GENAI_TEXT_TOOL_TYPE_FUNCTION, toolSchema.c_str());
        if (regResult != 0) {
            LOGE("注册工具失败，错误码: ", regResult);
        }
    }

    // 创建并设置工具选择配置
    auto config = genai_text_create_tool_choice_config(GENAI_TEXT_TOOL_CHOICE_REQUIRED);
    genai_text_tool_choice_config_set_specific_tool(config, "get_current_weather");
    genai_text_set_tool_choice_config(session, config);

    genai_text_destroy_tool_choice_config(&config);

    // 设置工具回调处理函数
    genai_text_set_tool_handler(session, toolCallCallback, session);

    // 发起聊天请求，可能会触发工具调用
    genai_text_chat_async(session, "我想知道我所在的位置，以及这里的天气");

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, pMainLoop] {
        while (std::getchar() != '\n') {
        }
        genai_text_stop_chat(session);
        genai_text_clear_tools(session);
        genai_text_destroy_session(&session);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test10_BigTest() {
    LOGI("一一一一一一一一一一一一一一一一一一一一一一一一一一一一一一");
    test10_Chat();
    LOGI("二二二二二二二二二二二二二二二二二二二二二二二二二二二二二二");
    test10_Chat();
    LOGI("三三三三三三三三三三三三三三三三三三三三三三三三三三三三三三");
    test10_Chat();
    LOGI("四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四");
    test10_Chat();
    LOGI("五五五五五五五五五五五五五五五五五五五五五五五五五五五五五五");
    test10_Chat();
    LOGI("六六六六六六六六六六六六六六六六六六六六六六六六六六六六六六");
    test10_Chat();
    LOGI("七七七七七七七七七七七七七七七七七七七七七七七七七七七七七七");
    test10_Chat();
    LOGI("八八八八八八八八八八八八八八八八八八八八八八八八八八八八八八");
    test10_Chat();
    LOGI("九九九九九九九九九九九九九九九九九九九九九九九九九九九九九九");
    test10_Chat();
    LOGI("十十十十十十十十十十十十十十十十十十十十十十十十十十十十十十");
    test10_Chat();
    LOGI("十一十一十一十一十一十一十一十一十一十一十一十一十一十一十一");
    test10_Chat();
    LOGI("十二十二十二十二十二十二十二十二十二十二十二十二十二十二十二");
    test10_Chat();
    LOGI("十三十三十三十三十三十三十三十三十三十三十三十三十三十三十三");
    test10_Chat();
    LOGI("十四十四十四十四十四十四十四十四十四十四十四十四十四十四十四");
    test10_Chat();
    LOGI("十五十五十五十五十五十五十五十五十五十五十五十五十五十五十五");
    test10_Chat();
    LOGI("十六十六十六十六十六十六十六十六十六十六十六十六十六十六十六");
    test10_Chat();
    LOGI("十七十七十七十七十七十七十七十七十七十七十七十七十七十七十七");
    test10_Chat();
    LOGI("十八十八十八十八十八十八十八十八十八十八十八十八十八十八十八");
    test10_Chat();
    LOGI("十九十九十九十九十九十九十九十九十九十九十九十九十九十九十九");
    test10_Chat();
    LOGI("二十二十二十二十二十二十二十二十二十二十二十二十二十二十二十");
    test10_Chat();
}

int main() {
    LOGI("一一一一一一一一一一一一一一一一一一一一一一一一一一一一一一");
    test01_Chat();
    LOGI("二二二二二二二二二二二二二二二二二二二二二二二二二二二二二二");
    test02_ChatHistoryMessages();
    LOGI("三三三三三三三三三三三三三三三三三三三三三三三三三三三三三三");
    test03_ClearChatMessage();
    LOGI("四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四");
    test04_SetModelConfig();
    LOGI("五五五五五五五五五五五五五五五五五五五五五五五五五五五五五五");
    test05_SetChatSystemPrompt();
    LOGI("六六六六六六六六六六六六六六六六六六六六六六六六六六六六六六");
    test06_GenerateContent();
    LOGI("七七七七七七七七七七七七七七七七七七七七七七七七七七七七七七");
    test07_ChatSystemPromptId_SUMMARY();
    LOGI("八八八八八八八八八八八八八八八八八八八八八八八八八八八八八八");
    test08_ChatSystemPromptId_TEXT_EXPANSION();
    LOGI("九九九九九九九九九九九九九九九九九九九九九九九九九九九九九九");
    test09_ChatSystemPromptId_TRANSLATE_CHINESE_TO_ENGLISH();
    LOGI("十十十十十十十十十十十十十十十十十十十十十十十十十十十十十十");
    test10_BigTest();
    test11_ToolCallHandler();

    return 0;
}

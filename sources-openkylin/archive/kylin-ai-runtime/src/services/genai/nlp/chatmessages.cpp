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

#include "chatmessages.h"

#include <jsoncpp/json/value.h>

#include "utils/logger.h"
#include "utils/util.h"

std::string ChatMessages::generateJsonString() {
    Json::Value messages(Json::arrayValue);

    Json::Value systemMessage(Json::objectValue);
    if (!systemMessage_.empty()) {
        systemMessage["role"] = "system";
        systemMessage["content"] = systemMessage_;
        messages.append(systemMessage);
    }

    for (const auto& item : messages_) {
        Json::Value message(Json::objectValue);
        if (item.type_ == ChatMessageType::USER) {
            message["role"] = "user";
            message["content"] = item.message_;
        } else if (item.type_ == ChatMessageType::ASSISTANT) {
            message["role"] = "assistant";
            auto messageJson = util::stringToJson(item.message_);
            if (messageJson.isArray()) {
                for (auto& item : messageJson) {
                    if (item.isObject()) {
                        message["tool_calls"].append(item);
                        message["content"] = "";
                    }
                }
            } else {
                message["content"] = item.message_;
            }
        } else if (item.type_ == ChatMessageType::TOOL) {
            message["role"] = "tool";
            auto toolResultJson = util::stringToJson(item.message_);
            if (toolResultJson.isObject() && toolResultJson.isMember("id") && toolResultJson.isMember("content")) {
                message["id"] = toolResultJson["id"];
                message["content"] = toolResultJson["content"].asString();
            } else {
                message["content"] = item.message_;
            }
        } else {
            // do nothing
        }
        messages.append(message);
    }

    Json::Value requestBody(Json::objectValue);
    requestBody["messages"] = messages;
    if (topK_ != 0) {
        requestBody["top_k"] = topK_;
    }

    if (taskId_ != -1) {
        requestBody["task_id"] = taskId_;
    }

    requestBody["stream"] = stream_;
    requestBody["web_search"] = webSearch_;
    requestBody["enable_thinking"] = enableThinking_;

    if (!tools_.empty()) {
        Json::Value toolChoice = util::stringToJson(toolChoice_);
        if (toolChoice.isObject() && toolChoice.isMember("tool_name") && toolChoice.isMember("type")) {
            requestBody["tool_choice"]["function"]["name"] = toolChoice["tool_name"];
            requestBody["tool_choice"]["type"] = toolChoice["type"];

        } else {
            requestBody["tool_choice"] = toolChoice_;
        }
        requestBody["tools"] = Json::Value(Json::arrayValue);
        for (const auto& tool : tools_) {
            Json::Value toolJson = util::stringToJson(tool.jsonSchema);
            requestBody["tools"].append(toolJson);
        }
    }

    std::string ret = util::jsonToString(requestBody);
    LOGD("Request body: {}", ret);
    return ret;
}

void ChatMessages::setSystemMessage(const std::string& systemMessage) { systemMessage_ = systemMessage; }

void ChatMessages::setSystemMessage(const std::string& systemMessage, int taskId) {
    taskId_ = taskId;
    systemMessage_ = systemMessage;
}

void ChatMessages::setTools(const std::vector<GenAiTool>& tools, Json::Value& toolChoice) {
    if (tools.empty()) {
        LOGE("tools is empty");
        return;
    }

    // 默认初始化
    toolChoice_ = "auto";
    tools_ = tools;

    if (!toolChoice.isObject()) {
        return;
    }

    if (!toolChoice.isMember("tool_mode") || !toolChoice["tool_mode"].isInt()) {
        LOGE("Invalid or missing 'tool_mode' in toolChoice");
        return;
    }

    int mode = toolChoice["tool_mode"].asInt();

    switch (mode) {
        case GENAI_TEXT_TOOL_CHOICE_AUTO:
            toolChoice_ = "auto";
            break;
        case GENAI_TEXT_TOOL_CHOICE_NONE:
            toolChoice_ = "none";
            tools_.clear();
            break;
        case GENAI_TEXT_TOOL_CHOICE_REQUIRED: {
            Json::Value toolJson;
            toolJson["type"] = "function";
            if (toolChoice.isMember("tool_name") && toolChoice["tool_name"].isString()) {
                toolJson["tool_name"] = toolChoice["tool_name"].asString();
            }
            toolChoice_ = util::jsonToString(toolJson);
            break;
        }
        default:
            LOGW("Unknown tool_mode value: %d", mode);
            toolChoice_ = "auto";
            break;
    }
}

void ChatMessages::resetToolChoice() { toolChoice_ = "auto"; }

void ChatMessages::addUserQuestion(const std::string& question) {
    if (question.empty()) {
        return;
    }
    messages_.push_back({ChatMessageType::USER, question});
}

void ChatMessages::addAssistantReply(const std::string& reply) {
    if (reply.empty()) {
        return;
    }
    messages_.push_back({ChatMessageType::ASSISTANT, reply});
}

void ChatMessages::addToolReply(const std::string& reply) {
    if (reply.empty()) {
        return;
    }
    messages_.push_back({ChatMessageType::TOOL, reply});
}

void ChatMessages::clear() {
    systemMessage_.clear();
    messages_.clear();
    taskId_ = -1;
    LOGD("clear");
}

void ChatMessages::setWebSearch(bool webSearch) { webSearch_ = webSearch; }

void ChatMessages::setThinkingEnabled(bool enabled) { enableThinking_ = enabled; }

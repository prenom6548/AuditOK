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

#ifndef CHAT_MESSAGES_H
#define CHAT_MESSAGES_H

#include <jsoncpp/json/json.h>

#include <string>
#include <vector>

#include "utils/logger.h"

enum class GenAiToolType {
    GENAI_TEXT_TOOL_TYPE_FUNCTION = 0,
};

enum GenAiToolChoiceMode {
    GENAI_TEXT_TOOL_CHOICE_AUTO = 0,
    GENAI_TEXT_TOOL_CHOICE_NONE,
    GENAI_TEXT_TOOL_CHOICE_REQUIRED
};

struct GenAiTool {
    std::string name;
    std::string description;
    GenAiToolType type;
    std::string jsonSchema;
};

enum class ChatMessageType { SYSTEM, USER, ASSISTANT, TOOL };

struct ChatMessage {
    ChatMessageType type_{ChatMessageType::USER};
    std::string message_;
};

class ChatMessages {
public:
    std::string generateJsonString();

    void setSystemMessage(const std::string& systemMessage);

    void setSystemMessage(const std::string& systemMessage, int taskId);

    void addUserQuestion(const std::string& question);

    void addAssistantReply(const std::string& reply);

    void addToolReply(const std::string& reply);

    void setTopK(int topK) { topK_ = topK; }

    void setStream(bool stream) { stream_ = stream; }

    void clear();

    void setTaskId(int taskId) { taskId_ = taskId; }

    void setWebSearch(bool webSearch);

    void setThinkingEnabled(bool enabled);

    void setTools(const std::vector<GenAiTool>& tools, Json::Value& toolChoice);

    void resetToolChoice();

private:
    std::string systemMessage_;
    std::vector<ChatMessage> messages_;
    int topK_{};
    int taskId_{-1};
    bool stream_{true};
    bool webSearch_{true};
    bool enableThinking_{false};
    std::vector<GenAiTool> tools_;
    std::string toolChoice_{"auto"};
};

#endif  // CHAT_MESSAGES_H

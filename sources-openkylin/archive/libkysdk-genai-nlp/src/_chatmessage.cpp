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

#include "chatmessage.h"

//

#include <jsoncpp/json/value.h>

#include "_chatmessage.h"
#include "logger.h"

ChatMessage *chat_message_create() { return new _ChatMessage{}; }

void chat_message_destroy(ChatMessage **chat_message) {
    if (chat_message == nullptr) {
        return;
    }
    delete *chat_message;
    *chat_message = nullptr;
}

void chat_message_add_system_message(ChatMessage *chat_message, const char *message) {
    if (chat_message == nullptr) {
        return;
    }
    chat_message->systemMessage_ = message;
}

void chat_message_add_user_message(ChatMessage *chat_message, const char *message) {
    if (chat_message == nullptr) {
        return;
    }
    chat_message->messages_.push_back({ChatMessageType::USER, message});
}

void chat_message_add_assistant_message(ChatMessage *chat_message, const char *message) {
    if (chat_message == nullptr) {
        return;
    }
    chat_message->messages_.push_back({ChatMessageType::ASSISTANT, message});
}

std::string _ChatMessage::generateJsonString() const {
    Json::Value messages(Json::arrayValue);

    if (!systemMessage_.empty()) {
        Json::Value systemMessage(Json::objectValue);
        systemMessage["role"] = "system";
        systemMessage["content"] = systemMessage_;
        messages.append(systemMessage);
    }
    for (const auto &item : messages_) {
        Json::Value message(Json::objectValue);
        if (item.type_ == ChatMessageType::USER) {
            message["role"] = "user";
            message["content"] = item.message_;
        } else if (item.type_ == ChatMessageType::ASSISTANT) {
            message["role"] = "assistant";
            message["content"] = item.message_;
        } else {
            // do nothing
        }
        messages.append(message);
    }

    Json::Value requestBody(Json::objectValue);
    requestBody["messages"] = messages;
    return requestBody.toStyledString();
}
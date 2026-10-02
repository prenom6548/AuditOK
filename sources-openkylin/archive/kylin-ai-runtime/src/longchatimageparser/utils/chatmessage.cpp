/*
 * Copyright (C) 2025 Kylin Software Co., Ltd.
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

namespace chat {

// ChatMessage 实现

ChatMessage::ChatMessage(MessageType type) : type(type) {}

ChatMessage::ChatMessage(MessageType type, const std::optional<std::string>& content,
                         const std::optional<std::string>& nickname, const std::optional<std::string>& time)
    : type(type), content(content), nickname(nickname), time(time) {}

Json::Value ChatMessage::to_dict() const {
    // 使用ordered_json确保字段顺序
    Json::Value result;

    // 根据消息类型设置相应字段
    if (type == MessageType::CHAT || type == MessageType::MY_CHAT) {
        result["type"] = message_type_to_string(type);
        result["昵称"] = nickname.value_or("未知");  // 昵称在前
        result["内容"] = content.value_or("");       // 内容在后
    } else if (type == MessageType::TIME) {
        result["type"] = message_type_to_string(type);
        result["time"] = time.value_or("");
    } else if (type == MessageType::GROUP_NAME) {
        result["type"] = message_type_to_string(type);
        result["群聊名称"] = content.value_or("");
    } else if (type == MessageType::SYSTEM_MESSAGE) {
        result["type"] = message_type_to_string(type);
        result["content"] = content.value_or("");
    } else if (type == MessageType::RETRACT_MESSAGE) {
        result["type"] = message_type_to_string(type);
        result["撤回信息"] = content.value_or("");
    } else {
        result["type"] = message_type_to_string(type);
        result["content"] = content.value_or("");
    }

    return result;
}

// ChatSession 实现

ChatSession::ChatSession() {}

ChatSession::ChatSession(const ChatSession& other) {
    group_name = other.group_name;
    messages.clear();
    messages.reserve(other.messages.size());

    // 深拷贝智能指针中的ChatMessage
    for (const auto& msg_ptr : other.messages) {
        if (msg_ptr) {
            auto new_msg = std::make_unique<ChatMessage>(*msg_ptr);
            messages.push_back(std::move(new_msg));
        }
    }
}

ChatSession& ChatSession::operator=(const ChatSession& other) {
    if (this != &other) {
        group_name = other.group_name;
        messages.clear();
        messages.reserve(other.messages.size());

        // 深拷贝智能指针中的ChatMessage
        for (const auto& msg_ptr : other.messages) {
            if (msg_ptr) {
                auto new_msg = std::make_unique<ChatMessage>(*msg_ptr);
                messages.push_back(std::move(new_msg));
            }
        }
    }
    return *this;
}

void ChatSession::addMessage(std::unique_ptr<ChatMessage> message) {
    if (!message) return;

    // 如果是群名消息，更新群名
    if (message->type == MessageType::GROUP_NAME) {
        group_name = message->content;
    }

    messages.push_back(std::move(message));
}

void ChatSession::add_message(const ChatMessage& message) {
    auto msg_ptr = std::make_unique<ChatMessage>(message);
    addMessage(std::move(msg_ptr));
}

std::unordered_map<std::string, int> ChatSession::getStatistics() const {
    std::unordered_map<std::string, int> stats;

    // 初始化统计计数
    stats["total"] = static_cast<int>(messages.size());
    stats["chat"] = 0;
    stats["my_chat"] = 0;
    stats["time"] = 0;
    stats["group_name"] = 0;
    stats["system_message"] = 0;
    stats["retract_message"] = 0;
    stats["unknown"] = 0;

    // 统计各类消息数量
    for (const auto& msg_ptr : messages) {
        if (!msg_ptr) continue;

        switch (msg_ptr->type) {
            case MessageType::CHAT:
                stats["chat"]++;
                break;
            case MessageType::MY_CHAT:
                stats["my_chat"]++;
                break;
            case MessageType::TIME:
                stats["time"]++;
                break;
            case MessageType::GROUP_NAME:
                stats["group_name"]++;
                break;
            case MessageType::SYSTEM_MESSAGE:
                stats["system_message"]++;
                break;
            case MessageType::RETRACT_MESSAGE:
                stats["retract_message"]++;
                break;
            default:
                stats["unknown"]++;
                break;
        }
    }

    return stats;
}

Json::Value ChatSession::to_dict() const {
    Json::Value result;

    // 转换所有消息为字典格式
    Json::Value chat_messages = Json::Value(Json::arrayValue);
    for (const auto& msg_ptr : messages) {
        if (msg_ptr) {
            chat_messages.append(msg_ptr->to_dict());
        }
    }

    result["chat_messages"] = chat_messages;
    result["metadata"] = Json::Value(Json::objectValue);
    result["metadata"]["total_messages"] = static_cast<Json::UInt64>(messages.size());
    if (group_name.has_value()) {
        result["metadata"]["group_name"] = group_name.value();
    } else {
        result["metadata"]["group_name"] = Json::Value();
    }

    return result;
}

}  // namespace chat

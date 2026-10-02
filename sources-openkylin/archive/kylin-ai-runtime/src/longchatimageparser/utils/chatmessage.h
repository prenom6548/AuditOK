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

#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H

#include <jsoncpp/json/json.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace chat {

// 消息类型枚举
enum class MessageType { CHAT, MY_CHAT, TIME, GROUP_NAME, SYSTEM_MESSAGE, RETRACT_MESSAGE, UNKNOWN };

// MessageType 转字符串
inline std::string message_type_to_string(MessageType type) {
    switch (type) {
        case MessageType::CHAT:
            return "chat";
        case MessageType::MY_CHAT:
            return "my_chat";
        case MessageType::TIME:
            return "time";
        case MessageType::GROUP_NAME:
            return "group_name";
        case MessageType::SYSTEM_MESSAGE:
            return "system_message";
        case MessageType::RETRACT_MESSAGE:
            return "retract_message";
        case MessageType::UNKNOWN:
            return "unknown";
        default:
            return "unknown";
    }
}

// 字符串转 MessageType
inline MessageType string_to_message_type(const std::string& str) {
    if (str == "chat") return MessageType::CHAT;
    if (str == "my_chat") return MessageType::MY_CHAT;
    if (str == "time") return MessageType::TIME;
    if (str == "group_name") return MessageType::GROUP_NAME;
    if (str == "system_message") return MessageType::SYSTEM_MESSAGE;
    if (str == "retract_message") return MessageType::RETRACT_MESSAGE;
    return MessageType::UNKNOWN;
}

// 聊天消息类
class ChatMessage {
public:
    MessageType type;
    std::optional<std::string> content;
    std::optional<std::string> nickname;
    std::optional<std::string> time;

    // 构造函数
    ChatMessage(MessageType type = MessageType::UNKNOWN);

    // 带参数的构造函数
    ChatMessage(MessageType type, const std::optional<std::string>& content,
                const std::optional<std::string>& nickname = std::nullopt,
                const std::optional<std::string>& time = std::nullopt);

    // 转换为字典格式（JSON）
    Json::Value to_dict() const;
};

// 聊天会话类
class ChatSession {
public:
    std::vector<std::unique_ptr<ChatMessage>> messages;
    std::optional<std::string> group_name;

    // 构造函数
    ChatSession();

    // 拷贝构造函数（需要深拷贝智能指针）
    ChatSession(const ChatSession& other);

    // 移动构造函数
    ChatSession(ChatSession&& other) noexcept = default;

    // 赋值运算符（需要深拷贝智能指针）
    ChatSession& operator=(const ChatSession& other);

    // 移动赋值运算符
    ChatSession& operator=(ChatSession&& other) noexcept = default;

    // 添加消息
    void addMessage(std::unique_ptr<ChatMessage> message);

    // 添加消息
    void add_message(const ChatMessage& message);

    // 转换为字典格式
    Json::Value to_dict() const;

    // 获取统计信息
    std::unordered_map<std::string, int> getStatistics() const;
};

}  // namespace chat

#endif  // CHAT_MESSAGE_H

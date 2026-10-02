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

#ifndef CHAT_MESSAGE_H__
#define CHAT_MESSAGE_H__

#include <string>
#include <vector>

enum class ChatMessageType { SYSTEM, USER, ASSISTANT };

struct ChatMessageItem {
    ChatMessageType type_{ChatMessageType::USER};
    std::string message_;
};

struct _ChatMessage {
    std::string systemMessage_;
    std::vector<ChatMessageItem> messages_;

    std::string generateJsonString() const;
};

#endif  // CHAT_MESSAGE_H__
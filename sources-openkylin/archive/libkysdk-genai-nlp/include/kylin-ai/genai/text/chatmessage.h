/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef __KYLIN_AI_GEN_AI_TEXT_CHAT_MESSAGE_H__
#define __KYLIN_AI_GEN_AI_TEXT_CHAT_MESSAGE_H__

#include <common/macros.h>

/**
 * @file chatmessage.h
 * @brief Helpers for constructing structured conversation histories.
 *
 * Mainly used together with @ref genai_text_chat_with_history_messages_async when you want to supply a full prompt
 * manually:
 * @code
 * ChatMessage *history = chat_message_create();
 * chat_message_add_system_message(history, "You are an experienced translator.");
 * chat_message_add_user_message(history, "Translate the following paragraph.");
 * chat_message_add_user_message(history, "Meeting notes: ...");
 * genai_text_chat_with_history_messages_async(session, history);
 * chat_message_destroy(&history);
 * @endcode
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declaration of the history container.
 */
typedef struct _ChatMessage ChatMessage;

/**
 * @brief Create an empty history list.
 *
 * @return A newly allocated container that must be destroyed with @ref chat_message_destroy.
 */
AISDK_EXTERN
ChatMessage *chat_message_create();

/**
 * @brief Destroy the history list and release all buffers.
 *
 * @param chat_message Address of the history pointer; it is nulled on return.
 */
AISDK_EXTERN
void chat_message_destroy(ChatMessage **chat_message);

/**
 * @brief Append a system-role message.
 *
 * System messages describe the assistant's persona, safety rails, or speaking style.
 *
 * @param chat_message History container.
 * @param message      UTF-8 text.
 */
AISDK_EXTERN
void chat_message_add_system_message(ChatMessage *chat_message, const char *message);

/**
 * @brief Append a user-role message.
 *
 * Call this repeatedly to capture every user turn in chronological order.
 *
 * @param chat_message History container.
 * @param message      UTF-8 user input.
 */
AISDK_EXTERN
void chat_message_add_user_message(ChatMessage *chat_message, const char *message);

/**
 * @brief Append an assistant-role message.
 *
 * Use this to replay earlier answers when you maintain conversation state yourself.
 *
 * @param chat_message History container.
 * @param message      Assistant reply text.
 */
AISDK_EXTERN
void chat_message_add_assistant_message(ChatMessage *chat_message, const char *message);

#ifdef __cplusplus
}
#endif

#endif  // __KYLIN_AI_GEN_AI_TEXT_CHAT_MESSAGE_H__

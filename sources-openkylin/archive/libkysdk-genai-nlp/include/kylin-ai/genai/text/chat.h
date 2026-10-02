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

#ifndef __KYLIN_AI_GEN_AI_TEXT_CHAT_H__
#define __KYLIN_AI_GEN_AI_TEXT_CHAT_H__

#include <common/macros.h>

#include "chatmessage.h"
#include "config.h"
#include "modelinfo.h"
#include "result.h"

/**
 * @file chat.h
 * @brief Core interfaces for text generation and multi-turn conversations.
 *
 * Typical usage flow:
 * @code
 * #include "kylin-ai/genai/text/chat.h"
 * #include "kylin-ai/genai/text/config.h"
 * #include "kylin-ai/genai/text/result.h"
 *
 * static void on_chat_result(ChatResult *result, void *user_data) {
 *     if (chat_result_get_error_code(result) == 0 && chat_result_get_assistant_message(result)) {
 *         printf("assistant: %s\n", chat_result_get_assistant_message(result));
 *     }
 * }
 *
 * void run_basic_chat(void) {
 *     GenAiTextSession *session = genai_text_create_session();
 *     if (!session) {
 *         return;
 *     }
 *
 *     genai_text_result_set_callback(session, on_chat_result, NULL);
 *     if (genai_text_init_session(session) != 0) {
 *         genai_text_destroy_session(&session);
 *         return;
 *     }
 *
 *     ChatModelConfig *config = chat_model_config_create();
 *     chat_model_config_set_name(config, "kylin-plus");
 *     genai_text_set_model_config(session, config);
 *     chat_model_config_destroy(&config);
 *
 *     genai_text_chat_async(session, "Hello, please draft a short meeting summary.");
 *     // The business layer runs its event loop and consumes streaming
 *     // callbacks.
 *     genai_text_destroy_session(&session);
 * }
 * @endcode
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Built-in prompt templates for popular conversation scenarios.
 *
 * Each ID maps to a server-side system prompt so you can quickly prototype without curating your own instructions.
 */
typedef enum {
    /**< Keep writing a paragraph or expand a bullet list. */
    TEXT_EXPANSION = 1,
    /**< Improve style, grammar, and clarity of copy. */
    TEXT_POLISHING = 2,
    /**< Translate Chinese passages into English. */
    TRANSLATE_CHINESE_TO_ENGLISH = 3,
    /**< Fix grammar and spelling issues. */
    TEXT_ERROR_CORRECTION = 8,
    /**< Produce concise summaries. */
    SUMMARY = 9,
    /**< Draft polite email replies from short notes. */
    EMAIL_REPLY = 10,
    /**< Prepare daily/weekly work reports. */
    WORK_SUMMARY = 11,
    /**< Extract structured keywords or tags. */
    TAG_EXTRACTION = 12,
} PromptId;

typedef struct _GenAiTextSession GenAiTextSession;

/**
 * @brief Prototype of the chat result callback.
 *
 * The SDK invokes this callback whenever an asynchronous API (for example @ref genai_text_chat_async) produces a new
 * token chunk or a final completion.
 *
 * @param chat_result The incremental or final result carrying both text and error state.
 * @param user_data   The opaque pointer that was provided during callback registration.
 */
typedef void (*ChatResultCallback)(ChatResult *chat_result, void *user_data);

/**
 * @brief Completion callback for @ref genai_text_init_session_async.
 *
 * The callback runs after the background session initialization finishes. Use it to begin issuing requests without
 * blocking the main thread.
 *
 * @param error_code Matches the synchronous return codes; 0 means success.
 * @param user_data  User data from registration.
 */
typedef void (*GenAiTextInitCallback)(int error_code, void *user_data);

/**
 * @brief Allocate a new text session handle.
 *
 * A session keeps transport state, credentials, and callbacks. You must eventually destroy it with @ref
 * genai_text_destroy_session to prevent leaks.
 *
 * @return A non-null pointer on success, or NULL if allocation fails.
 */
AISDK_EXTERN
GenAiTextSession *genai_text_create_session();

/**
 * @brief Tear down a session and release all related resources.
 *
 * @param session Address of the session pointer. The pointer is nulled after the call.
 */
AISDK_EXTERN
void genai_text_destroy_session(GenAiTextSession **session);

/**
 * @brief Initialize the session before issuing any generation requests.
 *
 * Initialization will negotiate credentials, open transports, and fetch server metadata. You must call this once per
 * session prior to any async chat call.
 *
 * @param session Session handle returned from @ref genai_text_create_session.
 * @return 0 on success; non-zero error codes match @ref GenAiTextErrorCode.
 */
AISDK_EXTERN
int genai_text_init_session(GenAiTextSession *session);

/**
 * @brief Initialize the session on a worker thread.
 *
 * The SDK runs the same logic as @ref genai_text_init_session but never blocks the caller. Keep the session alive until
 * the callback fires.
 *
 * @param session   Session instance.
 * @param callback  Optional completion callback; NULL disables notifications.
 * @param user_data User data forwarded to the callback.
 */
AISDK_EXTERN
void genai_text_init_session_async(GenAiTextSession *session, GenAiTextInitCallback callback, void *user_data);

/**
 * @brief Register the asynchronous result callback for streaming responses.
 *
 * @param session   Session instance.
 * @param callback  Callback invoked for each chunk; pass NULL to detach the callback.
 * @param user_data Optional pointer forwarded to every callback invocation.
 */
AISDK_EXTERN
void genai_text_result_set_callback(GenAiTextSession *session, ChatResultCallback callback, void *user_data);

/**
 * @brief Let the SDK drive its own event loop instead of relying on GLib.
 *
 * By default you must run a GLib loop to dispatch network events. Enabling the internal loop spawns a helper thread so
 * callbacks fire even if the host process does not run GLib.
 *
 * @param session Session instance.
 * @param enable  true enables the internal loop; false reverts to the default.
 */
AISDK_EXTERN
void genai_text_enable_internal_event_loop(GenAiTextSession *session, bool enable);

/**
 * @brief Fetch the list of models available to the current session.
 *
 * The returned pointer stays owned by the SDK; do not free it. The SDK keeps the list alive for the lifetime of the
 * process (and reuses it across sessions), so callers may safely cache the pointer while the session remains valid.
 *
 * @param session    Initialized session instance.
 * @param error_code Optional pointer that receives an error code on failure.
 * @return           Pointer to the list when successful, otherwise NULL.
 */
AISDK_EXTERN
const TextModelList *genai_text_get_model_list(GenAiTextSession *session, int *error_code);

/**
 * @brief Apply a model configuration to the session.
 *
 * Call this after initialization and whenever you need to switch models or sampling parameters.
 *
 * @param session Session instance.
 * @param config  Configuration object owned by the caller.
 */
AISDK_EXTERN
void genai_text_set_model_config(GenAiTextSession *session, ChatModelConfig *config);

/**
 * @brief Generate standalone content from a prompt.
 *
 * Use this when you do not need conversational memory.
 *
 * @param session Initialized session.
 * @param prompt  UTF-8 encoded prompt text.
 */
AISDK_EXTERN
void genai_text_generate_content_async(GenAiTextSession *session, const char *prompt);

/**
 * @brief Send a user turn that automatically references the session history.
 *
 * @param session  Initialized session.
 * @param question Current user utterance.
 */
AISDK_EXTERN
void genai_text_chat_async(GenAiTextSession *session, const char *question);

/**
 * @brief Send a user turn with a caller-supplied request ID.
 *
 * When request_id is NULL, the SDK automatically generates a request ID. When request_id is an empty string, this
 * behaves the same as @ref genai_text_chat_async.
 *
 * @param session     Initialized session.
 * @param question    Current user utterance.
 * @param request_id  Caller-supplied identifier for tracing.
 * @return AI_COMMON_SUCCESS on success; >0 error code on failure.
 */
AISDK_EXTERN
int genai_text_chat_async_with_request_id(GenAiTextSession *session, const char *request_id, const char *question);

/**
 * @brief Initiate a chat request with caller-supplied history messages.
 *
 * This is ideal when you store conversation state externally and want full control over the prompt. See @ref
 * chat_message_create for building the list.
 *
 * Example:
 * @code
 * ChatMessage *messages = chat_message_create();
 * chat_message_add_system_message(messages, "You are a concise assistant.");
 * chat_message_add_user_message(messages, "Translate the paragraph below.");
 * genai_text_chat_with_history_messages_async(session, messages);
 * chat_message_destroy(&messages);
 * @endcode
 *
 * @param session          Initialized session.
 * @param history_messages Message container managed by the caller.
 */
AISDK_EXTERN
void genai_text_chat_with_history_messages_async(GenAiTextSession *session, ChatMessage *history_messages);

/**
 * @brief Initiate a chat request with caller-supplied history messages and a request ID.
 *
 * When request_id is NULL, the SDK automatically generates a request ID. When  request_id is an empty string, this
 * behaves the same as @ref genai_text_chat_with_history_messages_async.
 *
 * @param session          Initialized session.
 * @param request_id       Caller-supplied identifier for tracing.
 * @param history_messages Message container managed by the caller.
 * @return AI_COMMON_SUCCESS on success; >0 error code on failure.
 */
AISDK_EXTERN
int genai_text_chat_with_history_messages_async_with_request_id(GenAiTextSession *session, const char *request_id,
                                                                ChatMessage *history_messages);

/**
 * @brief Define the system prompt to steer tone and behavior.
 *
 * @param session Initialized session.
 * @param prompt  UTF-8 string describing the assistant's role.
 */
AISDK_EXTERN
void genai_text_set_chat_system_prompt(GenAiTextSession *session, const char *prompt);

/**
 * @brief Apply one of the predefined system prompts.
 *
 * @param session   Initialized session.
 * @param prompt_id Identifier from @ref PromptId.
 */
AISDK_EXTERN
void genai_text_set_chat_system_prompt_id(GenAiTextSession *session, PromptId prompt_id);

/**
 * @brief Clear the cached dialogue history for the session.
 *
 * After this call the model responds as if it is a brand-new conversation.
 */
AISDK_EXTERN
void genai_text_clear_chat_history_messages(GenAiTextSession *session);

/**
 * @brief Interrupt the in-flight generation request if the user cancels.
 *
 * @param session Initialized session.
 */
AISDK_EXTERN
void genai_text_stop_chat(GenAiTextSession *session);

/**
 * @brief Interrupt the in-flight generation request with a request ID.
 *
 * When request_id is NULL or an empty string, this behaves the same as @ref genai_text_stop_chat.
 *
 * @param session     Initialized session.
 * @param request_id  Caller-supplied identifier for tracing.
 * @return AI_COMMON_SUCCESS on success; >0 error code on failure.
 */
AISDK_EXTERN
int genai_text_stop_chat_with_request_id(GenAiTextSession *session, const char *request_id);

#ifdef __cplusplus
}
#endif

#endif  // __KYLIN_AI_GEN_AI_TEXT_CHAT_H__

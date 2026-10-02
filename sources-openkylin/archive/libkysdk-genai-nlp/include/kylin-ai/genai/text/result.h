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

#ifndef __KYLIN_AI_GEN_AI_TEXT_RESULT_H__
#define __KYLIN_AI_GEN_AI_TEXT_RESULT_H__

#include <common/macros.h>
#include <stddef.h>

/**
 * @file result.h
 * @brief Accessors for asynchronous generation results.
 *
 * Example callback:
 * @code
 * static void on_chat_result(ChatResult *result, void *user_data) {
 *     if (chat_result_get_error_code(result) != 0) {
 *         fprintf(stderr, "error: %s\\n", chat_result_get_error_message(result));
 *         return;
 *     }
 *
 *     const char *delta = chat_result_get_assistant_message(result);
 *     if (delta) {
 *         printf("%s", delta);
 *     }
 *
 *     if (chat_result_get_is_end(result)) {
 *         printf("\\nfinish reason: %s\\n", chat_result_get_finish_reason_message(result));
 *     }
 * }
 * @endcode
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declaration for result objects delivered to callbacks.
 */
typedef struct _ChatResult ChatResult;
typedef struct _ChatResultSearchInfo ChatResultSearchInfo;
typedef struct _ChatResultSearchItem ChatResultSearchItem;
/**
 * @brief Retrieve assistant output text (maybe incremental).
 *
 * @param result Result object passed to the callback.
 * @return UTF-8 text pointer or NULL when no text is present.
 */
AISDK_EXTERN
const char *chat_result_get_assistant_message(ChatResult *result);

/**
 * @brief Access the reasoning / chain-of-thought text when available.
 *
 * @param result Result object.
 * @return Reasoning snippet or NULL if the model did not expose one.
 */
AISDK_EXTERN
const char *chat_result_get_reasoning_message(ChatResult *result);

/**
 * @brief Return the finish reason for the request.
 *
 * Typical values: `stop`, `length`, `content_filter`, or `null` while streaming.
 *
 * @param result Result object.
 * @return Finish reason string.
 */
AISDK_EXTERN
const char *chat_result_get_finish_reason_message(ChatResult *result);

/**
 * @brief Get the model name corresponding to the chat result.
 *
 * @return Model name.
 */
AISDK_EXTERN
const char *chat_result_get_model_name(ChatResult *result);

/**
 * @brief Get the request ID associated with the chat result.
 *
 * @return Request ID string or NULL when unavailable.
 */
AISDK_EXTERN
const char *chat_result_get_request_id(ChatResult *result);

/**
 * @brief Inspect the numeric error code.
 *
 * Combine with @ref GenAiTextErrorCode for module-specific diagnostics.
 *
 * @param result Result object.
 * @return Error code value.
 */
AISDK_EXTERN
int chat_result_get_error_code(ChatResult *result);

/**
 * @brief Read the human-readable error message.
 *
 * @param result Result object.
 * @return UTF-8 string, or NULL when no error occurred.
 */
AISDK_EXTERN
const char *chat_result_get_error_message(ChatResult *result);

/**
 * @brief Check whether the current callback corresponds to the final chunk.
 *
 * @param result Result object.
 * @return true when generation has finished.
 */
AISDK_EXTERN
bool chat_result_get_is_end(ChatResult *result);

/**
 * @brief Retrieve web search information from a chat result object.
 *
 * @param result [in] Valid ChatResult pointer containing chat response data.
 * @param error_code [out] Optional output parameter receiving 0 on success.
 * @return Pointer to ChatResultSearchInfo if successful; NULL if failed.
 */
AISDK_EXTERN
const ChatResultSearchInfo *chat_result_get_search_info(const ChatResult *result, int *error_code);

/**
 * @brief Get the total number of web search items in the search info object.
 *
 * @param info [in] Pointer to a valid ChatResultSearchInfo object.
 * @param error_code [out] Optional output parameter receiving 0 on success.
 * @return Total number of search items (0 if no items exist or on failure).
 */
AISDK_EXTERN
size_t chat_result_search_info_get_count(const ChatResultSearchInfo *info, int *error_code);

/**
 * @brief Retrieve a specific web search item by index from the search info object.
 *
 * @param info [in] Pointer to a valid ChatResultSearchInfo object.
 * @param index [in] Zero-based index of the search item to retrieve (must be < total count).
 * @param error_code [out] Optional output parameter receiving 0 on success.
 * @return Pointer to ChatResultSearchItem if successful; NULL if failed.
 */
AISDK_EXTERN
const ChatResultSearchItem *chat_result_search_info_get_item(const ChatResultSearchInfo *info, size_t index,
                                                             int *error_code);

/**
 * @brief Get the zero-based index of a specific web search item.
 *
 * @param item [in] Pointer to a valid ChatResultSearchItem object.
 * @param error_code [out] Optional output parameter receiving 0 on success.
 * @return Zero-based index of the search item; 0 on failure.
 */
AISDK_EXTERN
size_t chat_result_search_item_get_index(const ChatResultSearchItem *item, int *error_code);

/**
 * @brief Get the title of a web search item (e.g., page title).
 *
 * @param item [in] Pointer to a valid ChatResultSearchItem object.
 * @param error_code [out] Optional output parameter receiving 0 on success.
 * @return Const char pointer to the item's title if successful; NULL if no title or failed.
 */
AISDK_EXTERN
const char *chat_result_search_item_get_title(const ChatResultSearchItem *item, int *error_code);

/**
 * @brief Get the URL of a web search item (e.g., page URL).
 *
 * @param item [in] Pointer to a valid ChatResultSearchItem object.
 * @param error_code [out] Optional output parameter receiving 0 on success.
 * @return Const char pointer to the item's URL if successful; NULL if no URL or failed.
 */
AISDK_EXTERN
const char *chat_result_search_item_get_url(const ChatResultSearchItem *item, int *error_code);

#ifdef __cplusplus
}
#endif

#endif  // __KYLIN_AI_GEN_AI_TEXT_RESULT_H__

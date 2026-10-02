#ifndef CHATRESULT_H
#define CHATRESULT_H

#include "chatreference.h"
#include "chatsource.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file chatresult.h
 * @brief Chat Result API for Knowledge Base SDK
 * @version 1.0.0
 * @author Development Team
 *
 * This header provides chat result structure and operations.
 * Chat results contain the actual response content and metadata.
 */

/* Forward declaration */
typedef struct KBChatResult KBChatResult;

/**
 * @brief Check if the chat result is finished
 *
 * Returns whether the chat result is finished.
 *
 * @param result Chat result (must not be NULL)
 * @return true if the chat result is finished, false otherwise
 *
 * @note This function is thread-safe
 */
KB_API bool kb_chat_result_is_finished(const KBChatResult *result) KB_NONNULL;

/**
 * @brief Get question from chat result
 *
 * Returns the original question that was asked.
 *
 * @param result Chat result (must not be NULL)
 * @return Question string (do not free, valid until result is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_chat_result_get_question(const KBChatResult *result)
    KB_NONNULL;

/**
 * @brief Get response from chat result
 *
 * Returns the AI-generated response to the question.
 *
 * @param result Chat result (must not be NULL)
 * @return Response string (do not free, valid until result is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_chat_result_get_response(const KBChatResult *result)
    KB_NONNULL;

/**
 * @brief Get reasoning content from chat result
 *
 * Returns the AI-generated reasoning content to the question.
 *
 * @param result Chat result (must not be NULL)
 * @return Reasoning content string (do not free, valid until result is
 * destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_chat_result_get_reasoning_content(
    const KBChatResult *result) KB_NONNULL;

/**
 * @brief Get source from chat result
 *
 * Returns the array of source documents that were referenced in generating the
 * response.
 *
 * @param result Chat result (must not be NULL)
 * @param count Pointer to store number of source entries (must not be NULL)
 * @return Array of chat reference entries (do not free, valid until result is
 * destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const KBChatSource **kb_chat_result_get_source(
    const KBChatResult *result, int *count) KB_NONNULL;

/**
 * @brief Get reference files from chat result
 *
 * Returns the array of reference files that were referenced in generating
 * the response.
 *
 * @param result Chat result (must not be NULL)
 * @param count Pointer to store number of reference files entries (must not be
 * NULL)
 * @return Array of chat reference files entries (do not free, valid until
 * result is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const char **kb_chat_result_get_reference_file_ids(
    const KBChatResult *result, int *count) KB_NONNULL;

/**
 * @brief Get reference infos from chat result
 *
 * Returns the array of reference infos that were referenced in generating the
 * response.
 *
 * @param result Chat result (must not be NULL)
 * @param count Pointer to store number of reference infos entries (must not be
 * NULL)
 * @return Array of chat reference infos entries (do not free, valid until
 * result is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const KBChatReference **kb_chat_result_get_reference(
    const KBChatResult *result, int *count) KB_NONNULL;

/**
 * @brief Get model from chat result
 *
 * Returns the AI-generated response to the question.
 *
 * @param result Chat result (must not be NULL)
 * @return Model name string (do not free, valid until result is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_chat_result_get_model(const KBChatResult *result)
    KB_NONNULL;

#ifdef __cplusplus
}
#endif

#endif /* CHATRESULT_H */
#ifndef CHATRESPONSE_H
#define CHATRESPONSE_H

#include "chatresult.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file chatresponse.h
 * @brief Chat Response API for Knowledge Base SDK
 * @version 1.0.0
 * @author Development Team
 *
 * This header provides chat response structure and operations.
 * Chat responses contain the results of chat interactions.
 */

/* Forward declaration */
typedef struct KBChatResponse KBChatResponse;

/**
 * @brief Get chat ID from chat response
 *
 * Returns the unique chat session identifier from the response.
 * The chat ID is used to track the conversation and retrieve related data.
 *
 * @param response Chat response (must not be NULL)
 * @return Chat ID string (do not free, valid until response is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_chat_response_get_chat_id(const KBChatResponse *response)
    KB_NONNULL;

/**
 * @brief Get request ID from chat response
 *
 * Returns the unique request identifier from the response.
 * The request ID is used to track individual chat requests.
 *
 * @param response Chat response (must not be NULL)
 * @return Request ID string (do not free, valid until response is destroyed)
 *
 * @note This function is thread-safe
 * @note The request ID is either user-provided or system-generated
 */
KB_API const char *kb_chat_response_get_request_id(
    const KBChatResponse *response) KB_NONNULL;

/**
 * @brief Get error code from chat response
 *
 * Returns the error code indicating the success or failure of the chat
 * operation.
 *
 * @param response Chat response (must not be NULL)
 * @return Error code
 *
 * @note This function is thread-safe
 */
KB_API int kb_chat_response_get_error_code(const KBChatResponse *response)
    KB_NONNULL;

/**
 * @brief Get result from chat response
 *
 * Returns the chat result containing the actual response content and metadata.
 *
 * @param response Chat response (must not be NULL)
 * @return Chat result (do not free, valid until response is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const KBChatResult *kb_chat_response_get_result(
    const KBChatResponse *response) KB_NONNULL;

/**
 * @brief Get error message from chat response
 *
 * Returns the error message describing the error that occurred during the chat
 * operation.
 *
 * @param response Chat response (must not be NULL)
 * @return Error message string (do not free, valid until response is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_chat_response_get_error_message(
    const KBChatResponse *response) KB_NONNULL;

#ifdef __cplusplus
}
#endif

#endif /* CHATRESPONSE_H */

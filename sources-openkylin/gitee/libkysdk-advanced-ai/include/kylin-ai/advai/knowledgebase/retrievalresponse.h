#ifndef RETRIEVAL_RESPONSE_H
#define RETRIEVAL_RESPONSE_H

#include "chatsource.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file retrievalresponse.h
 * @brief Retrieval response data structure and accessors
 * @version 1.0.0
 */
typedef struct KBRetrievalResponse KBRetrievalResponse;

/**
 * @brief Get request id from retrieval response
 *
 * @param response Retrieval response (must not be NULL)
 * @return Request id string or NULL
 */
KB_API const char *kb_retrieval_response_get_request_id(
    const KBRetrievalResponse *response) KB_NONNULL;

/**
 * @brief Get knowledge base id from retrieval response
 *
 * @param response Retrieval response (must not be NULL)
 * @return Knowledge base id string or NULL
 */
KB_API const char *kb_retrieval_response_get_knowledgebase_id(
    const KBRetrievalResponse *response) KB_NONNULL;

/**
 * @brief Get query text from retrieval response
 *
 * @param response Retrieval response (must not be NULL)
 * @return Query string or NULL
 */
KB_API const char *kb_retrieval_response_get_query(
    const KBRetrievalResponse *response) KB_NONNULL;

/**
 * @brief Get retrieval error code
 *
 * @param response Retrieval response (must not be NULL)
 * @return Error code
 */
KB_API int kb_retrieval_response_get_error_code(
    const KBRetrievalResponse *response) KB_NONNULL;

/**
 * @brief Get number of retrieved sources
 *
 * @param response Retrieval response (must not be NULL)
 * @return Source count
 */
KB_API int kb_retrieval_response_get_source_count(
    const KBRetrievalResponse *response) KB_NONNULL;

/**
 * @brief Get source item by index
 *
 * @param response Retrieval response (must not be NULL)
 * @param index Source index
 * @return Source pointer or NULL if out of range
 */
KB_API const KBChatSource *kb_retrieval_response_get_source(
    const KBRetrievalResponse *response, int index) KB_NONNULL;

#ifdef __cplusplus
}
#endif

#endif /* RETRIEVAL_RESPONSE_H */

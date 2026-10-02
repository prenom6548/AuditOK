#ifndef CHATREFERENCE_H
#define CHATREFERENCE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file chatreference.h
 * @brief Chat Reference API for Knowledge Base SDK
 * @version 1.0.0
 * @author Development Team
 *
 * This header provides chat reference structure and operations.
 * Chat references to contain information about documents referenced in chat
 * responses.
 */

/* Forward declaration */
typedef struct KBChatReference KBChatReference;

/**
 * @brief Get file number from chat reference
 *
 * Returns the unique file identifier from the chat reference.
 *
 * @param reference Chat reference (must not be NULL)
 * @return File Number of reference file ids
 *
 * @note This function is thread-safe
 */
KB_API int kb_chat_reference_get_file_number(const KBChatReference *reference)
    KB_NONNULL;

/**
 * @brief Get content from chat reference
 *
 * Returns the content text from the chat reference.
 *
 * @param reference Chat reference (must not be NULL)
 * @return Chunk index of chunks
 *
 * @note This function is thread-safe
 */
KB_API int kb_chat_reference_get_chunk_index(const KBChatReference *reference)
    KB_NONNULL;

#ifdef __cplusplus
}
#endif

#endif  // CHATREFERENCE_H

#ifndef CHATSOURCE_H
#define CHATSOURCE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file chatsource.h
 * @brief Chat Source API for Knowledge Base SDK
 * @version 1.0.0
 * @author Development Team
 *
 * This header provides chat source structure and operations.
 * Chat sources contain information about documents referenced in chat
 * responses.
 */

/* Forward declaration */
typedef struct KBChatSource KBChatSource;

/**
 * @brief Get file ID from chat source
 *
 * Returns the unique file identifier from the chat source.
 *
 * @param source Chat source (must not be NULL)
 * @return File ID string (do not free, valid until source is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_chat_source_get_file_id(const KBChatSource *source)
    KB_NONNULL;

/**
 * @brief Get content from chat source
 *
 * Returns the content text from the chat source.
 *
 * @param source Chat source (must not be NULL)
 * @return Content string (do not free, valid until source is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_chat_source_get_content(const KBChatSource *source)
    KB_NONNULL;

/**
 * @brief Get confidence score from chat source
 *
 * Returns the confidence/relevance score as a numeric value.
 *
 * @param source Chat source (must not be NULL)
 * @return Confidence score value between 0.0 and 1.0, or -1.0 on failure
 *
 * @note This function is thread-safe
 */
KB_API double kb_chat_source_get_confidence(const KBChatSource *source)
    KB_NONNULL;

#ifdef __cplusplus
}
#endif

#endif /* CHATSOURCE_H */
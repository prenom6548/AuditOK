#ifndef CHATCONFIG_H
#define CHATCONFIG_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file chatconfig.h
 * @brief Chat Configuration API for Knowledge Base SDK
 * @version 1.0.0
 * @author Development Team
 *
 * This header provides chat configuration structure and operations for
 * configuring chat behavior, model selection, and query parameters.
 * Chat configurations control how the AI model interacts with knowledge bases.
 */

/* Forward declaration */
typedef struct KBChatConfig KBChatConfig;

/**
 * @brief Create a chat configuration structure
 *
 * Creates and initializes a new chat configuration structure with default
 * values. The caller is responsible for freeing the returned structure using
 * kb_chat_config_free().
 *
 * @return Pointer to new chat configuration, or NULL on failure
 *
 * @note This function is thread-safe
 * @see kb_chat_config_free()
 */
KB_API KBChatConfig *kb_chat_config_new();

/**
 * @brief Set whether to start a new chat session
 *
 * Configures the chat configuration to indicate whether a new chat session
 * should be started.
 *
 * @param config Pointer to chat configuration (must not be NULL)
 * @param new_chat true to start a new chat session, false to continue existing
 * session
 *
 * @note This function is thread-safe
 */
KB_API void kb_chat_config_set_new_chat(KBChatConfig *config,
                                        bool new_chat) KB_NONNULL;

/**
 * @brief Set AI model for chat configuration
 *
 * Specifies the AI model to use for generating chat responses. The model
 * determines the capabilities and behavior of the chat system.
 *
 * @param config Pointer to chat configuration (must not be NULL)
 * @param model Model name or identifier (must not be NULL)
 *
 * @note This function is thread-safe
 * @note The model name is copied internally and can be freed after calling
 * @note Model availability depends on the backend service configuration
 */
KB_API void kb_chat_config_set_model(KBChatConfig *config, const char *model);

/**
 * @brief Set document IDs for chat query
 *
 * Sets the document IDs to query against in the chat configuration.
 *
 * @param config Pointer to chat configuration (must not be NULL)
 * @param doc_ids Array of document ID strings (must not be NULL)
 * @param doc_ids_count Number of document IDs in array (must be > 0)
 *
 * @note This function is thread-safe
 * @note The document IDs are copied internally and can be freed after calling
 */
KB_API void kb_chat_config_set_doc_ids(KBChatConfig *config,
                                       const char **doc_ids,
                                       int doc_ids_count) KB_NONNULL;

/**
 * @brief Set whether to enable query diversification
 *
 * Configures the chat configuration to enable or disable query diversification.
 * When enabled, the system will diversify and expand the original query to
 * improve retrieval coverage and result quality by exploring different aspects
 * of the query intent.
 *
 * @param config Pointer to chat configuration (must not be NULL)
 * @param enable true to enable query diversification, false to disable
 *
 * @note This function is thread-safe
 */
KB_API void kb_chat_config_set_diversify_query_enabled(KBChatConfig *config,
                                                       bool enable) KB_NONNULL;

/**
 * @brief Destroy a chat configuration
 *
 * Frees all resources associated with the chat configuration.
 *
 * @param config Pointer to chat configuration (can be NULL)
 *
 * @note This function is thread-safe
 */
KB_API void kb_chat_config_free(KBChatConfig *config);

#ifdef __cplusplus
}
#endif

#endif /* CHATCONFIG_H */

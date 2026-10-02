#ifndef CHATHISTORY_H
#define CHATHISTORY_H

#include "chatresponse.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file chathistory.h
 * @brief Chat History API for Knowledge Base SDK
 * @version 1.0.0
 * @author Development Team
 *
 * This header provides chat history structure and operations.
 * A KBChatHistory represents a single chat session containing multiple
 * question-response pairs.
 */

/* Forward declaration */
typedef struct KBChatHistory KBChatHistory;

/**
 * @brief Get message count from chat history
 *
 * Retrieves the number of question-response message pairs in the chat history.
 *
 * @param history A pointer to the KBChatHistory object (must not be NULL)
 * @return The number of message pairs in the chat history, or -1 if an error
 * occurs
 *
 * @note This function is thread-safe
 * @note The returned count represents the total number of (question + response)
 * pairs
 */
KB_API KBChatHistory *kb_chat_history_new();

/**
 * @brief Add a new question to the chat history
 *
 * Adds a new question to the chat history. The question string is copied
 * internally, so the caller retains ownership of the original string.
 *
 * @param history A pointer to the KBChatHistory object (must not be NULL)
 * @param question The question string to add (must not be NULL)
 * @return index on success, or -1 if an error occurs (e.g., out of memory)
 *
 * @note This function is thread-safe
 * @note The chat history does not take ownership of the question string;
 *       it makes a copy internally
 */
KB_API int kb_chat_history_add_question(KBChatHistory *history,
                                        const char *question) KB_NONNULL;

/**
 * @brief Add a new response to the chat history
 *
 * Adds a new response to the chat history. The response string is copied
 * internally, so the caller retains ownership of the original string.
 *
 * @param history A pointer to the KBChatHistory object (must not be NULL)
 * @param response The response string to add (must not be NULL)
 * @return index on success, or -1 if an error occurs (e.g., out of memory)
 *
 * @note This function is thread-safe
 * @note The chat history does not take ownership of the response string;
 *       it makes a copy internally
 */
KB_API int kb_chat_history_add_response(KBChatHistory *history,
                                        const char *response) KB_NONNULL;

/**
 * @brief Delete question from chat history by index
 *
 * Deletes the question at the specified index from the chat history.
 *
 * @param history Chat history (must not be NULL)
 * @param index Message index (0-based, must be < message_count)
 * @return true if the question is deleted successfully, false otherwise
 *
 * @note This function is thread-safe
 */
KB_API bool kb_chat_history_delete_question(KBChatHistory *history,
                                            int index) KB_NONNULL;

/**
 * @brief Delete response from chat history by index
 *
 * Deletes the response at the specified index from the chat history.
 *
 * @param history Chat history (must not be NULL)
 * @param index Message index (0-based, must be < message_count)
 * @return true if the response is deleted successfully, false otherwise
 *
 * @note This function is thread-safe
 */
KB_API bool kb_chat_history_delete_response(KBChatHistory *history,
                                            int index) KB_NONNULL;

/**
 * @brief Get message count from chat history
 *
 * Returns the number of question-response pairs in the chat history.
 *
 * @param history Chat history (must not be NULL)
 * @return Number of message pairs in the history, or -1 on failure
 *
 * @note This function is thread-safe
 */
KB_API int kb_chat_history_get_message_count(const KBChatHistory *history)
    KB_NONNULL;

/**
 * @brief Get question from chat history by index
 *
 * Returns the question text from a specific message pair in the chat history.
 *
 * @param history Chat history (must not be NULL)
 * @param index Message index (0-based, must be < message_count)
 * @return Question string (do not free, valid until history is destroyed)
 *
 * @note This function is thread-safe
 * @note Returns NULL if index is out of range
 */
KB_API const char *kb_chat_history_get_question(const KBChatHistory *history,
                                                int index) KB_NONNULL;

/**
 * @brief Get response from chat history by index
 *
 * Returns the response text from a specific message pair in the chat history.
 *
 * @param history Chat history (must not be NULL)
 * @param index Message index (0-based, must be < message_count)
 * @return Response string (do not free, valid until history is destroyed)
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_chat_history_get_response(const KBChatHistory *history,
                                                int index) KB_NONNULL;

/**
 * @brief Free chat history
 *
 * Frees all resources associated with a chat history object.
 *
 * @param history Chat history object (can be NULL)
 *
 * @note This function is thread-safe
 */
KB_API void kb_chat_history_free(KBChatHistory *history);

#ifdef __cplusplus
}
#endif

#endif /* CHATHISTORY_H */
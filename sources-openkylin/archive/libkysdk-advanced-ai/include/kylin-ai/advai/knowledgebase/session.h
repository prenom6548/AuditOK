#ifndef KNOWLEDGEBASE_SESSION_H
#define KNOWLEDGEBASE_SESSION_H

#include "chatconfig.h"
#include "chathistory.h"
#include "chatresponse.h"
#include "knowledgebaseinfo.h"
#include "retrievalconfig.h"
#include "retrievalresponse.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file session.h
 * @brief Knowledge Base Session API for AI SDK
 * @version 1.0.0
 * @author Development Team
 *
 * This header provides session management and knowledge base operations.
 * Sessions are required for all remote knowledge base operations.
 * All functions are thread-safe unless otherwise noted.
 *
 * The API provides two sets of interfaces:
 * 1. Object-based API: Uses KnowledgeBaseInfo pointers, suitable for
 * applications that maintain knowledge base objects and need cached metadata.
 * 2. ID-based API: Uses string IDs only, suitable for lightweight operations,
 *    cross-process communication, and stateless applications.
 */

/* Forward declarations */
typedef struct KBSession KBSession;

/* ========================================================================
 * Session Management Functions
 * ======================================================================== */

/**
 * @brief Create a new knowledge base session with application registration
 *
 * Creates a session for accessing knowledge base services and registers the
 * application with the server. The server will generate a unique application ID
 * for isolation and resource management. The caller is responsible for
 * destroying the session using kb_session_destroy().
 *
 * @param app_name The name of the application (must not be NULL)
 * @param password The password for the application (must not be NULL)
 * @param database_path The path to the database directory (must not be NULL)
 * @param error_code Pointer to store error code (can be NULL)
 * @return Pointer to the new session instance, or NULL on failure
 *
 * @note This function is thread-safe
 * @see kb_session_destroy()
 */
KB_API KBSession *kb_session_create(const char *app_name, const char *password,
                                    const char *database_path, int *error_code);

/**
 * @brief Initialize a knowledge base session
 *
 * Initializes the session with default settings. This function must be called
 * before using any other session functions.
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @return Error code indicating success or failure
 *
 * @note This function is thread-safe
 */
KB_API int kb_session_init(KBSession *session) KB_NONNULL;

/**
 * @brief Destroy a knowledge base session
 *
 * Frees all resources associated with the session and closes any
 * active connections.
 *
 * @param session Pointer to the session instance (can be NULL)
 *
 * @note This function is thread-safe
 */
KB_API void kb_session_destroy(KBSession *session);

/* ========================================================================
 * Object-based Knowledge Base API
 *
 * These functions use KnowledgeBaseInfo object pointers and are suitable for:
 * - Applications that maintain knowledge base objects
 * - Operations that benefit from cached metadata
 * - Complex applications with multiple knowledge base interactions
 * ======================================================================== */

/**
 * @brief Create a knowledge base using KnowledgeBaseInfo object information
 *
 * Creates a new knowledge using the information from a KnowledgeBaseInfo
 * object. This function instantiates the knowledge base structure, including
 * necessary directories, metadata files, and initialization data. The object
 * should be created using kb_knowledgebase_info_new() and configured with the
 * desired name and description before calling this function. The caller is
 * responsible for freeing the returned knowledge base ID using
 * kb_free_string().
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_info Pointer to the KnowledgeBaseInfo object with
 * creation info (must not be NULL)
 * @param error_code Pointer to store error code (can be NULL)
 * @return Knowledge base ID string of the created knowledge base, or NULL on
 * failure
 *
 * @note This function is thread-safe
 * @note The knowledgebase object is not modified by this function
 * @note The returned ID can be used with other session functions
 * @note This operation creates persistent storage on disk
 * @see kb_knowledgebase_info_new(), kb_knowledgebase_info_delete(),
 * kb_free_string()
 *
 * @example
 * // Create a knowledge base object with desired properties
 * KnowledgeBaseInfo *kb = kb_knowledgebase_info_new("My Knowledge Base",
 * "A comprehensive knowledge base for AI");
 * if (kb) {
 *    int error;
 *    char *kb_id = kb_knowledgebase_create(session, kb, &error);
 *    if (kb_id) {
 *       printf("Knowledge base created on disk with ID: %s\n", kb_id);
 *       kb_free_string(kb_id);
 *    } else {
 *       printf("Failed to create knowledge base: %s\n",
 *       kb_get_error_message(error));
 *    }
 *    kb_knowledgebase_info_free(kb);
 * }
 */

KB_API char *kb_knowledgebase_create(
    KBSession *session, const KnowledgeBaseInfo *knowledgebase_info,
    int *error_code);

/**
 * @brief Delete a knowledge base from physical disk
 *
 * Permanently removes a knowledge base from the physical disk storage,
 * including all associated data, documents, metadata, and directory structures.
 * This operation is irreversible and will completely erase the knowledge base
 * and all its contents from the disk.
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_id The ID of the knowledge base to delete (must not be
 * NULL)
 * @return Error code indicating success or failure
 *
 * @note This function is thread-safe
 * @note This operation permanently deletes data from disk - use with caution
 * @note All documents and associated data will be lost
 * @note Any active operations on this knowledge base should be completed before
 * deletion
 * @see kb_knowledgebase_info_create()
 *
 * @example
 * // Delete a knowledge base permanently
 * int error = kb_knowledgebase_info_delete(session, "kb_12345", NULL);
 * if (error == KB_SUCCESS) {
 *     printf("Knowledge base successfully deleted from disk\n");
 * } else {
 *     printf("Failed to delete knowledge base: %s\n",
 * kb_get_error_message(error));
 * }
 */
KB_API int kb_knowledgebase_delete(KBSession *session,
                                   const char *knowledgebase_id) KB_NONNULL;

/**
 * @brief Get a knowledge base instance by ID
 *
 * Retrieves a knowledge base object by its ID from the session.
 * The caller is responsible for freeing the returned object using
 * kb_knowledgebase_info_free().
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_id The ID of the knowledge base (must not be NULL)
 * @param error_code Pointer to store error code (can be NULL)
 * @return Knowledge base object, or NULL on failure
 *
 * @note This function is thread-safe
 * @see kb_knowledgebase_info_free()
 *
 * @example
 * KnowledgeBaseInfo *kb_info = kb_knowledgebase_info_get(session, "kb_12345",
 * &error);
 * if (error == KB_SUCCESS) {
 *   printf("Knowledge base name:
 *          %s\n",kb_knowledgebase_info_get_name(kb_info));
 *   kb_knowledgebase_info_free(kb_info);
 * }
 */
KB_API KnowledgeBaseInfo *kb_knowledgebase_info_get(
    KBSession *session, const char *knowledgebase_id, int *error_code);

/**
 * @brief Get a list of all knowledge bases in the session
 *
 * Retrieves a list of all knowledge bases in the session.
 * The caller is responsible for freeing the returned array using
 * kb_knowledgebase_info_array_free().
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param count Pointer to store the number of knowledge bases (must not be
 * NULL)
 * @param error_code Pointer to store error code (can be NULL)
 * @return Array of knowledge base objects, or NULL on failure
 *
 * @note This function is thread-safe
 * @see kb_knowledgebase_info_array_free()
 *
 * @example
 * int count;
 * int error;
 * KnowledgeBaseInfo **kb_list = kb_knowledgebase_info_list(session, &count,
 * &error);
 * if (error == KB_SUCCESS) {
 *     kb_knowledgebase_info_array_free(kb_list, count);
 */
KB_API KnowledgeBaseInfo **kb_knowledgebase_info_list(KBSession *session,
                                                      int *count,
                                                      int *error_code);

/**
 * @brief Update a knowledge base information
 *
 * Updates the information of a knowledge base.
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_info Pointer to the KnowledgeBaseInfo object with update
 * info (must not be NULL)
 * @return Error code indicating success or failure
 */
KB_API int kb_knowledgebase_info_update(
    KBSession *session, const KnowledgeBaseInfo *knowledgebase_info) KB_NONNULL;

/**
 * @brief Process a document in a knowledge base
 *
 * Processes a document file in the specified knowledge base, including parsing,
 * chunking, vectorization, and indexing to make it searchable.
 * The caller is responsible for freeing the returned document ID using
 * kb_free_string().
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_id Knowledge base ID string (must not be NULL)
 * @param document_path Path to the document file (must not be NULL)
 * @param document_id Document ID string (must not be NULL)
 * @param reserved Reserved parameter for future use (must be NULL)
 * @return Error code indicating success or failure
 *
 * @note This function is thread-safe
 * @note The document file must be accessible and readable
 * @note The document will go through multiple processing stages:
 *       parsing, chunking, vectorization, and indexing
 * @see kb_document_get_processing_progress()
 */

KB_API int kb_document_add(KBSession *session, const char *knowledgebase_id,
                           const char *document_path, const char *document_id,
                           void *reserved);

/**
 * @brief Process multiple documents in a knowledge base
 *
 * Processes multiple document files in the specified knowledge base, including
 * parsing, chunking, vectorization, and indexing to make them searchable.
 * The caller is responsible for freeing the returned document ID array using
 * kb_document_list_destroy().
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_id Knowledge base ID string (must not be NULL)
 * @param document_paths Array of document file paths (must not be NULL)
 * @param document_ids Array of document ID strings (must not be NULL)
 * @param docs_count Number of documents in the array (must be > 0)
 * @param reserved Reserved parameter for future use (must be NULL)
 * @param error_codes Array of error pointers to store error codes
 * @return Error code indicating success or failure
 *
 * @note This function is thread-safe
 * @note The document files must be accessible and readable
 * @note Each document will go through multiple processing stages:
 *       parsing, chunking, vectorization, and indexing
 * @see kb_document_list_destroy(), kb_document_get_processing_progress()
 */
KB_API int kb_document_add_multi(KBSession *session,
                                 const char *knowledgebase_id,
                                 const char **document_paths,
                                 const char **document_ids, int docs_count,
                                 void *reserved, int **error_codes);

/**
 * @brief Get a list of all documents in a knowledge base
 *
 * Retrieves a list of all documents in the specified knowledge base.
 * The caller is responsible for freeing the returned array using
 * kb_free_string_array().
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_id Knowledge base ID string (must not be NULL)
 * @param count Pointer to store the number of documents (must not be NULL)
 * @param error_code Pointer to store error code (can be NULL)
 * @return Array of document ID strings, or NULL on failure
 *
 * @note This function is thread-safe
 * @see kb_free_string_array()
 *
 * @example
 * int count;
 * int error;
 * char **documents = kb_document_list(session, kb_id, &count, &error);
 * if (error == KB_SUCCESS) {
 *     kb_free_string_array(documents, count);
 * } else {
 *     printf("Failed to get documents: %s\n", kb_get_error_message(error));
 */
KB_API char **kb_document_list(KBSession *session, const char *knowledgebase_id,
                               int *count, int *error_code);

/**
 * @brief Delete a document from a knowledge base
 *
 * Removes a document from the specified knowledge base.
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_id Knowledge base ID string (must not be NULL)
 * @param document_id Document ID string (must not be NULL)
 * @return Error code indicating success or failure
 *
 * @note This function is thread-safe
 */
KB_API int kb_document_delete(KBSession *session, const char *knowledgebase_id,
                              const char *document_id) KB_NONNULL;

/**
 * @brief Delete multiple documents from a knowledge base
 *
 * Removes multiple documents from the specified knowledge base.
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_id Knowledge base ID string (must not be NULL)
 * @param document_ids Array of document ID strings (must not be NULL)
 * @param docs_count Number of documents in the array (must be > 0)
 * @param error_codes Array of error pointers to store error codes
 * @return Error code indicating success or failure
 *
 * @note This function is thread-safe
 */
KB_API int kb_document_delete_multi(KBSession *session,
                                    const char *knowledgebase_id,
                                    const char **document_ids, int docs_count,
                                    int **error_codes);

/**
 * @brief Cancel document addition operations
 *
 * Cancels ongoing document addition operations for one or more documents in the
 * specified knowledge base. This function allows external control over document
 * processing workflows and is useful when interrupting long-running document
 * addition tasks is needed.
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_id Knowledge base ID string (must not be NULL)
 * @param document_ids Array of document ID strings (must not be NULL)
 * @param docs_count Number of documents in the array (must be > 0)
 * @return Error code indicating success or failure
 *
 * @note This function is thread-safe
 * @note This operation cancels the document addition process rather than
 * removing already processed documents
 */
KB_API int kb_document_cancel_addition(KBSession *session,
                                       const char *knowledgebase_id,
                                       const char **document_ids,
                                       int docs_count) KB_NONNULL;

/**
 * @brief Get document processing progress
 *
 * Retrieves the current processing progress for a document in the knowledge
 * base. Returns the processing status as an integer value.
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase Pointer to the knowledge base instance (must not be
 * NULL)
 * @param document_id Document ID string (must not be NULL)
 * @param status Pointer to store document processing status (must not be NULL)
 * @param error_code Pointer to store error code (can be NULL)
 * @return Progress percentage (0-100), or -1 on failure
 *
 * @note This function is thread-safe
 * @note Check error_code for detailed error information on failure
 * @note The status value corresponds to KBDocumentStatus enumeration values:
 *       - 0: KB_DOC_STATUS_PENDING - 等待中 - Document is pending processing
 *       - 1: KB_DOC_STATUS_PARSING - 文档解析中 - Document is being parsed
 *       - 2: KB_DOC_STATUS_INDEXING - 索引建立中 - Document index is being
 * built
 *       - 3: KB_DOC_STATUS_COMPLETED - 已完成 - Document processing completed
 * successfully
 *       - 4: KB_DOC_STATUS_ERROR - 出错 - Document processing failed
 * @see KBDocumentStatus, kb_get_document_status_name()
 *
 * @example
 * int status;
 * int error;
 *
 * int progress = kb_document_get_processing_progress(session, kb, doc_id,
 * &status, &error); if (progress >= 0) { printf("Document processing: %s
 * (%d%%)\n", kb_get_document_status_name((KBDocumentStatus)status), progress);
 *
 *     if (status == KB_DOC_STATUS_COMPLETED) {
 *         printf("Document is ready for search!\n");
 *     } else if (status == KB_DOC_STATUS_ERROR) {
 *         printf("Document processing failed\n");
 *     }
 * } else {
 *     printf("Failed to get document processing progress: %s\n",
 *            kb_get_error_message(error));
 * }
 */
KB_API int kb_document_get_processing_progress(KBSession *session,
                                               KnowledgeBaseInfo *knowledgebase,
                                               const char *document_id,
                                               int *status, int *error_code);

/* ========================================================================
 * Retrieval API
 *
 * Provides direct retrieval capability without chat streaming.
 * ======================================================================== */

/**
 * @brief Retrieval result callback
 *
 * The SDK will invoke this callback asynchronously when retrieval finishes.
 * The response pointer is managed by SDK; valid only during callback.
 */
typedef void (*KBRetrievalCallback)(const KBRetrievalResponse *response,
                                    void *user_data);

/**
 * @brief Retrieve relevant knowledge base content for a query
 *
 * Executes a retrieval request against the specified knowledge base and
 * delivers matched content blocks asynchronously via callback.
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param knowledgebase_id Knowledge base ID string (must not be NULL)
 * @param request_id Request ID string (can be NULL)
 * @param query Query text to retrieve (must not be NULL or empty)
 * @param config Retrieval configuration (can be NULL for defaults)
 * @param callback Callback to receive retrieval response (must not be NULL)
 * @param user_data User-provided pointer passed to callback (can be NULL)
 * @return Error code indicating immediate request submission result
 */
KB_API int kb_retrieval(KBSession *session, const char *knowledgebase_id,
                        const char *request_id, const char *query,
                        const KBRetrievalConfig *config,
                        KBRetrievalCallback callback, void *user_data);

/* ========================================================================
 * Chat and Dialogue API
 *
 * These functions provide chat and dialogue capabilities with knowledge bases.
 * Supports streaming mode for real-time responses.
 * ======================================================================== */

/**
 * @brief Stream callback function type for chat operations
 *
 * This callback is invoked for each chunk of streaming chat response.
 *
 * @param response Chat response for current chunk
 * @param user_data User-provided data pointer
 * @return 0 to continue streaming, non-zero to stop
 *
 * @note This callback may be called multiple times for a single chat request
 * @note The response pointer is only valid during the callback execution
 */
typedef int (*KBChatStreamCallback)(const KBChatResponse *response,
                                    void *user_data);

/**
 * @brief Stream chat with knowledge base
 *
 * Initiates a streaming chat session with the specified knowledge base.
 * The response is delivered via callback function as streaming chunks.
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param request_id Request identifier for tracking (can be NULL for
 * auto-generation)
 * @param knowledgebase_id Knowledge base ID string (must not be NULL)
 * @param question Question to ask (must not be NULL)
 * @param config Chat configuration with parameters (can be NULL for defaults)
 * @param callback Callback function to handle streaming response (must not be
 * NULL)
 * @param user_data User-provided data pointer passed to callback (can be NULL)
 * @return Error code indicating success or failure
 *
 * @note This function is thread-safe
 * @note If request_id is NULL, the system will generate a unique identifier
 * @note The actual request_id used can be retrieved from the response via
 *       kb_chat_response_get_request_id()
 * @note Request ID should be unique within the session scope
 * @note Maximum request_id length is 128 characters
 * @note During streaming, only the result->response field has values in
 *       intermediate chunks. Other fields (history, source, cost) are returned
 *       in the final chunk.
 * @note The callback should return 0 to continue streaming, non-zero to stop.
 * @see kb_generate_request_id()
 * @see kb_is_valid_request_id()
 *
 * @example
 * int chat_callback(const KBChatResponse *response, void *user_data) {
 *     const KBChatResult *result = kb_chat_response_get_result(response);
 *     if (result) {
 *         const char *text = kb_chat_result_get_response(result);
 *         if (text) {
 *             printf("AI: %s", text);
 *             fflush(stdout);
 *         }
 *     }
 *     return 0; // Continue streaming
 * }
 *
 * // Chat with auto-generated request ID
 * int error = kb_chat_stream(session, NULL, "kb_001", "Hello", NULL,
 *                               chat_callback, NULL);
 *
 * // Or with custom request ID
 * char *req_id = kb_generate_request_id();
 * int error = kb_chat_stream(session, req_id, "kb_001", "Hello", NULL,
 *                               chat_callback, NULL);
 * kb_free_string(req_id);
 */

KB_API int kb_chat_stream(KBSession *session, const char *request_id,
                          const char *knowledgebase_id, const char *question,
                          const KBChatHistory *history,
                          const KBChatConfig *config,
                          KBChatStreamCallback callback, void *user_data);

/**
 * @brief Stop streaming chat conversation
 *
 * Stops an active streaming chat conversation by request ID.
 * This function allows external control over streaming operations.
 *
 * @param session Pointer to the session instance (must not be NULL)
 * @param request_id Request identifier of the streaming conversation to stop
 * (must not be NULL)
 * @return Error code indicating success or failure
 *
 * @note This function is thread-safe
 * @note The function returns immediately, the actual stop may take a moment
 * @note If the request_id is not found or the conversation is already finished,
 *       the function will return KB_ERROR_CHAT_REQUEST_NOT_FOUND
 * @note After stopping, the streaming callback will not be called anymore
 * @note The partial response data up to the stop point is still valid
 * @note Maximum request_id length is 128 characters
 * @see kb_chat_stream()
 * @see kb_is_valid_request_id()
 *
 * @example
 * // Start streaming chat
 * char *req_id = kb_generate_request_id();
 * int error = kb_chat_stream(session, req_id, "kb_001", "Long question",
 * NULL, chat_callback, NULL);
 *
 * // Stop streaming from another thread or signal handler
 * if (user_wants_to_stop) {
 *     kb_chat_stream_stop(session, req_id);
 * }
 *
 * kb_free_string(req_id);
 */
KB_API int kb_chat_stream_stop(KBSession *session, const char *request_id);

#ifdef __cplusplus
}
#endif

#endif /* KNOWLEDGEBASE_SESSION_H */
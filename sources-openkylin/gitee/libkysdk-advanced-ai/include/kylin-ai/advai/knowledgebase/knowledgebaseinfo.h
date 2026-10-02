#ifndef KNOWLEDGEBASE_H
#define KNOWLEDGEBASE_H

#include "error.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file knowledgebase.h
 * @brief Knowledge Base Core API for AI SDK
 * @version 1.0.0
 * @author Development Team
 *
 * This header provides the core knowledge base data structure and operations
 * for creating, managing, and manipulating knowledge base instances.
 * All functions are thread-safe unless otherwise noted.
 */

/* Forward declarations */
typedef struct KnowledgeBaseInfo KnowledgeBaseInfo;

/**
 * @brief Create a new knowledge base instance
 *
 * Creates a new knowledge base object with the specified name and description.
 * The caller is responsible for freeing the returned object using
 * kb_knowledgebase_info_free().
 *
 * @param name The name of the knowledge base (must not be NULL)
 * @param description Optional description of the knowledge base (can be NULL)
 * @return Pointer to the new KnowledgeBaseInfo instance, or NULL on failure
 *
 * @note This function is thread-safe
 * @see kb_knowledgebase_info_free()
 */
KB_API KnowledgeBaseInfo *kb_knowledgebase_info_new(const char *name,
                                                    const char *description);

/**
 * @brief Free a knowledge base instance
 *
 * Frees all resources associated with the knowledge base object.
 *
 * @param knowledgebase Pointer to the knowledge base instance (can be NULL)
 *
 * @note This function is thread-safe
 */
KB_API void kb_knowledgebase_info_free(KnowledgeBaseInfo *knowledgebase);

/**
 * @brief Get the name of a knowledge base
 *
 * Returns the name of the specified knowledge base.
 * The returned string is owned by the knowledge base instance and should not
 * be freed by the caller.
 *
 * @param knowledgebase Pointer to the knowledge base instance (must not be
 * NULL)
 * @return Name string, or NULL on failure
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_knowledgebase_info_get_name(
    const KnowledgeBaseInfo *knowledgebase) KB_NONNULL;

/**
 * @brief Get the description of a knowledge base
 *
 * Returns the description of the specified knowledge base.
 * The returned string is owned by the knowledge base instance and should not
 * be freed by the caller.
 *
 * @param knowledgebase Pointer to the knowledge base instance (must not be
 * NULL)
 * @return Description string, or NULL on failure
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_knowledgebase_info_get_description(
    const KnowledgeBaseInfo *knowledgebase) KB_NONNULL;

/**
 * @brief Get the ID of a knowledge base
 *
 * Returns the unique ID of the specified knowledge base.
 * The returned string is owned by the knowledge base instance and should not
 * be freed by the caller.
 *
 * @param knowledgebase Pointer to the knowledge base instance (must not be
 * NULL)
 * @return ID string, or NULL on failure
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_knowledgebase_info_get_id(
    const KnowledgeBaseInfo *knowledgebase) KB_NONNULL;

/**
 * @brief Set the knowledgebase_id of a knowledge base
 *
 * Updates the name of the knowledge base instance.
 *
 * @param knowledgebase Pointer to the knowledge base instance (must not be
 * NULL)
 * @param id The id (must not be NULL)
 *
 * @note This function is thread-safe
 */
KB_API void kb_knowledgebase_info_set_id(KnowledgeBaseInfo *knowledgebase,
                                         const char *id) KB_NONNULL;

/**
 * @brief Set the name of a knowledge base
 *
 * Updates the name of the knowledge base instance.
 *
 * @param knowledgebase Pointer to the knowledge base instance (must not be
 * NULL)
 * @param name The new name (must not be NULL)
 *
 * @note This function is thread-safe
 */
KB_API void kb_knowledgebase_info_set_name(KnowledgeBaseInfo *knowledgebase,
                                           const char *name) KB_NONNULL;

/**
 * @brief Set the description of a knowledge base
 *
 * Updates the description of the knowledge base instance.
 *
 * @param knowledgebase Pointer to the knowledge base instance (must not be
 * NULL)
 * @param description The new description (can be NULL)
 *
 * @note This function is thread-safe
 */
KB_API void kb_knowledgebase_info_set_description(
    KnowledgeBaseInfo *knowledgebase, const char *description) KB_NONNULL;

/* ========================================================================
 * Knowledge Base Array Management Functions
 * ======================================================================== */

/**
 * @brief Create a new knowledge base info array
 *
 * Allocates memory for an array of KnowledgeBaseInfo pointers with the
 * specified capacity. The array is initialized with NULL pointers. The caller
 * is responsible for freeing the returned array using
 * kb_knowledgebase_info_array_free().
 *
 * @param capacity The number of KnowledgeBaseInfo pointers the array can hold
 * (must be > 0)
 * @return Pointer to the new KnowledgeBaseInfo array, or NULL on failure
 *
 * @note This function is thread-safe
 * @note All array elements are initialized to NULL
 * @note The array capacity is fixed at creation time
 * @see kb_knowledgebase_info_array_free()
 *
 * @example
 * KBError error;
 * KnowledgeBaseInfo **kb_array = kb_knowledgebase_info_array_new(10);
 * if (kb_array != NULL) {
 *     // Use the array
 *     kb_array[0] = kb_knowledgebase_info_new("KB1", "Description1");
 *     kb_array[1] = kb_knowledgebase_info_new("KB2", "Description2");
 *     // ...
 *     kb_knowledgebase_info_array_free(kb_array, 10);
 * }
 */
KB_API KnowledgeBaseInfo **kb_knowledgebase_info_array_new(int capacity);

/**
 * @brief Free a knowledge base info array
 *
 * Frees all resources associated with a knowledge base info array.
 * This function will free each non-NULL KnowledgeBaseInfo instance in the array
 * and then free the array itself.
 *
 * @param knowledgebase_array Pointer to the knowledge base array (can be NULL)
 * @param count Number of elements in the array
 *
 * @note This function is thread-safe
 * @note This function will call kb_knowledgebase_info_free() for each non-NULL
 * element
 * @note It is safe to pass NULL for knowledgebase_array
 * @see kb_knowledgebase_info_array_new()
 *
 * @example
 * KnowledgeBaseInfo **kb_array = kb_knowledgebase_info_array_new(3, NULL);
 * if (kb_array != NULL) {
 *     kb_array[0] = kb_knowledgebase_info_new("KB1", "Description1");
 *     kb_array[1] = kb_knowledgebase_info_new("KB2", "Description2");
 *     // kb_array[2] remains NULL
 *
 *     // Free the array and all its elements
 *     kb_knowledgebase_info_array_free(kb_array, 3);
 * }
 */
KB_API void kb_knowledgebase_info_array_free(
    KnowledgeBaseInfo **knowledgebase_info_array, int count);

#ifdef __cplusplus
}
#endif

#endif /* KNOWLEDGEBASE_H */
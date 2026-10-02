#ifndef KB_TYPES_H
#define KB_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file types.h
 * @brief Common Types and Error Definitions for Knowledge Base SDK
 * @version 1.0.0
 * @author Development Team
 *
 * This header provides common type definitions, error codes, constants,
 * and utility macros used throughout the Knowledge Base SDK. It also
 * includes platform-specific API declarations and compiler attributes.
 */

/* API attributes */
#ifdef _WIN32
#ifdef KB_EXPORTS
#define KB_API __declspec(dllexport)
#else
#define KB_API __declspec(dllimport)
#endif
#else
#define KB_API __attribute__((visibility("default")))
#endif

/* Compiler attributes */
#ifdef __GNUC__
#define KB_DEPRECATED __attribute__((deprecated))
#define KB_UNUSED __attribute__((unused))
#define KB_NONNULL __attribute__((nonnull))
#else
#define KB_DEPRECATED
#define KB_UNUSED
#define KB_NONNULL
#endif

/* Boolean type for C89 compatibility */
#ifndef __cplusplus
#ifndef bool
#define bool int
#define true 1
#define false 0
#endif
#endif

/* ========================================================================
 * Document Processing Status Definitions
 * ======================================================================== */

/**
 * @brief Document processing status enumeration
 *
 * Defines the various stages of document processing within the knowledge base.
 * Documents go through multiple phases from upload to being ready for search.
 */
typedef enum {
    KB_DOC_STATUS_PENDING = 0, /**< 等待中 - Document is pending processing */
    KB_DOC_STATUS_PARSING = 1, /**< 文档解析中 - Document is being parsed */
    KB_DOC_STATUS_EMBEDDING = 2, /**< 向量嵌入中 - Document is being embedded */
    KB_DOC_STATUS_INDEXING =
        3, /**< 索引建立中 - Document index is being built */
    KB_DOC_STATUS_COMPLETED =
        4, /**< 已完成 - Document processing completed successfully */
    KB_DOC_STATUS_ERROR = 5, /**< 出错 - Document processing failed */
} KBDocumentStatus;

/**
 * @brief Free a string returned by SDK functions
 *
 * Frees memory allocated for strings returned by SDK functions.
 * This function should be used to free all strings returned by SDK functions
 * that allocate memory.
 *
 * @param str Pointer to the string to free (can be NULL)
 *
 * @note This function is thread-safe
 */
KB_API void kb_free_string(char *str);

/**
 * @brief Free an array of strings returned by SDK functions
 *
 * Frees memory allocated for string arrays returned by SDK functions.
 * This function frees both the individual strings and the array itself.
 *
 * @param str_array Array of string pointers to free (can be NULL)
 * @param count Number of strings in the array
 *
 * @note This function is thread-safe
 */
KB_API void kb_free_string_array(char **str_array, int count);

#ifdef __cplusplus
}
#endif

#endif /* KB_TYPES_H */
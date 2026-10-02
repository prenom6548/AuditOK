#ifndef ERROR_H
#define ERROR_H

#include <common/error.h>

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
typedef enum {
    /* Session errors (15000-15099) */
    KB_ERROR_APP_ID_INVALID = 15000,        /**< Invalid application ID */
    KB_ERROR_PASSWORD_INVALID = 15001,      /**< Invalid password */
    KB_ERROR_DATABASE_PATH_INVALID = 15002, /**< Invalid database path */
    /* Reserved: 15003-15099 for future session errors */

    /* Knowledge base errors (15100-15199) */
    KB_ERROR_KB_NOT_FOUND = 15100,      /**< Knowledge base not found */
    KB_ERROR_KB_ACCESS_DENIED = 15101,  /**< Knowledge base access denied */
    KB_ERROR_KB_CORRUPTED = 15102,      /**< Knowledge base corrupted */
    KB_ERROR_KB_ALREADY_EXISTS = 15103, /**< Knowledge base already exists */
    KB_ERROR_KB_EMPTY = 15104,          /**< Knowledge base is empty */
    /* Reserved: 15105-15199 for future KB errors */

    /* Document errors (15200-15299) */
    KB_ERROR_DOC_NOT_FOUND = 15200,      /**< Document not found */
    KB_ERROR_DOC_TOO_LARGE = 15201,      /**< Document too large */
    KB_ERROR_DOC_INVALID_FORMAT = 15202, /**< Document format not supported */
    KB_ERROR_DOC_ACCESS_DENIED = 15203,  /**< Document access denied */
    KB_ERROR_DOC_CORRUPTED = 15204,      /**< Document corrupted */
    KB_ERROR_DOC_EMPTY = 15205,          /**< Document is empty */
    KB_ERROR_DOC_ID_INVALID = 15206,     /**< Document ID is invalid */
    KB_ERROR_DOC_ID_NOT_FOUND = 15207,   /**< Document ID not found */
    KB_ERROR_DOC_ID_ALREADY_EXISTS = 15208, /**< Document ID already exists */
    KB_ERROR_DOC_BATCH_PARTIAL_FAILED =
        15209,                             /**< Document batch partial failed */
    KB_ERROR_DOC_BATCH_ALL_FAILED = 15210, /**< Document batch all failed */
    KB_ERROR_DOC_PARSE_FAILED = 15211,     /**< Document processing failed */

    /* Reserved: 15212-15299 for future document errors */

    /* Retrieval errors (15300-15399) */
    KB_ERROR_RETRIEVAL_FAILED = 15300,        /**< Retrieval operation failed */
    KB_ERROR_RETRIEVAL_NO_RESULTS = 15301,    /**< No results found */
    KB_ERROR_RETRIEVAL_INVALID_QUERY = 15302, /**< Invalid query */
    KB_ERROR_RETRIEVAL_TIMEOUT = 15303,       /**< Retrieval timeout */
    KB_ERROR_RETRIEVAL_FILEID_FILTER_INVALID =
        15304, /**< Invalid file ID filter */
    /* Reserved: 15305-15399 for future retrieval errors */

    /* Configuration errors (15400-15499) */
    KB_ERROR_CONFIG_INVALID = 15400, /**< Invalid configuration */
    KB_ERROR_CONFIG_MISSING = 15401, /**< Missing configuration */
    /* Reserved: 15402-15499 for future config errors */

    /* Future expansion (15500-15599) */
    /* Reserved: 15500-15599 for future error categories */
} KBErrorCode;

/**
 * @brief Get error message for an error code
 *
 * Returns a human-readable error message for the given error code.
 * The returned string is statically allocated and should not be freed.
 *
 * @param error_code The error code
 * @return Error message string (never NULL)
 *
 * @note This function is thread-safe
 */
KB_API const char *kb_get_error_message(int error_code);

#ifdef __cplusplus
}
#endif

#endif  // ERROR_H

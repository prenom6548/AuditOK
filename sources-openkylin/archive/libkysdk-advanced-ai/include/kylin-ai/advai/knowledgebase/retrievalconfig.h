#ifndef RETRIEVAL_CONFIG_H
#define RETRIEVAL_CONFIG_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file retrievalconfig.h
 * @brief Retrieval configuration for knowledge base queries
 * @version 1.0.0
 */

typedef enum {
    KB_RETRIEVAL_METHOD_SEMANTIC = 0,
    KB_RETRIEVAL_METHOD_TEXT = 1,
    KB_RETRIEVAL_METHOD_KEYWORD = 2,
    KB_RETRIEVAL_METHOD_HYBRID = 3, /* semantic + text + keyword (default)*/
} KBRetrievalMethod;

typedef enum {
    KB_RERANK_METHOD_RRF = 0,     /* reciprocal rank fusion */
    KB_RERANK_METHOD_WEIGHT = 1,  /* weighted fusion (default) */
} KBRerankMethod;

typedef struct KBRetrievalConfig KBRetrievalConfig;

/**
 * @brief Create a retrieval configuration
 */
KB_API KBRetrievalConfig *kb_retrieval_config_new();

/**
 * @brief Set document ids to limit retrieval scope
 *
 * If no documents are set, retrieval searches the whole knowledge base.
 * Passing NULL or zero count clears previous document filters.
 */
KB_API void kb_retrieval_config_set_doc_ids(KBRetrievalConfig *config,
                                            const char **doc_ids,
                                            int doc_ids_count) KB_NONNULL;

/**
 * @brief Set retrieval topK value (number of results to return)
 *
 * Values less than or equal to zero are ignored and will not override the
 * default behavior.
 */
KB_API void kb_retrieval_config_set_top_k(KBRetrievalConfig *config,
                                          int top_k) KB_NONNULL;

/**
 * @brief Set retrieval method
 *
 * Default is KB_RETRIEVAL_METHOD_HYBRID when not specified.
 */
KB_API void kb_retrieval_config_set_retrieval_method(
    KBRetrievalConfig *config, KBRetrievalMethod method) KB_NONNULL;

/**
 * @brief Set rerank method
 *
 * Default is KB_RERANK_METHOD_WEIGHT when not specified.
 */
KB_API void kb_retrieval_config_set_rerank_method(
    KBRetrievalConfig *config, KBRerankMethod method) KB_NONNULL;

/**
 * @brief Destroy retrieval configuration
 */
KB_API void kb_retrieval_config_free(KBRetrievalConfig *config);

#ifdef __cplusplus
}
#endif

#endif  // RETRIEVAL_CONFIG_H

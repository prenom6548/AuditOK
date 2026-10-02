#include "_retrievalconfig.h"

namespace {
bool isValidRetrievalMethod(KBRetrievalMethod method) {
    return method == KB_RETRIEVAL_METHOD_SEMANTIC ||
           method == KB_RETRIEVAL_METHOD_KEYWORD ||
           method == KB_RETRIEVAL_METHOD_HYBRID;
}

bool isValidRerankMethod(KBRerankMethod method) {
    return method == KB_RERANK_METHOD_RRF || method == KB_RERANK_METHOD_WEIGHT;
}
}  // namespace

KBRetrievalConfig *kb_retrieval_config_new() { return new KBRetrievalConfig(); }

void kb_retrieval_config_set_doc_ids(KBRetrievalConfig *config,
                                     const char **doc_ids, int doc_ids_count) {
    if (config == nullptr) {
        return;
    }
    // Empty input resets to search the whole knowledge base.
    if (doc_ids == nullptr || doc_ids_count <= 0) {
        config->docIds.clear();
        return;
    }
    config->docIds.clear();
    for (int i = 0; i < doc_ids_count; i++) {
        if (doc_ids[i] != nullptr) {
            config->docIds.emplace_back(doc_ids[i]);
        }
    }
}

void kb_retrieval_config_set_top_k(KBRetrievalConfig *config, int top_k) {
    if (config == nullptr || top_k <= 0) {
        return;
    }
    config->topK = top_k;
    config->topKSet = true;
}

void kb_retrieval_config_set_retrieval_method(KBRetrievalConfig *config,
                                              KBRetrievalMethod method) {
    if (config == nullptr || !isValidRetrievalMethod(method)) {
        return;
    }
    config->retrievalMethod =
        static_cast<kyai::advancedai::knowledgebase::core::RetrievalMethod>(
            method);
}

void kb_retrieval_config_set_rerank_method(KBRetrievalConfig *config,
                                           KBRerankMethod method) {
    if (config == nullptr || !isValidRerankMethod(method)) {
        return;
    }
    config->rerankMethod =
        static_cast<kyai::advancedai::knowledgebase::core::RerankMethod>(
            method);
}

void kb_retrieval_config_free(KBRetrievalConfig *config) {
    if (config == nullptr) {
        return;
    }
    delete config;
}

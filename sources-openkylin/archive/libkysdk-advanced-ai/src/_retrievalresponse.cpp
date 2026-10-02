#include "_retrievalresponse.h"

const char *kb_retrieval_response_get_request_id(
    const KBRetrievalResponse *response) {
    if (response == nullptr) {
        return nullptr;
    }
    return response->requestId.c_str();
}

const char *kb_retrieval_response_get_knowledgebase_id(
    const KBRetrievalResponse *response) {
    if (response == nullptr) {
        return nullptr;
    }
    return response->knowledgeBaseId.c_str();
}

const char *kb_retrieval_response_get_query(
    const KBRetrievalResponse *response) {
    if (response == nullptr) {
        return nullptr;
    }
    return response->query.c_str();
}

int kb_retrieval_response_get_error_code(const KBRetrievalResponse *response) {
    if (response == nullptr) {
        return -1;
    }
    return response->errorCode;
}

int kb_retrieval_response_get_source_count(
    const KBRetrievalResponse *response) {
    if (response == nullptr) {
        return 0;
    }
    return static_cast<int>(response->source.size());
}

const KBChatSource *kb_retrieval_response_get_source(
    const KBRetrievalResponse *response, int index) {
    if (response == nullptr || index < 0 ||
        index >= static_cast<int>(response->source.size())) {
        return nullptr;
    }
    return response->source[static_cast<size_t>(index)];
}

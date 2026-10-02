#ifndef RETRIEVAL_RESPONSE_INTERNAL_H_
#define RETRIEVAL_RESPONSE_INTERNAL_H_

#include <string>
#include <vector>

#include "_chatsource.h"
#include "retrievalresponse.h"

struct KBRetrievalResponse {
    std::string knowledgeBaseId;
    std::string requestId;
    std::string query;
    int errorCode{0};
    std::vector<KBChatSource *> source;

    ~KBRetrievalResponse() {
        for (auto *item : source) {
            delete item;
        }
        source.clear();
    }
};

#endif

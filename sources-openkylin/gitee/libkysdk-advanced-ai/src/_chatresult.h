#ifndef CHAT_RESULT_H_
#define CHAT_RESULT_H_

#include <memory>
#include <string>
#include <vector>

#include "_chatreference.h"
#include "_chatsource.h"
#include "chatresult.h"

struct KBChatResult {
    bool isFinished;
    std::string model;
    std::string question;
    std::string response;
    std::string reasoningContent;
    std::vector<KBChatSource*> source;
    std::vector<const char*> referenceFileIds;
    std::vector<KBChatReference*> reference;

    ~KBChatResult() {
        for (auto source : source) {
            delete source;
        }
        for (auto reference : reference) {
            delete reference;
        }
        for (auto fileId : referenceFileIds) {
            delete[] fileId;
        }
        source.clear();
        reference.clear();
        referenceFileIds.clear();
    }
};

#endif
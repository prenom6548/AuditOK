#ifndef CHAT_RESPONSE_H_
#define CHAT_RESPONSE_H_

#include <memory>
#include <string>

#include "_chatresult.h"
#include "chatresponse.h"

struct KBChatResponse {
    std::string chatId;
    std::string requestId;
    int errorCode;
    std::string errorMessage;
    std::string publicErrorMessage;
    KBChatResult* result;
    ~KBChatResponse() { delete result; }
};

#endif
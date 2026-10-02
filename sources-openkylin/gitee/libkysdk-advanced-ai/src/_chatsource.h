#ifndef CHAT_SOURCE_H_
#define CHAT_SOURCE_H_

#include <string>

#include "chatsource.h"

struct KBChatSource {
    std::string fileId;
    std::string content;
    double confidence;
};

#endif
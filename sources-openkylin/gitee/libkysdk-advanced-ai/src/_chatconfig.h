#ifndef CHAT_CONFIG_H_
#define CHAT_CONFIG_H_

#include <string>
#include <vector>

#include "chatconfig.h"

struct KBChatConfig {
    bool newChat;
    bool clearChatHistory;
    std::string model;
    std::vector<std::string> docIds;
    bool enableDiversifyQuery{true};
};
#endif
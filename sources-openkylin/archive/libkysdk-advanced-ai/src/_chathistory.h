#ifndef CHAT_HISTORY_H_
#define CHAT_HISTORY_H_

#include <string>
#include <vector>

#include "chathistory.h"

struct ChatTurn {
    std::string question;
    std::string response;
};

struct KBChatHistory {
    std::vector<ChatTurn> chatTurns;
};
#endif
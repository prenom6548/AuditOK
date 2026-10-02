#include "_chathistory.h"

#include <cstring>

#include "_chatresult.h"

KBChatHistory *kb_chat_history_new() { return new KBChatHistory(); }
int kb_chat_history_get_message_count(const KBChatHistory *history) {
    if (history == nullptr || history->chatTurns.empty()) {
        return 0;
    }
    return static_cast<int>(history->chatTurns.size());
}

int kb_chat_history_add_question(KBChatHistory *history, const char *question) {
    if (history == nullptr || question == nullptr || strlen(question) == 0) {
        return -1;
    }
    if (history->chatTurns.empty() ||
        (!history->chatTurns.back().question.empty() &&
         !history->chatTurns.back().response.empty())) {
        history->chatTurns.emplace_back();
    }
    history->chatTurns.back().question = std::string(question);
    return static_cast<int>(history->chatTurns.size() - 1);
}

int kb_chat_history_add_response(KBChatHistory *history, const char *response) {
    if (history == nullptr || response == nullptr || strlen(response) == 0) {
        return -1;
    }
    if (history->chatTurns.empty() ||
        (!history->chatTurns.back().question.empty() &&
         !history->chatTurns.back().response.empty())) {
        history->chatTurns.emplace_back();
    }
    history->chatTurns.back().response = response;
    return static_cast<int>(history->chatTurns.size() - 1);
}

bool kb_chat_history_delete_question(KBChatHistory *history, int index) {
    if (history == nullptr || index < 0 || index >= history->chatTurns.size()) {
        return false;
    }
    history->chatTurns[index].question.clear();
    return true;
}

bool kb_chat_history_delete_response(KBChatHistory *history, int index) {
    if (history == nullptr || index < 0 || index >= history->chatTurns.size()) {
        return false;
    }
    history->chatTurns[index].response.clear();
    return true;
}

const char *kb_chat_history_get_question(const KBChatHistory *history,
                                         const int index) {
    if (history == nullptr || index < 0 || index >= history->chatTurns.size()) {
        return nullptr;
    }
    return history->chatTurns[index].question.c_str();
}

const char *kb_chat_history_get_response(const KBChatHistory *history,
                                         const int index) {
    if (history == nullptr || index < 0 || index >= history->chatTurns.size()) {
        return nullptr;
    }
    return history->chatTurns[index].response.c_str();
}

void kb_chat_history_free(KBChatHistory *history) {
    if (history == nullptr) {
        return;
    }
    history->chatTurns.clear();
    delete history;
}
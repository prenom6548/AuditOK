#include "_chatsource.h"

const char *kb_chat_source_get_file_id(const KBChatSource *source) {
    if (source == nullptr) {
        return nullptr;
    }
    return source->fileId.c_str();
}

const char *kb_chat_source_get_content(const KBChatSource *source) {
    if (source == nullptr) {
        return nullptr;
    }
    return source->content.c_str();
}

double kb_chat_source_get_confidence(const KBChatSource *source) {
    if (source == nullptr) {
        return -1;
    }
    return source->confidence;
}
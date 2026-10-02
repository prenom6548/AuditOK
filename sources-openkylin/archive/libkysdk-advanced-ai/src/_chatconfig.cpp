#include "_chatconfig.h"

KBChatConfig *kb_chat_config_new() { return new KBChatConfig(); }

void kb_chat_config_set_new_chat(KBChatConfig *config, bool new_chat) {
    if (config == nullptr) {
        return;
    }
    config->newChat = new_chat;
}

void kb_chat_config_set_model(KBChatConfig *config, const char *model) {
    if (config != nullptr && model != nullptr) {
        config->model = model;
    }
}

void kb_chat_config_set_doc_ids(KBChatConfig *config, const char **doc_ids,
                                int doc_ids_count) {
    if (config == nullptr) {
        return;
    }
    if (doc_ids == nullptr || doc_ids_count <= 0) {
        config->docIds.clear();
        return;
    }
    config->docIds.clear();
    for (auto i = 0; i < doc_ids_count; i++) {
        if (doc_ids[i] != nullptr) {
            config->docIds.emplace_back(doc_ids[i]);
        }
    }
}

void kb_chat_config_set_diversify_query_enabled(KBChatConfig *config,
                                                bool enable) {
    if (config == nullptr) {
        return;
    }
    config->enableDiversifyQuery = enable;
}

void kb_chat_config_free(KBChatConfig *config) {
    if (config == nullptr) {
        return;
    }
    delete config;
}
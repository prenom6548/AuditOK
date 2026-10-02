#include "_chatresult.h"

bool kb_chat_result_is_finished(const KBChatResult *result) {
    if (result == nullptr) {
        return true;
    }
    return result->isFinished;
}

const char *kb_chat_result_get_question(const KBChatResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->question.c_str();
}

const char *kb_chat_result_get_response(const KBChatResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->response.c_str();
}

const char *kb_chat_result_get_reasoning_content(const KBChatResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->reasoningContent.c_str();
}

const KBChatSource **kb_chat_result_get_source(const KBChatResult *result,
                                               int *count) {
    if (result == nullptr || count == nullptr) {
        return nullptr;
    }
    *count = static_cast<int>(result->source.size());
    if (result->source.empty()) {
        return nullptr;
    }

    return const_cast<const KBChatSource **>(result->source.data());
}

const char *kb_chat_result_get_model(const KBChatResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->model.c_str();
}

const KBChatReference **kb_chat_result_get_reference(const KBChatResult *result,
                                                     int *count) {
    if (result == nullptr || count == nullptr) {
        return nullptr;
    }
    *count = static_cast<int>(result->reference.size());
    if (result->reference.empty()) {
        return nullptr;
    }
    return const_cast<const KBChatReference **>(result->reference.data());
}

const char **kb_chat_result_get_reference_file_ids(const KBChatResult *result,
                                                   int *count) {
    if (result == nullptr || count == nullptr) {
        return nullptr;
    }
    *count = static_cast<int>(result->referenceFileIds.size());
    if (result->referenceFileIds.empty()) {
        return nullptr;
    }
    return const_cast<const char **>(result->referenceFileIds.data());
}
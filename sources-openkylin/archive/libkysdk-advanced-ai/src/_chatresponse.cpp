#include "_chatresponse.h"

#include "_error.h"

const char *kb_chat_response_get_chat_id(const KBChatResponse *response) {
    if (response == nullptr) {
        return nullptr;
    }
    return response->chatId.c_str();
}

const char *kb_chat_response_get_request_id(const KBChatResponse *response) {
    if (response == nullptr) {
        return nullptr;
    }
    return response->requestId.c_str();
}

int kb_chat_response_get_error_code(const KBChatResponse *response) {
    if (response == nullptr) {
        return -1;
    }
    return response->errorCode;
}

const KBChatResult *kb_chat_response_get_result(
    const KBChatResponse *response) {
    if (response == nullptr) {
        return nullptr;
    }
    return response->result;
}

const char *kb_chat_response_get_error_message(const KBChatResponse *response) {
    if (response == nullptr) {
        return "";
    }
    return response->publicErrorMessage.c_str();
}
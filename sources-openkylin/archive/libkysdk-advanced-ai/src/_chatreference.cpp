#include "_chatreference.h"

int kb_chat_reference_get_file_number(const KBChatReference *reference) {
    if (reference == nullptr) {
        return -1;
    }
    return reference->fileNumber;
}

int kb_chat_reference_get_chunk_index(const KBChatReference *reference) {
    if (reference == nullptr) {
        return -1;
    }
    return reference->chunkIndex;
}
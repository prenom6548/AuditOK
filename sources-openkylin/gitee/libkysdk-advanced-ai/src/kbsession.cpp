#include "_error.h"
#include "_kbsession.h"
#include "session.h"

KBSession *kb_session_create(const char *app_name, const char *password,
                             const char *database_path, int *error_code) {
    if (!database_path || !(*database_path)) {
        if (error_code != nullptr) {
            *error_code = KB_ERROR_DATABASE_PATH_INVALID;
        }
        return nullptr;
    }

    return new KBSession(app_name, password, database_path);
}

int kb_session_init(KBSession *session) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->init();
}

void kb_session_destroy(KBSession *session) {
    if (session == nullptr) {
        return;
    }
    session->destroy();
    delete session;
}

char *kb_knowledgebase_create(KBSession *session,
                              const KnowledgeBaseInfo *knowledgebase_info,
                              int *error_code) {
    if (session == nullptr) {
        if (error_code != nullptr) {
            *error_code = AI_COMMON_INVALID_SESSION;
        }
        return nullptr;
    }
    return session->createKnowledgeBase(knowledgebase_info, error_code);
}

int kb_knowledgebase_delete(KBSession *session, const char *knowledgebase_id) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->deleteKnowledgeBase(knowledgebase_id);
}

KnowledgeBaseInfo *kb_knowledgebase_info_get(KBSession *session,
                                             const char *knowledgebase_id,
                                             int *error_code) {
    if (session == nullptr) {
        if (error_code != nullptr) {
            *error_code = AI_COMMON_INVALID_SESSION;
        }
        return nullptr;
    }
    return session->getKnowledgeBaseInfo(knowledgebase_id, error_code);
}

KnowledgeBaseInfo **kb_knowledgebase_info_list(KBSession *session, int *count,
                                               int *error_code) {
    if (session == nullptr) {
        if (error_code != nullptr) {
            *error_code = AI_COMMON_INVALID_SESSION;
        }
        return nullptr;
    }
    return session->getKnowledgeBaseInfoList(count, error_code);
}

int kb_knowledgebase_info_update(KBSession *session,
                                 const KnowledgeBaseInfo *knowledgebase_info) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->updateKnowledgeBase(knowledgebase_info);
}

int kb_document_add(KBSession *session, const char *knowledgebase_id,
                    const char *document_path, const char *document_id,
                    void *reserved) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->addDocument(knowledgebase_id, document_path, document_id,
                                reserved);
}

int kb_document_add_multi(KBSession *session, const char *knowledgebase_id,
                          const char **document_paths,
                          const char **document_ids, int docs_count,
                          void *reserved, int **error_codes) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->addMultipleDocuments(knowledgebase_id, document_paths,
                                         document_ids, docs_count, reserved,
                                         error_codes);
}

char **kb_document_list(KBSession *session, const char *knowledgebase_id,
                        int *count, int *error_code) {
    if (session == nullptr) {
        if (error_code != nullptr) {
            *error_code = AI_COMMON_INVALID_SESSION;
        }
        return nullptr;
    }
    return session->listDocuments(knowledgebase_id, count, error_code);
}

int kb_document_delete(KBSession *session, const char *knowledgebase_id,
                       const char *document_id) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->deleteDocument(knowledgebase_id, document_id);
}

int kb_document_delete_multi(KBSession *session, const char *knowledgebase_id,
                             const char **document_ids, int docs_count,
                             int **error_codes) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->deleteMultipleDocument(knowledgebase_id, document_ids,
                                           docs_count, error_codes);
}

int kb_document_cancel_addition(KBSession *session,
                                const char *knowledgebase_id,
                                const char **document_ids, int docs_count) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->cancelDocumentAddition(knowledgebase_id, document_ids,
                                           docs_count);
}

int kb_document_get_processing_progress(KBSession *session,
                                        KnowledgeBaseInfo *knowledgebase,
                                        const char *document_id, int *status,
                                        int *error_code) {
    if (session == nullptr) {
        if (error_code != nullptr) {
            *error_code = AI_COMMON_INVALID_SESSION;
        }
        return -1;
    }
    return session->getDocumentProcessingStatus(knowledgebase, document_id,
                                                status, error_code);
}

int kb_retrieval(KBSession *session, const char *knowledgebase_id,
                 const char *request_id, const char *query,
                 const KBRetrievalConfig *config, KBRetrievalCallback callback,
                 void *user_data) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->retrieval(knowledgebase_id, request_id, query, config,
                              callback, user_data);
}

int kb_chat_stream(KBSession *session, const char *request_id,
                   const char *knowledgebase_id, const char *question,
                   const KBChatHistory *history, const KBChatConfig *config,
                   KBChatStreamCallback callback, void *user_data) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->chat(request_id, knowledgebase_id, question, history,
                         config, callback, user_data);
}

int kb_chat_stream_stop(KBSession *session, const char *request_id) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->stopChat(request_id);
}

#include "knowledgebaseservice.h"

#include <nlohmann/json.hpp>

#include "log.h"

void knowledgeBaseserviceSyncResponse(const std::string& method, GObject* delegate, GDBusMethodInvocation* invocation,
                                      const std::string& output) {
    if (method == "initKnowledgeBaseSession") {
        ai_business_knowledge_base_service_complete_init_knowledge_base_session(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
    } else if (method == "destroyKnowledgeBaseSession") {
        ai_business_knowledge_base_service_complete_destroy_knowledge_base_session(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
    } else if (method == "createKnowledgeBase") {
        ai_business_knowledge_base_service_complete_create_knowledge_base(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("createKnowledgeBase method.\n");
    } else if (method == "deleteKnowledgeBase") {
        ai_business_knowledge_base_service_complete_delete_knowledge_base(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("deleteKnowledgeBase method.\n");
    } else if (method == "getKnowledgeBase") {
        ai_business_knowledge_base_service_complete_get_knowledge_base(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("getKnowledgeBase method.\n");
    } else if (method == "getAllKnowledgeBase") {
        ai_business_knowledge_base_service_complete_get_all_knowledge_base(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("getAllKnowledgeBase method.\n");
    } else if (method == "setKnowledgeBaseName") {
        ai_business_knowledge_base_service_complete_set_knowledge_base_name(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("setKnowledgeBaseName method.\n");
    } else if (method == "setKnowledgeBaseDescription") {
        ai_business_knowledge_base_service_complete_set_knowledge_base_description(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("setKnowledgeBaseDescription method.\n");
    } else if (method == "updateFilesContentData") {
        ai_business_knowledge_base_service_complete_upload_documents(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("updateFilesContentData method.\n");
    } else if (method == "getFileListInKnowledgeBase") {
        ai_business_knowledge_base_service_complete_get_file_list_in_knowledge_base(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("getAllFileInfos method.\n");
    } else if (method == "deleteFiles") {
        ai_business_knowledge_base_service_complete_delete_files(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("deleteFile method.\n");
    } else if (method == "getFileProcessingProgress") {
        ai_business_knowledge_base_service_complete_get_file_processing_progress(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("getFileProcessingProgress method.\n");
    } else if (method == "stopChat") {
        ai_business_knowledge_base_service_complete_stop_chat(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("stopChat method.\n");
    } else if (method == "stopUpload") {
        ai_business_knowledge_base_service_complete_stop_upload(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("stopUpload method.\n");
    } else {
        PRINT_ERROR("dataManagement service unknown method: %s\n", method.c_str());
    }
}

void knowledgeBaseserviceAsyncResponse(const std::string& method, GObject* delegate, GDBusMethodInvocation* invocation,
                                       const std::string& output) {
    if (method == "retrievalKnowledgeBase") {
        ai_business_knowledge_base_service_complete_retrieval_knowledge_base(
            reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("retrievalKnowledgeBase method.\n");
        return;
    }

    PRINT_ERROR("unknown method: %s\n", method.c_str());
}

void knowledgeBaseserviceAsyncFlowResponse(const std::string& method, GDBusConnection* connection, GObject* delegate,
                                           GDBusMethodInvocation* invocation, GVariant* output, bool end) {
    std::string interface = "com.kylinos.aisdk.business.KnowledgeBaseService";
    gchar* json_str = nullptr;
    g_variant_get(output, "(&s)", &json_str);
    try {
        auto json = nlohmann::json::parse(json_str);
        if (json.contains("data") && json["data"].contains("sessionId") && json["data"]["sessionId"].is_number()) {
            int session_id = json["data"]["sessionId"].get<int>();
            interface += "_" + std::to_string(session_id);
        }
        PRINT_DEBUG("interface: %s\n", interface.c_str());
    } catch (const std::exception& e) {
        PRINT_ERROR("Failed to parse sessionId from output: %s\n", e.what());
    }

    if (method == "uploadDocuments") {
        g_dbus_connection_emit_signal(connection, "cn.kylinos.aisdk.business.KnowledgeBaseService",
                                      "/com/kylin/AiBusiness/KnowledgeBaseService", interface.c_str(),
                                      "DocumentUploadProgress", output, nullptr);

        PRINT_DEBUG("uploadDocuments method.\n");
        if (end) {
            ai_business_knowledge_base_service_complete_upload_documents(
                reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, "");
        }
        return;
    }

    if (method == "chatWithKnowledgeBase") {
        g_dbus_connection_emit_signal(connection, "cn.kylinos.aisdk.business.KnowledgeBaseService",
                                      "/com/kylin/AiBusiness/KnowledgeBaseService", interface.c_str(),
                                      "StreamingChatResponse", output, nullptr);

        PRINT_DEBUG("chatWithKnowledgeBase search method.\n");
        if (end) {
            ai_business_knowledge_base_service_complete_chat_with_knowledge_base(
                reinterpret_cast<AiBusinessKnowledgeBaseService*>(delegate), invocation, "");
        }
        return;
    }

    PRINT_ERROR("unknown method: %s\n", method.c_str());
}

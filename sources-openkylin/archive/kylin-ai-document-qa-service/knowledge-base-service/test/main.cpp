#include <gio/gio.h>
#include <kyai-business-framework/log.h>
#include <kyai-business-framework/response.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include "../src/common/errno.h"
#include "advaiknowledgebaseserviceglue.h"

GDBusConnection* connection;
AiBusinessKnowledgeBaseService* knowledgeBaseProxy;
GMainLoop* loop;
const char* objectPath = "/com/kylin/AiBusiness/KnowledgeBaseService";
const char* interfaceName = "com.kylinos.aisdk.business.KnowledgeBaseService";
const char* appId = "test";
const char* passwd = "test";
const char* dbFilePath = "/home/kylin/testtest";

std::mutex mtx;
std::condition_variable cv;
bool uploadDocSucceed = false;
bool chatEnd = false;
int sessionId = -1;

// 同步接口
int initKnowledgeBaseSession() {
    gchar* output;
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["password"] = passwd;
    paramJson["dbFilePath"] = dbFilePath;
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_init_knowledge_base_session_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);
    if (!success) {
        if (error) {
            PRINT_ERROR("Call initKnowledgeBaseSession Error: %s\n", error->message);
            g_error_free(error);
            return -1;
        }
    }
    PRINT_INFO("Call initKnowledgeBaseSession result: %s\n", output);
    nlohmann::json jsonResponse = nlohmann::json::parse(output);
    return jsonResponse["data"]["sessionId"];
}

void destroyKnowledgeBaseSession(int sessionId) {
    gchar* output;
    nlohmann::json paramJson;
    paramJson["sessionId"] = sessionId;
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_destroy_knowledge_base_session_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);
    if (!success) {
        if (error) {
            PRINT_ERROR("Call destroyKnowledgeBaseSession Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
}

void createKnowledgeBase() {
    PRINT_INFO("in function:%s at line: %d\n", __FUNCTION__, __LINE__);
    gchar* output;
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["password"] = passwd;
    paramJson["sessionId"] = sessionId;
    paramJson["name"] = "aiassistent_kb";
    paramJson["description"] = "This is a knowledge base for kylin-ai-assistant";
    paramJson["type"] = "rag";
    paramJson["indexType"] = "FLAT";
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_create_knowledge_base_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);
    if (!success) {
        if (error) {
            PRINT_ERROR("Call createKnowledgeBase Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
    PRINT_INFO("Call createKnowledgeBase result: %s\n", output);
}

void deleteKnowledgeBase(const std::string& knowledgeBaseId) {
    PRINT_INFO("in function:%s at line: %d\n", __FUNCTION__, __LINE__);
    gchar* output;
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["password"] = passwd;
    paramJson["sessionId"] = sessionId;
    paramJson["knowledgeBaseId"] = knowledgeBaseId;
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_delete_knowledge_base_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);
    if (!success) {
        if (error) {
            PRINT_ERROR("Call deleteKnowledgeBase Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
    PRINT_INFO("Call deleteKnowledgeBase result: %s\n", output);
}

void getKnowledgeBase(const std::string& knowledgeBaseId) {
    PRINT_INFO("in function:%s at line: %d\n", __FUNCTION__, __LINE__);
    gchar* output;
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["sessionId"] = sessionId;
    paramJson["knowledgeBaseId"] = knowledgeBaseId;
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_get_knowledge_base_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);
    if (!success) {
        if (error) {
            PRINT_ERROR("Call getKnowledgeBase Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
    PRINT_INFO("Call getKnowledgeBase result: %s\n", output);
}

std::vector<std::string> getAllKnowledgeBase() {
    PRINT_INFO("in function:%s at line: %d\n", __FUNCTION__, __LINE__);
    gchar* output;
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["sessionId"] = sessionId;
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_get_all_knowledge_base_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);
    if (!success) {
        if (error) {
            PRINT_ERROR("Call getAllKnowledgeBase Error: %s\n", error->message);
            g_error_free(error);
            return std::vector<std::string>();
        }
    }
    PRINT_INFO("Call getAllKnowledgeBase result: %s\n", output);
    std::vector<std::string> knowledgeBaseIds;
    try {
        nlohmann::json jsonResponse = nlohmann::json::parse(output);
        if (jsonResponse["err_no"] == 0) {
            for (const auto& item : jsonResponse["data"]) {
                knowledgeBaseIds.push_back(item["knowledgeBaseId"]);
            }
        }
    } catch (const std::exception& e) {
        PRINT_ERROR("JSON parse error: %s\n", e.what());
    }

    return knowledgeBaseIds;
}

void setKnowledgeBaseName(const std::string& knowledgeBaseId, const std::string& name) {
    PRINT_INFO("in function:%s at line: %d\n", __FUNCTION__, __LINE__);
    gchar* output;
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["sessionId"] = sessionId;
    paramJson["knowledgeBaseId"] = knowledgeBaseId;
    paramJson["name"] = name;
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_set_knowledge_base_name_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);
    if (!success) {
        if (error) {
            PRINT_ERROR("Call setKnowledgeBaseName Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
    PRINT_INFO("Call setKnowledgeBaseName result: %s\n", output);
}

void setKnowledgeBaseDescription(const std::string& knowledgeBaseId, const std::string& description) {
    PRINT_INFO("in function:%s at line: %d\n", __FUNCTION__, __LINE__);
    gchar* output;
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["sessionId"] = sessionId;
    paramJson["knowledgeBaseId"] = knowledgeBaseId;
    paramJson["description"] = description;
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_set_knowledge_base_description_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);
    if (!success) {
        if (error) {
            PRINT_ERROR("Call setKnowledgeBaseDescription Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
    PRINT_INFO("Call setKnowledgeBaseDescription result: %s\n", output);
}

void uploadDocumentsAsync(const std::string& knowledgeBaseId,
                          const std::vector<std::pair<std::string, std::string>>& filePairs) {
    nlohmann::json fileJson = nlohmann::json::array();
    for (auto& filePair : filePairs) {
        fileJson.push_back({{"filePath", filePair.first}, {"fileId", filePair.second}});
    }

    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["password"] = passwd;
    paramJson["sessionId"] = sessionId;
    paramJson["knowledgeBaseId"] = knowledgeBaseId;
    paramJson["files"] = fileJson;
    paramJson["config"]["chunkMaxSize"] = 1024;
    paramJson["config"]["enhanceParse"] = false;

    ai_business_knowledge_base_service_call_upload_documents(knowledgeBaseProxy, paramJson.dump().c_str(), nullptr,
                                                             nullptr, nullptr);

    PRINT_INFO("uploadDocumentsAsync send success.\n");
}

void uploadDocumentsCallBack(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path,
                             const gchar* interface_name, const gchar* signal_name, GVariant* parameters,
                             gpointer user_data) {
    Response response = Response::parseFromGVariant(parameters);
    PRINT_INFO("uploadDocumentsCallBack 流式返回 response: %s\n", response.toString().c_str());
    if (response.getString("status") == "ALLCOMPLETED" ||
        (response.getString("fileId") == "" && response.getString("status") == "FAILED")) {
        std::unique_lock<std::mutex> lock(mtx);
        uploadDocSucceed = true;
        cv.notify_all();
    }
}

void getFileListInKnowledgeBase(const std::string& knowledgeBaseId) {
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["sessionId"] = sessionId;
    paramJson["knowledgeBaseId"] = knowledgeBaseId;
    gchar* output;
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_get_file_list_in_knowledge_base_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);

    if (!success) {
        if (error) {
            PRINT_ERROR("Call getFileListInKnowledgeBase Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
    PRINT_INFO("Call getFileListInKnowledgeBase result: %s\n", output);
}

void deleteFiles(const std::string& knowledgeBaseId, const std::vector<std::string>& fileIds) {
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["password"] = passwd;
    paramJson["sessionId"] = sessionId;
    paramJson["knowledgeBaseId"] = knowledgeBaseId;
    paramJson["fileIds"] = nlohmann::json(fileIds);
    gchar* output;
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_delete_files_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);

    if (!success) {
        if (error) {
            PRINT_ERROR("Call deleteFiles Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
    PRINT_INFO("Call deleteFiles result: %s\n", output);
}

void retrievalCallback(GObject* source_object, GAsyncResult* res, gpointer user_data) {
    gchar* output;
    GError* error = nullptr;

    ai_business_knowledge_base_service_call_retrieval_knowledge_base_finish(
        (AiBusinessKnowledgeBaseService*)source_object, &output, res, &error);
    if (error) {
        PRINT_ERROR("retrievalCallback Error: %s\n", error->message);
        g_error_free(error);
        return;
    }

    PRINT_INFO("retrievalCallback response: %s\n", output);
}

void retrievalKnowledgeBaseAsync(const std::string& knowledgeBaseId, const std::string& requestId,
                                 const std::string& query) {
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["password"] = passwd;
    paramJson["sessionId"] = sessionId;
    paramJson["knowledgeBaseId"] = knowledgeBaseId;
    paramJson["requestId"] = requestId;
    paramJson["query"] = query;
    paramJson["fileIds"] = {"file_002"};
    paramJson["config"]["rerankMethod"] = "WEIGHT";
    paramJson["config"]["scoreThreshold"] = 0.75f;
    paramJson["config"]["topK"] = 4;
    paramJson["config"]["retrievalMethod"] = "SEMANTIC";
    paramJson["config"]["maxTokens"] = 4096;
    paramJson["config"]["expandContext"] = true;

    ai_business_knowledge_base_service_call_retrieval_knowledge_base(knowledgeBaseProxy, paramJson.dump().c_str(),
                                                                     nullptr, retrievalCallback, nullptr);

    PRINT_INFO("retrievalKnowledgeBase send Success. \n");
}

void chatWithKnowledgeBaseAsync(const std::string& knowledgeBaseId, const std::string& requestId,
                                const std::string& query) {
    nlohmann::json paramJson;
    paramJson["appId"] = appId;
    paramJson["password"] = passwd;
    paramJson["sessionId"] = sessionId;
    paramJson["knowledgeBaseId"] = knowledgeBaseId;
    paramJson["requestId"] = requestId;
    paramJson["query"] = query;
    paramJson["config"]["rerankMethod"] = "WEIGHT";
    paramJson["config"]["scoreThreshold"] = 0.6f;
    paramJson["config"]["fileIds"] = {"file_002"};
    paramJson["config"]["topK"] = 5;
    paramJson["config"]["retrievalMethod"] = "HYBRID";
    paramJson["config"]["maxTokens"] = 4096;
    paramJson["config"]["expandContext"] = true;
    paramJson["config"]["newChat"] = true;
    // paramJson["config"]["modelName"] = "Qwen-2.5-3b_1.0";
    // paramJson["message"] = nlohmann::json::array(
    //     {{{"role", "user"}, {"content", "你好"}}, {{"role", "assistant"}, {"content",
    //     "你好，我是Qwen-2.5-3b_1.0"}}});

    ai_business_knowledge_base_service_call_chat_with_knowledge_base(knowledgeBaseProxy, paramJson.dump().c_str(),
                                                                     nullptr, nullptr, nullptr);

    PRINT_INFO("chatWithKnowledgeBaseAsync send success.\n");
}

void stopChatWithKnowledgeBase() {
    nlohmann::json paramJson;
    paramJson["sessionId"] = sessionId;
    paramJson["requestId"] = "request_id_002";
    gchar* output;
    GError* error = nullptr;

    gboolean success = ai_business_knowledge_base_service_call_stop_chat_sync(
        knowledgeBaseProxy, paramJson.dump().c_str(), &output, nullptr, &error);

    if (!success) {
        if (error) {
            PRINT_ERROR("Call stopChatWithKnowledgeBase Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
    PRINT_INFO("Call stopChatWithKnowledgeBase result: %s\n", output);
}

void chatCallback(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path,
                  const gchar* interface_name, const gchar* signal_name, GVariant* parameters, gpointer user_data) {
    Response response = Response::parseFromGVariant(parameters);
    static std::string outputMsg;
    PRINT_INFO("chatCallback: %s\n", response.getData().dump().c_str());
    outputMsg += response.getData()["result"];
    PRINT_INFO("chatCallback 流式返回\noutputMsg: %s\n", outputMsg.c_str());

    // static int count = 0;
    // count++;
    // if (count == 10) {
    //     PRINT_INFO("调用停止对话接口\n");
    //     stopChatWithKnowledgeBase();
    // }
    if (response.getData()["isFinal"] == true) {
        PRINT_INFO("流式返回结束\n");
        std::unique_lock<std::mutex> lock(mtx);
        chatEnd = true;
        cv.notify_all();
    }
}

void startServer() {
    GError* error = nullptr;

    PRINT_INFO("start connect\n");
    std::string serverUnixPath =
        "unix:abstract=/tmp/.kylin-ai-business-unix/" + std::to_string(getuid()) + "/KnowledgeBaseService.sock";
    connection = g_dbus_connection_new_for_address_sync(
        serverUnixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr, nullptr, &error);
    if (connection == nullptr) {
        PRINT_ERROR("Error connecting to D-Bus address %s: %s\n", serverUnixPath.c_str(), error->message);
        g_error_free(error);
        return;
    }

    PRINT_INFO("Connected. Negotiated capabilities: unix-fd-passing=%d\n",
               g_dbus_connection_get_capabilities(connection) & G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING);

    knowledgeBaseProxy = ai_business_knowledge_base_service_proxy_new_sync(connection, G_DBUS_PROXY_FLAGS_NONE, nullptr,
                                                                           objectPath, nullptr, &error);
    if (knowledgeBaseProxy == nullptr) {
        PRINT_ERROR("Error creating speech processor proxy %s: %s\n", objectPath, error->message);
        // g_error_free(error);
        return;
    }
}

// 新增一个函数来处理需要等待的任务
void handleAsyncTasks(const std::vector<std::string>& knowledgeBaseIds) {
    {
        // 等待上传完成
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []() { return uploadDocSucceed; });
    }

    PRINT_INFO("Upload documents succeed.");

    // 上传完成后执行后续操作
    getFileListInKnowledgeBase(knowledgeBaseIds[0]);
    // deleteFiles(knowledgeBaseIds[0], {"file_001"});
    // getFileListInKnowledgeBase(knowledgeBaseIds[0]);

    // retrievalKnowledgeBaseAsync(knowledgeBaseIds[0], "request_id_001", "如何报销？");

    chatWithKnowledgeBaseAsync(knowledgeBaseIds[0], "request_id_002", "如何报销？");

    {
        // 等待对话结束
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []() { return chatEnd; });
    }

    for (auto id : knowledgeBaseIds) {
        PRINT_INFO("delete knowledge base: %s", id.c_str());
        deleteKnowledgeBase(id);
    }

    if (sessionId > 0) {
        destroyKnowledgeBaseSession(sessionId);
    }

    // 退出主循环
    if (loop) {
        g_main_loop_quit(loop);
    }
}

int main() {
    startServer();

    // 同步请求
    sessionId = initKnowledgeBaseSession();
    PRINT_INFO("initKnowledgeBaseSession: %d\n", sessionId);
    createKnowledgeBase();
    std::vector<std::string> knowledgeBaseIds = getAllKnowledgeBase();
    for (auto id : knowledgeBaseIds) {
        getKnowledgeBase(id);
    }
    // setKnowledgeBaseName(knowledgeBaseIds[0], "new name");
    // setKnowledgeBaseDescription(knowledgeBaseIds[0], "new description");

    // 异步流式返回
    unsigned int ret = g_dbus_connection_signal_subscribe(connection, nullptr, interfaceName, "DocumentUploadProgress",
                                                          objectPath, nullptr, G_DBUS_SIGNAL_FLAGS_NONE,
                                                          uploadDocumentsCallBack, nullptr, nullptr);
    PRINT_INFO("DocumentUploadProgress connect ret = %d\n", ret);
    ret = g_dbus_connection_signal_subscribe(connection, nullptr, interfaceName, "StreamingChatResponse", objectPath,
                                             nullptr, G_DBUS_SIGNAL_FLAGS_NONE, chatCallback, nullptr, nullptr);
    PRINT_INFO("StreamingChatResponse connect ret = %d\n", ret);

    std::vector<std::pair<std::string, std::string>> filePairs;
    filePairs.emplace_back("xxx.pdf", "file_001");
    filePairs.emplace_back("xxx.docx", "file_002");
    uploadDocumentsAsync(knowledgeBaseIds[0], filePairs);

    // 在单独的线程中等待并执行后续操作
    std::thread taskThread(handleAsyncTasks, knowledgeBaseIds);

    loop = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop);

    if (taskThread.joinable()) {
        taskThread.join();
    }

    return 0;
}

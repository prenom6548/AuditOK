/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "knowledgebaseproxy.h"

#include <unistd.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

namespace {
std::string serverUnixPath =
    "unix:abstract=/tmp/.kylin-ai-business-unix/" + std::to_string(getuid()) + "/Knowledgebase.sock";
const std::string objectPath = "/com/kylin/AiBusiness/Knowledgebase";

const int CALL_KNOWLEDGEBASE_RETRY_TIME = 3;
}  // namespace
namespace knowledgebase {

// 下面为服务端错误码汇总 禁止修改
enum ERROR_CODE : int {
    // 0 - 100 公共错误码
    SYSTEM_ERR = -1,
    SUCCESS = 0,
    ASYNC_FLOW_END = 1,

    PARSE_CONFIG_FILE_FAIL = 11,
    INVALID_CONFIG = 12,
    INIT_DBUS_SERVER_FAIL = 13,
    PARSE_PARAM_FAILED = 14,
    PARSE_RESPONSE_FAILED = 15,

    CHUNK_FILE_INFO_FAILED = 50,
    EMBEDDING_FAILED = 51,
    INSERT_FILE_INFO_SQLITE_FAILED = 52,
    DELETE_FILE_INFO_SQLITE_FAILED = 53,
    UPDATE_FILE_INFO_SQLITE_FAILED = 54,
    CREATE_VECTOR_DATABASE_FAILED = 55,
    INSERT_VECTOR_DATABASE_FAILED = 56,
    DELETE_VECTOR_DATABASE_FAILED = 57,
    UPDATE_VECTOR_DATABASE_FAILED = 58,
    SEARCH_FILE_INFO_FAILED = 59,
    NLP_FAILED = 60,
    // 100 - 199 搜索场景和数据管家场景业务错误码
    DM_ERROR_UNKNOWN = 100,            // 未知错误
    DM_SESSION_ERROR = 101,            // 会话错误
    DM_ERROR_INVALID_INPUT = 102,      // 无效输入
    DM_ERROR_PERMISSION_DENIED = 103,  // 权限被拒绝

    // 200 - 299 文档问答错误码
    TEXT_CHUNKS_UNSUPPORT = 200,

    // 300 - 399 向量数据库错误码
    VECTOR_DATABASE_DATA_UNAVAILABLE = 300,

    // 400 - 1000 xxx

    ERROR_MAX = 10000,
};

// 服务端 -> 客户端 错误码映射表
const std::unordered_map<int, KnowledgebaseErrorCode> ServerToClientErrorMap = {
    // 公共错误码
    {0, KNOWLEDGEBASE_SUCCESS},
    {14, KNOWLEDGEBASE_PARSE_PARAM_FAILED},

    // 文件处理错误码
    {50, KNOWLEDGEBASE_CHUNK_FAILED},
    {51, KNOWLEDGEBASE_EMBEDDING_FAILED},
    {200, KNOWLEDGEBASE_DOCUMENT_EXCEEDS_LIMIT},

    // 向量数据库错误码
    {55, KNOWLEDGEBASE_VECTOR_DATABASE_FAILED},
    {56, KNOWLEDGEBASE_VECTOR_DATABASE_FAILED},
    {57, KNOWLEDGEBASE_VECTOR_DATABASE_FAILED},
    {300, KNOWLEDGEBASE_VECTOR_DATABASE_DATA_UNAVAILABLE},

    // 搜索错误码
    {59, KNOWLEDGEBASE_SEARCH_FILE_INFO_FAILED},
};

Status InitializeStatus(bool success, int errorCode, const std::string& errorMsg) {
    return Status{success, KnowledgebaseErrorCode(errorCode), errorMsg};
}

std::string createResponse(const std::string& outputData, int errorCode, const std::string& errorMsg) {
    nlohmann::json root;
    root["err_no"] = errorCode;
    root["err_msg"] = errorMsg;
    root["data"] = outputData;
    return root.dump();
}
Status handleJsonData(int serverErrorCode, std::string& serverErrorMsg, const nlohmann::json& jsonValue,
                      nlohmann::json& outputData) {
    bool hasData = jsonValue.contains("data") && !jsonValue["data"].is_null();

    KnowledgebaseErrorCode clientErrorCode;

    auto it = ServerToClientErrorMap.find(serverErrorCode);
    if (it != ServerToClientErrorMap.end()) {
        clientErrorCode = it->second;
    } else {
        clientErrorCode = KnowledgebaseErrorCode::KNOWLEDGEBASE_ERROR_UNKNOWN;
        serverErrorMsg += " knowledgebase error unknown";
    }

    if (hasData) {
        outputData = jsonValue["data"];
    }

    return (clientErrorCode == KnowledgebaseErrorCode::KNOWLEDGEBASE_SUCCESS)
               ? STATUS_SUCCESS
               : STATUS_FAILURE(clientErrorCode, serverErrorMsg);
}

Status parseResponse(const std::string& response, nlohmann::json& outputData) {
    nlohmann::json jsonValue = nlohmann::json::parse(response, nullptr, false);
    std::string msg = "Parse response error: " + response;
    if (jsonValue.is_discarded()) {
        return STATUS_FAILURE(KnowledgebaseErrorCode::KNOWLEDGEBASE_ERROR_UNKNOWN, msg);
    }

    bool hasError = jsonValue.contains("err_no") && jsonValue["err_no"].is_number_integer();
    bool hasErrorMsg = jsonValue.contains("err_msg") && jsonValue["err_msg"].is_string();

    if (!hasError || !hasErrorMsg) {
        return STATUS_FAILURE(KnowledgebaseErrorCode::KNOWLEDGEBASE_ERROR_UNKNOWN, msg);
    }

    int serverErrorCode = jsonValue["err_no"].get<int>();
    std::string serverErrorMsg = jsonValue["err_msg"].get<std::string>();

    // 如果是连接错误 json中是没有data的，所以这里直接返回错误
    if (serverErrorCode == KnowledgebaseErrorCode::KNOWLEDGEBASE_CONNECTION_ERROR) {
        return STATUS_FAILURE(serverErrorCode, serverErrorMsg);
    }

    return handleJsonData(serverErrorCode, serverErrorMsg, jsonValue, outputData);
}

Status parseResponse(const std::string& response, std::string& outputData, const std::string& key) {
    nlohmann::json result;
    Status ret = parseResponse(response, result);
    if (!ret.success) {
        return ret;
    }

    if (!result.contains(key)) {
        return STATUS_FAILURE(KnowledgebaseErrorCode::KNOWLEDGEBASE_ERROR_UNKNOWN, "parse response failed");
    }

    outputData = result[key].get<std::string>();
    return STATUS_SUCCESS;
}

Status parseResponse(const std::string& response) {
    nlohmann::json v;
    return parseResponse(response, v);
}

std::string KnowledgebaseProxy::handleGError(GError* gError, const std::string& fallbackMsg,
                                             KnowledgebaseErrorCode errorCode) {
    std::string errorMsg = fallbackMsg;  // 存在失败 但是error为空的情况
    if (gError) {
        errorMsg = gError->message;
        g_error_free(gError);
        gError = nullptr;
    }
    return createResponse(std::string(), errorCode, errorMsg);
}

Status KnowledgebaseProxy::handleDBusError(GError* gError, const std::string& contextMsg,
                                           KnowledgebaseErrorCode errorCode) {
    std::string fullErrorMsg = contextMsg;

    if (gError) {
        fullErrorMsg += " (GDBus Error: " + std::string(gError->message) + ")";
        g_printerr("%s\n", fullErrorMsg.c_str());  // 集中日志记录
        g_error_free(gError);
    } else {
        fullErrorMsg += " (Unknown error)";
        g_printerr("%s\n", fullErrorMsg.c_str());
    }

    return STATUS_FAILURE(errorCode, fullErrorMsg);
}

std::string KnowledgebaseProxy::syncFuncCall(const std::string& input, gint timeout_msec,
                                             AiBusinessKnowledgeBaseCallSyncFunc func) {
    char* output = nullptr;
    GError* gError = nullptr;
    std::string errorMsg;
    int retryTime = 0;
    bool success = func(delegate_, input.c_str(), G_DBUS_CALL_FLAGS_NONE, timeout_msec, &output, nullptr, &gError);
    while (!success && retryTime <= CALL_KNOWLEDGEBASE_RETRY_TIME) {
        if (retryTime == CALL_KNOWLEDGEBASE_RETRY_TIME) {
            return handleGError(gError, "Unknown error (GError not set)",
                                KnowledgebaseErrorCode::KNOWLEDGEBASE_CONNECTION_ERROR);
        }

        if (gError) {  // 测试过程中有发现过失败但error为空的情况
            std::fprintf(stdout, "WARN: call knowledgebase server failed, code: %d, msg: %s, retry: %d\n", gError->code,
                         gError->message, retryTime);
            g_error_free(gError);
            gError = nullptr;
        } else {
            std::fprintf(stdout, "WARN: call knowledgebase server failed, retry: %d\n", retryTime);
        }

        // 断开连接
        destroy();
        sleep(2);
        retryTime++;
        // 重连
        if (!connect().success) {
            continue;
        }
        success = func(delegate_, input.c_str(), G_DBUS_CALL_FLAGS_NONE, timeout_msec, &output, nullptr, &gError);
    }

    std::string response;
    if (output != nullptr) {
        response = output;
        g_free(output);
    }

    return response;
}
KnowledgebaseProxy::KnowledgebaseProxy() { connect(); }

KnowledgebaseProxy::~KnowledgebaseProxy() { destroy(); }

void KnowledgebaseProxy::destroy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
        delegate_ = nullptr;
    }
    if (connection_ != nullptr) {
        g_object_unref(connection_);
        connection_ = nullptr;
    }
    getConnection_ = false;
}

Status KnowledgebaseProxy::isSupported(bool& success) const {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("uname -m", "r"), pclose);

    if (!pipe) {
        std::cerr << "popen failed!" << std::endl;
        success = false;
        return STATUS_SUCCESS;
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // 移除可能存在的换行符
    if (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
        if (!result.empty() && result.back() == '\r') {
            result.pop_back();
        }
    }

    if (result != "x86_64" && result != "aarch64") {
        std::cerr << "unsupported architecture: " << result << std::endl;
        success = false;
        return STATUS_SUCCESS;
    }
    std::cout << "current architecture: " << result << std::endl;
    success = true;
    return STATUS_SUCCESS;
}

Status KnowledgebaseProxy::connect() {
    GError* error = nullptr;
    connection_ = g_dbus_connection_new_for_address_sync(
        serverUnixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr, nullptr, &error);
    if (!connection_) {
        getConnection_ = false;
        return handleDBusError(error, "Failed to connect to D-Bus address: " + serverUnixPath,
                               KnowledgebaseErrorCode::KNOWLEDGEBASE_CONNECTION_ERROR);
    }
    delegate_ = ai_business_knowledgebase_proxy_new_sync(connection_, G_DBUS_PROXY_FLAGS_NONE, nullptr,
                                                         objectPath.c_str(), nullptr, &error);
    if (!delegate_) {
        if (connection_) {  // 防止delegate_失败了 但是connection_成功了
            g_object_unref(connection_);
            connection_ = nullptr;
        }
        getConnection_ = false;
        return handleDBusError(error, "Failed to create proxy for object path: " + objectPath,
                               KnowledgebaseErrorCode::KNOWLEDGEBASE_CONNECTION_ERROR);
    }
    getConnection_ = true;
    return STATUS_SUCCESS;
}

Status KnowledgebaseProxy::createKnowledgebase(std::string& knowledgebaseName) {
    if (!getConnection_) {
        Status ret = connect();
        if (!ret.success) return ret;
    }

    const std::string emptyInput = "{}";
    const std::string res = syncFuncCall(emptyInput, -1, ai_business_knowledgebase_call_create_knowledge_base_sync);

    return parseResponse(res, knowledgebaseName, "knowledgebase_name");
}

Status KnowledgebaseProxy::deleteKnowledgebase(const std::string& knowledgebaseName) {
    if (!getConnection_) {
        Status ret = connect();
        if (!ret.success) return ret;
    }

    nlohmann::json inputJson;
    inputJson["knowledgebase_name"] = knowledgebaseName;
    const std::string input = inputJson.dump();

    const std::string res = syncFuncCall(input, -1, ai_business_knowledgebase_call_delete_knowledge_base_sync);
    return parseResponse(res);
}

Status KnowledgebaseProxy::similaritySearch(const std::string& searchCondition, std::string& result) {
    if (!getConnection_) {
        Status ret = connect();
        if (!ret.success) return ret;
    }

    const std::string res = syncFuncCall(searchCondition, -1, ai_business_knowledgebase_call_similarity_search_sync);
    return parseResponse(res, result, "search_result");
}
Status KnowledgebaseProxy::addTextFiles(const std::string& input) {
    if (!getConnection_) {
        Status ret = connect();
        if (!ret.success) return ret;
    }

    const std::string res = syncFuncCall(input, 600000, ai_business_knowledgebase_call_add_text_files_sync);
    return parseResponse(res);
}
Status KnowledgebaseProxy::getTextFileContents(const std::string& input, std::string& result) {
    if (!getConnection_) {
        Status ret = connect();
        if (!ret.success) return ret;
    }

    const std::string res = syncFuncCall(input, 600000, ai_business_knowledgebase_call_get_text_file_contents_sync);
    return parseResponse(res, result, "file_content");
}
}  // namespace knowledgebase
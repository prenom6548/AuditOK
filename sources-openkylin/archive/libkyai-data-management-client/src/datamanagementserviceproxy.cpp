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

#include "datamanagementserviceproxy.h"

#include <kylin-ai/business/data-management/common.h>
#include <unistd.h>

#include <nlohmann/json.hpp>
#include <string>

#include "dataManagementDBusSyncFuncs.h"

namespace {

const int CALL_DATA_MANAGEMENT_RETRY_TIME = 5;

std::string createResponse(const std::string &data, int errorCode, const std::string &errorMsg) {
    nlohmann::json root;
    root["err_no"] = errorCode;
    root["err_msg"] = errorMsg;
    root["data"] = data;
    return root.dump();
}

}  // namespace

std::string DataManagementServiceProxy::syncFuncCall(const std::string &input, gint timeout_msec,
                                                     const AiBusinessDataManagementCallSyncFunc &func) {
    char *output = nullptr;
    GError *gError = nullptr;
    std::string errorMsg;
    int retryTime = 0;
    bool success = func(delegate_, input.c_str(), G_DBUS_CALL_FLAGS_NONE, timeout_msec, &output, nullptr, &gError);

    while (!success && retryTime <= CALL_DATA_MANAGEMENT_RETRY_TIME) {
        if (retryTime == CALL_DATA_MANAGEMENT_RETRY_TIME) {
            if (gError) {  // 测试过程中有发现过失败但error为空的情况
                std::fprintf(stderr, "ERROR: call data-management server retry failed, code: %d, msg: %s.\n",
                             gError->code, gError->message);
                errorMsg = gError->message;
                g_error_free(gError);
                gError = nullptr;
            } else {
                std::fprintf(stderr, "ERROR: call data-management server retry failed.\n");
            }
            return createResponse(std::string(), ErrorCode::DM_SESSION_ERROR, errorMsg);
        }

        if (gError) {  // 测试过程中有发现过失败但error为空的情况
            std::fprintf(stdout, "WARN: call data-management server failed, code: %d, msg: %s, retry: %d\n",
                         gError->code, gError->message, retryTime);
            errorMsg = gError->message;
            g_error_free(gError);
            gError = nullptr;
        } else {
            std::fprintf(stdout, "WARN: call data-management server failed, retry: %d\n", retryTime);
        }
        // 断开连接
        destroy();

        sleep(1);
        retryTime++;
        // 重连
        if (!connect()) {
            continue;
        }
        success = func(delegate_, input.c_str(), G_DBUS_CALL_FLAGS_NONE, timeout_msec, &output, nullptr, &gError);
    }

    return output != nullptr ? output : "";
}

DataManagementServiceProxy::~DataManagementServiceProxy() { destroy(); }

bool DataManagementServiceProxy::connect() {
    GError *error = nullptr;

    std::string serverUnixPath =
        "unix:abstract=/tmp/.kylin-ai-business-unix/" + std::to_string(geteuid()) + "/DataManagement.sock";
    connection_ = g_dbus_connection_new_for_address_sync(
        serverUnixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr, nullptr, &error);

    if (connection_ == nullptr) {
        g_printerr("ERROR: connecting to data-management D-Bus address %s failed, code: %d, msg: %s.\n",
                   serverUnixPath.c_str(), error->code, error->message);
        g_error_free(error);
        error = nullptr;
        return false;
    }

    delegate_ = ai_business_data_management_proxy_new_sync(connection_, G_DBUS_PROXY_FLAGS_NONE, nullptr,
                                                           objectPath_.c_str(), nullptr, &error);

    if (delegate_ == nullptr) {
        g_printerr("ERROR: creating data-management processor proxy %s failed, code: %d, msg: %s.\n",
                   objectPath_.c_str(), error->code, error->message);
        g_error_free(error);
        error = nullptr;
        return false;
    }
    return true;
}

void DataManagementServiceProxy::destroy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
        delegate_ = nullptr;
    }
    if (connection_ != nullptr) {
        g_object_unref(connection_);
        connection_ = nullptr;
    }
}

std::string DataManagementServiceProxy::similaritySearch(const std::string &input) {
    return syncFuncCall(input, -1, g_func_similarity_search);
}

std::string DataManagementServiceProxy::addTextFiles(const std::string &input) {
    return syncFuncCall(input, 3600000, g_func_add_text_files);
}

std::string DataManagementServiceProxy::addImageFiles(const std::string &input) {
    return syncFuncCall(input, 3600000, g_func_add_image_files);
}

std::string DataManagementServiceProxy::deleteFiles(const std::string &input) {
    return syncFuncCall(input, -1, g_func_delete_files);
}

std::string DataManagementServiceProxy::updateFilesName(const std::string &input) {
    return syncFuncCall(input, 600000, g_func_update_files_name);
}

std::string DataManagementServiceProxy::updateContentData(const std::string &input) {
    return syncFuncCall(input, 600000, g_func_update_files_content_data);
}

std::string DataManagementServiceProxy::getAllFileinfos() {
    const std::string emptyInput = "{}";
    return syncFuncCall(emptyInput, -1, g_func_get_all_file_infos);
}

std::string DataManagementServiceProxy::getFeatureStatus() {
    const std::string emptyInput = "{}";
    return syncFuncCall(emptyInput, -1, g_func_get_feature_status);
}

std::string DataManagementServiceProxy::getTagsOfFiles(const std::string &input) {
    return syncFuncCall(input, 600000, g_func_get_tags_of_files);
}

std::string DataManagementServiceProxy::getAllTags() {
    const std::string emptyInput = "{}";
    return syncFuncCall(emptyInput, 600000, g_func_get_all_tags);
}

std::string DataManagementServiceProxy::searchSimilarTags(const std::string &input) {
    return syncFuncCall(input, -1, g_func_search_similar_tags);
}

std::string DataManagementServiceProxy::searchFilesAboutTags(const std::string &input) {
    return syncFuncCall(input, -1, g_func_search_files_about_tags);
}

std::string DataManagementServiceProxy::isFileRelevantToTags(const std::string &input) {
    return syncFuncCall(input, 3600000, g_func_is_file_relevant_to_tags);
}

std::string DataManagementServiceProxy::isFileRelevantToDescription(const std::string &input) {
    return syncFuncCall(input, 3600000, g_func_is_file_relevant_to_description);
}

std::string DataManagementServiceProxy::extractSearchConditions(const std::string &input) {
    return syncFuncCall(input, 600000, g_func_extract_search_conditions);
}

std::string DataManagementServiceProxy::getSummarysOfFiles(const std::string &input) {
    return syncFuncCall(input, 600000, g_func_get_summarys_of_files);
}

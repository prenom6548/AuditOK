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
                                                     AiBusinessDataManagementCallSyncFunc func) {
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

    std::string response;
    if (output != nullptr) response = output;

    return response;
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
    return syncFuncCall(input, -1, ai_business_data_management_call_similarity_search_sync);
}

std::string DataManagementServiceProxy::addTextFiles(const std::string &input) {
    return syncFuncCall(input, 3600000, ai_business_data_management_call_add_text_files_sync);
}

std::string DataManagementServiceProxy::addImageFiles(const std::string &input) {
    return syncFuncCall(input, 3600000, ai_business_data_management_call_add_image_files_sync);
}

std::string DataManagementServiceProxy::deleteFiles(const std::string &input) {
    return syncFuncCall(input, -1, ai_business_data_management_call_delete_files_sync);
}

std::string DataManagementServiceProxy::updateFilesName(const std::string &input) {
    return syncFuncCall(input, 600000, ai_business_data_management_call_update_files_name_sync);
}

std::string DataManagementServiceProxy::updateContentData(const std::string &input) {
    return syncFuncCall(input, 600000, ai_business_data_management_call_update_files_content_data_sync);
}

std::string DataManagementServiceProxy::getAllFileinfos() {
    const std::string emptyInput = "{}";
    return syncFuncCall(emptyInput, -1, ai_business_data_management_call_get_all_file_infos_sync);
}

std::string DataManagementServiceProxy::getFeatureStatus() {
    const std::string emptyInput = "{}";
    return syncFuncCall(emptyInput, -1, ai_business_data_management_call_get_feature_status_sync);
}

std::string DataManagementServiceProxy::getTagsOfFiles(const std::string &input) {
    return syncFuncCall(input, 600000, ai_business_data_management_call_get_tags_of_files_sync);
}

std::string DataManagementServiceProxy::getAllTags() {
    const std::string emptyInput = "{}";
    return syncFuncCall(emptyInput, 600000, ai_business_data_management_call_get_all_tags_sync);
}

std::string DataManagementServiceProxy::searchSimilarTags(const std::string &input) {
    return syncFuncCall(input, -1, ai_business_data_management_call_search_similar_tags_sync);
}

std::string DataManagementServiceProxy::searchFilesAboutTags(const std::string &input) {
    return syncFuncCall(input, -1, ai_business_data_management_call_search_files_about_tags_sync);
}

std::string DataManagementServiceProxy::isFileRelevantToTags(const std::string &input) {
    return syncFuncCall(input, 3600000, ai_business_data_management_call_is_file_relevant_to_tags_sync);
}

std::string DataManagementServiceProxy::isFileRelevantToDescription(const std::string &input) {
    return syncFuncCall(input, 3600000, ai_business_data_management_call_is_file_relevant_to_description_sync);
}

std::string DataManagementServiceProxy::extractSearchConditions(const std::string &input) {
    return syncFuncCall(input, 600000, ai_business_data_management_call_extract_search_conditions_sync);
}

std::string DataManagementServiceProxy::getSummarysOfFiles(const std::string &input) {
    return syncFuncCall(input, 600000, ai_business_data_management_call_get_summarys_of_files_sync);
}

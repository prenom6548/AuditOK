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

#include "_datamanagementsession.h"

#include <cstring>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <vector>

DataManagementStatus InitializeStatus(bool success, int errorCode, const char *errorMsg) {
    DataManagementStatus status;
    status.success = success;
    status.errorCode = errorCode;
    std::strncpy(status.errorMsg, errorMsg, sizeof(status.errorMsg) - 1);
    status.errorMsg[sizeof(status.errorMsg) - 1] = '\0';  // 确保字符串以null终止
    return status;
}

namespace {

std::string generateFormatCheckResult(bool supported, const std::string &format) {
    nlohmann::json root;
    root["supported"] = supported;
    root["format"] = format;
    return root.dump();
}

std::string getFileExtension(const std::string &filePath) {
    size_t pos = filePath.find_last_of('.');
    if (pos != std::string::npos) {
        return filePath.substr(pos + 1);
    }
    return std::string();
}

bool isSupportedFormat(const std::string &extension) {
    const std::vector<std::string> supportTextFormat = {"txt",  "pdf",  "docx", "pptx", "xlsx", "xls",
                                                        "xlsd", "html", "xml",  "md",   "json"};
    const std::vector<std::string> supportImageFormat = {"png", "jpg", "jpeg", "jpe", "bmp", "dib"};

    for (const auto &format : supportTextFormat) {
        if (format == extension) return true;
    }
    for (const auto &format : supportImageFormat) {
        if (format == extension) return true;
    }
    return false;
}

DataManagementStatus parseResponse(const std::string &response, nlohmann::json &data) {
    nlohmann::json jsonValue = nlohmann::json::parse(response, nullptr, false);
    std::string msg = "Parse response error: " + response;
    if (jsonValue.is_discarded()) {
        return STATUS_FAILURE(ErrorCode::DM_ERROR_UNKNOWN, msg.c_str());
    }

    bool hasError = jsonValue.contains("err_no");
    bool hasErrorMsg = jsonValue.contains("err_msg");
    bool hasData = jsonValue.contains("data") && !jsonValue["data"].is_null();

    if (!hasError || !hasErrorMsg) {
        return STATUS_FAILURE(ErrorCode::DM_ERROR_UNKNOWN, msg.c_str());
    }
    if (!jsonValue["err_no"].is_number_integer()) {
        return STATUS_FAILURE(ErrorCode::DM_ERROR_UNKNOWN, msg.c_str());
    }
    if (!jsonValue["err_msg"].is_string()) {
        return STATUS_FAILURE(ErrorCode::DM_ERROR_UNKNOWN, msg.c_str());
    }

    if (hasData) {
        data = jsonValue["data"];
    }
    return DATA_MANAGEMENT_STATUS(jsonValue["err_no"], jsonValue["err_msg"].get<std::string>().c_str());
}

DataManagementStatus parseResponse(const std::string &response) {
    nlohmann::json v;
    return parseResponse(response, v);
}

DataManagementStatus parseResponse(const std::string &response, std::string &data, const std::string &key = "") {
    nlohmann::json result;
    DataManagementStatus ret = parseResponse(response, result);
    if (ret.success) {
        data = key.empty() ? result.dump() : result[key].get<std::string>();
    }
    return ret;
}

DataManagementStatus parseResponse(const std::string &response, int &data, const std::string &key) {
    nlohmann::json result;
    DataManagementStatus ret = parseResponse(response, result);
    if (ret.success) {
        data = result[key].get<int>();
    }
    return ret;
}

DataManagementStatus parseResponse(const std::string &response, bool &data, const std::string &key) {
    nlohmann::json result;
    DataManagementStatus ret = parseResponse(response, result);
    if (ret.success) {
        data = result[key].get<bool>();
    }
    return ret;
}

}  // namespace

namespace datamanagement {

DataManagementStatus _DataManagementSession::checkFileFormat(const std::string &filePath, std::string &formatResult) {
    if (!std::filesystem::exists(filePath)) {
        std::string msg = filePath + " not exist.";
        return STATUS_FAILURE(PARSE_PARAM_FAILED, msg.c_str());
    }

    std::string format = getFileExtension(filePath);
    bool isSupported = isSupportedFormat(format);
    formatResult = generateFormatCheckResult(isSupported, format);

    return STATUS_SUCCESS;
}

DataManagementStatus _DataManagementSession::similaritySearch(const std::string &searchConditions,
                                                              std::string &searchResult) {
    std::string response = proxy_->similaritySearch(searchConditions);
    return parseResponse(response, searchResult);
}

DataManagementStatus _DataManagementSession::addTextFiles(const std::string &fileinfos) {
    std::string response = proxy_->addTextFiles(fileinfos);
    return parseResponse(response);
}

DataManagementStatus _DataManagementSession::addImageFiles(const std::string &fileinfos) {
    std::string response = proxy_->addImageFiles(fileinfos);
    return parseResponse(response);
}

DataManagementStatus _DataManagementSession::deleteFiles(const std::string &fileinfos) {
    std::string response = proxy_->deleteFiles(fileinfos);
    return parseResponse(response);
}

DataManagementStatus _DataManagementSession::updateFilesName(const std::string &fileinfos) {
    std::string response = proxy_->updateFilesName(fileinfos);
    return parseResponse(response);
}

DataManagementStatus _DataManagementSession::updateContentData(const std::string &fileinfos) {
    std::string response = proxy_->updateContentData(fileinfos);
    return parseResponse(response);
}

DataManagementStatus _DataManagementSession::getAllFileinfos(std::string &result) {
    std::string response = proxy_->getAllFileinfos();
    return parseResponse(response, result);
}

DataManagementStatus _DataManagementSession::getFeatureStatus(int &status) {
    std::string response = proxy_->getFeatureStatus();
    return parseResponse(response, status, "result");
}

DataManagementStatus _DataManagementSession::getTagsOfFiles(const std::string &files, std::string &result) {
    std::string response = proxy_->getTagsOfFiles(files);
    return parseResponse(response, result);
}

DataManagementStatus _DataManagementSession::getAllTags(std::string &result) {
    std::string response = proxy_->getAllTags();
    return parseResponse(response, result);
}

DataManagementStatus _DataManagementSession::searchSimilarTags(const std::string &tags, std::string &result) {
    std::string response = proxy_->searchSimilarTags(tags);
    return parseResponse(response, result);
}

DataManagementStatus _DataManagementSession::searchFilesAboutTags(const std::string &tags, std::string &result) {
    std::string response = proxy_->searchFilesAboutTags(tags);
    return parseResponse(response, result);
}

DataManagementStatus _DataManagementSession::isFileRelevantToTags(const std::string &fileInfos, bool &isRelevant) {
    std::string response = proxy_->isFileRelevantToTags(fileInfos);
    return parseResponse(response, isRelevant, "is_relevant");
}

DataManagementStatus _DataManagementSession::isFileRelevantToDescription(const std::string &fileInfos,
                                                                         bool &isRelevant) {
    std::string response = proxy_->isFileRelevantToDescription(fileInfos);
    return parseResponse(response, isRelevant, "is_relevant");
}

DataManagementStatus _DataManagementSession::extractSearchConditions(const std::string &description,
                                                                     std::string &conditions) {
    std::string response = proxy_->extractSearchConditions(description);
    return parseResponse(response, conditions);
}

DataManagementStatus _DataManagementSession::getSummarysOfFiles(const std::string &files, std::string &summary) {
    std::string response = proxy_->getSummarysOfFiles(files);
    return parseResponse(response, summary);
}

bool _DataManagementSession::connect() {
    bool ret = proxy_->connect();
    if (!ret) {
        std::fprintf(stderr, "ERROR: connect data-management server failed.\n");
        return false;
    }

    std::fprintf(stdout, "INFO: connect data-management server success.\n");
    return true;
}

}  // namespace datamanagement

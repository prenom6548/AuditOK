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

#include "kylin-ai/business/data-management/datamanagement.h"

#include <array>
#include <cstring>
#include <iostream>
#include <set>

#include "_datamanagementsession.h"

#define STATUS_SESSION_ERROR (DataManagementStatus){false, ErrorCode::DM_SESSION_ERROR, "Invalit session"}

namespace {
bool is_valid_datamanagement_session(void* session) {
    auto* dataManagementSession = static_cast<datamanagement::_DataManagementSession*>(session);
    return dataManagementSession != nullptr;
}

std::string get_uname() {
    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("uname -m", "r"), pclose);

    if (!pipe) {
        std::cerr << "ERROR: DataManagement popen uname failed!" << std::endl;
        return std::string();
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // 移除可能存在的换行符
    result.erase(result.find_last_not_of("\n\r") + 1);

    return result;
}

bool is_subsystem_plugin_installed() {
    const std::string command = "LC_ALL=C /usr/bin/dpkg -s kylin-ai-subsystem-plugin 2>/dev/null";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        std::cerr << "ERROR: DataManagement popen dpkg failed!" << std::endl;
        return false;
    }

    std::array<char, 512> buffer{};
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line(buffer.data());
        if (line.find("Status: install ok installed") != std::string::npos) {
            return true;
        }
    }

    return false;
}

}  // namespace

/**
 * @return true: 当前架构支持且子系统插件已安装；false：当前架构不支持或子系统插件未安装
 */
bool data_management_is_feature_supported() {
    // 目前只支持 x86_64 和 arm64
    const std::set<std::string> supportedUname = {
        "x86_64",
        "aarch64",
    };

    std::string uname = get_uname();
    bool is_installed = is_subsystem_plugin_installed();
    bool is_uname_supported = supportedUname.find(uname) != supportedUname.end();

    std::cout << "DataManagement: uname is " << uname.c_str() << ", "
              << (is_uname_supported ? "supported" : "not supported") << std::endl;
    std::cout << "DataManagement: subsystem plugin is "
              << (is_installed ? "installed" : "not installed") << std::endl;

    return is_installed && is_uname_supported;
}

DataManagementSession* data_management_create_session() {
    auto* dataManagementSession = new datamanagement::_DataManagementSession();
    if (dataManagementSession->connect()) {
        return reinterpret_cast<DataManagementSession*>(dataManagementSession);
    } else {
        delete dataManagementSession;
        return nullptr;
    }
}

void data_management_destroy_session(DataManagementSession* session) {
    if (session == nullptr) {
        return;
    }

    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    delete dataManagementSession;
}

DataManagementStatus data_management_get_feature_status(DataManagementSession* session, FeatureStatus* status) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    int status_int;
    DataManagementStatus getResult = dataManagementSession->getFeatureStatus(status_int);
    *status = static_cast<FeatureStatus>(status_int);

    return getResult;
}

DataManagementStatus data_management_check_file_format(DataManagementSession* session, const char* filepath,
                                                       char** check_result) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    std::string formatResult;
    DataManagementStatus checkResult = dataManagementSession->checkFileFormat(filepath, formatResult);

    if (!formatResult.empty()) {
        size_t length = formatResult.size() + 1;
        *check_result = new char[length];
        std::strcpy(*check_result, formatResult.c_str());
    }

    return checkResult;
}

DataManagementStatus data_management_similarity_search(DataManagementSession* session, const char* search_conditions,
                                                       char** search_result) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    std::string searchResult;
    DataManagementStatus result = dataManagementSession->similaritySearch(search_conditions, searchResult);

    size_t length = searchResult.size() + 1;
    *search_result = new char[length];
    std::strcpy(*search_result, searchResult.c_str());

    return result;
}

DataManagementStatus data_management_add_text_files(DataManagementSession* session, const char* fileinfos) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    return dataManagementSession->addTextFiles(fileinfos);
}

DataManagementStatus data_management_add_image_files(DataManagementSession* session, const char* fileinfos) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    return dataManagementSession->addImageFiles(fileinfos);
}

DataManagementStatus data_management_delete_files(DataManagementSession* session, const char* fileinfos) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    return dataManagementSession->deleteFiles(fileinfos);
}

DataManagementStatus data_management_update_files_name(DataManagementSession* session, const char* fileinfos) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    return dataManagementSession->updateFilesName(fileinfos);
}

DataManagementStatus data_management_update_files_content(DataManagementSession* session, const char* fileinfos) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    return dataManagementSession->updateContentData(fileinfos);
}

DataManagementStatus data_management_get_all_fileinfos(DataManagementSession* session, char** fileinfos_result) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    std::string fileinfosResult;
    DataManagementStatus result = dataManagementSession->getAllFileinfos(fileinfosResult);

    size_t length = fileinfosResult.size() + 1;
    *fileinfos_result = new char[length];
    std::strcpy(*fileinfos_result, fileinfosResult.c_str());

    return result;
}

void data_management_destroy_result(char* result) { delete[] result; }

DataManagementStatus data_management_get_tags_of_files(DataManagementSession* session, const char* files,
                                                       char** result) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    std::string tags;
    DataManagementStatus status = dataManagementSession->getTagsOfFiles(files, tags);

    size_t length = tags.size() + 1;
    *result = new char[length];
    std::strcpy(*result, tags.c_str());

    return status;
}

DataManagementStatus data_management_get_all_tags(DataManagementSession* session, char** result) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    std::string tags;
    DataManagementStatus status = dataManagementSession->getAllTags(tags);

    size_t length = tags.size() + 1;
    *result = new char[length];
    std::strcpy(*result, tags.c_str());

    return status;
}

DataManagementStatus data_management_search_similar_tags(DataManagementSession* session, const char* tags,
                                                         char** result) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    std::string similarTags;
    DataManagementStatus status = dataManagementSession->searchSimilarTags(tags, similarTags);

    size_t length = similarTags.size() + 1;
    *result = new char[length];
    std::strcpy(*result, similarTags.c_str());

    return status;
}

DataManagementStatus data_management_search_files_about_tags(DataManagementSession* session, const char* tags,
                                                             char** result) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    std::string files;
    DataManagementStatus status = dataManagementSession->searchFilesAboutTags(tags, files);

    size_t length = files.size() + 1;
    *result = new char[length];
    std::strcpy(*result, files.c_str());

    return status;
}

DataManagementStatus data_management_is_file_relevant_to_tags(DataManagementSession* session, const char* fileInfos,
                                                              bool* isRelevant) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    return dataManagementSession->isFileRelevantToTags(fileInfos, *isRelevant);
}

DataManagementStatus data_management_is_file_relevant_to_description(DataManagementSession* session,
                                                                     const char* fileInfos, bool* isRelevant) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    return dataManagementSession->isFileRelevantToDescription(fileInfos, *isRelevant);
}

DataManagementStatus data_management_extract_search_conditions(DataManagementSession* session, const char* description,
                                                               char** result) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    std::string conditions;
    DataManagementStatus status = dataManagementSession->extractSearchConditions(description, conditions);

    size_t length = conditions.size() + 1;
    *result = new char[length];
    std::strcpy(*result, conditions.c_str());

    return status;
}

DataManagementStatus data_management_get_summarys_of_files(DataManagementSession* session, const char* files,
                                                           char** result) {
    auto* dataManagementSession = reinterpret_cast<datamanagement::_DataManagementSession*>(session);
    if (!dataManagementSession) {
        return STATUS_SESSION_ERROR;
    }

    std::string summary;
    DataManagementStatus status = dataManagementSession->getSummarysOfFiles(files, summary);

    size_t length = summary.size() + 1;
    *result = new char[length];
    std::strcpy(*result, summary.c_str());

    return status;
}

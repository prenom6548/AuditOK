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

#include "dataManagement.h"

#include "log.h"

void dataManagementSyncResponse(const std::string& method, GObject* delegate, GDBusMethodInvocation* invocation,
                                const std::string& output) {
    if (method == "getFeatureStatus") {
        ai_business_data_management_complete_get_feature_status(reinterpret_cast<AiBusinessDataManagement*>(delegate),
                                                                invocation, output.c_str());
        PRINT_DEBUG("getFeatureStatus method.\n");
    } else if (method == "similaritySearch") {
        ai_business_data_management_complete_similarity_search(reinterpret_cast<AiBusinessDataManagement*>(delegate),
                                                               invocation, output.c_str());
        PRINT_DEBUG("similaritySearch method.\n");
    } else if (method == "addTextFiles") {
        ai_business_data_management_complete_add_text_files(reinterpret_cast<AiBusinessDataManagement*>(delegate),
                                                            invocation, output.c_str());
        PRINT_DEBUG("addTextFiles method.\n");
    } else if (method == "addImageFiles") {
        ai_business_data_management_complete_add_image_files(reinterpret_cast<AiBusinessDataManagement*>(delegate),
                                                             invocation, output.c_str());
        PRINT_DEBUG("addImageFiles method.\n");
    } else if (method == "deleteFiles") {
        ai_business_data_management_complete_delete_files(reinterpret_cast<AiBusinessDataManagement*>(delegate),
                                                          invocation, output.c_str());
        PRINT_DEBUG("deleteFiles method.\n");
    } else if (method == "updateFilesName") {
        ai_business_data_management_complete_update_files_name(reinterpret_cast<AiBusinessDataManagement*>(delegate),
                                                               invocation, output.c_str());
        PRINT_DEBUG("updateFilesName method.\n");
    } else if (method == "updateFilesContentData") {
        ai_business_data_management_complete_update_files_content_data(
            reinterpret_cast<AiBusinessDataManagement*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("updateFilesContentData method.\n");
    } else if (method == "getAllFileInfos") {
        ai_business_data_management_complete_get_all_file_infos(reinterpret_cast<AiBusinessDataManagement*>(delegate),
                                                                invocation, output.c_str());
        PRINT_DEBUG("getAllFileInfos method.\n");
    } else if (method == "getTagsOfFiles") {
        ai_business_data_management_complete_get_tags_of_files(reinterpret_cast<AiBusinessDataManagement*>(delegate),
                                                               invocation, output.c_str());
        PRINT_DEBUG("getTagsOfFiles method.\n");
    } else if (method == "getAllTags") {
        ai_business_data_management_complete_get_all_tags(reinterpret_cast<AiBusinessDataManagement*>(delegate),
                                                          invocation, output.c_str());
        PRINT_DEBUG("getAllTags method.\n");
    } else if (method == "searchSimilarTags") {
        ai_business_data_management_complete_search_similar_tags(reinterpret_cast<AiBusinessDataManagement*>(delegate),
                                                                 invocation, output.c_str());
        PRINT_DEBUG("searchSimilarTags method.\n");
    } else if (method == "searchFilesAboutTags") {
        ai_business_data_management_complete_search_files_about_tags(
            reinterpret_cast<AiBusinessDataManagement*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("searchFilesAboutTags method.\n");
    } else if (method == "isFileRelevantToTags") {
        ai_business_data_management_complete_is_file_relevant_to_tags(
            reinterpret_cast<AiBusinessDataManagement*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("isFileRelevantToTags method.\n");
    } else if (method == "isFileRelevantToDescription") {
        ai_business_data_management_complete_is_file_relevant_to_description(
            reinterpret_cast<AiBusinessDataManagement*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("isFileRelevantToDescription method.\n");
    } else if (method == "extractSearchConditions") {
        ai_business_data_management_complete_extract_search_conditions(
            reinterpret_cast<AiBusinessDataManagement*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("extractSearchConditions method.\n");
    } else if (method == "getSummarysOfFiles") {
        ai_business_data_management_complete_get_summarys_of_files(
            reinterpret_cast<AiBusinessDataManagement*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("getSummarysOfFiles method.\n");
    } else {
        PRINT_ERROR("dataManagement service unknown method: %s\n", method.c_str());
    }
}

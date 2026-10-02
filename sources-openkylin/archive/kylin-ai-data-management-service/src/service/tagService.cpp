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

#include "tagService.h"

#include <kyai-business-framework/log.h>
#include <kyai-business-framework/utils.h>

#include <algorithm>
#include <nlohmann/json.hpp>

#include "dataManagementVectorDatabase.h"
#include "embedding.h"

std::unordered_set<int64_t> TagService::searchSimilarTagIdsByTagNames(const std::vector<std::string>& tags) {
    std::unordered_set<int64_t> output;
    Embedding embedding(DataManagementEmbeddingDataType::Text, scene::TaskPriority::HIGH);

    for (const auto& tag : tags) {
        // 对 tag 内容进行向量化
        std::vector<float> embeddingResult = embedding.getEmbedding(tag);
        if (embeddingResult.empty()) {
            PRINT_ERROR("Text embedding result is empty.\n");
            return std::unordered_set<int64_t>();
        }

        // 根据向量值查询相似标签ID
        SimilaritySearchMetadataResult metadata = DataManagementVectorDatabase::getInstance().searchMetadata(
            FILE_TAGS_COLLECTION_NAME, embeddingResult, 10, TAG_SEARCH_THRESHOLD);
        if (metadata.empty()) {
            PRINT_WARN("SearchMetadata result is empty. embedding size: %zu\n", embeddingResult.size());
            return std::unordered_set<int64_t>();
        }

        for (const auto& item : metadata) {
            int64_t tagId = item.metadata["tag_id"];
            output.insert(tagId);
        }
    }
    return output;
}

std::vector<std::string> TagService::searchSimilarTagNames(const std::vector<std::string>& tags) {
    std::unordered_set<int64_t> tagIds = searchSimilarTagIdsByTagNames(tags);
    if (tagIds.empty()) {
        PRINT_WARN("SearchSimilarTagNames search tag id is empty.\n");
        return std::vector<std::string>();
    }

    std::vector<std::string> tagNames = FileDatabase::getTagNamesByTagIds(db, tagIds);
    if (tagNames.empty()) {
        PRINT_WARN("SearchSimilarTagNames search tag name is empty.\n");
        return std::vector<std::string>();
    }
    return tagNames;
}

std::vector<std::string> TagService::searchSimilarFilePaths(const std::vector<std::string>& tags) {
    std::unordered_set<int64_t> tagIds = searchSimilarTagIdsByTagNames(tags);
    if (tagIds.empty()) {
        PRINT_WARN("SearchSimilarFilePaths search tag id is empty.\n");
        return std::vector<std::string>();
    }

    std::vector<std::string> filePaths = FileDatabase::getFilePathsByTagIds(db, tagIds);
    if (filePaths.empty()) {
        PRINT_WARN("SearchSimilarFilePaths search file path is empty.\n");
        return std::vector<std::string>();
    }
    return filePaths;
}

void TagService::deleteFileTags(int64_t fileId, const std::vector<int64_t>& tagIds) {
    for (const auto& tagId : tagIds) {
        SQLite::Transaction transaction(db);
        FileDatabase::deleteFileTagRelation(db, fileId, tagId);

        if (FileDatabase::isTagInUse(db, tagId)) {
            transaction.commit();
            continue;
        }

        // 删除 tag 信息
        std::string expression = "tag_id == " + std::to_string(tagId);
        DataManagementVectorDatabase::getInstance().deleteVectorData(FILE_TAGS_COLLECTION_NAME, expression);

        FileDatabase::deleteTagInfo(db, tagId);
        transaction.commit();
    }
}

void TagService::deleteFileTags(const std::unordered_set<int64_t>& deletedFileIds) {
    if (deletedFileIds.empty()) {
        PRINT_WARN("TagService::deleteFileTags input is empty.\n");
        return;
    }

    for (const auto& fileId : deletedFileIds) {
        std::vector<int64_t> tagIds = FileDatabase::getTagIdsByFileId(db, fileId);
        deleteFileTags(fileId, tagIds);
    }
}

void TagService::updateFileTags(int64_t fileId, const std::set<std::string>& newTags) {
    std::vector<int64_t> newTagIds;
    std::vector<int64_t> oldTagIds = FileDatabase::getTagIdsByFileId(db, fileId);

    for (const auto& tagName : newTags) {
        SQLite::Transaction transaction(db);
        int64_t tagId;
        bool ret = FileDatabase::getTagIdByTagName(db, tagName, tagId);
        if (ret) {
            newTagIds.push_back(tagId);
            // 检查 tagId 和 fileId 的关系
            if (std::find(oldTagIds.begin(), oldTagIds.end(), tagId) != oldTagIds.end()) {
                PRINT_DEBUG("updateFileTags tag name: %s already exist.\n", tagName.c_str());
            } else {
                FileDatabase::insertFileTag(db, fileId, tagId);
            }
            transaction.commit();
            continue;
        }

        // 完全没记录，需要存入新标签
        bool success = FileDatabase::insertNewTagName(db, fileId, tagName, tagId);
        if (!success) {
            PRINT_ERROR("updateFileTags insert %s failed.\n", tagName.c_str());
            continue;
        }

        Embedding textEmbedding(DataManagementEmbeddingDataType::Text, scene::TaskPriority::MEDIUM);
        std::vector<float> embedding = textEmbedding.getEmbedding(tagName);
        if (embedding.empty()) {
            PRINT_ERROR("tag embedding result is empty. tag: %s\n", tagName.c_str());
            continue;
        }

        nlohmann::json metadata;
        metadata["tag_id"] = tagId;
        std::string uuid = Utils::generateUUID();
        PRINT_DEBUG("updateFileTags uuid: %s, metadata: %s.\n", uuid.c_str(), metadata.dump().c_str());

        // 存入标签数据库
        success = DataManagementVectorDatabase::getInstance().insertVectorData(
            FILE_TAGS_COLLECTION_NAME, {DataManagementVectorInfo{uuid, embedding, metadata}});
        if (!success) {
            PRINT_ERROR("updateFileTags insert into %s failed.\n", FILE_TAGS_COLLECTION_NAME);
            continue;
        }

        transaction.commit();
    }

    // 比较是否有冗余的标签
    std::vector<int64_t> difference;

    std::set_difference(oldTagIds.begin(), oldTagIds.end(), newTagIds.begin(), newTagIds.end(),
                        std::back_inserter(difference));
    PRINT_DEBUG("updateFileTags %s difference %s is %s.\n", FileDatabase::vectorToString(oldTagIds).c_str(),
                FileDatabase::vectorToString(newTagIds).c_str(), FileDatabase::vectorToString(difference).c_str());
    if (!difference.empty()) {
        // 删除以前多余的标签
        deleteFileTags(fileId, difference);
    }
}

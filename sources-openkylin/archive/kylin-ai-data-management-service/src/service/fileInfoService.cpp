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

#include "fileInfoService.h"

#include <kyai-business-framework/log.h>
#include <kyai-business-framework/utils.h>
#include <sys/stat.h>

#include <ctime>
#include <filesystem>
#include <set>

#include "aiIndex.h"
#include "constant.h"
#include "embedding.h"

// 该方法用于去获取新增加文件或更新文件的信息
void FileInfoService::classifyFile(const std::unordered_set<std::string>& filePaths,
                                   std::unordered_set<std::string>& newFilePaths,
                                   std::vector<FileInfoDO>& updateFileInfos) {
    if (filePaths.empty()) {
        PRINT_WARN("FileInfoService::classifyFile input is empty.\n");
        return;
    }

    for (const auto& filePath : filePaths) {
        FileInfoDO fileInfo;
        // 查询文件信息
        bool success = FileDatabase::getSingleFileInfo(db, filePath, fileInfo);
        if (!success) {
            // 全新的文件
            newFilePaths.insert(filePath);
            continue;
        }
        // 需要更新的文件
        updateFileInfos.push_back(fileInfo);
    }
}

bool FileInfoService::deleteTextFileVectorInfo(const std::unordered_set<int64_t>& fileIds) {
    if (fileIds.empty()) {
        PRINT_WARN("deleteTextFileVectorInfo input fileIds is empty, no need delete.\n");
        return true;
    }

    std::string ids = Utils::joinIds(fileIds);
    if (ids.empty()) {
        PRINT_ERROR("deleteTextFileVectorInfo join ids result is empty, fileIds size: %zu.\n", fileIds.size());
        return false;
    }

    std::string expression = "file_id in [" + ids + "]";
    PRINT_DEBUG("expression: %s\n", expression.c_str());
    bool ret =
        DataManagementVectorDatabase::getInstance().deleteVectorData(TEXT_FILE_CONTENT_COLLECTION_NAME, expression);
    if (!ret) {
        PRINT_ERROR("deleteTextFileVectorInfo delete text vector db failed.\n");
        return false;
    }

    return true;
}

bool FileInfoService::deleteFiles(const std::unordered_set<int64_t>& fileIds) {
    if (fileIds.empty()) {
        PRINT_WARN("DeleteFiles input fileIds is empty, no need delete.\n");
        return true;
    }

    // 先删除 向量数据库 中的数据，以免 file_id 对应不上
    std::string ids = Utils::joinIds(fileIds);
    if (ids.empty()) {
        PRINT_ERROR("DeleteFiles join ids result is empty, fileIds size: %zu.\n", fileIds.size());
        return false;
    }

    bool ret = deleteVectorData(ids);
    if (!ret) {
        PRINT_ERROR("DeleteFiles delete vector db data failed.\n");
        return false;
    }

    // 删除 SQLITE 中的数据
    ret = FileDatabase::deleteByFileIds(db, ids);
    if (!ret) {
        PRINT_ERROR("DeleteFiles delete sqlite db data failed.\n");
        return false;
    }

    return true;
}

bool FileInfoService::deleteVectorData(const std::string& fileIds) {
    std::string expression = "file_id in [" + fileIds + "]";
    PRINT_DEBUG("expression: %s\n", expression.c_str());
    bool ret =
        DataManagementVectorDatabase::getInstance().deleteVectorData(TEXT_FILE_CONTENT_COLLECTION_NAME, expression);
    if (!ret) {
        PRINT_ERROR("handleDeleteFiles delete text vector db failed.\n");
        return false;
    }

    ret = DataManagementVectorDatabase::getInstance().deleteVectorData(VISION_FILE_CONTENT_COLLECTION_NAME, expression);
    if (!ret) {
        PRINT_ERROR("handleDeleteFiles delete image vector db failed.\n");
        return false;
    }
    return true;
}

ERROR_CODE FileInfoService::searchFilesByText(const std::string& text, SimilaritySearchResult& result) {
    // 用于搜索的向量化请求需要设置为高优先级
    Embedding textEmbedding(DataManagementEmbeddingDataType::Text, scene::TaskPriority::HIGH);
    Embedding imageTextEmbedding(DataManagementEmbeddingDataType::ImageText, scene::TaskPriority::HIGH);

    PRINT_DEBUG("start embedding.\n");
    std::vector<float> textSideEmbedding = textEmbedding.getEmbedding(text);
    if (textSideEmbedding.empty()) {
        PRINT_ERROR("SearchFilesByText get text embedding failed, text: %s.\n", text.c_str());
        return EMBEDDING_FAILED;
    }
    std::vector<float> imageSideEmbedding = imageTextEmbedding.getEmbedding(text);
    if (imageSideEmbedding.empty()) {
        PRINT_ERROR("SearchFilesByText get imagetext embedding failed, text: %s.\n", text.c_str());
        return EMBEDDING_FAILED;
    }

    // 用向量去向量数据库里查
    PRINT_DEBUG("start to search vector db.\n");
    SimilaritySearchMetadataResult textMetadata = DataManagementVectorDatabase::getInstance().searchMetadata(
        TEXT_FILE_CONTENT_COLLECTION_NAME, textSideEmbedding, 50, TEXT_SEARCH_THRESHOLD);
    SimilaritySearchMetadataResult imagMetadata = DataManagementVectorDatabase::getInstance().searchMetadata(
        VISION_FILE_CONTENT_COLLECTION_NAME, imageSideEmbedding, 50, VISION_SEARCH_THRESHOLD);
    if (textMetadata.empty() && imagMetadata.empty()) {
        PRINT_INFO("Search nothing by text: %s.\n", text.c_str());
        result = SimilaritySearchResult();
        return SUCCESS;
    }

    if (!textMetadata.empty()) {
        SimilaritySearchResult textSearchResult = searchResultConvert(textMetadata);
        result.insert(result.end(), textSearchResult.begin(), textSearchResult.end());
    }

    if (!imagMetadata.empty()) {
        SimilaritySearchResult visionSearchResult = searchResultConvert(imagMetadata);
        result.insert(result.end(), visionSearchResult.begin(), visionSearchResult.end());
    }
    return SUCCESS;
}

/**
 * 获取文件的最后修改时间
 * @param filePath
 * @return
 */
std::string FileInfoService::getLastModifyTime(const std::string& filePath) {
    std::string defaultTime = "0000-00-00 00:00:00";

    if (!std::filesystem::exists(filePath)) {
        PRINT_ERROR("File %s don't exist, can't get modify time.\n", filePath.c_str());
        return defaultTime;
    }

    struct stat fileStat {};
    if (stat(filePath.c_str(), &fileStat) != 0) {
        PRINT_ERROR("File %s can't get modify time.\n", filePath.c_str());
        return defaultTime;
    }

    // 将时间戳转换为本地时间
    std::time_t mtime = fileStat.st_mtime;
    std::tm* tm_local = localtime(&mtime);
    if (tm_local == nullptr) {
        PRINT_ERROR("File %s converting time to local time failed.\n", filePath.c_str());
        return defaultTime;
    }

    // 定义输出格式
    char buffer[32];
    memset(buffer, 0, sizeof(buffer));  // 初始化buffer为全零，以防万一
    if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_local) == 0) {
        PRINT_ERROR("File %s formatting time failed.\n", filePath.c_str());
        return defaultTime;
    }

    return buffer;
}

SimilaritySearchResult FileInfoService::searchResultConvert(const SimilaritySearchMetadataResult& metadataResult) {
    SimilaritySearchResult output;
    std::set<int64_t> exist;
    for (const auto& item : metadataResult) {
        PRINT_DEBUG("SearchResultConvert metadata: %s, similarity: %f, id: %s.\n", item.metadata.dump().c_str(),
                    item.similarity, item.id.c_str());
        int64_t fileId = item.metadata["file_id"];

        // 由于文本文件是分割后存储，搜索可能会返回同一个文件中的多个段落，合并结果，只保留同一文件最高相似度的结果
        // 相同文件，结果只返回相似度的最大值，查询结果是按照距离排序的，所以相同文件只存第一个即可
        auto ret = exist.insert(fileId);
        if (!ret.second) {
            PRINT_DEBUG("SearchResultConvert file_id: %ld already add.\n", fileId);
            continue;
        }

        std::string filePath = FileDatabase::getFilePathById(db, fileId);
        if (filePath.empty()) {
            PRINT_ERROR("Illegal result! Cannot find file path, id: %ld\n", fileId);
            continue;
        }

        output.emplace_back(std::make_pair(filePath, item.similarity));
    }
    return output;
}

void FileInfoService::insertTask(const std::unordered_set<std::string>& filePaths) {
    if (filePaths.empty()) {
        PRINT_WARN("FileInfoService::insertTask input is empty.\n");
        return;
    }

    for (const auto& filePath : filePaths) {
        SQLite::Transaction transaction(db);
        // 查询文件修改时间
        std::string modifyTime = FileInfoService::getLastModifyTime(filePath);

        FileInfoDO fileInfo;
        fileInfo.enable = 0;
        fileInfo.filePath = filePath;
        fileInfo.modifyTime = modifyTime;

        bool success = FileDatabase::insertFileInfo(db, fileInfo);
        if (!success) {
            PRINT_ERROR("%s text insert file info failed.\n", filePath.c_str());
            continue;
        }

        success = FileDatabase::insertTextTask(db, fileInfo.fileId, filePath);
        if (!success) {
            PRINT_ERROR("%s text insert ai index task failed.\n", filePath.c_str());
            continue;
        }
        transaction.commit();
    }
    // 向量化任务同步执行
    DataManagement::AIIndex::getInstance().notifyTaskByType({DataManagement::TaskType::TASK_TYPE_TEXT_EMBEDDING});
}

std::unordered_set<int64_t> FileInfoService::updateTask(std::vector<FileInfoDO>& fileInfos) {
    if (fileInfos.empty()) {
        PRINT_WARN("FileInfoService::updateTask input is empty.\n");
        return {};
    }

    std::unordered_set<int64_t> updateIds;
    // 首先判断判断是否是真需要修改的文件
    for (auto& fileInfo : fileInfos) {
        SQLite::Transaction transaction(db);
        std::string modifyTime = FileInfoService::getLastModifyTime(fileInfo.filePath);

        // 有记录，更新时间相同，直接跳过
        if (modifyTime == fileInfo.modifyTime) {
            PRINT_INFO("%s modify time %s is same, skip this file.\n", fileInfo.filePath.c_str(), modifyTime.c_str());
            continue;
        }

        fileInfo.enable = 0;
        fileInfo.fileSummary = "";
        fileInfo.modifyTime = modifyTime;
        bool success = FileDatabase::updateFileInfo(db, fileInfo);
        if (!success) {
            PRINT_ERROR("%s text update file info failed.\n", fileInfo.filePath.c_str());
            continue;
        }

        success = FileDatabase::updateTextTask(db, fileInfo.fileId);
        if (!success) {
            PRINT_ERROR("%s text update ai index task failed.\n", fileInfo.filePath.c_str());
            continue;
        }
        transaction.commit();

        // 获取更新文件id
        updateIds.insert(fileInfo.fileId);
    }
    return updateIds;
}

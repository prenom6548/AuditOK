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

#include "imageService.h"

#include <kyai-business-framework/log.h>
#include <kyai-business-framework/utils.h>

#include "dataManagementVectorDatabase.h"
#include "embedding.h"
#include "fileInfoService.h"

void ImageService::embedding(const std::unordered_set<std::string>& imgFilePaths) {
    if (imgFilePaths.empty()) {
        PRINT_WARN("ImageService::embedding input is empty.\n");
        return;
    }

    for (const auto& filePath : imgFilePaths) {
        // 查询文件信息，根据更新时间判断文件是否真的有修改
        std::string modifyTime = FileInfoService::getLastModifyTime(filePath);
        FileInfoDO fileInfo;
        bool success = FileDatabase::getSingleFileInfo(db, filePath, fileInfo);
        if (success) {
            // 有记录，更新时间相同，直接跳过
            if (fileInfo.enable && modifyTime == fileInfo.modifyTime) {
                PRINT_INFO("%s modify time %s is same, skip this file.\n", filePath.c_str(), modifyTime.c_str());
                continue;
            }

            // 图片向量化
            Embedding imageEmbedding(DataManagementEmbeddingDataType::ImageFlie, scene::TaskPriority::MEDIUM);
            std::vector<float> embedding = imageEmbedding.getEmbedding(filePath);
            if (embedding.empty()) {
                PRINT_ERROR("%s image embedding result is empty.\n", filePath.c_str());
                continue;
            }

            // 向量化任务比较耗时，避免长时间占用sqlite的写锁，向量化任务结束后，在开启事务，更新文件信息和向量数据库
            SQLite::Transaction transaction(db);
            if (!updateImageFileInfo(fileInfo, modifyTime, embedding)) {
                PRINT_ERROR("%s update image file info failed.\n", filePath.c_str());
                continue;
            }

            transaction.commit();
            continue;
        }

        // 图片向量化
        Embedding imageEmbedding(DataManagementEmbeddingDataType::ImageFlie, scene::TaskPriority::MEDIUM);
        std::vector<float> embedding = imageEmbedding.getEmbedding(filePath);
        if (embedding.empty()) {
            PRINT_ERROR("%s image embedding result is empty.\n", filePath.c_str());
            continue;
        }

        SQLite::Transaction transaction(db);
        fileInfo.filePath = filePath;
        if (!insertImageFileInfo(fileInfo, modifyTime, embedding)) {
            PRINT_ERROR("%s insert image file info failed.\n", filePath.c_str());
            continue;
        }
        transaction.commit();
    }
}

bool ImageService::insertImageFileInfo(FileInfoDO& fileInfo, const std::string& modifyTime,
                                       const std::vector<float>& embedding) {
    // 全新的文件，新插入记录
    fileInfo.enable = 1;
    fileInfo.modifyTime = modifyTime;
    bool success = FileDatabase::insertFileInfo(db, fileInfo);
    if (!success) {
        PRINT_ERROR("%s image insert file info failed.\n", fileInfo.filePath.c_str());
        return false;
    }

    if (!updateVectorDatabaseData(fileInfo, embedding)) {
        PRINT_ERROR("%s image update vector data failed.\n", fileInfo.filePath.c_str());
        return false;
    }

    return true;
}

bool ImageService::updateImageFileInfo(FileInfoDO& fileInfo, const std::string& modifyTime,
                                       const std::vector<float>& embedding) {
    // 图片只需更新修改时间，无摘要信息
    fileInfo.enable = 1;
    fileInfo.modifyTime = modifyTime;
    bool success = FileDatabase::updateFileInfo(db, fileInfo);
    if (!success) {
        PRINT_ERROR("%s image update file info failed.\n", fileInfo.filePath.c_str());
        return false;
    }

    if (!updateVectorDatabaseData(fileInfo, embedding)) {
        PRINT_ERROR("%s image update vector data failed.\n", fileInfo.filePath.c_str());
        return false;
    }

    return true;
}

bool ImageService::updateVectorDatabaseData(const FileInfoDO& fileInfo, const std::vector<float>& embedding) {
    // 存入向量数据库，先删除再添加
    std::string expression = "file_id == " + std::to_string(fileInfo.fileId);
    bool success =
        DataManagementVectorDatabase::getInstance().deleteVectorData(VISION_FILE_CONTENT_COLLECTION_NAME, expression);
    if (!success) {
        PRINT_ERROR("%s delete image vector data failed.\n", fileInfo.filePath.c_str());
        return false;
    }

    std::string uuid = Utils::generateUUID();
    nlohmann::json metadata;
    metadata["file_id"] = fileInfo.fileId;
    success = DataManagementVectorDatabase::getInstance().insertVectorData(
        VISION_FILE_CONTENT_COLLECTION_NAME, {DataManagementVectorInfo{uuid, embedding, metadata}});
    if (!success) {
        PRINT_ERROR("%s insert image vector data failed.\n", fileInfo.filePath.c_str());
        return false;
    }

    return true;
}
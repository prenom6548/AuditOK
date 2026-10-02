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

#include <scene.h>

#include "fileDatabase.h"
#include "fileInfoService.h"
#include "tagService.h"

std::unordered_set<int64_t> getDeletedFileIds(const std::vector<std::string>& filePaths) {
    std::unordered_set<int64_t> deletedFileIds;
    SQLite::Database database = FileDatabase::createDatabase();

    // 从 sqlite 根据文件名查询 文件id
    for (const auto& filePath : filePaths) {
        std::vector<int64_t> fileIds = FileDatabase::getMultiFileId(database, filePath);
        if (fileIds.empty()) {
            PRINT_DEBUG("Can't get any data from path: %s\n", filePath.c_str());
            continue;
        }
        deletedFileIds.insert(fileIds.begin(), fileIds.end());
    }
    return deletedFileIds;
}

bool deleteFileInfo(const std::unordered_set<int64_t>& deletedFileIds) {
    FileInfoService fileInfoService;
    return fileInfoService.deleteFiles(deletedFileIds);
}

void deleteTagInfo(const std::unordered_set<int64_t>& deletedFileIds) {
    TagService tagService;
    tagService.deleteFileTags(deletedFileIds);
}

Response handleDeleteFiles(const Request& request) {
    PRINT_DEBUG("HandleDeleteFiles request: %s\n", request.toString().c_str());

    // 解析参数
    nlohmann::json values = request.getValues();
    if (!values.is_array() || values.empty()) {
        PRINT_ERROR("HandleDeleteFiles request is not array of is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    std::vector<std::string> filePaths;
    for (const auto& item : values) {
        if (!item.is_object() || !item.contains("filepath") || !item["filepath"].is_string()) {
            PRINT_ERROR("HandleDeleteFiles request json is error.\n");
            continue;
        }
        filePaths.push_back(item["filepath"].get<std::string>());
    }

    std::unordered_set<int64_t> deletedFileIds = getDeletedFileIds(filePaths);
    if (deletedFileIds.empty()) {
        PRINT_WARN("Can't get any data from input: %s\n", request.toString().c_str());
        return Response::success();
    }

    // 根据 file_id 删除文件信息
    bool ret = deleteFileInfo(deletedFileIds);
    if (!ret) {
        PRINT_ERROR("HandleDeleteFiles delete vector db data failed.\n");
        return Response(DELETE_VECTOR_DATABASE_FAILED);
    }

    // 根据 file_id 删除标签信息
    deleteTagInfo(deletedFileIds);

    return Response::success();
}

REGISTER_SERVICE_HANDLER("deleteFiles", handleDeleteFiles)

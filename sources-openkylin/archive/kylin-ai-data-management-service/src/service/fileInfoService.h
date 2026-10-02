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

#pragma once

#include <kyai-business-framework/errorCode.h>

#include <unordered_set>

#include "dataManagementVectorDatabase.h"
#include "fileDatabase.h"

class FileInfoService {
public:
    FileInfoService() : db(FileDatabase::createDatabase()) {}

    void classifyFile(const std::unordered_set<std::string>& filePaths, std::unordered_set<std::string>& newFilePaths,
                      std::vector<FileInfoDO>& updateFileInfos);

    static bool deleteTextFileVectorInfo(const std::unordered_set<int64_t>& fileIds);

    bool deleteFiles(const std::unordered_set<int64_t>& fileIds);

    ERROR_CODE searchFilesByText(const std::string& text, SimilaritySearchResult& result);

    static std::string getLastModifyTime(const std::string& filePath);

    void insertTask(const std::unordered_set<std::string>& filePaths);

    std::unordered_set<int64_t> updateTask(std::vector<FileInfoDO>& fileInfos);

    SQLite::Database getDatabase() { return std::move(db); }

    ~FileInfoService() = default;

private:
    static bool deleteVectorData(const std::string& fileIds);

    SimilaritySearchResult searchResultConvert(const SimilaritySearchMetadataResult& metadataResult);

private:
    SQLite::Database db;
};

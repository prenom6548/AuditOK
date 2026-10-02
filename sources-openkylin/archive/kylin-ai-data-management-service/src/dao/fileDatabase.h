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

#include <kyai-business-framework/SQLiteCpp/Database.h>
#include <kyai-business-framework/SQLiteCpp/Transaction.h>

#include <unordered_set>
#include <vector>

typedef struct fileInfoDO {
    int64_t fileId;
    int enable;
    std::string filePath;
    std::string fileSummary;
    std::string modifyTime;
} FileInfoDO;

class FileDatabase {
public:
    FileDatabase() = delete;

    static SQLite::Database createDatabase();

    static void createFileInfoTable(SQLite::Database& database);

    static std::vector<int64_t> getMultiFileId(SQLite::Database& database, const std::string& filepath);

    static bool deleteByFileIds(SQLite::Database& database, const std::string& expression);

    static bool getSingleFileInfo(SQLite::Database& database, const std::string& filepath, FileInfoDO& fileInfo);

    static std::vector<FileInfoDO> getAllFileInfos(SQLite::Database& database);

    static std::string getFilePathById(SQLite::Database& database, int64_t fileId);

    static std::vector<std::string> getAllTags(SQLite::Database& database);

    static std::vector<std::string> getFileTags(SQLite::Database& database, const std::string& filepath);

    static std::vector<std::string> getTagNamesByTagIds(SQLite::Database& database,
                                                        const std::unordered_set<int64_t>& tagIds);

    static std::vector<std::string> getFilePathsByTagIds(SQLite::Database& database,
                                                         const std::unordered_set<int64_t>& tagIds);

    static std::string getFileSummary(SQLite::Database& database, const std::string& filepath);

    static bool getTagIdByTagName(SQLite::Database& database, const std::string& tag, int64_t& tagId);

    static std::vector<int64_t> getTagIdsByFileId(SQLite::Database& database, int64_t fileId);

    static bool isTagInUse(SQLite::Database& database, int64_t tagId);

    static bool insertNewTagName(SQLite::Database& database, int64_t fileId, const std::string& name, int64_t& tagId);

    static bool deleteFileTagRelation(SQLite::Database& database, int64_t fileId, int64_t tagId);

    static bool insertFileTag(SQLite::Database& database, int64_t fileId, int64_t tagId);

    static bool deleteTagInfo(SQLite::Database& database, int64_t tagId);

    static bool insertFileInfo(SQLite::Database& database, FileInfoDO& fileInfo);

    static bool updateFileInfo(SQLite::Database& database, const FileInfoDO& fileInfo);

    static bool insertTextTask(SQLite::Database& database, int64_t fileId, const std::string& filePath);

    static bool updateTextTask(SQLite::Database& database, int64_t fileId);

    ~FileDatabase() = default;

    static std::string vectorToString(const std::vector<int64_t>& input);

    static std::string getDbFilePath();
};

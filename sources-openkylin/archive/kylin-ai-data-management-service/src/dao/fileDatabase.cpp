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

#include "fileDatabase.h"

#include <kyai-business-framework/log.h>
#include <kyai-business-framework/utils.h>

#include <filesystem>

std::string FileDatabase::getDbFilePath() {
    std::string homePath = std::getenv("HOME") ? std::string(std::getenv("HOME")) : "";
    std::string databasePath = homePath + "/.local/kylin-ai-business-framework/datamanagement/database/";

    if (!std::filesystem::exists(databasePath)) {
        // 创建目录
        bool ret = std::filesystem::create_directories(databasePath);
        if (!ret) {
            PRINT_ERROR("Create db path failed.\n");
            return "";
        }
        PRINT_INFO("Create db path success.\n");
    }

    return databasePath + "fileinfo.db";
}

void FileDatabase::createFileInfoTable(SQLite::Database &database) {
    // 创建Files表
    std::string createFileInfoTableSQL = R"(
        CREATE TABLE IF NOT EXISTS t_file_info (
            file_id INTEGER PRIMARY KEY AUTOINCREMENT,
            enable INTEGER NOT NULL DEFAULT 0,
            file_path TEXT NOT NULL UNIQUE,
            file_summary TEXT NOT NULL DEFAULT '',
            file_modify_time TEXT NOT NULL DEFAULT '0000-00-00 00:00:00',
            create_time TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            last_modify_time TEXT NOT NULL DEFAULT (datetime('now', 'localtime'))))";

    // 创建Tags表
    std::string createTagsTableSQL = R"(
        CREATE TABLE IF NOT EXISTS t_tag_info (
            tag_id INTEGER PRIMARY KEY AUTOINCREMENT,
            tag_name TEXT NOT NULL UNIQUE))";

    // 创建FileTags表
    std::string createFileTagsTableSQL = R"(
        CREATE TABLE IF NOT EXISTS t_file_tag_info (
            file_id INTEGER,
            tag_id INTEGER,
            PRIMARY KEY (file_id, tag_id)))";

    if (!database.tableExists("t_file_info")) {
        PRINT_DEBUG("Create table t_file_info, SQL:%s.\n", createFileInfoTableSQL.c_str());
        database.exec(createFileInfoTableSQL);
    }

    if (!database.tableExists("t_tag_info")) {
        PRINT_DEBUG("Create table t_tag_info, SQL:%s.\n", createTagsTableSQL.c_str());
        database.exec(createTagsTableSQL);
    }

    if (!database.tableExists("t_file_tag_info")) {
        PRINT_DEBUG("Create table t_file_tag_info, SQL:%s.\n", createFileTagsTableSQL.c_str());
        database.exec(createFileTagsTableSQL);
    }

    std::string createTaskTableSQL = R"(
        CREATE TABLE IF NOT EXISTS t_ai_index_task (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            file_id INTEGER NOT NULL,
            file_path TEXT NOT NULL,
            state INTEGER NOT NULL DEFAULT 0,
            type INTEGER NOT NULL,
            create_time TEXT NOT NULL DEFAULT (datetime('now', 'localtime')),
            start_time TEXT NOT NULL DEFAULT '0000-00-00 00:00:00',
            end_time TEXT NOT NULL DEFAULT '0000-00-00 00:00:00',
            retry_num INTEGER NOT NULL DEFAULT 0,
            reserve_int INTEGER NOT NULL DEFAULT 0,
            reserve_char TEXT NOT NULL DEFAULT '',
            UNIQUE (file_id, type),
            UNIQUE (file_path, type)
        ))";

    if (!database.tableExists("t_ai_index_task")) {
        PRINT_DEBUG("Create table t_ai_index_task, SQL:%s.\n", createTaskTableSQL.c_str());
        database.exec(createTaskTableSQL);
    }
}

SQLite::Database FileDatabase::createDatabase() {
    SQLite::Database db(getDbFilePath(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE, 300000);
    createFileInfoTable(db);
    return db;
}

std::vector<int64_t> FileDatabase::getMultiFileId(SQLite::Database &database, const std::string &filepath) {
    std::vector<int64_t> out;
    SQLite::Statement query(database, "SELECT file_id FROM t_file_info WHERE file_path LIKE ?");
    query.bind(1, filepath + "%");
    while (query.executeStep()) {
        out.emplace_back(query.getColumn(0));
    }
    PRINT_DEBUG("SQL: %s, result size: %zu.\n", query.getExpandedSQL().c_str(), out.size());
    return out;
}

bool FileDatabase::deleteByFileIds(SQLite::Database &database, const std::string &expression) {
    SQLite::Transaction transaction(database);
    std::string sql = "DELETE FROM t_file_info WHERE file_id IN (" + expression + ")";
    SQLite::Statement deleteFiles(database, sql);
    int rows = deleteFiles.exec();
    PRINT_DEBUG("SQL: %s, rows: %d.\n", deleteFiles.getExpandedSQL().c_str(), rows);

    // 删除任务
    sql = "DELETE FROM t_ai_index_task WHERE file_id IN (" + expression + ")";
    SQLite::Statement statement(database, sql);
    rows = statement.exec();

    transaction.commit();
    PRINT_DEBUG("SQL: %s, rows: %d.\n", statement.getExpandedSQL().c_str(), rows);
    return true;
}

bool FileDatabase::getSingleFileInfo(SQLite::Database &database, const std::string &filepath, FileInfoDO &fileInfo) {
    std::string sql =
        "SELECT file_id,enable,file_path,file_summary,file_modify_time FROM t_file_info WHERE file_path = ?";
    SQLite::Statement query(database, sql);
    query.bind(1, filepath);
    // 只会有一行数据
    if (!query.executeStep()) {
        PRINT_DEBUG("%s, %s, no record.\n", query.getExpandedSQL().c_str(), filepath.c_str());
        return false;
    }
    fileInfo.fileId = query.getColumn(0);
    fileInfo.enable = query.getColumn(1);
    fileInfo.filePath = query.getColumn(2).getString();
    fileInfo.fileSummary = query.getColumn(3).getString();
    fileInfo.modifyTime = query.getColumn(4).getString();
    PRINT_DEBUG("SQL: %s, file id:%ld, enable:%d modify time:%s.\n", query.getExpandedSQL().c_str(), fileInfo.fileId,
                fileInfo.enable, fileInfo.modifyTime.c_str());
    return true;
}

std::vector<FileInfoDO> FileDatabase::getAllFileInfos(SQLite::Database &database) {
    std::vector<FileInfoDO> out;
    SQLite::Statement query(database, "SELECT file_id,enable,file_path,file_summary,file_modify_time FROM t_file_info");
    while (query.executeStep()) {
        out.emplace_back(FileInfoDO{query.getColumn(0), query.getColumn(1), query.getColumn(2), query.getColumn(3),
                                    query.getColumn(4)});
    }
    PRINT_DEBUG("SQL: %s, result: %zu.\n", query.getExpandedSQL().c_str(), out.size());
    return out;
}

std::string FileDatabase::getFilePathById(SQLite::Database &database, int64_t fileId) {
    std::string sql = "SELECT file_path FROM t_file_info WHERE file_id = ?";
    SQLite::Statement query(database, sql);
    query.bind(1, fileId);
    // 只会有一行数据
    if (!query.executeStep()) {
        PRINT_ERROR("SQL: %s, %ld, no record.\n", query.getExpandedSQL().c_str(), fileId);
        return std::string();
    }
    std::string filepath = query.getColumn(0);
    PRINT_DEBUG("SQL: %s, result: %s.\n", query.getExpandedSQL().c_str(), filepath.c_str());
    return filepath;
}

std::vector<std::string> FileDatabase::getAllTags(SQLite::Database &database) {
    std::vector<std::string> out;
    std::string sqlGetTags =
        "SELECT t_tag_info.tag_name, COUNT(t_file_info.file_id) AS file_count "
        "FROM t_tag_info "
        "JOIN t_file_tag_info ON t_tag_info.tag_id = t_file_tag_info.tag_id "
        "JOIN t_file_info ON t_file_tag_info.file_id = t_file_info.file_id "
        "GROUP BY t_tag_info.tag_name "
        "ORDER BY file_count DESC";

    SQLite::Statement query(database, sqlGetTags);
    while (query.executeStep()) {
        out.emplace_back(query.getColumn(0));
    }
    PRINT_DEBUG("SQL: %s, result: %zu.\n", query.getExpandedSQL().c_str(), out.size());
    return out;
}

std::vector<std::string> FileDatabase::getFileTags(SQLite::Database &database, const std::string &filepath) {
    std::vector<std::string> tags;
    std::string sqlGetTags =
        "SELECT tag_name "
        "FROM t_tag_info "
        "JOIN t_file_tag_info ON t_tag_info.tag_id = t_file_tag_info.tag_id "
        "JOIN t_file_info ON t_file_tag_info.file_id = t_file_info.file_id "
        "WHERE t_file_info.file_path = ?";

    SQLite::Statement query(database, sqlGetTags);
    query.bind(1, filepath);
    while (query.executeStep()) {
        tags.push_back(query.getColumn(0));
    }
    PRINT_DEBUG("SQL: %s, result: %zu.\n", query.getExpandedSQL().c_str(), tags.size());
    return tags;
}

std::vector<std::string> FileDatabase::getTagNamesByTagIds(SQLite::Database &database,
                                                           const std::unordered_set<int64_t> &tagIds) {
    std::string expression = Utils::joinIds(tagIds);
    std::string sql = "SELECT tag_name FROM t_tag_info WHERE tag_id IN (" + expression + ")";

    SQLite::Statement query(database, sql);
    std::vector<std::string> result;
    while (query.executeStep()) {
        result.push_back(query.getColumn(0));
    }
    PRINT_DEBUG("SQL: %s, result size: %zu.\n", query.getExpandedSQL().c_str(), result.size());
    return result;
}

std::vector<std::string> FileDatabase::getFilePathsByTagIds(SQLite::Database &database,
                                                            const std::unordered_set<int64_t> &tagIds) {
    std::string expression = Utils::joinIds(tagIds);
    std::string sql =
        "SELECT t_file_info.file_path "
        "FROM t_file_info "
        "JOIN t_file_tag_info ON t_file_info.file_id = t_file_tag_info.file_id "
        "JOIN t_tag_info ON t_file_tag_info.tag_id = t_tag_info.tag_id "
        "WHERE t_tag_info.tag_id IN (" +
        expression + ")";

    SQLite::Statement query(database, sql);
    std::vector<std::string> filePaths;
    while (query.executeStep()) {
        filePaths.push_back(query.getColumn(0));
    }
    PRINT_DEBUG("SQL: %s, result size: %zu.\n", query.getExpandedSQL().c_str(), filePaths.size());
    return filePaths;
}

std::string FileDatabase::getFileSummary(SQLite::Database &database, const std::string &filepath) {
    std::string sql = "SELECT file_summary FROM t_file_info WHERE file_path = ?";
    SQLite::Statement query(database, sql);
    query.bind(1, filepath);
    // 只会有一行数据
    if (!query.executeStep()) {
        PRINT_WARN("SQL: %s, %s, no record.\n", query.getExpandedSQL().c_str(), filepath.c_str());
        return std::string();
    }
    std::string summary = query.getColumn(0);
    PRINT_DEBUG("SQL: %s, result: %s.\n", query.getExpandedSQL().c_str(), summary.c_str());
    return summary;
}

bool FileDatabase::getTagIdByTagName(SQLite::Database &database, const std::string &tag, int64_t &tagId) {
    std::string sql = "SELECT tag_id FROM t_tag_info WHERE tag_name = ?";
    SQLite::Statement query(database, sql);
    query.bind(1, tag);
    // 只会有一行数据
    if (!query.executeStep()) {
        PRINT_WARN("SQL: %s, %s, no record.\n", query.getExpandedSQL().c_str(), tag.c_str());
        return false;
    }
    tagId = query.getColumn(0);
    PRINT_DEBUG("SQL: %s, result: %ld.\n", query.getExpandedSQL().c_str(), tagId);
    return true;
}

std::vector<int64_t> FileDatabase::getTagIdsByFileId(SQLite::Database &database, int64_t fileId) {
    std::string sql = "SELECT tag_id FROM t_file_tag_info WHERE file_id = ?";
    SQLite::Statement query(database, sql);
    query.bind(1, fileId);

    std::vector<int64_t> tagIds;
    while (query.executeStep()) {
        tagIds.push_back(query.getColumn(0));
    }
    PRINT_DEBUG("SQL: %s, result: %s.\n", query.getExpandedSQL().c_str(), vectorToString(tagIds).c_str());
    return tagIds;
}

bool FileDatabase::isTagInUse(SQLite::Database &database, int64_t tagId) {
    std::string sql = "SELECT count(*) FROM t_file_tag_info WHERE tag_id = ?";
    SQLite::Statement query(database, sql);
    query.bind(1, tagId);

    if (!query.executeStep()) {
        PRINT_ERROR("SQL: %s, no record.\n", query.getExpandedSQL().c_str());
        return false;
    }
    int count = query.getColumn(0).getInt();
    PRINT_DEBUG("SQL: %s, result: %d.\n", query.getExpandedSQL().c_str(), count);
    return count != 0;
}

// 调用函数已经开启了事务
bool FileDatabase::insertNewTagName(SQLite::Database &database, int64_t fileId, const std::string &name,
                                    int64_t &tagId) {
    std::string tagSQL = "INSERT INTO t_tag_info (tag_name) VALUES (?)";
    SQLite::Statement tagStatement(database, tagSQL);
    tagStatement.bind(1, name);

    int rows = tagStatement.exec();
    if (rows != 1) {
        PRINT_ERROR("SQL: %s, insert tag name failed, rows: %d.\n", tagStatement.getExpandedSQL().c_str(), rows);
        return false;
    }

    tagId = database.getLastInsertRowid();
    PRINT_DEBUG("SQL: %s, tagId: %ld.\n", tagStatement.getExpandedSQL().c_str(), tagId);

    std::string fileTagsSQL = "INSERT INTO t_file_tag_info (file_id, tag_id) VALUES (?, ?)";
    SQLite::Statement fileTagStatement(database, fileTagsSQL);
    fileTagStatement.bind(1, fileId);
    fileTagStatement.bind(2, tagId);

    rows = fileTagStatement.exec();
    if (rows != 1) {
        PRINT_ERROR("SQL: %s, insert file tag relation failed, rows: %d.\n", fileTagStatement.getExpandedSQL().c_str(),
                    rows);
        return false;
    }
    PRINT_DEBUG("SQL: %s, success.\n", fileTagStatement.getExpandedSQL().c_str());

    return true;
}

bool FileDatabase::deleteFileTagRelation(SQLite::Database &database, int64_t fileId, int64_t tagId) {
    std::string sql = "DELETE FROM t_file_tag_info WHERE file_id = ? AND tag_id = ?";
    SQLite::Statement statement(database, sql);
    statement.bind(1, fileId);
    statement.bind(2, tagId);

    int rows = statement.exec();
    PRINT_DEBUG("SQL: %s, success %d rows.\n", statement.getExpandedSQL().c_str(), rows);
    return true;
}

bool FileDatabase::insertFileTag(SQLite::Database &database, int64_t fileId, int64_t tagId) {
    std::string sql = "INSERT INTO t_file_tag_info VALUES(?, ?)";
    SQLite::Statement statement(database, sql);
    statement.bind(1, fileId);
    statement.bind(2, tagId);

    int rows = statement.exec();
    if (rows != 1) {
        PRINT_ERROR("SQL: %s, insert file tag relation failed, rows: %d.\n", statement.getExpandedSQL().c_str(), rows);
        return false;
    }
    PRINT_DEBUG("SQL: %s, success.\n", statement.getExpandedSQL().c_str());
    return true;
}

bool FileDatabase::deleteTagInfo(SQLite::Database &database, int64_t tagId) {
    std::string sql = "DELETE FROM t_tag_info WHERE tag_id = ?";
    SQLite::Statement statement(database, sql);
    statement.bind(1, tagId);

    int rows = statement.exec();
    PRINT_DEBUG("SQL: %s, success %d rows.\n", statement.getExpandedSQL().c_str(), rows);
    return true;
}

bool FileDatabase::insertFileInfo(SQLite::Database &database, FileInfoDO &fileInfo) {
    std::string sql = "INSERT OR IGNORE INTO t_file_info(enable, file_path, file_modify_time) VALUES(?, ?, ?)";
    SQLite::Statement statement(database, sql);
    statement.bind(1, fileInfo.enable);
    statement.bind(2, fileInfo.filePath);
    statement.bind(3, fileInfo.modifyTime);

    int rows = statement.exec();
    if (rows != 1) {
        PRINT_ERROR("SQL: %s, insert file info failed, rows: %d.\n", statement.getExpandedSQL().c_str(), rows);
        return false;
    }
    // 回填 file_id
    fileInfo.fileId = database.getLastInsertRowid();

    PRINT_DEBUG("SQL: %s, success, file_id = %ld.\n", statement.getExpandedSQL().c_str(), fileInfo.fileId);
    return true;
}

bool FileDatabase::updateFileInfo(SQLite::Database &database, const FileInfoDO &fileInfo) {
    std::string sql =
        "UPDATE t_file_info SET enable = ?, file_summary = ?, file_modify_time = ?, "
        "last_modify_time = datetime('now', 'localtime') WHERE file_id = ?";
    SQLite::Statement statement(database, sql);
    statement.bind(1, fileInfo.enable);
    statement.bind(2, fileInfo.fileSummary);
    statement.bind(3, fileInfo.modifyTime);
    statement.bind(4, fileInfo.fileId);

    int rows = statement.exec();
    PRINT_DEBUG("SQL: %s, success %d rows.\n", statement.getExpandedSQL().c_str(), rows);
    return true;
}

bool FileDatabase::insertTextTask(SQLite::Database &database, int64_t fileId, const std::string &filePath) {
    std::string sql =
        "INSERT OR IGNORE INTO t_ai_index_task(file_id, file_path, type) VALUES(?, ?, ?), (?, ?, ?), (?, ?, ?)";
    SQLite::Statement statement(database, sql);
    statement.bind(1, fileId);
    statement.bind(2, filePath);
    statement.bind(3, 1);
    statement.bind(4, fileId);
    statement.bind(5, filePath);
    statement.bind(6, 2);
    statement.bind(7, fileId);
    statement.bind(8, filePath);
    statement.bind(9, 3);

    int rows = statement.exec();
    if (rows != 3) {
        PRINT_ERROR("SQL: %s, insert text ai index task failed, rows: %d.\n", statement.getExpandedSQL().c_str(), rows);
        return false;
    }

    PRINT_DEBUG("SQL: %s, success %d rows.\n", statement.getExpandedSQL().c_str(), rows);
    return true;
}

bool FileDatabase::updateTextTask(SQLite::Database &database, int64_t fileId) {
    std::string sql =
        "UPDATE t_ai_index_task SET state = 0, start_time = '0000-00-00 00:00:00', end_time = '0000-00-00 00:00:00', "
        "retry_num = 0 WHERE file_id = ?";
    SQLite::Statement statement(database, sql);
    statement.bind(1, fileId);

    int rows = statement.exec();
    PRINT_DEBUG("SQL: %s, success %d rows.\n", statement.getExpandedSQL().c_str(), rows);
    return true;
}

std::string FileDatabase::vectorToString(const std::vector<int64_t> &input) {
    if (input.empty()) {
        return "[]";
    }

    std::string out = "[";
    for (const auto &item : input) {
        out += (std::to_string(item) + ",");
    }
    out[out.length() - 1] = ']';
    return out;
}

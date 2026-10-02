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

// #include "SQLiteCpp/Database.h"

// #include <gtest/gtest.h>
// #include <sqlite3.h>

// #include "SQLiteCpp/Transaction.h"

// TEST(SQLiteCpp, version) {
//     EXPECT_STREQ(SQLITE_VERSION, SQLite::VERSION);
//     EXPECT_EQ(SQLITE_VERSION_NUMBER, SQLite::VERSION_NUMBER);
//     EXPECT_STREQ(SQLITE_VERSION, SQLite::getLibVersion());
//     EXPECT_EQ(SQLITE_VERSION_NUMBER, SQLite::getLibVersionNumber());
// }

// void createTable(SQLite::Database &db) {
//     // 创建Files表
//     const char *createFilesTableSQL =
//         "CREATE TABLE IF NOT EXISTS Files ("
//         "file_id INTEGER PRIMARY KEY AUTOINCREMENT,"
//         "file_path TEXT NOT NULL UNIQUE,"
//         "file_summary TEXT,"
//         "modified_time DATETIME"
//         ");";

//     // 创建Tags表
//     // const char *createTagsTableSQL =
//     //     "CREATE TABLE IF NOT EXISTS Tags ("
//     //     "tag_id INTEGER PRIMARY KEY AUTOINCREMENT,"
//     //     "tag_name TEXT NOT NULL UNIQUE"
//     //     ");";

//     // 创建FileTags表
//     const char *createFileTagsTableSQL =
//         "CREATE TABLE IF NOT EXISTS FileTags ("
//         "file_id INTEGER,"
//         "tag_id INTEGER,"
//         "FOREIGN KEY (file_id) REFERENCES Files(file_id) ON DELETE CASCADE,"
//         // "FOREIGN KEY (tag_id) REFERENCES Tags(tag_id) ON DELETE CASCADE,"
//         "PRIMARY KEY (file_id, tag_id)"
//         ");";

//     int ret = db.exec(createFilesTableSQL);
//     std::cout << "create Files table, ret = " << ret << std::endl;
//     ret = db.exec(createFileTagsTableSQL);
//     std::cout << "create FileTags table, ret = " << ret << std::endl;
// }

// void insert(SQLite::Database &db) {
//     const char *filesSql = "INSERT OR IGNORE INTO Files (file_path, modified_time, file_summary) VALUES (?, ?, ?)";
//     const char *fileTagsSQL = "INSERT OR IGNORE INTO FileTags (file_id, tag_id) VALUES (?, ?)";

//     SQLite::Statement filesQuery(db, filesSql);
//     filesQuery.bind(1, "hello.txt");
//     filesQuery.bind(2, 1733101806);
//     filesQuery.bind(3, "hello");
//     int rows = filesQuery.exec();
//     std::cout << filesQuery.getExpandedSQL() << ", rows = " << rows << std::endl;

//     int64_t fileId = db.getLastInsertRowid();
//     SQLite::Statement fileTagsQuery(db, fileTagsSQL);
//     fileTagsQuery.bind(1, fileId);
//     fileTagsQuery.bind(2, 1);
//     rows = fileTagsQuery.exec();
//     std::cout << fileTagsQuery.getExpandedSQL() << ", rows = " << rows << std::endl;

//     fileTagsQuery.reset();
//     fileTagsQuery.clearBindings();
//     fileTagsQuery.bind(1, fileId);
//     fileTagsQuery.bind(2, 2);
//     rows = fileTagsQuery.exec();
//     std::cout << fileTagsQuery.getExpandedSQL() << ", rows = " << rows << std::endl;
// }

// void deleteDate(SQLite::Database &db, int fileId) {
//     // std::cout << "deleteDate start" << std::endl;
//     std::string sqlDeleteFiles = "DELETE FROM Files WHERE file_id IN (" + std::to_string(fileId) + ")";
//     SQLite::Statement deleteFiles(db, sqlDeleteFiles);
//     int rows = deleteFiles.exec();
//     std::cout << deleteFiles.getExpandedSQL() << ", rows = " << rows << std::endl;
// }

// int main() {
//     SQLite::Database db("/home/hezhe/code/kylin-ai-business-framework/test/sqlitecpp/test.db",
//                         SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE, 5000);

//     int ret = db.exec("PRAGMA foreign_keys = ON");
//     std::cout << "foreign key on, ret = " << ret << std::endl;
//     bool foreignKeys = db.execAndGet("PRAGMA foreign_keys").getInt();
//     std::cout << "foreign = " << foreignKeys << std::endl;

//     SQLite::Transaction transaction(db);
//     // createTable(db);
//     // insert(db);
//     deleteDate(db, 4);

//     transaction.commit();
//     return 0;
// }

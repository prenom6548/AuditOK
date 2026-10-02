#include "recoverdb.h"

#include <kyai-business-framework/SQLiteCpp/Database.h>
#include <kyai-business-framework/SQLiteCpp/Statement.h>
#include <kyai-business-framework/log.h>

#include <filesystem>

#include "sqlite3recover.h"
bool Sqlite3Recover::checkDatabaseIntegrity(const std::string &dbPath) {
    std::error_code ec;
    if (!std::filesystem::exists(std::filesystem::path(dbPath), ec) || ec) {
        PRINT_ERROR("DB filePath:%s not exists, check database integrity failed,or error:%s\n", dbPath.c_str(),
                    ec.message().c_str());
        return false;
    }

    try {
        SQLite::Database db(dbPath, SQLite::OPEN_READONLY);

        // 执行完整性检查（PRAGMA）
        SQLite::Statement query(db, "PRAGMA integrity_check;");

        while (query.executeStep()) {
            std::string result = query.getColumn(0);
            if (result != "ok") {
                PRINT_ERROR("db is corrupted:%s.\n", result.c_str());
                return false;
            }
        }
    } catch (const std::exception &e) {
        PRINT_ERROR("caught a sqlite3 exception. [error_string: %s].\n", e.what());
        return false;
    } catch (...) {
        PRINT_ERROR("caught an unknown exception.\n");
        return false;
    }

    return true;
}

/*
** This function is called to recover data from the database.
*/
bool Sqlite3Recover::recoverDatabase(const std::string &dbFilePath) {
    PRINT_INFO("Sqlite3Recover recover malformed database start.\n");

    int rc;                              /* Return code from this routine */
    const char *zLAF = "lost_and_found"; /* Name of "lost_and_found" table */
    int bFreelist = 1;                   /* True to scan the freelist */
    int bRowids = 1;                     /* True to restore ROWID values */
    sqlite3_recover *p;                  /* The recovery object */
    sqlite3 *db;

    std::error_code ec;
    const std::filesystem::path filePath = dbFilePath;
    if (!std::filesystem::exists(filePath, ec) || ec) {  // 既然是恢复数据库，所以输入文件必须是有效数据库路径
        PRINT_ERROR("DB filePath:%s not exists, recover database failed,or error:%s\n", dbFilePath.c_str(),
                    ec.message().c_str());
        return false;
    }

    // 在输入目录下创建临时数据库
    std::filesystem::path directory = filePath.parent_path();
    std::string tmpDbFilePath = directory.string() + "/data-management-recovered.db";
    const char *tmpDB = tmpDbFilePath.c_str();

    rc = sqlite3_open(dbFilePath.c_str(), &db);
    p = sqlite3_recover_init(db, "main", tmpDB);
    sqlite3_recover_config(p, SQLITE_RECOVER_LOST_AND_FOUND, (void *)zLAF);
    sqlite3_recover_config(p, SQLITE_RECOVER_ROWIDS, (void *)&bRowids);
    sqlite3_recover_config(p, SQLITE_RECOVER_FREELIST_CORRUPT, (void *)&bFreelist);
    sqlite3_recover_run(p);
    if (sqlite3_recover_errcode(p) != SQLITE_OK) {
        const char *zErr = sqlite3_recover_errmsg(p);
        int errCode = sqlite3_recover_errcode(p);
        PRINT_ERROR("sqlite3 recovery error:%s (%d).\n", zErr, errCode);
        sqlite3_close(db);
        return false;
    }

    rc = sqlite3_recover_finish(p);
    if (rc != SQLITE_OK) {
        const char *zErr = sqlite3_errmsg(db);
        PRINT_ERROR("sqlite3_recover_finish returns error:%s (%d).\n", zErr, rc);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    if (!replaceDatabase(dbFilePath, tmpDbFilePath)) {
        return false;
    }

    PRINT_INFO("Sqlite3Recover recover malformed database sucess.\n");
    return true;
}

bool Sqlite3Recover::replaceDatabase(const std::string &targetFilePath, const std::string &tmpFilePath) {
    std::filesystem::path tmp = tmpFilePath;
    std::filesystem::path target = targetFilePath;
    std::error_code ec;

    if (!std::filesystem::exists(tmp, ec) || ec) {
        PRINT_ERROR("Temporary filePath:%s does not exist or error: %s\n", tmp.c_str(), ec.message().c_str());
        return false;
    }

    if (std::filesystem::exists(target, ec) && !ec) {
        if (!std::filesystem::remove(target, ec) || ec) {
            PRINT_ERROR("Failed to remove old file: %s, error: %s\n", target.c_str(), ec.message().c_str());
            return false;
        }
    } else if (ec) {
        PRINT_ERROR("Failed to check target file existence: %s\n", ec.message().c_str());
        return false;
    }

    std::filesystem::rename(tmp, target, ec);
    if (ec) {
        PRINT_ERROR("Failed to rename file from %s to %s, error: %s\n", tmp.c_str(), target.c_str(),
                    ec.message().c_str());
        return false;
    }

    return true;
}
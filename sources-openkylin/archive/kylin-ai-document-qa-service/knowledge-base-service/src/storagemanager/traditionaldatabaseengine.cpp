#include "traditionaldatabaseengine.h"

#include <SQLiteCpp/Column.h>
#include <SQLiteCpp/Exception.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>
#include <kyai-business-framework/log.h>

#include <cstdlib>
#include <filesystem>
#include <string>

#include "../common/utils.h"

namespace knowledgebase {

// 知识库管理表
/*
    id INT  主键，自增 ID，唯一
    knowledge_base_id    VARCHAR 知识库ID，唯一
    app_id  VARCHAR 所属APP
    name    VARCHAR 知识库显示名
    description VARCHAR 描述信息：json
    update_time VARCHAR 上次更新时间
    type    INT 知识库类别，0：docmount，1：faq
    vector_db_name  VARCHAR 关联的向量数据库表名（collection_name）
    search_engine_db_name   VARCHAR 关联的全文搜索引擎数据库表名
*/

// 知识库文件表知识库文件表
/*
    id  INT 主键，自增 ID
    file_id VARCHAR 文件唯一id
    knowledge_base_id    VARCHAR 知识库ID
*/

std::string getCurrentTime() {
    time_t now = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buf);
}

bool isTableExist(SQLite::Database& db, const std::string& tableName) {
    std::string query = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name=?";
    SQLite::Statement check(db, query);
    check.bind(1, tableName);
    check.executeStep();
    return check.getColumn(0).getInt() > 0;
}

void checkAndCreatePreBuiltKnowledgeBase(SQLite::Database& k_db, const std::string knowledgeBaseId,
                                         const std::string appId, const std::string name, const std::string description,
                                         const int type, const std::string vectorDbName,
                                         const std::string searchEngineDbName) {
    const std::string checkSQL = "SELECT COUNT(*) FROM knowledge_base WHERE knowledge_base_id = ?;";
    SQLite::Statement query(k_db, checkSQL);
    query.bind(1, knowledgeBaseId);
    query.executeStep();

    int count = query.getColumn(0);
    if (count == 0) {
        PRINT_INFO("Info: knowledge_base_id '%s' does not exist, creating default entry.\n", knowledgeBaseId.c_str());

        // 插入默认条目（请根据实际业务需求调整字段值）
        const char* insertSQL =
            "INSERT INTO knowledge_base ("
            "knowledge_base_id, app_id, name, description, update_time, type, vector_db_name, search_engine_db_name) "
            "VALUES (?, ?, ?, ?, datetime('now'), ?, ?, ?)";

        SQLite::Statement insert(k_db, insertSQL);
        insert.bind(1, knowledgeBaseId);     // knowledge_base_id
        insert.bind(2, appId);               // app_id
        insert.bind(3, name);                // name
        insert.bind(4, description);         // description
        insert.bind(5, type);                // type
        insert.bind(6, vectorDbName);        // vector_db_name
        insert.bind(7, searchEngineDbName);  // search_engine_db_name

        insert.exec();
        PRINT_INFO("Info: Default entry with knowledge_base_id '%s' created successfully.\n", knowledgeBaseId.c_str());
    } else {
        PRINT_INFO("Info: knowledge_base_id '%s' already exists, no insertion needed.\n", knowledgeBaseId.c_str());
    }
}

bool TraditionalDatabaseEngine::initTraditionalDatabase(std::unique_ptr<SQLite::Database>& k_db, KBErrorCode& errCode) {
    // 1. 构造目标路径
    std::string k_db_path = dbFilePath_ + "/knowledge_base.db";

    // 2. 创建目录
    try {
        PRINT_INFO("Info: Creating directory: %s\n", dbFilePath_.c_str());
        std::filesystem::create_directories(dbFilePath_);
    } catch (const std::filesystem::filesystem_error& ex) {
        PRINT_INFO("Error creating directory: %s\n", std::string(ex.what()).c_str());
    }

    // 3. 使用 unique_ptr 构造数据库对象
    try {
        k_db = std::make_unique<SQLite::Database>(k_db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        // 启用 WAL 模式
        k_db->exec("PRAGMA journal_mode=WAL;");
    } catch (const SQLite::Exception& ex) {
        PRINT_ERROR("SQLite error opening database: %s (code: %d)\n", ex.what(), ex.getErrorCode());
        errCode = KBErrorCode::KB_ACCESS_DENIED;
        return false;  // 打开失败，不再继续
    } catch (const std::exception& ex) {
        PRINT_ERROR("Error opening database (general): %s\n", ex.what());
        errCode = KBErrorCode::KB_ACCESS_DENIED;
        return false;
    }

    return true;
}

TraditionalDatabaseEngine::TraditionalDatabaseEngine(const std::string dbFilePath) {
    dbFilePath_ = dbFilePath;
    // 初始化知识库管理表
    KBErrorCode errCode;
    if (!initTraditionalDatabase(k_db, errCode)) {
        return;
    }

    // 创建表（使用 get() 获取引用）
    if (!isTableExist(*k_db, "knowledge_base")) {
        PRINT_INFO("Info: Table 'knowledge_base' does not exist, create a new one!\n");
        const char* createTableSQL =
            "CREATE TABLE IF NOT EXISTS knowledge_base ("
            "knowledge_base_id TEXT PRIMARY KEY,"
            "app_id TEXT NOT NULL,"
            "name TEXT NOT NULL,"
            "description TEXT,"
            "update_time TEXT NOT NULL,"
            "type INTEGER NOT NULL,"
            "vector_db_name TEXT,"
            "search_engine_db_name TEXT)";

        k_db->exec(createTableSQL);
    }

    if (!isTableExist(*k_db, "knowledge_file_base")) {
        PRINT_INFO("Info: Table 'knowledge_file_base' does not exist, create a new one!\n");
        const char* createTableSQL =
            "CREATE TABLE IF NOT EXISTS knowledge_file_base ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "knowledge_base_id TEXT NOT NULL,"
            "file_id TEXT NOT NULL,"
            "file_md5sum TEXT NOT NULL"
            ");";

        k_db->exec(createTableSQL);
    }

    // 检查 knowledge_base 表中是否存在预制知识库 knowledge_base_id = 'kakbs_user_manual' 的记录，不存在则添加
    checkAndCreatePreBuiltKnowledgeBase(
        *k_db, "kakbs_user_manual", "kylin-ai-knowledge-base-service", "Predefined_user_manual",
        "Predefined user manual knowledge base created by system", 0, "kakbs_user_manual", "");
    // 检查 knowledge_base 表中是否存在预制知识库 knowledge_base_id = 'kakbs_kylin_om_faq' 的记录，不存在则添加
    checkAndCreatePreBuiltKnowledgeBase(
        *k_db, "kakbs_kylin_om_faq", "kylin-ai-knowledge-base-service", "Predefined_kylin_om_faq",
        "Predefined yhkylin Operations and Maintenance faq knowledge base created by system", 1, "kakbs_kylin_om_faq",
        "");
}

bool TraditionalDatabaseEngine::createKnowledgeBase(const std::string& appId, const std::string& name,
                                                    const std::string& description, const std::string& collectionName,
                                                    const std::string& textSearchDbName, std::string& knowledgeBaseId,
                                                    KBType type, IndexType indexType, KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return false;
        }
    }
    try {
        std::string updateTime = getCurrentTime();

        SQLite::Statement query(*k_db,
                                "INSERT INTO knowledge_base "
                                "(knowledge_base_id, app_id, name, description, update_time, type, vector_db_name, "
                                "search_engine_db_name) "
                                "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

        query.bind(1, knowledgeBaseId);
        query.bind(2, appId);
        query.bind(3, name);
        query.bind(4, description);
        query.bind(5, updateTime);
        query.bind(6, static_cast<int>(type));
        query.bind(7, collectionName);
        query.bind(8, textSearchDbName);

        query.exec();
        errCode = KBErrorCode::SUCCESS;
        return true;
    } catch (const SQLite::Exception& e) {  // 捕获SQLite特定异常
        PRINT_ERROR("SQLite error: %s\n", e.what());
        PRINT_ERROR("SQLite error code: %d\n", e.getErrorCode());
        PRINT_ERROR("SQLite extended error code: %d\n", e.getExtendedErrorCode());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    } catch (const std::exception& e) {  // 捕获标准异常
        PRINT_ERROR("Standard exception: %s\n", e.what());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    } catch (...) {  // 捕获其他所有异常
        PRINT_ERROR("Unknown exception occurred\n");
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    }
}

bool TraditionalDatabaseEngine::deleteKnowledgeBase(const std::string& knowledgeBaseId, KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return false;
        }
    }
    try {
        SQLite::Transaction txn1(*k_db);
        SQLite::Statement del1(*k_db, "DELETE FROM knowledge_file_base WHERE knowledge_base_id = ?");
        del1.bind(1, knowledgeBaseId);
        del1.exec();
        txn1.commit();

        SQLite::Transaction txn2(*k_db);
        SQLite::Statement del2(*k_db, "DELETE FROM knowledge_base WHERE knowledge_base_id = ?");
        del2.bind(1, knowledgeBaseId);
        del2.exec();
        txn2.commit();
        errCode = KBErrorCode::SUCCESS;
        return true;
    } catch (const SQLite::Exception& e) {  // 捕获SQLite特定异常
        PRINT_ERROR("SQLite error: %s\n", e.what());
        PRINT_ERROR("SQLite error code: %d\n", e.getErrorCode());
        PRINT_ERROR("SQLite extended error code: %d\n", e.getExtendedErrorCode());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    } catch (const std::exception& e) {  // 捕获标准异常
        PRINT_ERROR("Standard exception: %s\n", e.what());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    } catch (...) {  // 捕获其他所有异常
        PRINT_ERROR("Unknown exception occurred\n");
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    }
}

bool TraditionalDatabaseEngine::changeKnowledgeBaseName(const std::string& knowledgeBaseId, const std::string& name,
                                                        KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return false;
        }
    }
    try {
        SQLite::Statement stmt(*k_db,
                               "UPDATE knowledge_base SET name = ?, update_time = ? WHERE knowledge_base_id = ?");
        stmt.bind(1, name);
        stmt.bind(2, getCurrentTime());
        stmt.bind(3, knowledgeBaseId);
        stmt.exec();
        errCode = KBErrorCode::SUCCESS;
        return true;
    } catch (...) {
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    }
}

bool TraditionalDatabaseEngine::changeKnowledgeBaseDescription(const std::string& knowledgeBaseId,
                                                               const std::string& description, KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return false;
        }
    }
    try {
        SQLite::Statement stmt(
            *k_db, "UPDATE knowledge_base SET description = ?, update_time = ? WHERE knowledge_base_id = ?");
        stmt.bind(1, description);
        stmt.bind(2, getCurrentTime());
        stmt.bind(3, knowledgeBaseId);
        stmt.exec();
        errCode = KBErrorCode::SUCCESS;
        return true;
    } catch (...) {
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    }
}

KnowledgeBaseInfo TraditionalDatabaseEngine::getKnowledgeBase(const std::string& knowledgeBaseId,
                                                              KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return KnowledgeBaseInfo();
        }
    }
    try {
        SQLite::Statement stmt(*k_db,
                               "SELECT name, description, knowledge_base_id, update_time, type, vector_db_name FROM "
                               "knowledge_base WHERE knowledge_base_id = ?");
        stmt.bind(1, knowledgeBaseId);

        if (stmt.executeStep()) {
            KnowledgeBaseInfo info;
            info.name_ = stmt.getColumn(0).getString();
            info.description_ = stmt.getColumn(1).getString();
            info.knowledgeBaseId_ = stmt.getColumn(2).getString();
            info.changeTime_ = stmt.getColumn(3).getString();
            info.type_ = static_cast<KBType>(stmt.getColumn(4).getInt());
            info.indexType = IndexType::FLAT;  // 可根据 vector_db_name 决定
            errCode = KBErrorCode::SUCCESS;
            return info;
        } else {
            errCode = KBErrorCode::KB_NOT_FOUND;
            return KnowledgeBaseInfo();
        }
    } catch (...) {
        errCode = KBErrorCode::KB_NOT_FOUND;
        return KnowledgeBaseInfo();
    }
}

std::vector<KnowledgeBaseInfo> TraditionalDatabaseEngine::getAllKnowledgeBase(const std::string appId,
                                                                              KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return {};
        }
    }
    std::vector<KnowledgeBaseInfo> result;

    try {
        SQLite::Statement query(*k_db,
                                "SELECT name, description, knowledge_base_id, update_time, type, vector_db_name "
                                "FROM knowledge_base WHERE app_id = ?");

        query.bind(1, appId);

        while (query.executeStep()) {
            KnowledgeBaseInfo info;
            info.name_ = query.getColumn(0).getString();
            info.description_ = query.getColumn(1).getString();
            info.knowledgeBaseId_ = query.getColumn(2).getString();
            info.changeTime_ = query.getColumn(3).getString();
            info.type_ = static_cast<KBType>(query.getColumn(4).getInt());

            // 简化逻辑：根据 vector_db_name 判断 indexType
            const std::string& vecDbName = query.getColumn(5).getString();
            info.indexType = IndexType::FLAT;

            result.push_back(info);
        }

        if (result.empty()) {
            errCode = KBErrorCode::KB_NOT_FOUND;
        }
        errCode = KBErrorCode::SUCCESS;
        return result;

    } catch (...) {
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return {};
    }
    return {};
}

std::vector<std::string> TraditionalDatabaseEngine::getFileListInKnowledgeBase(const std::string& knowledgeBaseId,
                                                                               KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return {};
        }
    }
    std::vector<std::string> files;
    try {
        SQLite::Statement stmt(*k_db, "SELECT file_id FROM knowledge_file_base WHERE knowledge_base_id = ?");
        stmt.bind(1, knowledgeBaseId);

        while (stmt.executeStep()) {
            files.push_back(stmt.getColumn(0).getString());
        }

        errCode = KBErrorCode::SUCCESS;
        return files;
    } catch (...) {
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return {};
    }
}

bool TraditionalDatabaseEngine::addDocumentsBatch(const std::string& knowledgeBaseId,
                                                  const std::unordered_map<std::string, std::string>& files,
                                                  KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return false;
        }
    }
    try {
        SQLite::Transaction txn(*k_db);

        // 先准备检查是否存在的语句
        SQLite::Statement checkStmt(
            *k_db, "SELECT 1 FROM knowledge_file_base WHERE file_id = ? AND knowledge_base_id = ? LIMIT 1");

        // 准备插入语句
        SQLite::Statement insertStmt(
            *k_db, "INSERT INTO knowledge_file_base (file_id, knowledge_base_id, file_md5sum) VALUES (?, ?, ?)");

        int addedCount = 0;
        for (const auto& filePair : files) {
            // 检查是否已存在
            checkStmt.bind(1, filePair.first);
            checkStmt.bind(2, knowledgeBaseId);

            if (!checkStmt.executeStep()) {  // 如果不存在
                // 执行插入
                insertStmt.bind(1, filePair.first);
                insertStmt.bind(2, knowledgeBaseId);
                std::string md5sum = computeMD5FromFile(filePair.second);
                insertStmt.bind(3, md5sum);
                insertStmt.exec();
                insertStmt.reset();
                addedCount++;
            }

            checkStmt.reset();  // 重置检查语句以便下次使用
        }

        txn.commit();
        return true;
    } catch (const SQLite::Exception& e) {  // 捕获SQLite特定异常
        PRINT_ERROR("SQLite error: %s\n", e.what());
        PRINT_ERROR("SQLite error code: %d\n", e.getErrorCode());
        PRINT_ERROR("SQLite extended error code: %d\n", e.getExtendedErrorCode());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    } catch (const std::exception& e) {  // 捕获标准异常
        PRINT_ERROR("Standard exception: %s\n", e.what());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    } catch (...) {  // 捕获其他所有异常
        PRINT_ERROR("Unknown exception occurred\n");
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    }
}

bool TraditionalDatabaseEngine::deleteDocuments(const std::string& knowledgeBaseId,
                                                const std::vector<std::string>& fileIds, KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return false;
        }
    }
    try {
        SQLite::Transaction txn(*k_db);
        SQLite::Statement stmt(*k_db, "DELETE FROM knowledge_file_base WHERE knowledge_base_id = ? AND file_id = ?");

        for (const auto& id : fileIds) {
            stmt.bind(1, knowledgeBaseId);
            stmt.bind(2, id);
            PRINT_INFO("Final SQL: %s\n", stmt.getExpandedSQL().c_str());  // 如果SQLite库支持
            stmt.exec();
            stmt.reset();
        }

        txn.commit();
        errCode = KBErrorCode::SUCCESS;
        return true;
    } catch (const SQLite::Exception& e) {  // 捕获SQLite特定异常
        PRINT_ERROR("SQLite error: %s\n", e.what());
        PRINT_ERROR("SQLite error code: %d\n", e.getErrorCode());
        PRINT_ERROR("SQLite extended error code: %d\n", e.getExtendedErrorCode());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    } catch (const std::exception& e) {  // 捕获标准异常
        PRINT_ERROR("Standard exception: %s\n", e.what());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    } catch (...) {  // 捕获其他所有异常
        PRINT_ERROR("Unknown exception occurred\n");
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    }
}

bool TraditionalDatabaseEngine::getKnowledgeBaseDatabaseName(const std::string& knowledgeBaseId,
                                                             std::string& collectionName, std::string& textSearchDbName,
                                                             KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return false;
        }
    }
    try {
        SQLite::Statement stmt(
            *k_db, "SELECT vector_db_name, search_engine_db_name FROM knowledge_base WHERE knowledge_base_id = ?");
        stmt.bind(1, knowledgeBaseId);
        if (stmt.executeStep()) {
            collectionName = stmt.getColumn(0).getString();
            textSearchDbName = stmt.getColumn(1).getString();
            errCode = KBErrorCode::SUCCESS;
            return true;
        } else {
            errCode = KBErrorCode::KB_NOT_FOUND;
            return false;
        }
    } catch (...) {
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    }
}

bool TraditionalDatabaseEngine::hasAccessPermission(const std::string& appId, const std::string& knowledgeBaseId,
                                                    KBErrorCode& errCode) {
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            errCode = KBErrorCode::KB_NOT_FOUND;
            return false;
        }
    }
    try {
        if (std::filesystem::exists(dbFilePath_)) {
            if (!pathReadable(dbFilePath_) || !pathWritable(dbFilePath_)) {
                PRINT_ERROR("No read/write permission for directories : %s\n", dbFilePath_.c_str());
                errCode = KBErrorCode::KB_ACCESS_DENIED;
                return false;
            }
        } else {
            PRINT_ERROR("Database dir not exists : %s\n", dbFilePath_.c_str());
            errCode = KBErrorCode::KB_NOT_FOUND;
            return false;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        if (e.code() == std::errc::permission_denied) {
            PRINT_ERROR("Directory permission exception : %s\n", dbFilePath_.c_str());
            errCode = KBErrorCode::KB_ACCESS_DENIED;
            return false;
        } else {
            PRINT_ERROR("Directory check error:%s. error msg:\n", dbFilePath_.c_str(), e.what());
            errCode = KBErrorCode::UNKNOWN_ERROR;
            return false;
        }
    }
    try {
        SQLite::Statement checkIdStmt(*k_db, "SELECT app_id FROM knowledge_base WHERE knowledge_base_id = ?");
        checkIdStmt.bind(1, knowledgeBaseId);

        std::vector<std::string> appIds;
        while (checkIdStmt.executeStep()) {
            appIds.push_back(checkIdStmt.getColumn(0).getString());
        }

        if (appIds.empty()) {
            errCode = KBErrorCode::KB_NOT_FOUND;
            return false;
        }

        bool hasPermission = false;
        for (const auto& appId : appIds) {
            if (appId == appId) {
                hasPermission = true;
                break;
            }
        }

        if (hasPermission) {
            errCode = KBErrorCode::SUCCESS;
            return true;
        } else {
            errCode = KBErrorCode::KB_ACCESS_DENIED;
            return false;
        }
    } catch (const SQLite::Exception& e) {  // 捕获SQLite特定异常
        PRINT_ERROR("SQLite error: %s\n", e.what());
        PRINT_ERROR("SQLite error code: %d\n", e.getErrorCode());
        PRINT_ERROR("SQLite extended error code: %d\n", e.getExtendedErrorCode());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    } catch (const std::exception& e) {  // 捕获标准异常
        PRINT_ERROR("Standard exception: %s\n", e.what());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    } catch (...) {  // 捕获其他所有异常
        PRINT_ERROR("Unknown exception occurred\n");
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return false;
    }
}

// 鉴定knowledgeBaseId是哪一种知识库：RAG或者FAQ
KBType TraditionalDatabaseEngine::knowledgeBaseType(const std::string& appId, const std::string& knowledgeBaseId,
                                                    KBErrorCode& errCode) {
    KBType res = KBType::RAG;
    if (!k_db) {
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s, default return KBType::RAG\n", __FUNCTION__);
            return res;
        }
    }
    try {
        SQLite::Statement stmt(*k_db, "SELECT type FROM knowledge_base WHERE app_id = ? AND knowledge_base_id = ?");
        stmt.bind(1, appId);
        stmt.bind(2, knowledgeBaseId);

        if (stmt.executeStep()) {
            res = static_cast<KBType>(stmt.getColumn(0).getInt());
        }
        return res;
    } catch (const SQLite::Exception& e) {  // 捕获SQLite特定异常
        PRINT_ERROR("SQLite error: %s\n", e.what());
        PRINT_ERROR("SQLite error code: %d\n", e.getErrorCode());
        PRINT_ERROR("SQLite extended error code: %d\n", e.getExtendedErrorCode());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return KBType::RAG;
    } catch (const std::exception& e) {  // 捕获标准异常
        PRINT_ERROR("Standard exception: %s\n", e.what());
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return KBType::RAG;
    } catch (...) {  // 捕获其他所有异常
        PRINT_ERROR("Unknown exception occurred\n");
        errCode = KBErrorCode::UNKNOWN_ERROR;
        return KBType::RAG;
    }
}

bool TraditionalDatabaseEngine::isKnowledgeBaseExisted(const std::string& appId, const std::string& knowledgeBaseName) {
    if (!k_db) {
        KBErrorCode errCode;
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return false;
        }
    }
    try {
        SQLite::Statement stmt(*k_db, "SELECT COUNT(*) FROM knowledge_base WHERE app_id = ? AND name = ?");
        stmt.bind(1, appId);
        stmt.bind(2, knowledgeBaseName);

        if (stmt.executeStep()) {
            return stmt.getColumn(0).getInt() > 0;
        }
        return false;
    } catch (...) {
        return false;
    }
    return false;
}

bool TraditionalDatabaseEngine::hasFileInKnowledgeBase(const std::string& knowledgeBaseId, const std::string& fileId) {
    if (!k_db) {
        KBErrorCode errCode;
        if (!initTraditionalDatabase(k_db, errCode)) {
            PRINT_ERROR("Error: Database not initialized, by func:%s\n", __FUNCTION__);
            return false;
        }
    }
    try {
        SQLite::Statement stmt(*k_db,
                               "SELECT COUNT(*) FROM knowledge_file_base WHERE file_id = ? AND knowledge_base_id = ?");
        stmt.bind(1, fileId);
        stmt.bind(2, knowledgeBaseId);

        if (stmt.executeStep()) {
            return stmt.getColumn(0).getInt() > 0;
        }
        return false;
    } catch (...) {
        return false;
    }
    return false;
}

}  // namespace knowledgebase

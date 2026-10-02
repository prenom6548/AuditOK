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

#include "aiIndex.h"

#include <kyai-business-framework/conf.h>
#include <kyai-business-framework/log.h>

#include "cpptime.h"
#include "embedding.h"
#include "fileInfoService.h"
#include "recoverdb.h"
#include "summaryTask.h"
#include "systemCallback.h"
#include "tagTask.h"
#include "textEmbeddingTask.h"

namespace DataManagement {
AIIndex& AIIndex::getInstance() {
    static AIIndex aiIndex;
    return aiIndex;
}

AIIndex::AIIndex()
    : idleSignalConnection(nullptr), idleSignalId(0), idleState(IDLE_STATE_BUSY), gSettings(nullptr), schema(nullptr) {}

int AIIndex::exec() {
    if (checkAndRecoverDatabase()) {
        return -1;
    }

    migrateVectorDatabase();

    syncData();

    resetFailedTasksOnRestart();

    if (listenAIIndexToggle()) {
        return -1;
    }

    if (listenSignal()) {
        return -1;
    }

    createWorkTask();

    startWorkTask();

    return 0;
}

void AIIndex::setIdleState(IdleState newStatus) {
    PRINT_INFO("SetIdleState from %d to %d.\n", idleState, newStatus);

    std::unique_lock<std::shared_mutex> lock(idleLock);
    if (!checkIdleState(newStatus)) {
        PRINT_ERROR("SetIdleState status is wrong, from %d to %d.\n", idleState, newStatus);
        return;
    }

    idleState = newStatus;
}

void AIIndex::setAIIndexEnable(bool enable) {
    PRINT_INFO("SetAIIndexEnable from %s to %s.\n", isAIIndexEnable ? "true" : "false", enable ? "true" : "false");
    std::unique_lock<std::shared_mutex> lock(aiIndexLock);
    isAIIndexEnable = enable;
}

bool AIIndex::isEnable() {
    std::shared_lock<std::shared_mutex> lock(aiIndexLock);
    return isAIIndexEnable;
}

void AIIndex::notifyTaskByType(std::set<TaskType> taskType) {
    if (taskType.empty()) {
        return;
    }

    for (const auto& worker : taskLists) {
        if (taskType.find(worker->getTaskType()) != taskType.end()) {
            worker->awakeSelf();
        }
    }
}

void AIIndex::resetTaskState() {
    PRINT_INFO("AIIndex::resetTaskState...\n");
    for (const auto& worker : taskLists) {
        worker->resetTaskState();
    }
}

bool AIIndex::isIdle() {
    std::shared_lock<std::shared_mutex> lock(idleLock);
    return idleState == IDLE_STATE_TRUE_IDLE;
}

int AIIndex::listenSignal() {
    PRINT_INFO("Start listening system idle signal...\n");

    // 连接到会话总线，dbus demo的方式
    GError* error = nullptr;
    idleSignalConnection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
    if (idleSignalConnection == nullptr) {
        if (error) {
            PRINT_ERROR("Connecting dbus demo failed, error: %s.\n", error->message);
            g_error_free(error);
        } else {
            PRINT_ERROR("Connecting dbus demo failed, but error is null.\n");
        }
        return -1;
    }

    // 系统空闲信号的地址
    const char* sender = "org.gnome.SessionManager";
    const char* interfaceName = "org.gnome.SessionManager.Presence";
    const char* objectPath = "/org/gnome/SessionManager/Presence";
    const char* single = "StatusChanged";
    idleSignalId = g_dbus_connection_signal_subscribe(idleSignalConnection, sender, interfaceName, single, objectPath,
                                                      nullptr, G_DBUS_SIGNAL_FLAGS_NONE, judgeIsIdle, nullptr, nullptr);

    PRINT_INFO("Start listening system idle signal success, id: %d\n", idleSignalId);
    return 0;
}

int AIIndex::listenAIIndexToggle() {
    PRINT_INFO("Start listening ai index toggle signal...\n");

    // 监听AI Index Toggle
    const gchar* schemaID = "org.ukui.search.settings";
    const gchar* keyName = "ai-index-enable";
    const gchar* expectedType = "b";
    const gchar* single = "changed::ai-index-enable";

    GSettingsSchemaSource* source = g_settings_schema_source_get_default();
    schema = g_settings_schema_source_lookup(source, schemaID, TRUE);
    if (!schema) {
        PRINT_ERROR("Schema %s not found.", schemaID);
        return -1;
    }

    gSettings = g_settings_new(schemaID);
    if (!gSettings) {
        g_settings_schema_unref(schema);
        schema = nullptr;
        PRINT_ERROR("Failed to create GSettings object with schema ID: %s", schemaID);
        return -1;
    }

    if (!isAiIndexKeyValid(schema, gSettings, keyName, expectedType)) {
        g_settings_schema_unref(schema);
        schema = nullptr;
        g_object_unref(gSettings);
        gSettings = nullptr;
        return -1;
    }

    g_signal_connect(gSettings, single, G_CALLBACK(judgeIsAIIndexEnable), NULL);

    // isAIIndexEnable初始化
    isAIIndexEnable = g_settings_get_boolean(gSettings, keyName);
    PRINT_INFO("AIIndex::listenAIIndexToggle current AIIndexEnable is: %s.\n", isAIIndexEnable ? "true" : "false");
    if (isAIIndexEnable) {
        // 如果在进程启动并开着ai索引时，这个时候采取开启定时器去加载模型
        loadResourcesAfterDelay();
    }

    return 0;
}

bool AIIndex::isAiIndexKeyValid(GSettingsSchema* schema, GSettings* settings, const gchar* key,
                                const gchar* expectedType) {
    if (!schema || !settings || !key || !expectedType) {
        PRINT_ERROR("AIIndex::isAiIndexKeyValid parameter input error.\n");
        return false;
    }

    gchar** keys = g_settings_schema_list_keys(schema);
    if (!keys) {
        PRINT_ERROR("AIIndex::isAiIndexKeyValid get schema list keys error.\n");
        return false;
    }

    // 通过g_settings_get_value接口获取不存在的键会越界
    bool isKeyExists = false;
    for (gchar** k = keys; *k; k++) {
        if (g_strcmp0(*k, key) == 0) {
            isKeyExists = true;
            break;
        }
    }
    g_strfreev(keys);
    if (!isKeyExists) {
        PRINT_ERROR("Key %s does not exist in schema.\n", key);
        return false;
    }

    // 检查键类型是否匹配
    GVariantType* expectedVarType = g_variant_type_new(expectedType);
    GVariant* value = g_settings_get_value(settings, key);
    const GVariantType* actualVarType = g_variant_get_type(value);  // 返回是引用不需要释放
    bool type_matches = g_variant_type_equal(expectedVarType, actualVarType);

    g_variant_unref(value);
    g_variant_type_free(expectedVarType);

    if (!type_matches) {
        PRINT_ERROR("Key %s type does not match expected type %s.\n", key, expectedType);
        return false;
    }

    return true;
}

bool AIIndex::checkIdleState(IdleState newStatus) const {
    switch (newStatus) {
        case IDLE_STATE_BUSY:
            return idleState == IDLE_STATE_SYSTEM_IDLE || idleState == IDLE_STATE_TRUE_IDLE;
        case IDLE_STATE_SYSTEM_IDLE:
            return idleState == IDLE_STATE_BUSY;
        case IDLE_STATE_TRUE_IDLE:
            return idleState == IDLE_STATE_SYSTEM_IDLE;
        default:
            return false;
    }
}

void AIIndex::createWorkTask() {
    const std::map<int, std::shared_ptr<BaseTask>> taskTypeMap = {
        {TASK_TYPE_TEXT_EMBEDDING, std::make_shared<TextEmbeddingTask>()},
        {TASK_TYPE_TAG, std::make_shared<TagTask>()},
        {TASK_TYPE_SUMMARY, std::make_shared<SummaryTask>()},
    };

    std::vector<int> taskType = Configuration::getIntArray("ai_index_task_type", {1, 2, 3});
    for (const auto& item : taskType) {
        const std::map<int, std::shared_ptr<BaseTask>>::const_iterator& it = taskTypeMap.find(item);
        if (it == taskTypeMap.end()) {
            PRINT_ERROR("createWorkTask cannot find task type: %d.\n", item);
            continue;
        }
        PRINT_INFO("createWorkTask %d.\n", item);
        taskLists.push_back(it->second);
    }
}

void AIIndex::startWorkTask() {
    for (const auto& worker : taskLists) {
        worker->start();
    }
}

/*
*** 启动10s后加载向量模型 防止在没有加载向量化模型时
*** 第一次搜索等待时间过长 (等向量化模型加载时间没有问题的时候可以去掉)
*/
void AIIndex::loadResourcesAfterDelay() {
    int waitSecond = 10;
    auto t = CppTime::getGlobalTimer();
    t->add(std::chrono::seconds(waitSecond), [t](CppTime::timer_id id) {
        Embedding textEmbedding(DataManagementEmbeddingDataType::Text, scene::TaskPriority::MEDIUM);
        Embedding imageTextEmbedding(DataManagementEmbeddingDataType::ImageText, scene::TaskPriority::HIGH);
        PRINT_INFO("AIIndex::loadResourcesAfterDelay load embedding model resources.\n");
        t->discard(id);
    });
}

/**
 * 版本升级，同步已经建好的数据
 */
void AIIndex::syncData() {
    SQLite::Database db = FileDatabase::createDatabase();
    if (!db.tableExists("Files")) {
        PRINT_INFO("AIIndex::syncData Files table not exist.\n");
        return;
    }

    SQLite::Statement query(db, "SELECT file_id, file_path FROM Files");
    int rows;
    while (query.executeStep()) {
        int64_t fileId = query.getColumn(0);
        std::string filePath = query.getColumn(1);
        std::string modifyTime = FileInfoService::getLastModifyTime(filePath);

        // 图片向量化已经做完，无需创建任务
        const std::vector<std::string> imageSupportFormat = {"png", "jpg", "jpeg", "jpe", "bmp", "dib"};
        if (std::find(imageSupportFormat.begin(), imageSupportFormat.end(), filePath.substr(filePath.rfind('.') + 1)) !=
            imageSupportFormat.end()) {
            PRINT_DEBUG("file path: %s, is image.\n", filePath.c_str());
            SQLite::Statement fileInfoStatement(
                db, "INSERT INTO t_file_info(file_id, enable, file_path, file_modify_time) VALUES(?, 1, ?, ?)");
            fileInfoStatement.bind(1, fileId);
            fileInfoStatement.bind(2, filePath);
            fileInfoStatement.bind(3, modifyTime);

            rows = fileInfoStatement.exec();
            PRINT_DEBUG("SQL: %s, success %d rows.\n", fileInfoStatement.getExpandedSQL().c_str(), rows);
            continue;
        }

        SQLite::Statement fileInfoStatement(
            db, "INSERT INTO t_file_info(file_id, file_path, file_modify_time) VALUES(?, ?, ?)");
        fileInfoStatement.bind(1, fileId);
        fileInfoStatement.bind(2, filePath);
        fileInfoStatement.bind(3, modifyTime);

        rows = fileInfoStatement.exec();
        PRINT_DEBUG("SQL: %s, success %d rows.\n", fileInfoStatement.getExpandedSQL().c_str(), rows);

        // 同步过来的数据，向量化已经做完，标签和摘要未做完
        SQLite::Statement aiIndexStatement(
            db, "INSERT INTO t_ai_index_task(file_id, file_path, state, type) VALUES(?,?,?,?), (?,?,?,?), (?,?,?,?)");
        aiIndexStatement.bind(1, fileId);
        aiIndexStatement.bind(2, filePath);
        aiIndexStatement.bind(3, TASK_STATE_SUCCESS);
        aiIndexStatement.bind(4, TASK_TYPE_TEXT_EMBEDDING);
        aiIndexStatement.bind(5, fileId);
        aiIndexStatement.bind(6, filePath);
        aiIndexStatement.bind(7, TASK_STATE_INIT);
        aiIndexStatement.bind(8, TASK_TYPE_TAG);
        aiIndexStatement.bind(9, fileId);
        aiIndexStatement.bind(10, filePath);
        aiIndexStatement.bind(11, TASK_STATE_INIT);
        aiIndexStatement.bind(12, TASK_TYPE_SUMMARY);
        PRINT_DEBUG("SQL: %s.\n", aiIndexStatement.getExpandedSQL().c_str());
        rows = aiIndexStatement.exec();
        PRINT_DEBUG("SQL: %s, success %d rows.\n", aiIndexStatement.getExpandedSQL().c_str(), rows);
    }

    db.exec("DROP TABLE Files");
    db.exec("DROP TABLE Tags");
    db.exec("DROP TABLE FileTags");
    PRINT_INFO("AIIndex::syncData drop table: Files, Tags, FileTags.\n");

    DataManagementVectorDatabase::getInstance().dropCollection("files_tags");
    PRINT_INFO("AIIndex::syncData drop vector-db collection: files_tags.\n");

    // 单例类的构建函数会去创建所有需要的表,这里为了同步数据把files_tags表删了,所以需要删完表之后在手动创建该表
    DataManagementVectorDatabase::getInstance().createCollection("files_tags");
}

void AIIndex::migrateVectorDatabase() {
    // 向量引擎服务升级，db文件拆分，服务自己管理自己的db文件，提供适配逻辑
    std::string homePath = std::getenv("HOME") ? std::string(std::getenv("HOME")) : "";
    std::string databasePath = homePath + "/.local/kylin-ai-business-framework/datamanagement/database/default.db";
    if (std::filesystem::exists(databasePath)) {
        // 1.data-management服务自己的向量引擎服务db文件已经存在，不需要拷贝
        PRINT_DEBUG("AIIndex::migrateVectorDatabase db file already exist.\n");
        return;
    }

    std::string vectorEngineSourceDB = homePath + "/.local/share/kylin-ai-vector-engine/default.db";
    if (!std::filesystem::exists(vectorEngineSourceDB)) {
        // 2. 全新环境，这里直接返回
        PRINT_DEBUG("AIIndex::migrateVectorDatabase default db file is not exist.\n");
        return;
    }

    // 3.升级时，拷贝db文件
    std::filesystem::path dbDir = std::filesystem::path(databasePath).parent_path();
    std::filesystem::copy_file(vectorEngineSourceDB, databasePath);
    PRINT_DEBUG("AIIndex::migrateVectorDatabase copy db file success.\n");

    // 删除不需要的表
    std::vector<std::string> collections = DataManagementVectorDatabase::getInstance().showCollection();
    for (const auto& item : collections) {
        PRINT_DEBUG("AIIndex::migrateVectorDatabase collection: %s.\n", item.c_str());
        if (item != FILE_TAGS_COLLECTION_NAME && item != VISION_FILE_CONTENT_COLLECTION_NAME &&
            item != TEXT_FILE_CONTENT_COLLECTION_NAME) {
            DataManagementVectorDatabase::getInstance().dropCollection(item);
        }
    }
}

/**
 * 失败且重试次数等于3的任务，在服务重启后，任务重置
 */
void AIIndex::resetFailedTasksOnRestart() {
    SQLite::Database db = FileDatabase::createDatabase();
    SQLite::Statement query(db, "SELECT id FROM t_ai_index_task WHERE (state=5 AND retry_num>=3)");
    int rows;
    while (query.executeStep()) {
        int id = query.getColumn(0);
        SQLite::Statement statement(db, "UPDATE t_ai_index_task SET state=0, retry_num=0 WHERE id=?");
        statement.bind(1, id);
        rows = statement.exec();
        PRINT_DEBUG("SQL: %s, success %d rows.\n", statement.getExpandedSQL().c_str(), rows);
    }
}

#ifdef ENABLE_SQLITE_DB_RECOVERY
int AIIndex::checkAndRecoverDatabase() {
    const std::string dbPath = FileDatabase::getDbFilePath();
    std::error_code ec;
    if (!std::filesystem::exists(std::filesystem::path(dbPath), ec) || ec) {
        // 在新环境中第一次启动，还没有数据库，所以不用去检查数据库是否损坏
        return 0;
    }

    PRINT_DEBUG("AIIndex::checkAndRecoverDatabase database integrity check start.\n");
    if (Sqlite3Recover::checkDatabaseIntegrity(dbPath)) {
        PRINT_DEBUG("AIIndex::checkAndRecoverDatabase database integrity check success.\n");
        return 0;
    }

    if (!Sqlite3Recover::recoverDatabase(dbPath)) {
        PRINT_ERROR("AIIndex::checkAndRecoverDatabase database recover failed.\n");
        return -1;
    }

    PRINT_DEBUG("AIIndex::checkAndRecoverDatabase database integrity check success.\n");
    return 0;
}
#else
int AIIndex::checkAndRecoverDatabase() {
    PRINT_DEBUG("SQLite3 version is too old (< 3.42.0), database recovery is disabled.\n");
    return 0;
}
#endif

AIIndex::~AIIndex() {
    if (idleSignalConnection && idleSignalId > 0) {
        g_dbus_connection_signal_unsubscribe(idleSignalConnection, idleSignalId);
        idleSignalId = 0;
        g_object_unref(idleSignalConnection);
        idleSignalConnection = nullptr;
    }

    if (gSettings) {
        g_object_unref(gSettings);
        gSettings = nullptr;
    }

    if (schema) {
        g_settings_schema_unref(schema);
        schema = nullptr;
    }
}
}  // namespace DataManagement

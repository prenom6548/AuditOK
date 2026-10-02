#include "storagemanager.h"

#include <kyai-business-framework/log.h>

#include <chrono>
#include <future>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

#include "../common/performancetimer.h"
#include "textsearchengine.h"
#include "vectorizationengine.h"

namespace knowledgebase {

std::vector<FileUploadResponse> StorageManager::uploadFileStatus_;

static constexpr int KB_UPLOAD_FAILED_PROGRESS = 0;
static constexpr int KB_UPLOAD_PENDING_PROGRESS = 0;
static constexpr int KB_UPLOAD_PARSING_PROGRESS = 10;
static constexpr int KB_UPLOAD_EMBEDDING_PROGRESS = 20;
static constexpr int KB_UPLOAD_INDEXING_PROGRESS = 80;
static constexpr int KB_UPLOAD_STORAGING_PROGRESS = 90;
static constexpr int KB_UPLOAD_COMPLETED_PROGRESS = 100;

static constexpr int MAX_CHARACTER_SIZE = 250000;

KBErrorCode filePaserErrorCodeToKBErrorCode(int errCode) {
    switch (errCode) {
        case 1:
            return KBErrorCode::DOC_FORMAT_UNSUPPORTED;
        case 2:
            return KBErrorCode::DOC_NOT_FOUND;
        case 3:
            return KBErrorCode::DOC_PROCESSING_FAILED;
        case 200:
            return KBErrorCode::DOC_CONTENT_TOO_LONG;
        default:
            return KBErrorCode::UNKNOWN_ERROR;
    }
}

StorageManager::StorageManager(const std::string appId, const std::string password, const std::string dbFilePath)
    : appId_(appId), password_(password), dbFilePath_(dbFilePath) {
    // 创建固定大小的线程池（例如 CPU 核心数）
    traditionalDatabaseEngine_ = std::make_unique<TraditionalDatabaseEngine>(dbFilePath);
    size_t maxThreads = std::thread::hardware_concurrency() / 2;
    maxThreads = maxThreads > 0 ? maxThreads : 4;
    threadPool_ = std::make_unique<ThreadPool>(maxThreads);
}

// 获取当前时间并格式化为 YYYYMMDDHHMMSS
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    localtime_r(&time_t_now, &tm_now);  // 使用线程安全版本

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << (tm_now.tm_year + 1900) << std::setw(2) << (tm_now.tm_mon + 1)
        << std::setw(2) << tm_now.tm_mday << std::setw(2) << tm_now.tm_hour << std::setw(2) << tm_now.tm_min
        << std::setw(2) << tm_now.tm_sec;

    return oss.str();
}

bool StorageManager::createKnowledgeBase(const std::string& name, const std::string& description,
                                         std::string& knowledgeBaseId, KBType type, IndexType indexType,
                                         KBErrorCode& errCode) {
    if (createDirectoryIfNotExists(dbFilePath_) == false) {
        errCode = KBErrorCode::KB_CREATE_FAILED;
        return false;
    }

    VectorizationEngine& vectorizationEngine = VectorizationEngine::getInstance();
    TextSearchEngine& textSearchEngine = TextSearchEngine::getInstance();
    knowledgeBaseId = name + getCurrentTimestamp();
    traditionalDatabaseEngine_->getKnowledgeBase(knowledgeBaseId, errCode);
    if (errCode != KBErrorCode::KB_NOT_FOUND) {
        errCode = KBErrorCode::KB_ALREADY_EXISTS;
        return false;
    }
    std::string collectionName = "vecdb_" + knowledgeBaseId;
    std::string textSearchDbName = "textdb_" + knowledgeBaseId;
    vectorizationEngine.createVectorDatabase(appId_, password_, dbFilePath_, collectionName, description, type,
                                             indexType, errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        return false;
    }

    std::string textSearchDbPath = dbFilePath_ + "/" + textSearchDbName;
    textSearchEngine.createTextSearchDatabase(appId_, textSearchDbPath, errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        KBErrorCode rollbackErrCode;
        vectorizationEngine.deleteVectorDatabase(appId_, password_, dbFilePath_, collectionName, rollbackErrCode);
        return false;
    }

    traditionalDatabaseEngine_->createKnowledgeBase(appId_, name, description, collectionName, textSearchDbName,
                                                    knowledgeBaseId, type, indexType, errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        KBErrorCode rollbackErrCode;
        textSearchEngine.deleteTextDatabase(dbFilePath_ + "/" + textSearchDbName, rollbackErrCode);
        vectorizationEngine.deleteVectorDatabase(appId_, password_, dbFilePath_, collectionName, rollbackErrCode);
        return false;
    }

    return true;
}

bool StorageManager::deleteKnowledgeBase(const std::string& knowledgeBaseId, KBErrorCode& errCode) {
    if (!hasAccessPermission(knowledgeBaseId, errCode)) {
        errCode = KBErrorCode::KB_ACCESS_DENIED;
        return false;
    }
    VectorizationEngine& vectorizationEngine = VectorizationEngine::getInstance();
    TextSearchEngine& textSearchEngine = TextSearchEngine::getInstance();

    std::string collectionName, textSearchDbName;
    traditionalDatabaseEngine_->getKnowledgeBaseDatabaseName(knowledgeBaseId, collectionName, textSearchDbName,
                                                             errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        return false;
    }
    vectorizationEngine.deleteVectorDatabase(appId_, password_, dbFilePath_, collectionName, errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        return false;
    }
    std::string textSearchDbPath = dbFilePath_ + "/" + textSearchDbName;
    textSearchEngine.deleteTextDatabase(textSearchDbPath, errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        return false;
    }
    traditionalDatabaseEngine_->deleteKnowledgeBase(knowledgeBaseId, errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        return false;
    }

    return true;
}

void StorageManager::updateFileProcessingProgress(const std::string& knowledgeBaseId, const std::string& fileId,
                                                  const int& progress, const FileUploadStatus& status,
                                                  const KBErrorCode& errCode) {
    std::lock_guard<std::mutex> lock(statusMutex_);

    if (progress == 100) {  // 文档上传完成后清除此记录
        for (auto it = uploadFileStatus_.begin(); it != uploadFileStatus_.end();) {
            if (it->fileId == fileId && it->knowledgeBaseId == knowledgeBaseId) {  // 找到条目后清除完成条目
                it = uploadFileStatus_.erase(it);                                  // erase 返回下一个有效迭代器
            } else {
                ++it;
            }
        }
    } else {
        bool found = false;  // 标记是否找到匹配的条目
        for (auto it = uploadFileStatus_.begin(); it != uploadFileStatus_.end();) {
            if (it->fileId == fileId && it->knowledgeBaseId == knowledgeBaseId) {  // 找到条目后更新条目
                it->progress = progress;
                it->status = status;
                it->errCode = errCode;
                found = true;  // 标记已找到
                break;         // 找到后可以提前退出循环
            } else {
                ++it;
            }
        }
        if (!found) {  // 如果没有找到匹配的元素，则添加新的元素
            uploadFileStatus_.push_back(FileUploadResponse{knowledgeBaseId, fileId, progress, status, errCode});
        }
    }
}

bool StorageManager::isFileUploadFailed(const std::string& knowledgeBaseId, const std::string& fileId) {
    for (auto it = uploadFileStatus_.begin(); it != uploadFileStatus_.end();) {
        if (it->knowledgeBaseId == knowledgeBaseId && it->fileId == fileId) {
            return it->status == FileUploadStatus::FAILED;
        }
        it++;
    }
    return false;
}

bool StorageManager::uploadDocumentsBatch(const std::string& knowledgeBaseId,
                                          const std::vector<std::pair<std::string, std::string>>& files,
                                          UploadDocumentProgressCallback callback, const FileProcessorConfig& config,
                                          KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();  // 获取单例

    if (!checkUploadPermission(knowledgeBaseId, callback, errCode)) {
        return false;
    }

    struct ValidFileInfo {
        std::string filePath;
        std::string fileId;
    };
    std::vector<ValidFileInfo> validFiles;

    for (const auto& file : files) {
        const std::string& filePath = file.first;
        const std::string& fileId = file.second;

        if (fileId.empty()) {
            notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_FAILED_PROGRESS, FileUploadStatus::FAILED, callback,
                           KBErrorCode::DOC_FILEID_INVALID);
            continue;
        }
        if (traditionalDatabaseEngine_->hasFileInKnowledgeBase(knowledgeBaseId, fileId)) {
            notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_FAILED_PROGRESS, FileUploadStatus::FAILED, callback,
                           KBErrorCode::DOC_ALREADY_EXISTS);
            continue;
        }

        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PENDING_PROGRESS, FileUploadStatus::PENDING, callback);
        validFiles.push_back({filePath, fileId});
    }

    if (validFiles.empty()) {
        errCode = KBErrorCode::DOC_UPLOAD_FAILED;
        FileUploadResponse response{knowledgeBaseId, "", KB_UPLOAD_FAILED_PROGRESS, FileUploadStatus::FAILED};
        callback(response, errCode);
        return false;
    }

    std::vector<std::future<void>> fileProcessingFutures;

    for (const auto& info : validFiles) {
        const std::string& filePath = info.filePath;
        const std::string& fileId = info.fileId;

        fileProcessingFutures.emplace_back(
            threadPool_->enqueue([this, &knowledgeBaseId, filePath, fileId, &callback, &config]() {
                storageSingleFile(knowledgeBaseId, filePath, fileId, callback, config);
            }));
    }

    // 等待所有文件处理完成
    for (auto& future : fileProcessingFutures) {
        try {
            future.wait();
        } catch (const std::exception& e) {
            PRINT_ERROR("Error: %s\n", e.what());
        }
    }

    notifyProgress(knowledgeBaseId, "", KB_UPLOAD_COMPLETED_PROGRESS, FileUploadStatus::ALLCOMPLETED, callback,
                   errCode);
    return true;
}

void StorageManager::storageSingleFile(const std::string& knowledgeBaseId, const std::string& filePath,
                                       const std::string& fileId, UploadDocumentProgressCallback& callback,
                                       const FileProcessorConfig& config) {
    auto& timer = PerformanceTimer::getInstance();  // 获取单例
    KBErrorCode errCode;
    addUploadTask(fileId);

    // 检查终止标志
    PRINT_DEBUG("【上传阶段标识】开始：文档上传 / fileId: %s\n", fileId.c_str());
    if (!isFileIdInTaskList(fileId)) {
        errCode = KBErrorCode::SUCCESS;
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::STOPPED, callback,
                       errCode);
        removeUploadTask(fileId);
        return;
    }

    // 使用 std::async 替代手动创建线程，这样可以更好地管理资源
    std::future<std::vector<ChunkResult>> parseFuture =
        std::async(std::launch::async, [&]() -> std::vector<ChunkResult> {
            FileParser parser;
            PRINT_DEBUG("【上传阶段标识】开始：文档解析 / fileId: %s\n", fileId.c_str());
            timer.start("文件解析: " + fileId);
            std::vector<ChunkResult> chunkResults = parser.chunkFiles({filePath}, {MAX_CHARACTER_SIZE});
            timer.end("文件解析: " + fileId);
            PRINT_DEBUG("【上传阶段标识】完成：文档解析 / fileId: %s\n", fileId.c_str());
            return chunkResults;
        });

    // // 持续检查终止标志，直到解析完成或被终止
    std::vector<ChunkResult> chunkResults;
    bool parseComplete = false;
    while (!parseComplete && isFileIdInTaskList(fileId)) {
        // 使用 wait_for 检查未来状态，避免无限等待
        std::future_status status = parseFuture.wait_for(std::chrono::milliseconds(100));
        if (status == std::future_status::ready) {
            try {
                chunkResults = parseFuture.get();
                parseComplete = true;
            } catch (...) {
                // 处理异常情况
                parseComplete = true;
            }
        }
    }
    // 如果在解析过程中被终止，或者解析失败
    if (!isFileIdInTaskList(fileId)) {
        errCode = KBErrorCode::SUCCESS;
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::STOPPED, callback,
                       errCode);
        // std::async 的线程会在 future 析构时自动清理
        // 但我们无法强制停止正在运行的解析操作
        // 这里只能放弃等待结果
        removeUploadTask(fileId);
        return;
    }
    // 检查解析结果
    if (chunkResults.empty() || chunkResults.size() != 1 || !parseComplete) {
        errCode = KBErrorCode::DOC_PROCESSING_FAILED;
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::FAILED, callback,
                       errCode);
        removeUploadTask(fileId);
        return;
    }

    const ChunkResult& chunkResult = chunkResults[0];
    if (chunkResult.errorCode != 0) {
        errCode = filePaserErrorCodeToKBErrorCode(chunkResult.errorCode);
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::FAILED, callback,
                       errCode);
        PRINT_ERROR("Parsing file failed: %s  ErrCode:%d ErrMsg:%s\n", chunkResult.filePath.c_str(),
                    chunkResult.errorCode, chunkResult.errorMsg.c_str());
        removeUploadTask(fileId);
        return;
    } else if (chunkResult.chunk.empty()) {
        errCode = KBErrorCode::DOC_EMPTY;
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::FAILED, callback,
                       errCode);
        removeUploadTask(fileId);
        return;
    }

    // 检查终止标志
    if (!isFileIdInTaskList(fileId)) {
        errCode = KBErrorCode::SUCCESS;
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::STOPPED, callback,
                       errCode);
        removeUploadTask(fileId);
        return;
    }

    // 解析成功，继续处理
    std::vector<KnowledgeItem> knowledgeItems;
    int chunkIndex = 0;
    const std::vector<std::string>& chunks = chunkResult.chunk;
    for (const auto& chunk : chunks) {
        try {
            // 解析为 JSON 对象
            nlohmann::json j = nlohmann::json::parse(chunk);

            std::shared_ptr<FileChunk> chunkPtr = std::make_shared<FileChunk>();
            chunkPtr->knowledgeBaseId_ = knowledgeBaseId;
            chunkPtr->fileId_ = fileId;
            chunkPtr->chunkIndex_ = chunkIndex++;               // 从0自增
            chunkPtr->sectionTitles_ = j["title_path"].dump();  // 将 vector 转换为 JSON 字符串
            chunkPtr->content_ = j["content"];
            chunkPtr->embedding_ = {};                   // 暂不填充，后续由向量数据库填充
            chunkPtr->metaData_ = j["metadata"].dump();  // 将 metadata 对象转换为 JSON 字符串
            chunkPtr->similarity_ = 0.0f;                // 默认值为 0.0f，查询时使用

            KnowledgeItem item(chunkPtr, KBType::RAG);
            knowledgeItems.push_back(item);
        } catch (const nlohmann::json::exception& e) {
            PRINT_ERROR("JSON error: %s\n", e.what());
        }

        // 在循环中也检查终止标志
        if (!isFileIdInTaskList(fileId)) {
            errCode = KBErrorCode::SUCCESS;
            notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::STOPPED, callback,
                           errCode);
            removeUploadTask(fileId);
            return;
        }
    }

    // 检查终止标志
    if (!isFileIdInTaskList(fileId)) {
        errCode = KBErrorCode::SUCCESS;
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::STOPPED, callback,
                       errCode);
        removeUploadTask(fileId);
        return;
    }

    // 继续执行存储逻辑
    storageProcessedItems(knowledgeBaseId, filePath, fileId, knowledgeItems, callback);
    removeUploadTask(fileId);
    return;
}
void StorageManager::storageProcessedItems(const std::string& knowledgeBaseId, const std::string& filePath,
                                           const std::string& fileId, const std::vector<KnowledgeItem>& items,
                                           UploadDocumentProgressCallback& callback) {
    auto& timer = PerformanceTimer::getInstance();
    KBErrorCode errCode;

    // 检查终止标志
    if (!isFileIdInTaskList(fileId)) {
        errCode = KBErrorCode::SUCCESS;
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::STOPPED, callback,
                       errCode);
        return;
    }

    notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::PARSING, callback);

    std::string collectionName, textSearchDbName;
    traditionalDatabaseEngine_->getKnowledgeBaseDatabaseName(knowledgeBaseId, collectionName, textSearchDbName,
                                                             errCode);

    // 检查终止标志
    if (!isFileIdInTaskList(fileId)) {
        errCode = KBErrorCode::SUCCESS;
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_PARSING_PROGRESS, FileUploadStatus::STOPPED, callback,
                       errCode);
        return;
    }

    // 向量化处理
    auto embeddingProgressCallback = [&](float progress) {
        int globalProgress = KB_UPLOAD_EMBEDDING_PROGRESS +
                             static_cast<int>((KB_UPLOAD_INDEXING_PROGRESS - KB_UPLOAD_EMBEDDING_PROGRESS) * progress);
        PRINT_INFO("Embedding progress: %d%%\n", globalProgress);
        notifyProgress(knowledgeBaseId, fileId, globalProgress, FileUploadStatus::EMBEDDING, callback);
    };

    VectorizationEngine& vectorizationEngine = VectorizationEngine::getInstance();
    PRINT_DEBUG("【上传阶段标识】开始：向量化+存储 / fileId: %s\n", fileId.c_str());
    vectorizationEngine.addVectors(getTaskIdByFileId(fileId), appId_, password_, dbFilePath_, collectionName, items,
                                   embeddingProgressCallback, errCode);
    PRINT_DEBUG("【上传阶段标识】完成：向量化+存储 / fileId: %s\n", fileId.c_str());

    if (!isFileIdInTaskList(fileId)) {
        errCode = KBErrorCode::SUCCESS;
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_INDEXING_PROGRESS, FileUploadStatus::STOPPED, callback,
                       errCode);
        return;
    }

    if (errCode != KBErrorCode::SUCCESS) {
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_INDEXING_PROGRESS, FileUploadStatus::FAILED, callback,
                       errCode);
        return;
    }

    // 文本索引处理
    auto indexingProgressCallback = [&](float progress) {
        int globalProgress = KB_UPLOAD_INDEXING_PROGRESS +
                             static_cast<int>((KB_UPLOAD_STORAGING_PROGRESS - KB_UPLOAD_INDEXING_PROGRESS) * progress);
        PRINT_INFO("Indexing progress: %d%%\n", globalProgress);
        notifyProgress(knowledgeBaseId, fileId, globalProgress, FileUploadStatus::INDEXING, callback);
    };

    TextSearchEngine& textSearchEngine = TextSearchEngine::getInstance();
    std::string textSearchDbPath = dbFilePath_ + "/" + textSearchDbName;
    PRINT_DEBUG("【上传阶段标识】开始：分词+存储 / fileId: %s\n", fileId.c_str());
    textSearchEngine.addKnowledgeItems(textSearchDbPath, items, indexingProgressCallback, errCode);
    PRINT_DEBUG("【上传阶段标识】完成：分词+存储 / fileId: %s\n", fileId.c_str());

    if (errCode != KBErrorCode::SUCCESS) {
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_INDEXING_PROGRESS, FileUploadStatus::FAILED, callback,
                       errCode);
        rollbackVectorDatabase(collectionName, {fileId}, errCode);
        return;
    }

    // 添加文档记录
    std::unordered_map<std::string, std::string> successfulFilePair;
    successfulFilePair.emplace(fileId, filePath);
    timer.start("文件记录: " + fileId);
    PRINT_DEBUG("【上传阶段标识】开始：建立索引 / fileId: %s\n", fileId.c_str());
    traditionalDatabaseEngine_->addDocumentsBatch(knowledgeBaseId, successfulFilePair, errCode);
    PRINT_DEBUG("【上传阶段标识】完成：建立索引 / fileId: %s\n", fileId.c_str());
    timer.end("文件记录: " + fileId);

    if (errCode != KBErrorCode::SUCCESS) {
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_STORAGING_PROGRESS, FileUploadStatus::FAILED, callback,
                       errCode);
        // 回滚向量数据库和文本搜索数据库
        if (!rollbackVectorDatabase(collectionName, {fileId}, errCode)) {
            return;
        }
        rollbackTextSearchDatabase(textSearchDbPath, {fileId}, errCode);
        return;
    }

    // 检查终止标志
    if (!isFileIdInTaskList(fileId)) {
        // rollback后上报终止
        std::vector<std::string> failedFile;
        // 上传逻辑运行到这里已经完全存储完成了，所以rollback直接调用deleteDocuments即可
        deleteDocuments(knowledgeBaseId, {fileId}, failedFile, errCode);
        errCode = KBErrorCode::SUCCESS;
        notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_STORAGING_PROGRESS, FileUploadStatus::STOPPED, callback,
                       errCode);
        return;
    }

    notifyProgress(knowledgeBaseId, fileId, KB_UPLOAD_COMPLETED_PROGRESS, FileUploadStatus::COMPLETED, callback,
                   errCode);
    PRINT_DEBUG("【上传阶段标识】完成：文档上传 / fileId: %s\n", fileId.c_str());
}

// 重构后的 deleteDocuments 函数
bool StorageManager::deleteDocuments(const std::string& knowledgeBaseId, const std::vector<std::string>& fileIds,
                                     std::vector<std::string>& failedFile, KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();
    if (!hasAccessPermission(knowledgeBaseId, errCode)) {
        errCode = KBErrorCode::KB_ACCESS_DENIED;
        return false;
    }
    if (fileIds.empty()) {
        errCode = KBErrorCode::SUCCESS;
        return false;
    }

    VectorizationEngine& vectorizationEngine = VectorizationEngine::getInstance();
    TextSearchEngine& textSearchEngine = TextSearchEngine::getInstance();

    std::string collectionName, textSearchDbName;
    traditionalDatabaseEngine_->getKnowledgeBaseDatabaseName(knowledgeBaseId, collectionName, textSearchDbName,
                                                             errCode);
    std::string textSearchDbPath = dbFilePath_ + "/" + textSearchDbName;
    KBType kbType = traditionalDatabaseEngine_->knowledgeBaseType(appId_, knowledgeBaseId, errCode);

    if (errCode != KBErrorCode::SUCCESS) {
        return false;
    }

    // 获取待删除的数据（用于可能的回滚）
    timer.start("文档删除-向量提取总耗时");
    std::vector<KnowledgeItem> vectorItems =
        vectorizationEngine.getVectors(appId_, password_, dbFilePath_, collectionName, fileIds, kbType, errCode);
    timer.end("文档删除-向量提取总耗时");
    timer.start("文档删除-文本提取总耗时");
    std::vector<KnowledgeItem> textItems =
        textSearchEngine.getKnowledgeItems(textSearchDbPath, fileIds, kbType, errCode);
    timer.end("文档删除-文本提取总耗时");

    // 删除向量数据库数据
    timer.start("文档删除-向量删除总耗时");
    vectorizationEngine.deleteVectors(appId_, password_, dbFilePath_, collectionName, fileIds, errCode);
    timer.end("文档删除-向量删除总耗时");
    if (errCode != KBErrorCode::SUCCESS) {
        return false;
    }

    // 删除文本搜索数据库数据
    timer.start("文档删除-文本删除总耗时");
    textSearchEngine.deleteKnowledgeItems(textSearchDbPath, fileIds, errCode);
    timer.end("文档删除-文本删除总耗时");
    if (errCode != KBErrorCode::SUCCESS) {
        // 创建临时任务ID用于回滚
        std::string delFileId = "del_" + fileIds[0];
        addUploadTask(delFileId);

        restoreVectorDatabase(collectionName, vectorItems, getTaskIdByFileId(delFileId), errCode);
        return false;
    }

    // 删除传统数据库记录
    traditionalDatabaseEngine_->deleteDocuments(knowledgeBaseId, fileIds, errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        // 创建临时任务ID用于回滚
        std::string delFileId = "del_" + fileIds[0];
        addUploadTask(delFileId);

        // 回滚向量数据库和文本搜索数据库
        if (!restoreVectorDatabase(collectionName, vectorItems, getTaskIdByFileId(delFileId), errCode)) {
            return false;
        }
        restoreTextSearchDatabase(textSearchDbPath, textItems, errCode);
        return false;
    }

    return true;
}

bool StorageManager::changeKnowledgeBaseName(const std::string& knowledgeBaseId, const std::string& name,
                                             KBErrorCode& errCode) {
    if (!hasAccessPermission(knowledgeBaseId, errCode)) {
        errCode = KBErrorCode::KB_ACCESS_DENIED;
        return false;
    }
    traditionalDatabaseEngine_->changeKnowledgeBaseName(knowledgeBaseId, name, errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        return false;
    }
    return true;
}

bool StorageManager::changeKnowledgeBaseDescription(const std::string& knowledgeBaseId, const std::string& description,
                                                    KBErrorCode& errCode) {
    if (!hasAccessPermission(knowledgeBaseId, errCode)) {
        errCode = KBErrorCode::KB_ACCESS_DENIED;
        return false;
    }
    traditionalDatabaseEngine_->changeKnowledgeBaseDescription(knowledgeBaseId, description, errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        return false;
    }

    return true;
}

KnowledgeBaseInfo StorageManager::getKnowledgeBase(const std::string& knowledgeBaseId, KBErrorCode& errCode) {
    if (!hasAccessPermission(knowledgeBaseId, errCode)) {
        errCode = KBErrorCode::KB_ACCESS_DENIED;
        return KnowledgeBaseInfo();
    }
    KnowledgeBaseInfo kbInfo = traditionalDatabaseEngine_->getKnowledgeBase(knowledgeBaseId, errCode);

    return kbInfo;
}

std::vector<KnowledgeBaseInfo> StorageManager::getAllKnowledgeBase(KBErrorCode& errCode) {
    std::vector<KnowledgeBaseInfo> infoList = traditionalDatabaseEngine_->getAllKnowledgeBase(appId_, errCode);

    return infoList;
}

std::vector<std::string> StorageManager::getFileListInKnowledgeBase(const std::string& knowledgeBaseId,
                                                                    KBErrorCode& errCode) {
    if (!hasAccessPermission(knowledgeBaseId, errCode)) {
        errCode = KBErrorCode::KB_ACCESS_DENIED;
        return {};
    }
    std::vector<std::string> fileIdList =
        traditionalDatabaseEngine_->getFileListInKnowledgeBase(knowledgeBaseId, errCode);

    return fileIdList;
}

void StorageManager::getFileProcessingProgress(const std::string& knowledgeBaseId, const std::string& fileId,
                                               int& progress, FileUploadStatus& status, KBErrorCode& errCode) {
    if (!hasAccessPermission(knowledgeBaseId, errCode)) {
        return;
    }

    std::lock_guard<std::mutex> lock(statusMutex_);
    bool find = false;
    for (auto it = uploadFileStatus_.begin(); it != uploadFileStatus_.end();) {
        if (it->knowledgeBaseId == knowledgeBaseId && it->fileId == fileId) {  // 找到条目后更新条目
            progress = it->progress;
            status = it->status;
            errCode = it->errCode;
            find = true;
            break;  // 找到后可以提前退出循环
        } else {
            ++it;
        }
    }
    if (!find) {
        if (traditionalDatabaseEngine_->hasFileInKnowledgeBase(knowledgeBaseId, fileId)) {
            progress = 100;
            status = FileUploadStatus::COMPLETED;
            errCode = KBErrorCode::SUCCESS;
        } else {
            errCode = KBErrorCode::DOC_NOT_FOUND;
        }
    }

    return;
}

bool StorageManager::hasAccessPermission(const std::string& knowledgeBaseId, KBErrorCode& errCode) {
    return traditionalDatabaseEngine_->hasAccessPermission(appId_, knowledgeBaseId, errCode);
}

bool StorageManager::checkUploadPermission(const std::string& knowledgeBaseId, UploadDocumentProgressCallback callback,
                                           KBErrorCode& errCode) {
    if (!traditionalDatabaseEngine_->hasAccessPermission(appId_, knowledgeBaseId, errCode)) {
        FileUploadResponse response{knowledgeBaseId, "", 0, FileUploadStatus::FAILED};
        callback(response, errCode);
        return false;
    }

    KBType knowledgeBaseType = traditionalDatabaseEngine_->knowledgeBaseType(appId_, knowledgeBaseId, errCode);
    if (knowledgeBaseType == KBType::FAQ) {
        errCode = KBErrorCode::KB_ACCESS_DENIED;
        FileUploadResponse response{knowledgeBaseId, "", 0, FileUploadStatus::FAILED};
        callback(response, errCode);
        return false;
    }
    return true;
}

bool StorageManager::isKnowledgeBaseExisted(const std::string& knowledgeBaseName) {
    return traditionalDatabaseEngine_->isKnowledgeBaseExisted(appId_, knowledgeBaseName);
}

void StorageManager::removeFailedFile(std::vector<std::string>& fileIds, const std::string& fileId) {
    std::lock_guard<std::mutex> lock(fileIdsMutex_);
    auto newEnd = std::remove(fileIds.begin(), fileIds.end(), fileId);
    fileIds.erase(newEnd, fileIds.end());
}

void StorageManager::notifyProgress(const std::string& knowledgeBaseId, const std::string& fileId, int progress,
                                    FileUploadStatus status, UploadDocumentProgressCallback& callback,
                                    KBErrorCode errCode) {
    FileUploadResponse response{knowledgeBaseId, fileId, progress, status};
    callback(response, errCode);
    updateFileProcessingProgress(knowledgeBaseId, fileId, progress, status, errCode);
}

void StorageManager::stopDocumentsUpload(const std::string& knowledgeBaseId, const std::vector<std::string>& fileIds,
                                         KBErrorCode& errCode) {
    VectorizationEngine& vectorizationEngine = VectorizationEngine::getInstance();
    std::vector<int> taskIds;
    for (auto fileId : fileIds) {
        taskIds.push_back(getTaskIdByFileId(fileId));
        removeUploadTask(fileId);
    }
    vectorizationEngine.stopVectorsUpload(taskIds, errCode);

    std::vector<std::string> failedFile;
}

void StorageManager::addUploadTask(const std::string& fileId) {
    int taskId;
    bool unique;
    do {
        taskId = std::rand();
        unique = true;
        // 检查 taskId 是否已作为 value 存在于 map 中
        for (const auto& entry : documentUploadTaskList_) {
            if (entry.second == taskId) {
                unique = false;
                break;
            }
        }
    } while (!unique);

    documentUploadTaskList_[fileId] = taskId;
}

void StorageManager::removeUploadTask(const std::string& fileId) {
    auto it = documentUploadTaskList_.find(fileId);
    if (it != documentUploadTaskList_.end()) {
        int taskId = it->second;
        documentUploadTaskList_.erase(it);
    }
}

bool StorageManager::isFileIdInTaskList(const std::string& fileId) const {
    return documentUploadTaskList_.find(fileId) != documentUploadTaskList_.end();
}

int StorageManager::getTaskIdByFileId(const std::string& fileId) const {
    auto it = documentUploadTaskList_.find(fileId);
    if (it != documentUploadTaskList_.end()) {
        return it->second;
    }
    return -1;  // 表示未找到
}

// 回滚向量数据库操作
bool StorageManager::rollbackVectorDatabase(const std::string& collectionName, const std::vector<std::string>& fileIds,
                                            KBErrorCode& errCode) {
    KBErrorCode rollbackErrCode;
    VectorizationEngine& vectorizationEngine = VectorizationEngine::getInstance();
    vectorizationEngine.deleteVectors(appId_, password_, dbFilePath_, collectionName, fileIds, rollbackErrCode);
    if (rollbackErrCode != KBErrorCode::SUCCESS) {
        PRINT_ERROR("upload vectorDatabase rollback failed, error Code: %d\n", rollbackErrCode);
        errCode = rollbackErrCode;
        return false;
    }
    return true;
}

// 回滚文本搜索数据库操作
bool StorageManager::rollbackTextSearchDatabase(const std::string& textSearchDbPath,
                                                const std::vector<std::string>& fileIds, KBErrorCode& errCode) {
    KBErrorCode rollbackErrCode;
    TextSearchEngine& textSearchEngine = TextSearchEngine::getInstance();
    textSearchEngine.deleteKnowledgeItems(textSearchDbPath, fileIds, rollbackErrCode);
    if (rollbackErrCode != KBErrorCode::SUCCESS) {
        PRINT_ERROR("upload textSearchDatabase rollback failed, error Code: %d\n", rollbackErrCode);
        errCode = rollbackErrCode;
        return false;
    }
    return true;
}

// 恢复向量数据库数据（用于删除操作的回滚）
bool StorageManager::restoreVectorDatabase(const std::string& collectionName, const std::vector<KnowledgeItem>& items,
                                           const int& tempTaskId, KBErrorCode& errCode) {
    KBErrorCode rollbackErrCode;
    VectorizationEngine& vectorizationEngine = VectorizationEngine::getInstance();
    auto emptyCallback = [](float progress) { PRINT_INFO("rollback progress: %.2f%%\n", progress * 100); };

    vectorizationEngine.addVectors(tempTaskId, appId_, password_, dbFilePath_, collectionName, items, emptyCallback,
                                   rollbackErrCode);
    if (rollbackErrCode != KBErrorCode::SUCCESS) {
        PRINT_ERROR("vectorDatabase rollback failed, error Code: %d\n", rollbackErrCode);
        errCode = rollbackErrCode;
        return false;
    }
    return true;
}

// 恢复文本搜索数据库数据（用于删除操作的回滚）
bool StorageManager::restoreTextSearchDatabase(const std::string& textSearchDbPath,
                                               const std::vector<KnowledgeItem>& items, KBErrorCode& errCode) {
    KBErrorCode rollbackErrCode;
    TextSearchEngine& textSearchEngine = TextSearchEngine::getInstance();
    auto emptyCallback = [](float progress) { PRINT_INFO("rollback progress: %.2f%%\n", progress * 100); };

    textSearchEngine.addKnowledgeItems(textSearchDbPath, items, emptyCallback, rollbackErrCode);
    if (rollbackErrCode != KBErrorCode::SUCCESS) {
        PRINT_ERROR("textSearchDatabase rollback failed, error Code: %d\n", rollbackErrCode);
        errCode = rollbackErrCode;
        return false;
    }
    return true;
}

}  // namespace knowledgebase

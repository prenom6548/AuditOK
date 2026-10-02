#pragma once
#include <unistd.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include "errno.h"

namespace knowledgebase {
// 知识库信息
struct KnowledgeBaseInfo {
    std::string name_;
    std::string description_;
    std::string knowledgeBaseId_;
    std::string changeTime_;
    IndexType indexType;
    KBType type_;
};
// 文件处理配置
struct FileProcessorConfig {  // 参考“麒麟知识库可配置参数”
    uint32_t chunkMaxSize_;
    bool enhanceParse_;  // 是否开启增强解析
};
// 问题多样化配置
struct QueryDiversifyConfig {
    bool enable = true;
    int topK = 40;
    double topP = 0.9f;
    double temperature = 0.7f;
    int maxTokens = 2048;
    std::string modelName;  // query diversify 模型名
};
// 问题多样化扩展
struct QueryDiversification {
    std::vector<std::string> queryTexts;
    std::vector<std::string> keywords;
};
// 大语言模型重排与分类配置
struct ChunkEvaluateConfig {
    bool enable = true;
    int topK = 40;
    double topP = 0.9f;
    double temperature = 0.3f;
    int maxTokens = 512;
    std::string modelName;  // LLM Rerank Classification 模型名
    std::string userPrompt;
    std::string systemPrompt;
};
// 检索配置
struct RetrievalConfig {  // 参考“麒麟知识库可配置参数”
    RetrievalMethod retrievalMethod = RetrievalMethod::HYBRID;
    RerankMethod rerankMethod = RerankMethod::RRF;
    std::vector<std::string> fileIds;           // 文件id
    int topK = 4;                               // 返回结果数量
    float vectorScoreThreshold = 0.75f;         // 向量化检索召回分数阈值
    float bm25ScoreThreshold = 0.5f;            // bm25检索召回分数阈值
    bool expandContext = false;                 // 是否启动chunk上下文扩展
    bool fuzzywuzzyRerankEnable = false;        // 是否启动fuzzywuzzyRerank
    uint32_t maxTokens;                         // 4096
    QueryDiversifyConfig queryDiversifyConfig;  // query diversify 配置
    ChunkEvaluateConfig chunkEvaluateConfig;    // 大语言模型重排与分类配置
};
// 聊天配置
struct ChatConfig {  // 参考“麒麟知识库可配置参数”
    int topK = 40;
    double topP = 0.9f;
    double temperature = 0.7f;
    int maxTokens = 4096;
    std::string modelName;            // llm 模型名
    bool creatNewChat = false;        // 为True时创建新聊天，为False时保留对话上下文
    RetrievalConfig retrievalConfig;  // 检索配置
};
// 请求状态
struct AsyncRequestStatus {
    std::string requestId;
    RequestProcessingStatus status = RequestProcessingStatus::PENDING;
};
// 基础块对象
struct BaseItem {
    std::vector<float> embedding_;  // FaqItem中的embedding_表示的是问题 + 问题描述向量
    float similarity_;
    std::string content_;  // FaqItem中的content_表示的是答案

    virtual ~BaseItem() = default;
};
// 文件分块
struct FileChunk : public BaseItem {
    std::string knowledgeBaseId_;
    std::string fileId_;
    int chunkIndex_;
    std::string sectionTitles_;  // json : [ "一级标题"，"语义子段 1" ],
    std::string metaData_;
    std::string md5sum_;

    bool operator==(const FileChunk& other) const {
        return fileId_ == other.fileId_ && chunkIndex_ == other.chunkIndex_ && md5sum_ == other.md5sum_;
    }
};
// FAQ条目
struct FaqItem : public BaseItem {
    std::string knowledgeId_;
    std::string question_;
    std::string description_;  // 问题描述

    bool operator==(const FaqItem& other) const {
        return knowledgeId_ == other.knowledgeId_ && question_ == other.question_;
    }
};
// 知识条目
struct KnowledgeItem {
    std::shared_ptr<BaseItem> item_ = nullptr;
    KBType type_;  // RAG类型或FAQ类型

    KnowledgeItem() = default;
    KnowledgeItem(std::shared_ptr<BaseItem> item, KBType type) : item_(item), type_(type) {}
    KnowledgeItem(const KnowledgeItem& other) : item_(other.item_), type_(other.type_) {}
    KnowledgeItem& operator=(const KnowledgeItem& other) {
        if (this != &other) {
            item_ = other.item_;
            type_ = other.type_;
        }
        return *this;
    }
};
// 知识领域
enum class KnowledgeField {
    GENERAL = 0,
    MEDICAL,
    LEGAL,
    FINANCIAL,
    TECHNICAL,
    OFFICE,
    GOVERNMENT_ENTERPRISE,
    // 特殊标记：必须放在最后,用于统计数量
    KNOWLEDGE_FIELD_COUNT
};
const std::unordered_map<KnowledgeField, std::string> knowledgeFieldMap = {
    {KnowledgeField::GENERAL, "general"},
    {KnowledgeField::MEDICAL, "medical"},
    {KnowledgeField::LEGAL, "legal"},
    {KnowledgeField::FINANCIAL, "financial"},
    {KnowledgeField::TECHNICAL, "technical"},
    {KnowledgeField::OFFICE, "office"},
    {KnowledgeField::GOVERNMENT_ENTERPRISE, "government_enterprise"},
    {KnowledgeField::KNOWLEDGE_FIELD_COUNT, "knowledge_field_count"}};
// 检索结果
struct RetrievalResult {
    std::string requestId;
    std::string queryText;
    std::vector<KnowledgeItem> items;
    KnowledgeField field;
};
// 聊天响应
struct StreamingChatResponse {
    std::string chatId;
    std::string requestId;     // 用来判断是不是新的聊天请求
    std::string modelName;     // 返回模型名
    bool isFinal = true;       // 用来判断是不是最后一个token输出
    std::string result;        // 大模型输出文本
    std::string reasoningMsg;  // 大模型思考过程
    std::vector<std::pair<int, int>>
        referenceIndices;           // pair.first 为参考来源的文件编号（可以直接用作溯源编号，值大于等于1），pair.second
                                    // 为参考来源在检索结果中的chunk索引（用于辅助获取检索数据中的信息，值大于等于0）
    RetrievalResult retrievalData;  // 检索数据（仅在最终响应包含）
    std::vector<std::string> referenceFileIds;  // 参考来源的文件ID（仅在最终响应包含）
};

// 聊天配置
struct FileUploadResponse {       // 文档上传状态，文档上传完成后清除对应条目，失败记录持续保存到程序重启
    std::string knowledgeBaseId;  // 知识库ID
    std::string fileId;           // 文档ID
    int progress;                 // 文档上传进度
    FileUploadStatus status;      // 文档上传状态
    KBErrorCode errCode = KBErrorCode::SUCCESS;
};

using UploadDocumentProgressCallback = std::function<void(FileUploadResponse& response, KBErrorCode& errCode)>;

using StreamingChatCallback =
    std::function<void(StreamingChatResponse& response, KBErrorCode& errCode, std::string& errMsg)>;

using RetrievalCallback = std::function<void(RetrievalResult& response, KBErrorCode& errCode, std::string& errMsg)>;

using KnowledgeItemsMap = std::unordered_map<RetrievalMethod, std::vector<KnowledgeItem>>;

}  // namespace knowledgebase

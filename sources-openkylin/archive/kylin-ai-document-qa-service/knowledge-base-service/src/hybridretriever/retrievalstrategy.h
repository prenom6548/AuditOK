#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../common/common.h"
#include "../storagemanager/textsearchengine.h"
#include "../storagemanager/vectorizationengine.h"

namespace knowledgebase {

// 检索策略接口
class RetrievalStrategy {
public:
    RetrievalStrategy(const std::string& dbFilePath) : dbFilePath_(dbFilePath) {}

    virtual ~RetrievalStrategy() = default;

    // 执行检索
    virtual KnowledgeItemsMap search(const QueryDiversification& queryDiverdification, const RetrievalConfig& config,
                                     const KBType kbType, KBErrorCode& errCode) = 0;
    virtual void stopRetrieve() {};

protected:
    std::string dbFilePath_;
};

// BM25文本检索策略
class TextRetrievalStrategy : public RetrievalStrategy {
public:
    TextRetrievalStrategy(const std::string& textSearchDbName, const std::string& dbFilePath)
        : RetrievalStrategy(dbFilePath), textSearchDbName_(textSearchDbName) {}
    KnowledgeItemsMap search(const QueryDiversification& queryDiverdification, const RetrievalConfig& config,
                             const KBType kbType, KBErrorCode& errCode) override;
    void stopRetrieve() override;

private:
    std::string textSearchDbName_;
    std::atomic<bool> stopFlag_;
};

// BM25关键词检索策略
class KeywordRetrievalStrategy : public RetrievalStrategy {
public:
    KeywordRetrievalStrategy(const std::string& textSearchDbName, const std::string& dbFilePath)
        : RetrievalStrategy(dbFilePath), textSearchDbName_(textSearchDbName) {}
    KnowledgeItemsMap search(const QueryDiversification& queryDiverdification, const RetrievalConfig& config,
                             const KBType kbType, KBErrorCode& errCode) override;
    void stopRetrieve() override;

private:
    std::string textSearchDbName_;
    std::atomic<bool> stopFlag_;
};

// 向量化检索策略
class VectorRetrievalStrategy : public RetrievalStrategy {
public:
    VectorRetrievalStrategy(const std::string& vectorDbName, const std::string& appId, const std::string& password,
                            const std::string& dbFilePath)
        : RetrievalStrategy(dbFilePath), vectorDbName_(vectorDbName), appId_(appId), password_(password) {}

    KnowledgeItemsMap search(const QueryDiversification& queryDiverdification, const RetrievalConfig& config,
                             const KBType kbType, KBErrorCode& errCode) override;
    void stopRetrieve() override;

private:
    std::string vectorDbName_;
    std::string appId_;
    std::string password_;
    std::atomic<bool> stopFlag_;
};

// 混合检索策略
class HybridRetrievalStrategy : public RetrievalStrategy {
public:
    HybridRetrievalStrategy(const std::string& textSearchDbName, const std::string& vectorDbName,
                            const std::string& appId, const std::string& password, const std::string& dbFilePath)
        : RetrievalStrategy(dbFilePath),
          textSearchDbName_(textSearchDbName),
          vectorDbName_(vectorDbName),
          appId_(appId),
          password_(password) {}

    KnowledgeItemsMap search(const QueryDiversification& queryDiverdification, const RetrievalConfig& config,
                             const KBType kbType, KBErrorCode& errCode) override;
    void stopRetrieve() override;

private:
    std::string textSearchDbName_;
    std::string vectorDbName_;
    std::string appId_;
    std::string password_;
    std::atomic<bool> stopFlag_;
};

// 检索策略工厂
class RetrievalStrategyFactory {
public:
    static std::unique_ptr<RetrievalStrategy> createStrategy(
        RetrievalMethod method, const std::string& textSearchDbName = "", const std::string& vectorDbName = "",
        const std::string& appId = "", const std::string& password = "", const std::string& dbFilePath = "");
};

}  // namespace knowledgebase
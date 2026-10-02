#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../common/common.h"

namespace knowledgebase {

// 重排策略接口
class RerankingStrategy {
public:
    virtual ~RerankingStrategy() = default;
    virtual std::vector<KnowledgeItem> rerank(const std::string& query, const KnowledgeItemsMap& retrievalResults) = 0;
};

// RRF重排策略
class RRFrerankingStrategy : public RerankingStrategy {
public:
    std::vector<KnowledgeItem> rerank(const std::string& query, const KnowledgeItemsMap& retrievalResults) override;
};

// 基于归一化和权重的重排策略
class WeightRerankingStrategy : public RerankingStrategy {
public:
    std::vector<KnowledgeItem> rerank(const std::string& query, const KnowledgeItemsMap& retrievalResults) override;
};

// 重排策略工厂
class RerankingStrategyFactory {
public:
    static std::unique_ptr<RerankingStrategy> createStrategy(RerankMethod method);
};

}  // namespace knowledgebase

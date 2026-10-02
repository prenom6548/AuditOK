#include "rerankstrategy.h"

#include <kyai-business-framework/log.h>

#include <algorithm>
#include <unordered_set>

namespace knowledgebase {

std::vector<KnowledgeItem> RRFrerankingStrategy::rerank(const std::string& query,
                                                        const KnowledgeItemsMap& retrievalResults) {
    if (retrievalResults.size() == 0) {
        return {};
    } else if (retrievalResults.size() == 1) {
        return retrievalResults.begin()->second;
    }

    // 常数k，用于平滑低排名的影响（通常取60，此处可根据需求调整）
    constexpr int k = 60;
    struct ItemInfo {
        KnowledgeItem item;
        std::unordered_map<RetrievalMethod, int> ranks;
    };
    std::unordered_map<std::string, ItemInfo> itemMap;

    // 先统计每一个item在不同检索结果中的排名
    for (const auto& retrievalResult : retrievalResults) {
        RetrievalMethod method = retrievalResult.first;
        std::vector<KnowledgeItem> knowledgeItems = retrievalResult.second;

        for (size_t i = 0; i < knowledgeItems.size(); ++i) {
            std::string key;
            if (knowledgeItems[i].type_ == KBType::RAG) {
                FileChunk* chunk = static_cast<FileChunk*>(knowledgeItems[i].item_.get());
                key = chunk->fileId_ + ":" + std::to_string(chunk->chunkIndex_) + chunk->md5sum_;
            } else if (knowledgeItems[i].type_ == KBType::FAQ) {
                FaqItem* faq = static_cast<FaqItem*>(knowledgeItems[i].item_.get());
                key = faq->question_;
            }
            auto& info = itemMap[key];
            if (info.ranks.size() == 0) {
                info.item = knowledgeItems[i];
            }
            info.ranks[method] = static_cast<int>(i) + 1;
        }
    }

    // RRF 重排序
    std::vector<std::pair<KnowledgeItem, double>> scoredItems;
    scoredItems.reserve(itemMap.size());
    for (auto& [key, info] : itemMap) {
        double score = 0.0;
        for (auto& rank : info.ranks) {
            score += 1.0 / (k + rank.second);
        }
        scoredItems.emplace_back(std::move(info.item), score);
    }
    std::sort(scoredItems.begin(), scoredItems.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    std::vector<KnowledgeItem> finalResult;
    finalResult.reserve(scoredItems.size());
    for (auto& [item, score] : scoredItems) {
        finalResult.push_back(std::move(item));
    }
    return finalResult;
}

std::unique_ptr<RerankingStrategy> RerankingStrategyFactory::createStrategy(RerankMethod method) {
    switch (method) {
        case RerankMethod::RRF:
            return std::make_unique<RRFrerankingStrategy>();
        case RerankMethod::WEIGHT:
            return std::make_unique<WeightRerankingStrategy>();
        default:
            return std::make_unique<RRFrerankingStrategy>();
    }
}

}  // namespace knowledgebase

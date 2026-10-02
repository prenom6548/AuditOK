#include <kyai-business-framework/log.h>

#include <cmath>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "rerankstrategy.h"

namespace knowledgebase {

// 辅助函数：获取分数的 min 和 max，处理空或全相同情况
std::pair<float, float> getMinMax(const std::vector<float>& scores) {
    if (scores.empty()) {
        return {0.0f, 0.0f};
    }
    auto [minIt, maxIt] = std::minmax_element(scores.begin(), scores.end());
    return {*minIt, *maxIt};
}

// Min-Max 归一化到 [0, 1]
float normalizeScore(float score, float minVal, float maxVal) {
    if (maxVal <= minVal) {  // 防止除零或无效区间
        return 0.0f;
    }
    float norm = (score - minVal) / (maxVal - minVal);
    return std::max(0.0f, std::min(1.0f, norm));  // clamp to [0,1]
}
std::vector<KnowledgeItem> WeightRerankingStrategy::rerank(const std::string& query,
                                                           const KnowledgeItemsMap& retrievalResults) {
    // 权重定义
    const float WEIGHT_SEMANTIC = 0.4f;
    const float WEIGHT_TEXT = 0.3f;
    const float WEIGHT_KEYWORD = 0.3f;

    // 用于聚合每个 content 的信息
    struct AggregatedItem {
        std::shared_ptr<BaseItem> item;
        KBType type;
        float bestSemanticScore = -std::numeric_limits<float>::infinity();
        float bestTextScore = -std::numeric_limits<float>::infinity();
        float bestKeywordScore = -std::numeric_limits<float>::infinity();
        int hitCount = 0;
        std::set<std::string> matchedContents;  // 可用于 debug，记录哪些 query 角度命中
    };

    std::unordered_map<std::string, AggregatedItem> contentAgg;

    // 所有原始分数收集
    std::vector<float> semanticScores;
    std::vector<float> textScores;
    std::vector<float> keywordScores;

    // 第一步：遍历所有检索方式，提取结果并聚合
    for (const auto& [method, items] : retrievalResults) {
        std::string methodTag = (method == RetrievalMethod::SEMANTIC)  ? "semantic"
                                : (method == RetrievalMethod::TEXT)    ? "text"
                                : (method == RetrievalMethod::KEYWORD) ? "keyword"
                                                                       : "hybrid";

        for (const auto& knowledgeItem : items) {
            if (!knowledgeItem.item_) continue;
            const auto& baseItem = knowledgeItem.item_;
            const std::string& content = baseItem->content_;
            float sim = baseItem->similarity_;

            // 初始化或获取聚合项
            auto& agg = contentAgg[content];
            if (agg.item == nullptr) {
                agg.item = baseItem;
                agg.type = knowledgeItem.type_;
            }
            agg.hitCount++;
            agg.matchedContents.insert(methodTag);

            // 更新各类最高分，并收集原始分数
            if (method == RetrievalMethod::SEMANTIC) {
                if (sim > agg.bestSemanticScore) {
                    agg.bestSemanticScore = sim;
                }
                semanticScores.push_back(sim);
            } else if (method == RetrievalMethod::TEXT) {
                if (sim > agg.bestTextScore) {
                    agg.bestTextScore = sim;
                }
                textScores.push_back(sim);
            } else if (method == RetrievalMethod::KEYWORD) {
                if (sim > agg.bestKeywordScore) {
                    agg.bestKeywordScore = sim;
                }
                keywordScores.push_back(sim);
            }
            // 忽略 HYBRID
        }
    }

    // 第二步：计算归一化所需的 min/max
    auto [semMin, semMax] = getMinMax(semanticScores);
    auto [txtMin, txtMax] = getMinMax(textScores);
    auto [kwMin, kwMax] = getMinMax(keywordScores);

    // 第三步：融合打分并生成结果
    std::vector<std::pair<KnowledgeItem, float>> scoredResults;  // (item, final_score)

    for (const auto& [content, agg] : contentAgg) {
        float normSemantic = (agg.bestSemanticScore > std::numeric_limits<float>::lowest())
                                 ? normalizeScore(agg.bestSemanticScore, semMin, semMax)
                                 : 0.0f;

        float normText = (agg.bestTextScore > std::numeric_limits<float>::lowest())
                             ? normalizeScore(agg.bestTextScore, txtMin, txtMax)
                             : 0.0f;

        float normKeyword = (agg.bestKeywordScore > std::numeric_limits<float>::lowest())
                                ? normalizeScore(agg.bestKeywordScore, kwMin, kwMax)
                                : 0.0f;

        // 加权融合
        float finalScore = WEIGHT_SEMANTIC * normSemantic + WEIGHT_TEXT * normText + WEIGHT_KEYWORD * normKeyword;

        scoredResults.emplace_back(KnowledgeItem(agg.item, agg.type), finalScore);
    }

    // 第四步：按最终分数降序排序
    std::sort(scoredResults.begin(), scoredResults.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // 提取排序后的 KnowledgeItem
    std::vector<KnowledgeItem> result;
    result.reserve(scoredResults.size());
    for (auto& [item, score] : scoredResults) {
        PRINT_DEBUG("[Reranker] (%f)\n", score);
        result.push_back(item);
    }

    return result;
}

}  // namespace knowledgebase

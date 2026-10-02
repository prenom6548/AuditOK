/*
 * Copyright 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "intentslotrules.h"
#include "vectordatabase.h"

class TextEmbeddingInterface;

class RuleBasedIntentRetriever {
public:
    RuleBasedIntentRetriever(std::shared_ptr<IntentSlotRules> rules);
    ~RuleBasedIntentRetriever() = default;
    void recognizeIntent(const std::string& user_input, std::string& predict_intent_name,
                         std::vector<intent::IntentResult>& low_score_intents, float& max_similarity_score);
    void setTextEmbeddingInterface(TextEmbeddingInterface* interface);

private:
    std::vector<float> normalizeVector(const std::vector<float>& vector);
    // 获取带实体融合的向量
    std::vector<float> getVectorWithEntityFusion(const std::string& user_input,
                                                 const std::vector<float>& user_input_vector);
    // 获取匹配的实体
    std::vector<std::pair<std::string, std::vector<std::string>>> getMatchedEntitiesFromText(
        const std::string& user_input);
    // 为实体添加权重
    std::map<std::string, double> addWeightToEntities(
        const std::vector<std::pair<std::string, std::vector<std::string>>>& matched_intent_entities);
    // 融合向量
    std::vector<float> fuseVectors(const std::vector<float>& user_input_vector,
                                   const std::vector<std::vector<float>>& entity_vector_list,
                                   const std::vector<double>& entity_weight_list);
    // 过滤低分意图，提取最终意图
    std::string extractFinalIntent(const std::vector<intent::IntentResult>& predict_intents,
                                   const std::string& input_text);
    // 验证输入文本是否满足指定意图的实体规则（包含必需实体且不包含排除实体）
    bool validateIntentEntityRules(const std::string& predict_intent_name, const std::string& input_text);
    // 检查实体列表中是否有至少一个实体在文本中
    bool anyEntityInText(const std::vector<std::string>& entity_list, const std::string& text);
    // 检查实体字段组合是否匹配（核心逻辑）
    bool checkAllEntitiesPresent(const std::map<std::string, std::vector<std::string>>& include_entities,
                                 const std::string& text);

private:
    std::shared_ptr<IntentSlotRules> intentSlotRules_;
    intent::VectorDatabase vectorDatabase_;

    double lowScoreThreshold_;
    double midScoreThreshold_;
    double highScoreThreshold_;
    double queryWeight_;
    double entityTotalWeight_;

    TextEmbeddingInterface* stringEmbeddingInterface_{nullptr};
};
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

#include "rulebasedintentretriever.h"

#include <algorithm>
#include <map>
#include <set>

#include "assistant/intentdialoguemanager/dialoguemessage.h"
#include "assistant/intentdialoguemanager/textembedding/textembeddinginterface.h"
#include "utils/logger.h"

using namespace intent;

RuleBasedIntentRetriever::RuleBasedIntentRetriever(std::shared_ptr<IntentSlotRules> rules) : intentSlotRules_(rules) {
    lowScoreThreshold_ = DialogueConfig::low_intent_threshold;
    midScoreThreshold_ = DialogueConfig::mid_intent_threshold;
    highScoreThreshold_ = DialogueConfig::high_intent_threshold;
    queryWeight_ = DialogueConfig::query_weight;
    entityTotalWeight_ = DialogueConfig::entity_total_weight;
}

void RuleBasedIntentRetriever::setTextEmbeddingInterface(TextEmbeddingInterface* interface) {
    stringEmbeddingInterface_ = interface;
}

void RuleBasedIntentRetriever::recognizeIntent(const std::string& user_input, std::string& predict_intent_name,
                                               std::vector<IntentResult>& low_score_intents,
                                               float& max_similarity_score) {
    // 识别用户输入的意图
    std::vector<float> user_input_vector;
    if (stringEmbeddingInterface_ != nullptr) {
        DialogueCallback::TextEmbeddingResult result = stringEmbeddingInterface_->textVectorization(user_input);
        user_input_vector = result.embedding;
        if (user_input_vector.empty()) {
            LOGW("用户输入向量化失败，返回空向量");
            return;
        }
    } else {
        LOGW("警告: TextEmbeddingInterface 未设置，返回空向量");
        user_input_vector = {};
    }

    // 尝试匹配实体并融合原始输入的向量
    std::vector<float> processed_vector = getVectorWithEntityFusion(user_input, user_input_vector);

    // 使用向量数据库识别意图
    std::vector<IntentResult> all_intents = vectorDatabase_.search(processed_vector);
    std::vector<std::string> temporary_removed_intents = {"自动调节显示器亮度", "显示电池百分比"};
    all_intents.erase(std::remove_if(all_intents.begin(), all_intents.end(),
                                     [&temporary_removed_intents](const IntentResult& r) {
                                         return std::find(temporary_removed_intents.begin(),
                                                          temporary_removed_intents.end(),
                                                          r.intent_name) != temporary_removed_intents.end();
                                     }),
                      all_intents.end());

    if (!all_intents.empty()) {
        max_similarity_score = all_intents[0].intent_score;
    } else {
        max_similarity_score = 0.0f;
    }

    for (const auto& intent : all_intents) {
        LOGD("意图: {} 得分: {} 召回文本: {}", intent.intent_name, intent.intent_score, intent.recall_text);
        if (intent.intent_score < lowScoreThreshold_) {
            continue;
        }
        low_score_intents.push_back(intent);
    }

    // 对 low_score_intents 去重，保留每个 intent_name 得分最高的结果
    std::map<std::string, IntentResult> low_score_dedup;
    for (const auto& intent : low_score_intents) {
        auto it = low_score_dedup.find(intent.intent_name);
        if (it == low_score_dedup.end() || intent.intent_score > it->second.intent_score) {
            low_score_dedup[intent.intent_name] = intent;
        }
    }
    low_score_intents.clear();
    for (const auto& pair : low_score_dedup) {
        low_score_intents.push_back(pair.second);
    }

    LOGD("识别到 {} 个结果", low_score_intents.size());
    if (low_score_intents.empty()) {
        predict_intent_name = "";
        return;
    }
    for (const auto& intent : low_score_intents) {
        LOGD("意图: {} 得分: {} 召回文本: {} 意图类型: {}", intent.intent_name, intent.intent_score, intent.recall_text,
             intent.intent_type);
    }

    // 过滤掉得分较低的意图，提取最终意图
    predict_intent_name = extractFinalIntent(low_score_intents, user_input);
}

std::string RuleBasedIntentRetriever::extractFinalIntent(const std::vector<IntentResult>& predict_intents,
                                                         const std::string& input_text) {
    if (predict_intents.empty()) {
        return "";
    }

    std::vector<IntentResult> sorted_intents = predict_intents;
    std::sort(sorted_intents.begin(), sorted_intents.end(),
              [](const IntentResult& a, const IntentResult& b) { return a.intent_score > b.intent_score; });
    IntentResult& top_intent = sorted_intents[0];
    std::string predict_intent_name = top_intent.intent_name;
    double predict_intent_score = top_intent.intent_score;

    // 如果意图自定义了分数阈值，则使用自定义的分数阈值
    std::map<std::string, IntentDefinition> intent_slot_data_map = intentSlotRules_->getIntentSlotDataMap();
    double custom_score_threshold = intent_slot_data_map[predict_intent_name].score_threshold;
    if (custom_score_threshold > 0.0) {
        if (predict_intent_score >= custom_score_threshold) {
            return predict_intent_name;
        }
    }

    // 如果置信度得分高于阈值，直接返回该标签
    if (predict_intent_score >= highScoreThreshold_) {
        return predict_intent_name;
    }

    // 验证用户输入是否满足最高得分意图的实体规则
    std::vector<IntentResult> mid_score_intents;
    for (const auto& intent : sorted_intents) {
        if (intent.intent_score > midScoreThreshold_) {
            mid_score_intents.push_back(intent);
        }
    }
    if (mid_score_intents.empty()) {
        return "";
    }
    return validateIntentEntityRules(predict_intent_name, input_text) ? predict_intent_name : "";
}

std::vector<float> RuleBasedIntentRetriever::normalizeVector(const std::vector<float>& vector) {
    // 计算向量的L2范数
    float norm = 0.0f;
    for (float val : vector) {
        norm += val * val;
    }
    norm = std::sqrt(norm);

    // 如果范数不为0则归一化，否则返回原向量
    if (norm > 0.0f) {
        std::vector<float> normalized(vector.size());
        for (size_t i = 0; i < vector.size(); ++i) {
            normalized[i] = vector[i] / norm;
        }
        return normalized;
    } else {
        return vector;
    }
}

bool RuleBasedIntentRetriever::anyEntityInText(const std::vector<std::string>& entity_list, const std::string& text) {
    for (const auto& entity : entity_list) {
        if (text.find(entity) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool RuleBasedIntentRetriever::checkAllEntitiesPresent(
    const std::map<std::string, std::vector<std::string>>& include_entities, const std::string& text) {
    bool has_top = include_entities.find("top_entities") != include_entities.end();
    bool has_mid = include_entities.find("mid_entities") != include_entities.end();
    bool has_base = include_entities.find("base_entities") != include_entities.end();

    std::vector<std::string> top_entities = has_top ? include_entities.at("top_entities") : std::vector<std::string>();
    std::vector<std::string> mid_entities = has_mid ? include_entities.at("mid_entities") : std::vector<std::string>();
    std::vector<std::string> base_entities =
        has_base ? include_entities.at("base_entities") : std::vector<std::string>();

    // 根据字段组合判断是否所有关键词都包含在query中
    if (has_top && has_mid && has_base) {
        // 同时存在三个字段：需全部字段至少有一个实体被包含
        return (anyEntityInText(top_entities, text) && anyEntityInText(mid_entities, text) &&
                anyEntityInText(base_entities, text));
    } else if (has_mid && has_base && !has_top) {
        // 存在mid和base字段：需两者至少有一个实体被包含
        return (anyEntityInText(mid_entities, text) && anyEntityInText(base_entities, text));
    } else if (has_base && !has_top && !has_mid) {
        // 仅存在base字段：需其至少有一个实体被包含
        return anyEntityInText(base_entities, text);
    }

    return false;
}

std::vector<std::pair<std::string, std::vector<std::string>>> RuleBasedIntentRetriever::getMatchedEntitiesFromText(
    const std::string& user_input) {
    // 遍历实体字典，根据字段组合规则返回 user_input 中匹配的意图名称及对应实体
    const auto& include_entities_map = intentSlotRules_->getIncludeEntitiesMap();
    const auto& exclude_entities_map = intentSlotRules_->getExcludeEntitiesMap();
    std::vector<std::pair<std::string, std::vector<std::string>>> matched_intent_entities;

    // 遍历实体字典
    for (const auto& [intent_name, entity_map] : include_entities_map) {
        // 使用公共函数检查实体字段组合是否匹配（大小写敏感，保持原有行为）
        if (false == checkAllEntitiesPresent(entity_map, user_input)) {
            continue;
        }

        // 收集所有匹配的实体（使用 set 去重）
        std::set<std::string> matched_entities_set;
        for (const auto& [entity_type, entity_list] : entity_map) {
            if (entity_type == "weight") {
                continue;
            }
            for (const auto& entity : entity_list) {
                if (user_input.find(entity) != std::string::npos) {
                    matched_entities_set.insert(entity);
                }
            }
        }

        // 转换为 vector
        std::vector<std::string> matched_entities(matched_entities_set.begin(), matched_entities_set.end());

        // 检查当前intent是否有排除实体配置
        auto exclude_it = exclude_entities_map.find(intent_name);
        if (exclude_it != exclude_entities_map.end()) {
            const auto& forbidden_entities = exclude_it->second;
            bool has_forbidden = false;
            for (const auto& ent : matched_entities) {
                if (std::find(forbidden_entities.begin(), forbidden_entities.end(), ent) != forbidden_entities.end()) {
                    has_forbidden = true;
                    break;
                }
            }
            if (has_forbidden) {
                continue;
            }
        }

        // 满足所有条件，加入结果
        matched_intent_entities.emplace_back(intent_name, matched_entities);
    }

    return matched_intent_entities;
}

std::map<std::string, double> RuleBasedIntentRetriever::addWeightToEntities(
    const std::vector<std::pair<std::string, std::vector<std::string>>>& matched_intent_entities) {
    std::map<std::string, double> entity_and_weight_list;
    double default_weight = 1.0;

    for (const auto& intent_entities : matched_intent_entities) {
        auto intent_name = intent_entities.first;
        auto matched_entities = intent_entities.second;

        const auto& include_entities_map = intentSlotRules_->getIncludeEntitiesMap();
        auto all_entities_it = include_entities_map.find(intent_name);
        if (all_entities_it == include_entities_map.end()) {
            continue;
        }
        auto all_entities = all_entities_it->second;

        for (const auto& entity : matched_entities) {
            auto base_entities_it = all_entities.find("base_entities");
            double weight = default_weight;

            if (base_entities_it != all_entities.end()) {
                auto base_entities = base_entities_it->second;
                if (std::find(base_entities.begin(), base_entities.end(), entity) != base_entities.end()) {
                    // 在实际实现中，这里会获取权重值
                    weight = default_weight;
                }
            }

            if (entity_and_weight_list.count(entity) == 0) {
                entity_and_weight_list[entity] = weight;
            }
        }
    }

    return entity_and_weight_list;
}

std::vector<float> RuleBasedIntentRetriever::getVectorWithEntityFusion(const std::string& user_input,
                                                                       const std::vector<float>& user_input_vector) {
    auto matched_intent_entities = getMatchedEntitiesFromText(user_input);
    if (matched_intent_entities.empty()) {
        LOGD("未匹配到实体，使用原始输入向量");
        return user_input_vector;
    }

    LOGD("匹配到的意图和实体数量: {}", matched_intent_entities.size());
    auto entity_and_weight_list = addWeightToEntities(matched_intent_entities);

    if (!stringEmbeddingInterface_) {
        LOGW("警告: TextEmbeddingInterface 未设置，向量化失败");
        return user_input_vector;
    }

    std::vector<std::vector<float>> entity_vector_list;
    std::vector<double> entity_weight_list;

    for (const auto& [entity, weight] : entity_and_weight_list) {
        auto result = stringEmbeddingInterface_->textVectorization(entity);
        if (result.embedding.empty()) {
            LOGW("实体「{}」向量化失败", entity);
            continue;
        }
        entity_vector_list.push_back(std::move(result.embedding));
        entity_weight_list.push_back(weight);
    }

    if (entity_vector_list.empty()) {
        LOGW("意图识别匹配的实体向量化失败，使用原始输入向量");
        return user_input_vector;
    }

    LOGD("使用优化后的输入向量");
    return fuseVectors(user_input_vector, entity_vector_list, entity_weight_list);
}

std::vector<float> RuleBasedIntentRetriever::fuseVectors(const std::vector<float>& user_input_vector,
                                                         const std::vector<std::vector<float>>& entity_vector_list,
                                                         const std::vector<double>& entity_weight_list) {
    // 计算实体权重的总和
    double sum_entity_weights = 0.0;
    for (double weight : entity_weight_list) {
        sum_entity_weights += weight;
    }

    // 计算实体向量的加权平均
    std::vector<float> entity_vector_weighted_avg(user_input_vector.size(), 0.0f);
    for (size_t i = 0; i < entity_vector_list.size(); ++i) {
        const auto& entity_vector = entity_vector_list[i];
        double weight = entity_weight_list[i];
        for (size_t j = 0; j < entity_vector.size(); ++j) {
            entity_vector_weighted_avg[j] += static_cast<float>(entity_vector[j] * weight);
        }
    }

    // 加权求和并归一化（加权平均）
    if (sum_entity_weights > 0.0) {
        for (auto& val : entity_vector_weighted_avg) {
            val /= static_cast<float>(sum_entity_weights);
        }
    }

    // 加权融合：query向量 + 实体加权平均向量（保留整体权重比例）
    std::vector<float> fused_vector(user_input_vector.size());
    for (size_t i = 0; i < user_input_vector.size(); ++i) {
        fused_vector[i] = queryWeight_ * user_input_vector[i] + entityTotalWeight_ * entity_vector_weighted_avg[i];
    }

    // 归一化融合向量（消除长度对相似度计算的影响）
    return normalizeVector(fused_vector);
}

bool RuleBasedIntentRetriever::validateIntentEntityRules(const std::string& predict_intent_name,
                                                         const std::string& input_text) {
    const auto& include_entities_map = intentSlotRules_->getIncludeEntitiesMap();
    const auto& exclude_entities_map = intentSlotRules_->getExcludeEntitiesMap();

    // 检查包含实体规则
    auto include_intent_it = include_entities_map.find(predict_intent_name);
    if (include_intent_it != include_entities_map.end()) {
        const auto& include_entities = include_intent_it->second;
        if (!checkAllEntitiesPresent(include_entities, input_text)) {
            return false;
        }
    }

    // 统一检查排除实体规则
    auto exclude_intent_it = exclude_entities_map.find(predict_intent_name);
    if (exclude_intent_it != exclude_entities_map.end()) {
        const std::vector<std::string>& exclude_entities = exclude_intent_it->second;
        if (anyEntityInText(exclude_entities, input_text)) {
            return false;
        }
    }

    return true;
}
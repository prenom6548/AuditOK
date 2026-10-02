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

#include "intentslotrecognizer.h"

#include <algorithm>

#include "assistant/intentdialoguemanager/textembedding/textembeddinginterface.h"
#include "utils/logger.h"

using namespace intent;

static std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

IntentSlotRecognizer::IntentSlotRecognizer()
    : intentSlotRules_(std::make_shared<IntentSlotRules>()),
      ruleBasedIntentRetriever_(intentSlotRules_),
      slotExtractor_(intentSlotRules_) {}

void IntentSlotRecognizer::setTextEmbeddingInterface(TextEmbeddingInterface* interface) {
    ruleBasedIntentRetriever_.setTextEmbeddingInterface(interface);
}

IntentRecognitionResult IntentSlotRecognizer::recognizeIntentAndSlots(const std::string& user_input) {
    // 获取意图识别结果
    IntentRecognitionResult intentResult{false, "", "", Json::arrayValue, Json::nullValue};
    Json::Value intentResultJson;
    intentResultJson["intent_category"] = "";
    intentResultJson["success"] = false;
    intentResultJson["inquiry"] = "";
    intentResultJson["slots"] = Json::Value(Json::arrayValue);
    intentResultJson["candidate_intents"] = Json::nullValue;

    std::string predict_intent_name;
    std::vector<IntentResult> low_score_intents;
    std::string user_input_lower = toLowerCase(user_input);
    float max_similarity_score = 0.0f;
    ruleBasedIntentRetriever_.recognizeIntent(user_input_lower, predict_intent_name, low_score_intents,
                                              max_similarity_score);

    if (low_score_intents.empty()) {
        if (max_similarity_score > 0.7f) {
            intentResult.success = false;
            intentResult.intent_category = "未知";
        } else {
            intentResult.success = true;
            intentResult.intent_category = "闲聊";
        }
        return intentResult;
    } else if (!predict_intent_name.empty()) {
        LOGD("识别结果：{}", predict_intent_name);

        // 检查是否为无槽位意图
        const auto& no_slots_intents = intentSlotRules_->getIntentListWithoutSlots();
        if (std::find(no_slots_intents.begin(), no_slots_intents.end(), predict_intent_name) !=
            no_slots_intents.end()) {
            LOGD("无槽位意图");
            intentResult.success = true;
            intentResult.intent_category = predict_intent_name;
            return intentResult;
        }

        // 获取槽位提取结果
        auto slots = slotExtractor_.extractSlots(user_input_lower, predict_intent_name);
        for (const auto& slot : slots) {
            if (slot.isRequired && slot.slotValue.empty()) {
                intentResult.success = false;
                LOGD("槽位: {} 是必填槽, 但是提取结果为空", slot.slotName);
                continue;
            }
            intentResult.success = true;
        }
        Json::Value candidate_intents =
            intentResult.success ? Json::nullValue : buildCandidateIntentSlots(low_score_intents);
        LOGD("槽位结果已提取");
        intentResult.intent_category = predict_intent_name;
        for (const auto& slot : slots) {
            Json::Value slot_json(Json::objectValue);
            slot_json["name"] = slot.slotName;
            slot_json["value"] = slot.slotValue;
            intentResult.slots.append(slot_json);
        }
        intentResult.candidate_intents = candidate_intents;
        return intentResult;
    } else {
        Json::Value candidate_intents = buildCandidateIntentSlots(low_score_intents);
        intentResult.candidate_intents = candidate_intents;
        return intentResult;
    }
}

Json::Value IntentSlotRecognizer::buildCandidateIntentSlots(const std::vector<IntentResult>& predict_intents) {
    LOGD("使用LLM进行意图识别和槽位提取");
    // 候选意图过滤
    std::vector<std::string> candidate_intents;
    for (const auto& intent_info : predict_intents) {
        candidate_intents.push_back(intent_info.intent_name);
    }

    LOGD("候选意图数量: {}", candidate_intents.size());

    // 获取候选意图的槽位信息
    Json::Value intent_slots(Json::arrayValue);

    const auto& intent_slot_data_map = intentSlotRules_->getIntentSlotDataMap();
    for (const auto& intent_name : candidate_intents) {
        auto intent_data_it = intent_slot_data_map.find(intent_name);
        if (intent_data_it != intent_slot_data_map.end()) {
            IntentDefinition intentDefinition = intent_data_it->second;
            if (!intentDefinition.slots.empty()) {
                Json::Value intent_json;
                intent_json["name"] = intent_name;
                intent_json["description"] = intentDefinition.description;
                std::vector<SlotDefinition> slots = intentDefinition.slots;
                Json::Value slots_json(Json::arrayValue);
                for (auto& slot : slots) {
                    Json::Value slot_json;
                    slot_json["name"] = slot.name;
                    slot_json["description"] = slot.description;
                    Json::Value values_array(Json::arrayValue);
                    for (const auto& val : slot.values) {
                        values_array.append(val);
                    }
                    slot_json["values"] = values_array;
                    slots_json.append(slot_json);
                }
                intent_json["slots"] = slots_json;

                // 在实际实现中，这里会添加槽位信息
                intent_slots.append(intent_json);
            }
        }
    }

    return intent_slots;
}

Json::Value IntentSlotRecognizer::getSupplementedCandidateIntents(const Json::Value& baseIntents) const {
    Json::Value result = baseIntents;

    if (!intentSlotRules_) {
        LOGW("IntentSlotRules 未初始化，无法追加补充意图。");
        return result;
    }

    const auto& dataMap = intentSlotRules_->getIntentSlotDataMap();
    // 2. 遍历补充列表并查找定义
    for (const auto& intent_name : candidateIntentSupplementaryList) {
        auto it = dataMap.find(intent_name);
        if (it == dataMap.end()) {
            LOGD("未找到补充意图的定义: {}", intent_name);
            continue;
        }

        const IntentDefinition& def = it->second;

        // 构建意图 JSON 对象
        Json::Value intentObj;
        intentObj["name"] = intent_name;
        intentObj["description"] = def.description;

        Json::Value slotsArray(Json::arrayValue);
        for (const auto& slot : def.slots) {
            Json::Value slotObj;
            slotObj["name"] = slot.name;
            slotObj["description"] = slot.description;

            Json::Value valuesArray(Json::arrayValue);
            for (const auto& val : slot.values) {
                valuesArray.append(val);
            }
            slotObj["values"] = valuesArray;
            slotsArray.append(slotObj);
        }
        intentObj["slots"] = slotsArray;  // 为空也设为空数组，保持格式统一

        // 追加到结果
        result.append(intentObj);
    }

    LOGI("补充完成，最终候选意图数量: {}", result.size());
    return result;
}
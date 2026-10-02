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

#include "intentslotrules.h"

#include <jsoncpp/json/json.h>

#include <fstream>

#include "assistant/intentdialoguemanager/dialoguemessage.h"
#include "utils/logger.h"

IntentSlotRules::IntentSlotRules() { loadIntentSlotData(DialogueConfig::intentSlotDataFile); }

void IntentSlotRules::loadIntentSlotData(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        LOGE("无法打开意图槽位数据文件: {}", file_path);
        return;
    }

    Json::Value data;
    Json::CharReaderBuilder readerBuilder;
    std::string errors;
    if (!Json::parseFromStream(readerBuilder, file, &data, &errors)) {
        LOGE("JSON解析错误: {}", errors);
        return;
    }

    // 解析意图列表和数据
    for (const auto& intent_name : data.getMemberNames()) {
        Json::Value intent_data = data[intent_name];
        intentList_.push_back(intent_name);
        std::string primary_intent = intent_data.get("primary_intent", "").asString();
        if (!primary_intent.empty()) {
            primaryIntentList_.insert(primary_intent);
        }

        IntentDefinition intent_def;
        intent_def.description = intent_data.get("description", "").asString();
        intent_def.primary_intent_name = primary_intent;
        intent_def.score_threshold = intent_data.get("score_threshold", -1.0).asDouble();

        // 解析实体
        if (intent_data.isMember("entities") && intent_data["entities"].isObject()) {
            Json::Value entities = intent_data["entities"];
            std::map<std::string, std::vector<std::string>> include_entity_map;
            std::vector<std::string> exclude_entity_array;
            for (const auto& entity_type : entities.getMemberNames()) {
                Json::Value entity_list = entities[entity_type];
                std::vector<std::string> entity_values;
                if (entity_list.isArray()) {
                    for (const auto& entity : entity_list) {
                        entity_values.push_back(entity.asString());

                        if (entity_type == "exclude_entities") {
                            exclude_entity_array.push_back(entity.asString());
                        } else {
                            include_entity_map[entity_type].push_back(entity.asString());
                        }
                    }
                }
                intent_def.entities[entity_type] = entity_values;
            }

            if (!exclude_entity_array.empty()) {
                excludeEntitiesMap_[intent_name] = exclude_entity_array;
            }
            if (!include_entity_map.empty()) {
                includeEntitiesMap_[intent_name] = include_entity_map;
            }
        }

        // 解析槽位
        if (intent_data.isMember("slots")) {
            if (intent_data["slots"].isArray()) {
                for (const auto& slot : intent_data["slots"]) {
                    SlotDefinition slot_def;
                    slot_def.name = slot.get("name", "").asString();
                    slot_def.description = slot.get("description", "").asString();
                    slot_def.required = slot.get("required", false).asBool();
                    if (slot.isMember("value") && slot["value"].isArray()) {
                        for (const auto& val : slot["value"]) {
                            slot_def.values.push_back(val.asString());
                        }
                    }
                    intent_def.slots.push_back(slot_def);
                }
            } else {
                LOGE("意图槽位数据格式错误: {}", intent_name);
            }
        } else {
            withoutSlotIntents_.emplace_back(intent_name);
        }

        intentSlotDataMap_[intent_name] = intent_def;
    }

    LOGI("意图槽位规则加载完成，共加载 {} 个意图，其中无槽位的意图有 {} 个，一级意图标签有 {} 个", intentList_.size(),
         withoutSlotIntents_.size(), primaryIntentList_.size());
}
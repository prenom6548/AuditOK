/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "tagTask.h"

#include <kyai-business-framework/conf.h>
#include <kyai-business-framework/log.h>

#include <nlohmann/json.hpp>

#include "tagService.h"

namespace DataManagement {
TagTask::TagTask() : NlpBaseTask("tag-task", TASK_TYPE_TAG) {}

ChatModelConfig* TagTask::getChatModelConfig() {
    ChatModelConfig* config = chat_model_config_create();

    chat_model_config_set_name(config, Configuration::getString("extract_tag_nlp_model_name").c_str());
    if (Configuration::getString("extract_tag_nlp_model_deploy_type") == "PrivateCloud") {
        PRINT_DEBUG("[%s] nlp model deploy type is PrivateCloud.\n", taskName.c_str());
        chat_model_config_set_deploy_type(config, ModelDeployType::PrivateCloud);
    } else {
        PRINT_DEBUG("[%s] nlp model deploy type is OnDevice.\n", taskName.c_str());
        chat_model_config_set_deploy_type(config, ModelDeployType::OnDevice);
    }

    return config;
}

void TagTask::setPrompt() { genai_text_set_chat_system_prompt_id(session, TAG_EXTRACTION); }

bool TagTask::worker() {
    TagService tagService;
    if (nlpResult.empty()) {
        // 删除所有标签
        tagService.deleteFileTags({taskDO.fileId});
        return true;
    }

    stripThoughtContent();
    std::set<std::string> result = parseResult();
    if (result.empty()) {
        return false;
    }

    tagService.updateFileTags(taskDO.fileId, result);
    return true;
}

std::set<std::string> TagTask::parseResult() {
    PRINT_DEBUG("[%s] chat result parseTags start:%s.\n", taskName.c_str(), nlpResult.c_str());

    std::multimap<double, std::string> map;
    std::set<std::string> result;
    // 解析JSON字符串
    nlohmann::json jsonData = nlohmann::json::parse(nlpResult, nullptr, false);
    if (jsonData.is_discarded()) {
        PRINT_ERROR("[%s] tag chat parse result json format error.\n", taskName.c_str());
        return {};
    }

    if (!jsonData.contains("预置文档标签") || !jsonData["预置文档标签"].is_array() ||
        jsonData["预置文档标签"].empty()) {
        PRINT_ERROR("[%s] Parse tags json is empty.\n", taskName.c_str());
        return {};
    }
    for (const auto& item : jsonData["预置文档标签"]) {
        if (!item.contains("标签") || !item.contains("置信度") || !item["标签"].is_string()) {
            PRINT_WARN("[%s] Parse tags json get table error, %s.\n", taskName.c_str(), jsonData.dump().c_str());
            continue;
        }
        std::string table = item["标签"];
        double sim = 0.0;
        if (item["置信度"].is_string()) {
            std::string confidence = item["置信度"];
            sim = stod(confidence);
        } else {
            sim = item["置信度"].get<double>();
        }
        if (table != "其他") {
            map.insert(std::make_pair(sim, table));
        }
    }

    // 优先存储预制标签 预制标签存储到6个提示词就直接返回，不要自由标签
    for (auto it = map.rbegin(); it != map.rend(); ++it) {
        if (result.size() >= Configuration::getInt("extract_tag_nums", 6)) {
            return result;
        }

        result.insert(it->second);
    }

    if (!jsonData.contains("自由文档标签") || !jsonData["自由文档标签"].is_array() ||
        jsonData["自由文档标签"].empty()) {
        PRINT_ERROR("[%s] Parse free tags json is empty.\n", taskName.c_str());
        return result;
    }

    map.clear();
    for (const auto& item : jsonData["自由文档标签"]) {
        if (!item.contains("标签") || !item.contains("置信度") || !item["标签"].is_string()) {
            PRINT_WARN("[%s] Parse tags json get table error, %s.\n", taskName.c_str(), jsonData.dump().c_str());
            continue;
        }
        std::string table = item["标签"];
        double sim = 0.0;
        if (item["置信度"].is_string()) {
            std::string confidence = item["置信度"];
            sim = stod(confidence);
        } else {
            sim = item["置信度"].get<double>();
        }

        // 自有标签存在一定的不可靠性，所以置信度设为0.9以上才会存储
        if (sim >= 0.9) {
            map.insert(std::make_pair(sim, table));
        }
    }

    for (auto it = map.rbegin(); it != map.rend(); ++it) {
        if (result.size() >= Configuration::getInt("extract_tag_nums", 6)) {
            return result;
        }

        result.insert(it->second);
    }

    PRINT_DEBUG("[%s] chat result parseTags end.\n", taskName.c_str());
    return result;
}
}  // namespace DataManagement

/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#ifndef GENAI_NLP_GENAINLPMODELCONFIG_H
#define GENAI_NLP_GENAINLPMODELCONFIG_H

#include "modelconfig/modelconfigmanager.h"
#include "utils/util.h"

struct GenNlpModelConfig {
    std::string name{};
    DeployType type{DeployType::Unknown};
    int topK{};
    bool stream{true};
    bool webSearch{true};
    bool enableThinking{false};

    inline bool isDefaultMode() const { return name.empty() && (type == DeployType::Unknown); }

    bool isSameModel(const GenNlpModelConfig& other) const { return name == other.name && type == other.type; }

    inline static DeployType parseDeployType(const std::string& typeStr) {
        if (typeStr == "Local") {
            return DeployType::Local;
        }
        if (typeStr == "Custom") {
            return DeployType::Custom;
        }
        if (typeStr == "PublicCloud") {
            return DeployType::PublicCloud;
        }
        return DeployType::Unknown;  // default
    }

    inline static GenNlpModelConfig parseModelConfig(const std::string& jsonString) {
        // 暂时不写成构造函数
        GenNlpModelConfig config{};
        if (jsonString.empty()) {
            return config;
        }

        Json::Value root = util::stringToJson(jsonString);

        if (root.isNull()) {
            return config;
        }
        if (root.isMember("model_name") && root["model_name"].isString()) {
            std::string targetName = root["model_name"].asString();
            if (ModelConfigManager::getInstance().isDisplayName(targetName)) {
                config.name = ModelConfigManager::getInstance().getModelNameByDisplayName(targetName);
            } else {
                config.name = root["model_name"].asString();
            }
        }
        if (root.isMember("deploy_type") && root["deploy_type"].isString()) {
            std::string typeStr = root["deploy_type"].asString();
            config.type = parseDeployType(typeStr);
        }
        if (root.isMember("top_k") && root["top_k"].isInt()) {
            config.topK = root["top_k"].asInt();
        }
        if (root.isMember("stream") && root["stream"].isBool()) {
            config.stream = root["stream"].asBool();
        }
        if (root.isMember("web_search") && root["web_search"].isBool()) {
            config.webSearch = root["web_search"].asBool();
        }
        if (root.isMember("enable_thinking") && root["enable_thinking"].isBool()) {
            config.enableThinking = root["enable_thinking"].asBool();
        }
        return config;
    }
};

#endif  // GENAI_NLP_GENAINLPMODELCONFIG_H

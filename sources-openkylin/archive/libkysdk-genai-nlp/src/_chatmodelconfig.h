/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <jsoncpp/json/json.h>

#include <optional>
#include <string>

#include "kylin-ai/common/enums.h"

struct _ChatModelConfig {
    std::string name_{};
    ModelDeployType type_{ModelDeployType(-1)};
    int topK_{};
    bool stream_{true};
    bool webSearch_{true};
    bool enableThinking_{false};
    std::optional<double> temperature_{};
    std::optional<double> topP_{};
    std::optional<int> maxCompletionTokens_{};
    std::optional<double> presencePenalty_{};
    std::optional<int> n_{};
};

Json::Value modelConfigToJson(const _ChatModelConfig &modelConfig);

std::string getModelConfigType(const _ChatModelConfig &modelConfig);

#endif  // MODEL_CONFIG_H

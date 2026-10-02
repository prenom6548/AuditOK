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

#include "_chatmodelconfig.h"

#include <common/error.h>

#include <memory>

#include "config.h"
#include "jsoncpp/json/json.h"

namespace {
void setErrorCode(int *pErrorCode, int errorCode) {
    if (pErrorCode != nullptr) {
        *pErrorCode = errorCode;
    }
}
}  // namespace

ChatModelConfig *chat_model_config_create() { return new _ChatModelConfig(); }

void chat_model_config_destroy(ChatModelConfig **config) {
    if (config == nullptr) {
        return;
    }
    delete *config;
    *config = nullptr;
}

void chat_model_config_set_name(ChatModelConfig *config, const char *name) {
    if (config == nullptr) {
        return;
    }
    config->name_ = name;
}

void chat_model_config_set_top_k(ChatModelConfig *config, int top_k) {
    if (config == nullptr) {
        return;
    }
    config->topK_ = top_k;
}

void chat_model_config_set_deploy_type(ChatModelConfig *config, ModelDeployType type) {
    if (config == nullptr) {
        return;
    }
    config->type_ = type;
}

void chat_model_config_set_stream(ChatModelConfig *config, bool stream) {
    if (config == nullptr) {
        return;
    }
    config->stream_ = stream;
}

void chat_model_config_set_web_search_enabled(ChatModelConfig *config, bool enabled, int *error_code) {
    if (config == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    config->webSearch_ = enabled;
}

void chat_model_config_set_thinking_enabled(ChatModelConfig *config, bool enabled, int *error_code) {
    if (config == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    config->enableThinking_ = enabled;
}

int chat_model_config_set_temperature(ChatModelConfig *config, double temperature) {
    if (config == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }

    if (temperature >= 0 && temperature <= 2.0) {
        config->temperature_ = temperature;
    } else {
        config->temperature_.reset();
    }
    return AI_COMMON_SUCCESS;
}

int chat_model_config_set_top_p(ChatModelConfig *config, double top_p) {
    if (config == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }

    if (top_p >= 0 && top_p <= 1.0) {
        config->topP_ = top_p;
    } else {
        config->topP_.reset();
    }
    return AI_COMMON_SUCCESS;
}

int chat_model_config_set_max_completion_tokens(ChatModelConfig *config, int max_tokens) {
    if (config == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }

    if (max_tokens > 0) {
        config->maxCompletionTokens_ = max_tokens;
    } else {
        config->maxCompletionTokens_.reset();
    }
    return AI_COMMON_SUCCESS;
}

int chat_model_config_set_presence_penalty(ChatModelConfig *config, double presence_penalty) {
    if (config == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }

    if (presence_penalty >= -2.0 && presence_penalty <= 2.0) {
        config->presencePenalty_ = presence_penalty;
    } else {
        config->presencePenalty_.reset();
    }
    return AI_COMMON_SUCCESS;
}

int chat_model_config_set_n(ChatModelConfig *config, int n) {
    if (config == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }

    if (n >= 1 && n <= 4) {
        config->n_ = n;
    } else {
        config->n_.reset();
    }
    return AI_COMMON_SUCCESS;
}

Json::Value modelConfigToJson(const _ChatModelConfig &modelConfig) {
    Json::Value root;
    root["model_name"] = modelConfig.name_;
    root["deploy_type"] = getModelConfigType(modelConfig);
    root["top_k"] = modelConfig.topK_;
    root["stream"] = modelConfig.stream_;
    root["web_search"] = modelConfig.webSearch_;
    root["enable_thinking"] = modelConfig.enableThinking_;

    if (modelConfig.temperature_.has_value()) {
        root["temperature"] = modelConfig.temperature_.value();
    }
    if (modelConfig.topP_.has_value()) {
        root["top_p"] = modelConfig.topP_.value();
    }
    if (modelConfig.maxCompletionTokens_.has_value()) {
        root["max_completion_tokens"] = modelConfig.maxCompletionTokens_.value();
    }
    if (modelConfig.presencePenalty_.has_value()) {
        root["presence_penalty"] = modelConfig.presencePenalty_.value();
    }
    if (modelConfig.n_.has_value()) {
        root["n"] = modelConfig.n_.value();
    }
    return root;
}

std::string getModelConfigType(const _ChatModelConfig &modelConfig) {
    switch (modelConfig.type_) {
        case ModelDeployType::OnDevice:
            return "Local";
        case ModelDeployType::Custom:
            return "Custom";
        case ModelDeployType::PublicCloud:
            return "PublicCloud";
        default:
            return "Default";
    }
}

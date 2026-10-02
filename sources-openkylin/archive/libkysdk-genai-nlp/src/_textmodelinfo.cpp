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

#include "_textmodelinfo.h"

#include "_genaitextsession.h"
#include "error.h"

namespace {
void setErrorCode(int* pErrorCode, int errorCode) {
    if (pErrorCode != nullptr) {
        *pErrorCode = errorCode;
    }
}

std::string jsonToString(const Json::Value& value) {
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    return Json::writeString(builder, value);
}
}  // namespace

int genai_text_model_list_get_count(const TextModelList* model_list) {
    if (model_list == nullptr) {
        return -1;
    }
    return model_list->size();
}

const TextModelInfo* genai_text_model_list_get_model_info(const TextModelList* model_list, int index, int* error_code) {
    if (model_list == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    // getModelInfo函数超出范围会返回nullptr
    TextModelInfo* modelInfo = model_list->getModelInfo(index);
    if (modelInfo == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
    } else {
        setErrorCode(error_code, AI_COMMON_SUCCESS);
    }
    return modelInfo;
}

const char* genai_text_model_info_get_model_name(const TextModelInfo* model_info, int* error_code) {
    if (model_info == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_info->modelName.c_str();
}

const char* genai_text_model_info_get_display_name(const TextModelInfo* model_info, int* error_code) {
    if (model_info == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_info->displayName.c_str();
}

ModelDeployType genai_text_model_info_get_deploy_type(const TextModelInfo* model_info, int* error_code) {
    if (model_info == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return ModelDeployType::PublicCloud;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_info->deployType;
}

bool genai_text_model_info_supports_web_search(const TextModelInfo* model_info, int* error_code) {
    if (model_info == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return false;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_info->supportsWebSearch;
}

bool genai_text_model_info_supports_thinking(const TextModelInfo* model_info, int* error_code) {
    if (model_info == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return false;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_info->supportsThinking;
}

bool genai_text_model_info_supports_thinking_toggle(const TextModelInfo* model_info, int* error_code) {
    if (model_info == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return false;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_info->supportsThinkingToggle;
}

const char* genai_text_model_info_get_brand(const TextModelInfo* model_info, int* error_code) {
    if (model_info == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_info->brand.c_str();
}

bool genai_text_model_info_supports_tool_choice(const TextModelInfo* model_info, int* error_code) {
    if (model_info == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return false;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_info->supportsToolChoice;
}

_TextModelList::_TextModelList(const Json::Value& value) {
    if (!value.isMember("modelInfos") || !value["modelInfos"].isArray()) {
        return;
    }
    bool formatError{false};
    for (int i = 0; i < value["modelInfos"].size(); ++i) {
        const Json::Value& item = value["modelInfos"][i];
        if (!item.isMember("modelName") || !item["modelName"].isString()) {
            formatError = true;
            continue;
        }
        if (!item.isMember("displayName") || !item["displayName"].isString()) {
            formatError = true;
            continue;
        }
        if (!item.isMember("deployType") || !item["deployType"].isInt()) {
            formatError = true;
            continue;
        }
        if (modelInfos_.find(i) == modelInfos_.end()) {
            modelInfos_[i] = std::make_shared<_TextModelInfo>();
        }
        modelInfos_[i]->modelName = item["modelName"].asString();
        modelInfos_[i]->displayName = item["displayName"].asString();
        modelInfos_[i]->deployType = ModelDeployType(item["deployType"].asInt());
        if (item.isMember("supportsWebSearch") && item["supportsWebSearch"].isBool()) {
            modelInfos_[i]->supportsWebSearch = item["supportsWebSearch"].asBool();
        }
        if (item.isMember("supportsThinking") && item["supportsThinking"].isBool()) {
            modelInfos_[i]->supportsThinking = item["supportsThinking"].asBool();
        }
        if (item.isMember("supportsThinkingToggle") && item["supportsThinkingToggle"].isBool()) {
            modelInfos_[i]->supportsThinkingToggle = item["supportsThinkingToggle"].asBool();
        }
        if (item.isMember("brand") && item["brand"].isString()) {
            modelInfos_[i]->brand = item["brand"].asString();
        }
        if (item.isMember("supportsToolChoice") && item["supportsToolChoice"].isBool()) {
            modelInfos_[i]->supportsToolChoice = item["supportsToolChoice"].asBool();
        }
    }
    if (formatError) {
        LOGE("TextModelInfo format error: ", jsonToString(value));
    }
}

int _TextModelList::size() const { return int(modelInfos_.size()); }

TextModelInfo* _TextModelList::getModelInfo(int index) const {
    if (modelInfos_.find(index) == modelInfos_.end()) {
        return nullptr;
    }
    return modelInfos_.at(index).get();
}

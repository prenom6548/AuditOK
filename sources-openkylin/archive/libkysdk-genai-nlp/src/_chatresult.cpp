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

#include <jsoncpp/json/json.h>

#include <optional>

#include "error.h"
#include "result.h"

//
#include "_chatresult.h"
#include "logger.h"

namespace {
void setErrorCode(int *pErrorCode, int errorCode) {
    if (pErrorCode != nullptr) {
        *pErrorCode = errorCode;
    }
}

std::optional<Json::Value> stringToJson(const std::string &string) {
    Json::Value value;
    if (Json::Reader reader; reader.parse(string, value)) {
        return value;
    } else {
        return std::nullopt;
    }
}

}  // namespace

const char *chat_result_get_assistant_message(ChatResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->assistantMessage_.c_str();
}

const char *chat_result_get_reasoning_message(ChatResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->reasoningMessage_.c_str();
}

const char *chat_result_get_model_name(ChatResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->modelName_.c_str();
}

const char *chat_result_get_request_id(ChatResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->requestId_.c_str();
}

const char *chat_result_get_finish_reason_message(ChatResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->finishReasonMessage_.c_str();
}

int chat_result_get_error_code(ChatResult *result) {
    if (result == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    return result->errorCode_;
}

const char *chat_result_get_error_message(ChatResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->errorMessage_.c_str();
}

bool chat_result_get_is_end(ChatResult *result) {
    if (result == nullptr) {
        return true;
    }
    return result->isEnd_;
}

const ChatResultSearchInfo *chat_result_get_search_info(const ChatResult *result, int *error_code) {
    if (result == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return &result->searchInfo;
}

size_t chat_result_search_info_get_count(const ChatResultSearchInfo *info, int *error_code) {
    if (info == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return 0;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return info->searchItems.size();
}

const ChatResultSearchItem *chat_result_search_info_get_item(const ChatResultSearchInfo *info, size_t index,
                                                             int *error_code) {
    if (info == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    if (index >= info->searchItems.size()) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return &info->searchItems[index];
}

size_t chat_result_search_item_get_index(const ChatResultSearchItem *item, int *error_code) {
    if (item == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return 0;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return item->index;
}

const char *chat_result_search_item_get_title(const ChatResultSearchItem *item, int *error_code) {
    if (item == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return item->title.c_str();
}

const char *chat_result_search_item_get_url(const ChatResultSearchItem *item, int *error_code) {
    if (item == nullptr) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }
    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return item->url.c_str();
}

_ChatResultSearchItem::_ChatResultSearchItem(const Json::Value &value) {
    if (value.isMember("index") && value["index"].isInt()) {
        index = value["index"].asInt();
    }
    if (value.isMember("title") && value["title"].isString()) {
        title = value["title"].asString();
    }
    if (value.isMember("url") && value["url"].isString()) {
        url = value["url"].asString();
    }
}

_ChatResultSearchInfo::_ChatResultSearchInfo(const std::string &info) {
    auto optional = stringToJson(info);
    if (!optional.has_value()) {
        return;
    }
    Json::Value value = optional.value();
    for (const auto &item : value) {
        _ChatResultSearchItem chatResultSearchItem(item);
        searchItems.push_back(chatResultSearchItem);
    }
}

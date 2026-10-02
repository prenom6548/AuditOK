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

#ifndef CHAT_RESULT_H
#define CHAT_RESULT_H

#include <string>
#include <vector>

#include "error.h"

struct _ChatResultSearchItem {
    _ChatResultSearchItem() = default;
    _ChatResultSearchItem(const Json::Value &value);
    ~_ChatResultSearchItem() = default;
    int index{};
    std::string title;
    std::string url;
};

struct _ChatResultSearchInfo {
    _ChatResultSearchInfo() = default;
    _ChatResultSearchInfo(const std::string &info);
    ~_ChatResultSearchInfo() = default;
    std::vector<_ChatResultSearchItem> searchItems;
};

struct _ChatResult {
    std::string assistantMessage_;
    std::string reasoningMessage_;
    std::string finishReasonMessage_;
    std::string modelName_;
    int errorCode_{AISDK_NO_ERROR};
    std::string errorMessage_;
    bool isEnd_{true};  // 默认暂定false默认
    _ChatResultSearchInfo searchInfo;
    std::string requestId_;
};

#endif  // CHAT_RESULT_H

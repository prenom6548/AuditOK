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

#include "servererror.h"
#include <kylin-ai/common/error.h>
#include "util.h"

namespace {

const std::map<int, int> textToImageErrorCodeMap = {
    {10003, AI_COMMON_VISION_INPUT_TEXT_LENGTH_INVALID},
    {10004, AI_COMMON_AUTHENTICATION_FAILURE},
    {10005, AI_COMMON_VISION_INPUT_TEXT_LENGTH_INVALID},
    {10008, AI_COMMON_TOO_MANY_REQUESTS},
    {10110, AI_COMMON_VISION_IMAGE_GENERATION_BLOCKED},
    {10021, AI_COMMON_VISION_IMAGE_GENERATION_BLOCKED},
    {10022, AI_COMMON_VISION_IMAGE_GENERATION_BLOCKED},
    {11202, AI_COMMON_TOO_MANY_REQUESTS},
    {11201, AI_COMMON_VISION_IMAGE_GENERATION_INSUFFICIENT_LICENSE},
};

}; // namespace

int xunfei_vision_server_error::parseErrorCode(const std::string &result) {
    Json::Value obj = xunfei_vision_util::formatJsonFromString(result);
    if (!obj.isObject() || obj.isNull()) {
        return 0;
    }
    if (obj.isMember("header") && obj["header"].isMember("code") && obj["header"].isMember("message")) {
        return obj["header"]["code"].asInt();
    }
    return 0;
}

int xunfei_vision_server_error::convertErrorCode(int errorCode) {
    if (textToImageErrorCodeMap.find(errorCode) != textToImageErrorCodeMap.end()) {
        return textToImageErrorCodeMap.at(errorCode);
    }
    return AI_COMMON_UNKNOWN_ERROR;
}

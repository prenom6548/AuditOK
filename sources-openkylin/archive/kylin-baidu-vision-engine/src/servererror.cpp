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
#include "util.h"

namespace {

const std::map<int, int> textToImageErrorCodeMap = {
    {4, AI_COMMON_TOO_MANY_REQUESTS},
    {13, AI_COMMON_TOO_MANY_REQUESTS},
    {15, AI_COMMON_TOO_MANY_REQUESTS},
    {17, AI_COMMON_TOO_MANY_REQUESTS},
    {18, AI_COMMON_TOO_MANY_REQUESTS},
    {216100, AI_COMMON_VISION_INPUT_TEXT_LENGTH_INVALID},
    {216201, AI_COMMON_VISION_INPUT_TEXT_LENGTH_INVALID},
    {216303, AI_COMMON_SERVICE_ERROR},
    {216630, AI_COMMON_VISION_INPUT_TEXT_LENGTH_INVALID},
    {282000, AI_COMMON_SERVICE_ERROR},
    {282003, AI_COMMON_VISION_INPUT_TEXT_LENGTH_INVALID},
    {282004, AI_COMMON_VISION_IMAGE_GENERATION_BLOCKED},
};
};

int baidu_vision_server_error::parseErrorCode(const std::string &result) {
    Json::Value obj = baidu_vision_util::formatJsonFromString(result);
    if (!obj.isObject() || obj.isNull()) {
        return 0;
    }
    if (obj.isMember("error_code") && obj.isMember("error_msg")) {
        return obj["error_code"].asInt();
    }
    if (obj.isMember("err_msg") && obj.isMember("err_no")) {
        return obj["err_no"].asInt();
    }
    return 0;
}

int baidu_vision_server_error::convertErrorCode(int errorCode) {
    if (textToImageErrorCodeMap.find(errorCode) != textToImageErrorCodeMap.end()) {
        return textToImageErrorCodeMap.at(errorCode);
    }
    return AI_COMMON_UNKNOWN_ERROR;
}

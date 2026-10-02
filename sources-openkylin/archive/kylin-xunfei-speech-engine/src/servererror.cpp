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

const std::map<int, int> asrErrorCodeMap = {
    {10005, AI_COMMON_AUTHENTICATION_FAILURE},
    {10006, AI_COMMON_SPEECH_SYNTHESIS_TEXT_LENGTH_INVALID},
    {10007, AI_COMMON_SPEECH_PARAM_INVALID},
    {10010, AI_COMMON_AUTHENTICATION_FAILURE},
    {10014, AI_COMMON_SERVICE_ERROR},
    {10019, AI_COMMON_SERVICE_ERROR},
    {10043, AI_COMMON_SPEECH_PARAM_INVALID},
    {10101, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {10114, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {10139, AI_COMMON_SPEECH_PARAM_INVALID},
    {10313, AI_COMMON_AUTHENTICATION_FAILURE},
    {10317, AI_COMMON_SPEECH_PARAM_INVALID},
    {11200, AI_COMMON_AUTHENTICATION_FAILURE},
    {11201, AI_COMMON_TOO_MANY_REQUESTS},
    {10160, AI_COMMON_SPEECH_PARAM_INVALID},
    {10161, AI_COMMON_SPEECH_PARAM_INVALID},
    {10163, AI_COMMON_SPEECH_PARAM_INVALID},
    {10165, AI_COMMON_SPEECH_SYNTHESIS_TEXT_LENGTH_INVALID},
    {10200, AI_COMMON_SERVICE_ERROR},
};

// https://www.xfyun.cn/doc/asr/rtasr/API.html#%E9%94%99%E8%AF%AF%E7%A0%81
const std::map<int, int> realtimeAsrErrorCodeMap = {
    {10105, AI_COMMON_AUTHENTICATION_FAILURE},
    {10106, AI_COMMON_AUTHENTICATION_FAILURE},
    {10107, AI_COMMON_SPEECH_PARAM_INVALID},
    {10110, AI_COMMON_AUTHENTICATION_FAILURE},
    {10700, AI_COMMON_SERVICE_ERROR},
    {10202, AISDK_SERVICE_ERROR},
    {10204, AI_COMMON_SERVICE_ERROR},
    {10205, AI_COMMON_SERVICE_ERROR},
    {16003, AI_COMMON_SERVICE_ERROR},
    {10800, AI_COMMON_TOO_MANY_REQUESTS},
    {37005, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
};

const std::map<int, int> ttsErrorCodeMap = {
    {10005, AI_COMMON_AUTHENTICATION_FAILURE},
    {10006, AI_COMMON_SPEECH_SYNTHESIS_TEXT_LENGTH_INVALID},
    {10007, AI_COMMON_SPEECH_PARAM_INVALID},
    {10010, AI_COMMON_AUTHENTICATION_FAILURE},
    {10109, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {10019, AI_COMMON_SERVICE_ERROR},
    {10101, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {10313, AI_COMMON_AUTHENTICATION_FAILURE},
    {10317, AI_COMMON_SPEECH_PARAM_INVALID},
    {11200, AI_COMMON_AUTHENTICATION_FAILURE},
    {11201, AI_COMMON_TOO_MANY_REQUESTS},
    {10160, AI_COMMON_SPEECH_PARAM_INVALID},
    {10161, AI_COMMON_SPEECH_PARAM_INVALID},
    {10163, AI_COMMON_SPEECH_PARAM_INVALID},
    {10200, AI_COMMON_SERVICE_ERROR},
    {10222, AI_COMMON_NET_ERROR},
};

};  // namespace

int xunfei_speech_server_error::parseErrorCode(const std::string &result) {
    Json::Value obj = xunfei_speech_util::formatJsonFromString(result);
    if (!obj.isObject() || obj.isNull()) {
        return -1;
    }
    if (obj.isMember("code") && obj.isMember("message")) {
        return obj["code"].asInt();
    }
    if (obj.isMember("code") && obj.isMember("action")) {
        return std::stoi(obj["code"].asString());
    }
    return 0;
}

int xunfei_speech_server_error::asrErrorCode2speechResult(int errorCode) {
    if (asrErrorCodeMap.count(errorCode) > 0) {
        return asrErrorCodeMap.at(errorCode);
    }

    return AI_COMMON_UNKNOWN_ERROR;
}

int xunfei_speech_server_error::realtimeAsrErrorCode2speechResult(int errorCode) {
    if (realtimeAsrErrorCodeMap.count(errorCode) > 0) {
        return realtimeAsrErrorCodeMap.at(errorCode);
    }

    return AI_COMMON_UNKNOWN_ERROR;
}

int xunfei_speech_server_error::ttsErrorCode2speechResult(int errorCode) {
    if (ttsErrorCodeMap.count(errorCode) > 0) {
        return ttsErrorCodeMap.at(errorCode);
    }

    return AI_COMMON_UNKNOWN_ERROR;
}

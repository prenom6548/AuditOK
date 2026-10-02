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

#include <kylin-ai/common/error.h>

namespace {

const std::map<int, int> asrErrorCodeMap = {
    {4, AI_COMMON_TOO_MANY_REQUESTS},
    {6, AI_COMMON_AUTHENTICATION_FAILURE},
    {14, AI_COMMON_AUTHENTICATION_FAILURE},
    {17, AI_COMMON_TOO_MANY_REQUESTS},
    {18, AI_COMMON_TOO_MANY_REQUESTS},
    {19, AI_COMMON_TOO_MANY_REQUESTS},
    {100, AI_COMMON_SPEECH_PARAM_INVALID},
    {110, AI_COMMON_AUTHENTICATION_FAILURE},
    {111, AI_COMMON_AUTHENTICATION_FAILURE},
    {3300, AI_COMMON_SPEECH_PARAM_INVALID},
    {3301, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {3302, AI_COMMON_SPEECH_PARAM_INVALID},
    {3303, AI_COMMON_SERVICE_ERROR},
    {3304, AI_COMMON_TOO_MANY_REQUESTS},
    {3305, AI_COMMON_TOO_MANY_REQUESTS},
    {3307, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {3008, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {3009, AI_COMMON_SPEECH_PARAM_INVALID},
    {3010, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {3011, AI_COMMON_SPEECH_PARAM_INVALID},
    {3012, AI_COMMON_SPEECH_PARAM_INVALID},
    {3013, AI_COMMON_SERVICE_ERROR},
    {3014, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {3015, AI_COMMON_SERVICE_ERROR},
    {3016, AI_COMMON_SPEECH_PARAM_INVALID},
};

const std::map<int, int> realtimeAsrErrorCodeMap = {
    {-3001, AI_COMMON_SERVICE_ERROR},
    {-3003, AI_COMMON_SERVICE_ERROR},
    {-3004, AI_COMMON_AUTHENTICATION_FAILURE},
    {-3005, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {-3006, AI_COMMON_SERVICE_ERROR},
    {-3008, AI_COMMON_SPEECH_PARAM_INVALID},
    {-3011, AI_COMMON_SERVICE_ERROR},
    {-3012, AI_COMMON_SERVICE_ERROR},
    {-3013, AI_COMMON_SERVICE_ERROR},
    {-3014, AI_COMMON_SERVICE_ERROR},
    {-3101, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
    {-3102, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID},
};

const std::map<int, int> ttsErrorCodeMap = {
    {500, AI_COMMON_NET_ERROR},
    {501, AI_COMMON_SPEECH_PARAM_INVALID},
    {502, AI_COMMON_AUTHENTICATION_FAILURE},
    {503, AI_COMMON_SPEECH_PARAM_INVALID},
    {513, AI_COMMON_SPEECH_PARAM_INVALID},
    {6, AI_COMMON_AUTHENTICATION_FAILURE},
    {13, AI_COMMON_TOO_MANY_REQUESTS},
    {15, AI_COMMON_TOO_MANY_REQUESTS},
    {16, AI_COMMON_SPEECH_PARAM_INVALID},
    {18, AI_COMMON_SERVICE_ERROR},
    {330, AI_COMMON_SERVICE_ERROR},
    {999, AI_COMMON_SERVICE_ERROR},
};

}; // namespace

int baidu_speech_server_error::parseErrorCode(const std::string &result) {
    Json::Value obj = baidu_speech_util::formatJsonFromString(result);
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

void baidu_speech_server_error::parseErrorInfo(const std::string &result, int &errorCode, int &subErrorCode,
                                               std::string &errorMessage) {
    // 样例如下：
    // {
    //     "convert_offline":false,
    //     "err_detail":"110: Access token invalid or no longer valid",
    //     "err_msg":"110: Access token invalid or no longer valid",
    //     "err_no":502,
    //     "err_subcode":110,
    //     "tts_logid":3278535902
    // }
    Json::Value value = baidu_speech_util::stringToJson(result);
    if (value.isMember("err_no") && value["err_no"].isInt()) {
        errorCode = value["err_no"].asInt();
    }
    if (value.isMember("err_subcode") && value["err_subcode"].isInt()) {
        subErrorCode = value["err_subcode"].asInt();
    }
    if (value.isMember("err_msg") && value["err_no"].isString()) {
        errorMessage = value["err_msg"].asString();
    }
}

int baidu_speech_server_error::asrErrorCode2speechResult(int errorCode) {
    if (asrErrorCodeMap.count(errorCode) > 0) {
        return asrErrorCodeMap.at(errorCode);
    }

    return AI_COMMON_UNKNOWN_ERROR;
}

int baidu_speech_server_error::realtimeAsrErrorCode2speechResult(int errorCode) {
    if (realtimeAsrErrorCodeMap.count(errorCode) > 0) {
        return realtimeAsrErrorCodeMap.at(errorCode);
    }

    return AI_COMMON_UNKNOWN_ERROR;
}

int baidu_speech_server_error::ttsErrorCode2speechResult(int errorCode) {
    if (ttsErrorCodeMap.count(errorCode) > 0) {
        return ttsErrorCodeMap.at(errorCode);
    }

    return AI_COMMON_UNKNOWN_ERROR;
}

const bool baidu_speech_server_error::isErrorRecoverable(int errorCode) {
    if (errorCode == -3003 || errorCode == -3005 || errorCode == -3013) {
        return true;
    }
    return false;
}

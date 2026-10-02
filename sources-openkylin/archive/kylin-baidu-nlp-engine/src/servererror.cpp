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

#include <kylin-ai/ai-engine/aiengineerror.h>
#include <kylin-ai/common/error.h>

#include "servererror.h"
#include "util.h"

namespace {

const std::map<int, ai_engine::NlpEngineErrorCode> nlpChatErrorCodeMap = {
    {1, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {2, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {3, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {4, ai_engine::NlpEngineErrorCode::NoServerService},
    {6, ai_engine::NlpEngineErrorCode::Unauthorized},
    {13, ai_engine::NlpEngineErrorCode::Unauthorized},
    {14, ai_engine::NlpEngineErrorCode::Unauthorized},
    {15, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {17, ai_engine::NlpEngineErrorCode::TooManyRequest},
    {18, ai_engine::NlpEngineErrorCode::TooFrequentRequest},
    {19, ai_engine::NlpEngineErrorCode::TooManyRequest},
    {100, ai_engine::NlpEngineErrorCode::Unauthorized},
    {110, ai_engine::NlpEngineErrorCode::Unauthorized},
    {111, ai_engine::NlpEngineErrorCode::Unauthorized},
    {200, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {336000, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {336001, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {336002, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {336003, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {336004, ai_engine::NlpEngineErrorCode::Unauthorized},
    {336005, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {336006, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {336008, ai_engine::NlpEngineErrorCode::InvalidWords},
    {336100, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {336101, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {336102, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {336103, ai_engine::NlpEngineErrorCode::TooManyData},
    {336104, ai_engine::NlpEngineErrorCode::InvalidWords},
    {336105, ai_engine::NlpEngineErrorCode::InvalidWords},
    {336106, ai_engine::NlpEngineErrorCode::InvalidWords},
    {336501, ai_engine::NlpEngineErrorCode::TooFrequentRequest},
    {336502, ai_engine::NlpEngineErrorCode::TooFrequentRequest},
};

const std::map<int, AiSdkCommonErrorCode> newNlpChatErrorCodeMap{
    {336007, AiSdkCommonErrorCode::AI_COMMON_NLP_TOKEN_LIMITED},
};

} // namespace

int baidu_nlp_server_error::parseErrorCode(const std::string &result) {
    Json::Value obj = baidu_nlp_util::formatJsonFromString(result);
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

int baidu_nlp_server_error::errorCode2nlpResult(int errorCode) {
    if (nlpChatErrorCodeMap.count(errorCode) > 0) {
        return int(nlpChatErrorCodeMap.at(errorCode));
    }
    if (newNlpChatErrorCodeMap.count(errorCode) > 0) {
        return int(newNlpChatErrorCodeMap.at(errorCode));
    }
    return int(ai_engine::NlpEngineErrorCode::UnknownError);
}

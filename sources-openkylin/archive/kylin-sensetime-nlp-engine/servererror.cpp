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

const std::map<int, ai_engine::NlpEngineErrorCode> nlpChatErrorCodeMap = {
    {1, ai_engine::NlpEngineErrorCode::FailedToConnectServer},
    {2, ai_engine::NlpEngineErrorCode::NoServerService},
    {3, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {4, ai_engine::NlpEngineErrorCode::FailedToAnswer},
    {5, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {6, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {7, ai_engine::NlpEngineErrorCode::Unauthorized},
    {8, ai_engine::NlpEngineErrorCode::TooFrequentRequest},
    {9, ai_engine::NlpEngineErrorCode::FailedToAnswer},
    {10, ai_engine::NlpEngineErrorCode::TooManyRequest},
    {11, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {12, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {13, ai_engine::NlpEngineErrorCode::NoServerService},
    {14, ai_engine::NlpEngineErrorCode::NoServerService},
    {15, ai_engine::NlpEngineErrorCode::NoServerService},
    {16, ai_engine::NlpEngineErrorCode::Unauthorized},
    {17, ai_engine::NlpEngineErrorCode::TooManyData},
    {18, ai_engine::NlpEngineErrorCode::InvalidParamType}};

};

int sensetime_nlp_server_error::parseErrorCode(const std::string &result) {
    Json::Value obj = sensetime_nlp_util::formatJsonFromString(result);
    if (!obj.isObject() || obj.isNull()) {
        return 0;
    }
    if (obj.isMember("error") && obj["error"].isMember("code") &&
        obj["error"].isMember("message")) {
        return obj["error"]["code"].asInt();
    }
    return 0;
}

ai_engine::NlpEngineErrorCode
sensetime_nlp_server_error::errorCode2nlpResult(int errorCode) {
    if (nlpChatErrorCodeMap.count(errorCode) > 0) {
        return nlpChatErrorCodeMap.at(errorCode);
    }

    return ai_engine::NlpEngineErrorCode::UnknownError;
}

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
    {10000, ai_engine::NlpEngineErrorCode::FailedToAnswer},
    {10001, ai_engine::NlpEngineErrorCode::NoServerService},
    {10002, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {10003, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {10004, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {10005, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {10006, ai_engine::NlpEngineErrorCode::TooManyRequest},
    {10007, ai_engine::NlpEngineErrorCode::TooFrequentRequest},
    {10008, ai_engine::NlpEngineErrorCode::TooManyRequest},
    {10009, ai_engine::NlpEngineErrorCode::FailedToConnectServer},
    {10010, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {10011, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {10012, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {10013, ai_engine::NlpEngineErrorCode::InputTextBlocked},
    {10014, ai_engine::NlpEngineErrorCode::OutputTextBlocked},
    {10015, ai_engine::NlpEngineErrorCode::Unauthorized},
    {10016, ai_engine::NlpEngineErrorCode::Unauthorized},
    {10017, ai_engine::NlpEngineErrorCode::NoServerService},
    {10019, ai_engine::NlpEngineErrorCode::InputTextBlocked},
    {10021, ai_engine::NlpEngineErrorCode::InputTextBlocked},
    {10110, ai_engine::NlpEngineErrorCode::NoServerService},
    {10163, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {10222, ai_engine::NlpEngineErrorCode::ServerNoResponse},
    {10907, ai_engine::NlpEngineErrorCode::TooManyData},
    {11200, ai_engine::NlpEngineErrorCode::Unauthorized},
    {11201, ai_engine::NlpEngineErrorCode::TooManyRequest},
    {11202, ai_engine::NlpEngineErrorCode::TooFrequentRequest},
    {11203, ai_engine::NlpEngineErrorCode::TooFrequentRequest}};

};

int xunfei_nlp_server_error::parseErrorCode(const std::string &result) {
    Json::Value obj = xunfei_nlp_util::stringToJson(result);
    if (!obj.isObject() || obj.isNull()) {
        return 0;
    }
    if (obj.isMember("header") && obj["header"].isMember("code") && obj["header"].isMember("message")) {
        return obj["header"]["code"].asInt();
    }
    return 0;
}

ai_engine::NlpEngineErrorCode xunfei_nlp_server_error::errorCode2nlpResult(int errorCode) {
    if (nlpChatErrorCodeMap.count(errorCode) > 0) {
        return nlpChatErrorCodeMap.at(errorCode);
    }

    return ai_engine::NlpEngineErrorCode::UnknownError;
}

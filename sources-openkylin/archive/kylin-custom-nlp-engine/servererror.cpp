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

#include "servererror.h"

#include "util.h"

namespace {
const std::map<int, ai_engine::NlpEngineErrorCode> nlpChatErrorCodeMap = {
    {302, ai_engine::NlpEngineErrorCode::NetworkDisconnected}, {400, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {401, ai_engine::NlpEngineErrorCode::Unauthorized},        {402, ai_engine::NlpEngineErrorCode::Unauthorized},
    {404, ai_engine::NlpEngineErrorCode::NetworkDisconnected}, {422, ai_engine::NlpEngineErrorCode::InvalidParamType},
    {429, ai_engine::NlpEngineErrorCode::TooFrequentRequest},  {503, ai_engine::NlpEngineErrorCode::ServerNoResponse},
};
};

ai_engine::NlpEngineErrorCode custom_nlp_server_error::errorCode2nlpResult(long errorCode) {
    if (nlpChatErrorCodeMap.count(errorCode) > 0) {
        return nlpChatErrorCodeMap.at(errorCode);
    }

    return ai_engine::NlpEngineErrorCode::UnknownError;
}

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

#ifndef SERVICES_GEN_AI_NLP_ERROR_H
#define SERVICES_GEN_AI_NLP_ERROR_H

#include <kylin-ai/aiengine.h>
#include <kylin-ai/common/error.h>

#include <map>

typedef enum {
    NLP_INPUT_INVALID = 100,
    NLP_PARAM_ERROR,
} GenAiTextErrorCode;

namespace gen_ai::nlp::error {

constexpr int NLP_ERROR_CODE_MIN = 12000;
constexpr int NLP_ERROR_CODE_MAX = 12999;

inline std::map<int, int> NlpErrorCodeMap{
    {-1, AiSdkCommonErrorCode::AISDK_NO_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::NomoreMemory, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::FileNotFound, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::VersionMismatched, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},

    {(int)ai_engine::NlpEngineErrorCode::FilePermissionDenied, AiSdkCommonErrorCode::AISDK_AUTHENTICATION_FAILURE},
    {(int)ai_engine::NlpEngineErrorCode::NetworkDisconnected, AiSdkCommonErrorCode::AISDK_NET_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::NetworkLatencyTooLong, AiSdkCommonErrorCode::AISDK_NET_ERROR},

    {(int)ai_engine::NlpEngineErrorCode::FailedToConnectServer, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::ServerNoResponse, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::NoServerService, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},

    {(int)ai_engine::NlpEngineErrorCode::Unauthorized, AiSdkCommonErrorCode::AISDK_AUTHENTICATION_FAILURE},
    {(int)ai_engine::NlpEngineErrorCode::TooLittleParams, GenAiTextErrorCode::NLP_PARAM_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::TooManyParams, GenAiTextErrorCode::NLP_PARAM_ERROR},

    {(int)ai_engine::NlpEngineErrorCode::InvalidWords, AiSdkCommonErrorCode::AISDK_NET_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::InvalidParamType, GenAiTextErrorCode::NLP_PARAM_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::TooLittleData, GenAiTextErrorCode::NLP_INPUT_INVALID},
    {(int)ai_engine::NlpEngineErrorCode::TooManyData, GenAiTextErrorCode::NLP_INPUT_INVALID},

    {(int)ai_engine::NlpEngineErrorCode::TooFrequentRequest, AiSdkCommonErrorCode::AISDK_TOO_MANY_REQUESTS},
    {(int)ai_engine::NlpEngineErrorCode::TooManyRequest, AiSdkCommonErrorCode::AISDK_TOO_MANY_REQUESTS},
    {(int)ai_engine::NlpEngineErrorCode::SslCertificateError, AiSdkCommonErrorCode::AISDK_SSL_CERTIFICATE_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::FailedToAnswer, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},
    {(int)ai_engine::NlpEngineErrorCode::LocalNlpModelNotLoaded, AiSdkCommonErrorCode::AI_COMMON_MODEL_NOT_FOUND},
    {(int)ai_engine::NlpEngineErrorCode::UnknownError, AiSdkCommonErrorCode::AI_COMMON_UNKNOWN_ERROR},
    {int(ai_engine::NlpEngineErrorCode::FreeAllocatedQuotaExceeded), AI_COMMON_FREE_ALLOCATED_QUOTA_EXCEEDED},
};

static int getNlpErrorCode(int engineErrorCode) {
    if (gen_ai::nlp::error::NlpErrorCodeMap.find(engineErrorCode) != gen_ai::nlp::error::NlpErrorCodeMap.end()) {
        return gen_ai::nlp::error::NlpErrorCodeMap.at(engineErrorCode);
    } else if (engineErrorCode >= NLP_ERROR_CODE_MIN && engineErrorCode <= NLP_ERROR_CODE_MAX) {
        return engineErrorCode;
    } else {
        return AI_COMMON_UNKNOWN_ERROR;
    }
}
}  // namespace gen_ai::nlp::error

#endif  // SERVICES_GEN_AI_NLP_ERROR_H
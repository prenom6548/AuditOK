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

#ifndef VISION_H
#define VISION_H

#include <kylin-ai/aiengine.h>
#include <kylin-ai/common/error.h>

#include <map>

#include "utils/logger.h"

typedef enum {
    OCR_IMAGE_FILE_ERROR = 100,
    OCR_PARAM_INVALID,
} OcrErrorCode;

namespace visionEngineError {

constexpr int COREAI_VISION_ERROR_CODE_MIN = 10000;
constexpr int COREAI_VISION_ERROR_CODE_MAX = 10999;

inline std::map<int, int> OcrErrorCodeMap{
    {-1, AiSdkCommonErrorCode::AISDK_NO_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::NomoreMemory, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::FileNotFound, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::VersionMismatched, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},

    {(int)ai_engine::VisionEngineErrorCode::FilePermissionDenied, AiSdkCommonErrorCode::AISDK_AUTHENTICATION_FAILURE},
    {(int)ai_engine::VisionEngineErrorCode::NetworkDisconnected, AiSdkCommonErrorCode::AISDK_NET_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::NetworkLatencyTooLong, AiSdkCommonErrorCode::AISDK_NET_ERROR},

    {(int)ai_engine::VisionEngineErrorCode::FailedToConnectServer, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::ServerNoResponse, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::NoServerService, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR},

    {(int)ai_engine::VisionEngineErrorCode::Unauthorized, AiSdkCommonErrorCode::AISDK_AUTHENTICATION_FAILURE},
    {(int)ai_engine::VisionEngineErrorCode::TooLittleParams, OcrErrorCode::OCR_PARAM_INVALID},
    {(int)ai_engine::VisionEngineErrorCode::TooManyParams, OcrErrorCode::OCR_PARAM_INVALID},

    {(int)ai_engine::VisionEngineErrorCode::InvalidParamType, OcrErrorCode::OCR_IMAGE_FILE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::TooLittleData, OcrErrorCode::OCR_IMAGE_FILE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::TooManyData, OcrErrorCode::OCR_IMAGE_FILE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::ImageFileReadError, OcrErrorCode::OCR_IMAGE_FILE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::OcrModelError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_FAILED},
    {(int)ai_engine::VisionEngineErrorCode::ModelEarlyLoaded, AiSdkCommonErrorCode::AISDK_MODEL_RUN_FAILED},
    {(int)ai_engine::VisionEngineErrorCode::OcrTimeout, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT},

    {(int)ai_engine::VisionEngineErrorCode::TooFrequentRequest, AiSdkCommonErrorCode::AISDK_TOO_MANY_REQUESTS},
    {(int)ai_engine::VisionEngineErrorCode::TooManyRequest, AiSdkCommonErrorCode::AISDK_TOO_MANY_REQUESTS}

};

inline std::map<int, int> SubjectSegmentationErrorCodeMap{
    {-1, AiSdkCommonErrorCode::AISDK_NO_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::NomoreMemory, AiSdkCommonErrorCode::AI_COMMON_SERVICE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::FileNotFound, AiSdkCommonErrorCode::AI_COMMON_SERVICE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::VersionMismatched, AiSdkCommonErrorCode::AI_COMMON_SERVICE_ERROR},

    {(int)ai_engine::VisionEngineErrorCode::FilePermissionDenied,
     AiSdkCommonErrorCode::AI_COMMON_AUTHENTICATION_FAILURE},
    {(int)ai_engine::VisionEngineErrorCode::NetworkDisconnected, AiSdkCommonErrorCode::AISDK_NET_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::NetworkLatencyTooLong, AiSdkCommonErrorCode::AISDK_NET_ERROR},

    {(int)ai_engine::VisionEngineErrorCode::FailedToConnectServer, AiSdkCommonErrorCode::AI_COMMON_SERVICE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::ServerNoResponse, AiSdkCommonErrorCode::AI_COMMON_SERVICE_ERROR},
    {(int)ai_engine::VisionEngineErrorCode::NoServerService, AiSdkCommonErrorCode::AI_COMMON_SERVICE_ERROR},

    {(int)ai_engine::VisionEngineErrorCode::Unauthorized, AiSdkCommonErrorCode::AI_COMMON_AUTHENTICATION_FAILURE},
    {(int)ai_engine::VisionEngineErrorCode::TooLittleParams, AiSdkCommonErrorCode::AI_COMMON_INVALID_PARAMETER},
    {(int)ai_engine::VisionEngineErrorCode::TooManyParams, AiSdkCommonErrorCode::AI_COMMON_INVALID_PARAMETER},

    {(int)ai_engine::VisionEngineErrorCode::InvalidParamType, AiSdkCommonErrorCode::AI_COMMON_INVALID_PARAMETER},
    {(int)ai_engine::VisionEngineErrorCode::TooLittleData, AiSdkCommonErrorCode::AI_COMMON_IMAGE_DATA_INVALID},
    {(int)ai_engine::VisionEngineErrorCode::TooManyData, AiSdkCommonErrorCode::AI_COMMON_IMAGE_DATA_INVALID},

    {(int)ai_engine::VisionEngineErrorCode::ImageFileReadError, AiSdkCommonErrorCode::AI_COMMON_IMAGE_DATA_INVALID},
    {(int)ai_engine::VisionEngineErrorCode::MattingModelError, AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_FAILED},
    {(int)ai_engine::VisionEngineErrorCode::ModelEarlyLoaded, AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_FAILED},

    {(int)ai_engine::VisionEngineErrorCode::OcrTimeout, AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_TIME_OUT},
    {(int)ai_engine::VisionEngineErrorCode::TooFrequentRequest, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS},
    {(int)ai_engine::VisionEngineErrorCode::TooManyRequest, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS}

};

static int getCoreVisionErrorCode(int engineErrorCode) {
    if (visionEngineError::OcrErrorCodeMap.find(engineErrorCode) != visionEngineError::OcrErrorCodeMap.end()) {
        return visionEngineError::OcrErrorCodeMap.at(engineErrorCode);
    } else {
        //        LOGE("Vision engine error code is not exists!");
        return AiSdkCommonErrorCode::AISDK_NO_ERROR;
    }
}
};  // namespace visionEngineError

#endif
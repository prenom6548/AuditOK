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

#include "error.h"

#include <kylin-ai/ai-engine/aiengineerror.h>
#include <kylin-ai/common/error.h>

#include <map>

#include "assistant/intentdialoguemanager/dialoguemessage.h"
#ifdef SUPPORT_VECTOR_DB
#include "business/knowledge-base/common.h"
#include "knowledgebase/knowledgebaseproxy.h"
#endif

namespace assistant {

std::map<ai_engine::NlpEngineErrorCode, assistant::ErrorCode> nlpEngineErrorMap{
    {ai_engine::NlpEngineErrorCode(-1), ErrorCode::Ok},
    {ai_engine::NlpEngineErrorCode::NomoreMemory, ErrorCode::NlpServerError},
    {ai_engine::NlpEngineErrorCode::NetworkDisconnected, ErrorCode::NetError},
    {ai_engine::NlpEngineErrorCode::NetworkLatencyTooLong, ErrorCode::NetError},
    {ai_engine::NlpEngineErrorCode::FailedToConnectServer, ErrorCode::NetError},
    {ai_engine::NlpEngineErrorCode::ServerNoResponse, ErrorCode::NlpServerError},
    {ai_engine::NlpEngineErrorCode::NoServerService, ErrorCode::NlpServerError},
    {ai_engine::NlpEngineErrorCode::Unauthorized, ErrorCode::UnAuthorized},
    {ai_engine::NlpEngineErrorCode::TooLittleParams, ErrorCode::ParamError},
    {ai_engine::NlpEngineErrorCode::TooManyParams, ErrorCode::ParamError},
    {ai_engine::NlpEngineErrorCode::InvalidWords, ErrorCode::NetError},
    {ai_engine::NlpEngineErrorCode::InvalidParamType, ErrorCode::ParamError},
    {ai_engine::NlpEngineErrorCode::TooLittleData, ErrorCode::ParamError},
    {ai_engine::NlpEngineErrorCode::TooManyData, ErrorCode::ParamError},
    {ai_engine::NlpEngineErrorCode::TooFrequentRequest, ErrorCode::RequestTooMany},
    {ai_engine::NlpEngineErrorCode::TooManyRequest, ErrorCode::RequestTooMany},
    {ai_engine::NlpEngineErrorCode::FailedToAnswer, ErrorCode::NlpServerError},
    {ai_engine::NlpEngineErrorCode::InputTextBlocked, ErrorCode::InputTextBlocked},
    {ai_engine::NlpEngineErrorCode::OutputTextBlocked, ErrorCode::OutputTextBlocked},
    {ai_engine::NlpEngineErrorCode::FreeAllocatedQuotaExceeded, ErrorCode::FreeAllocatedQuotaExceeded},
    {ai_engine::NlpEngineErrorCode::LocalNlpModelNotLoaded, ErrorCode::NlpModelNotFound},
    {ai_engine::NlpEngineErrorCode::SslCertificateError, ErrorCode::SslCertificateError},
};

std::map<int, assistant::ErrorCode> visionEngineErrorMap{
    {-1, ErrorCode::Ok},
    {AI_COMMON_SUCCESS, ErrorCode::Ok},
    {AI_COMMON_NET_ERROR, ErrorCode::NetError},
    {AI_COMMON_SERVICE_ERROR, ErrorCode::NlpServerError},
    {AI_COMMON_AUTHENTICATION_FAILURE, ErrorCode::UnAuthorized},
    {AI_COMMON_VISION_INPUT_TEXT_LENGTH_INVALID, ErrorCode::ParamError},
    {AI_COMMON_TOO_MANY_REQUESTS, ErrorCode::RequestTooMany},
    {AI_COMMON_VISION_IMAGE_GENERATION_FAILED, ErrorCode::GenerateImageFailed},
    {AI_COMMON_VISION_IMAGE_GENERATION_BLOCKED, ErrorCode::GenerateImageBlocked},
    {AI_COMMON_SSL_CERTIFICATE_ERROR, ErrorCode::SslCertificateError},
    {(int)ai_engine::VisionEngineErrorCode::MattingModelError, ErrorCode::ParamError},
    {AI_COMMON_VISION_IMAGE_SIZE_INVALID, ErrorCode::ParamError},
    {AI_COMMON_IMAGE_DATA_INVALID, ErrorCode::ParamError},
    {AI_COMMON_FILE_NOT_FOUND, ErrorCode::ParamError},
    {AI_COMMON_VISION_IMAGE_GENERATION_INSUFFICIENT_LICENSE, ErrorCode::VisionImageGenerationInsufficientLicense},
};

std::map<DialogueCallback::DialogueControlErrorCode, assistant::ErrorCode> DialogueControlErrorMap{
    {DialogueCallback::DialogueControlErrorCode::InvalidInput, ErrorCode::InputTextInvalid},
    {DialogueCallback::DialogueControlErrorCode::SystemOperationFailed, ErrorCode::SystemOperationFailed},
    {DialogueCallback::DialogueControlErrorCode::IntentionRecognitionFailed, ErrorCode::IntentionRecognitionFailed},
};

#ifdef SUPPORT_VECTOR_DB
std::map<knowledgebase::KnowledgebaseErrorCode, assistant::ErrorCode> FileKnowledgeBaseErrorMap{
    {knowledgebase::KnowledgebaseErrorCode::KNOWLEDGEBASE_CHUNK_FAILED, ErrorCode::FileParseFailed},
    {knowledgebase::KnowledgebaseErrorCode::KNOWLEDGEBASE_DOCUMENT_EXCEEDS_LIMIT, ErrorCode::FileExceedSizeLimit},
    {knowledgebase::KnowledgebaseErrorCode::KNOWLEDGEBASE_VECTOR_DATABASE_DATA_UNAVAILABLE,
     ErrorCode::FileDatabaseCorrupted},
};
std::map<advanced::knowledgebase::KBErrorCode, ErrorCode> kbErrorMap{
    {advanced::knowledgebase::SUCCESS, ErrorCode::Ok},

    {advanced::knowledgebase::KBErrorCode::KB_ID_INVALID, ErrorCode::ServerError},
    {advanced::knowledgebase::KBErrorCode::KB_NOT_FOUND, ErrorCode::ServerError},
    {advanced::knowledgebase::KBErrorCode::KB_ACCESS_DENIED, ErrorCode::ServerError},
    {advanced::knowledgebase::KBErrorCode::KB_CORRUPTED, ErrorCode::ServerError},

    {advanced::knowledgebase::KBErrorCode::RETRIEVAL_FAILED, ErrorCode::ServerError},

    {advanced::knowledgebase::KBErrorCode::CHAT_MODEL_NOT_EXIST, ErrorCode::NlpModelNotFound},
    {advanced::knowledgebase::KBErrorCode::CHAT_MODEL_UNAVAILABLE, ErrorCode::NlpServerError},
    {advanced::knowledgebase::KBErrorCode::CHAT_MODEL_NET_ERROR, ErrorCode::NetError},
    {advanced::knowledgebase::KBErrorCode::CHAT_MODEL_AUTHENTICATION_FAILURE, ErrorCode::UnAuthorized},
    {advanced::knowledgebase::KBErrorCode::CHAT_MODEL_RUNTIME_ERROR, ErrorCode::ServerError},
    {advanced::knowledgebase::KBErrorCode::CHAT_MODEL_TOO_MANY_REQUESTS, ErrorCode::RequestTooMany},
    {advanced::knowledgebase::KBErrorCode::CHAT_MODEL_SERVICE_ERROR, ErrorCode::NlpServerError},
    {advanced::knowledgebase::KBErrorCode::CHAT_MODEL_SERVICE_TIMEOUT, ErrorCode::TaskTimeout},
    {advanced::knowledgebase::KBErrorCode::CHAT_MODEL_RUN_TIME_OUT, ErrorCode::TaskTimeout},
    {advanced::knowledgebase::KBErrorCode::CHAT_MODEL_INVALID_SESSION, ErrorCode::NlpServerError},
    {advanced::knowledgebase::KBErrorCode::CHAT_SSL_CERTIFICATE_ERROR, ErrorCode::SslCertificateError},

    {advanced::knowledgebase::KBErrorCode::INVALID_PARAMETER, ErrorCode::ParamError},

    {advanced::knowledgebase::KBErrorCode::UNKNOWN_ERROR, ErrorCode::UnknownError},
};

#endif

ErrorCode Error::getErrorCodeFromNlpEngine(int engineErrorCode) {
    auto code = ai_engine::NlpEngineErrorCode(engineErrorCode);
    if (nlpEngineErrorMap.find(code) != nlpEngineErrorMap.end()) {
        return nlpEngineErrorMap[code];
    }
    return ErrorCode::UnknownError;
}

ErrorCode Error::getErrorCodeFromVisionEngine(int engineErrorCode) {
    if (visionEngineErrorMap.find(engineErrorCode) != visionEngineErrorMap.end()) {
        return visionEngineErrorMap[engineErrorCode];
    }
    return ErrorCode::UnknownError;
}

ErrorCode Error::getErrorCodeFromDialogueControl(int dialogueErrorCode) {
    if (dialogueErrorCode == 0 || dialogueErrorCode == -1) {
        return ErrorCode::Ok;
    }

    if (DialogueControlErrorMap.find((DialogueCallback::DialogueControlErrorCode)dialogueErrorCode) !=
        DialogueControlErrorMap.end()) {
        return static_cast<ErrorCode>(
            DialogueControlErrorMap[(DialogueCallback::DialogueControlErrorCode)dialogueErrorCode]);
    }

    return ErrorCode::UnknownError;
}
#ifdef SUPPORT_VECTOR_DB
ErrorCode Error::getErrorCodeFromFileKnowledgeBase(int engineErrorCode) {
    if (engineErrorCode == 0) {
        return ErrorCode::Ok;
    }

    if (FileKnowledgeBaseErrorMap.find((knowledgebase::KnowledgebaseErrorCode)engineErrorCode) !=
        FileKnowledgeBaseErrorMap.end()) {
        return static_cast<ErrorCode>(
            FileKnowledgeBaseErrorMap[(knowledgebase::KnowledgebaseErrorCode)engineErrorCode]);
    }

    return ErrorCode::UnknownError;
}

ErrorCode Error::getErrorCodeFromAdvancedKnowledgeBase(int errorCode) {
    if (errorCode == 0) {
        return ErrorCode::Ok;
    }
    if (kbErrorMap.find((advanced::knowledgebase::KBErrorCode)errorCode) != kbErrorMap.end()) {
        return kbErrorMap[(advanced::knowledgebase::KBErrorCode)errorCode];
    }
    return ErrorCode::UnknownError;
}
#endif

}  // namespace assistant

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

#ifndef ERROR_H
#define ERROR_H

#include <string>

namespace assistant {

enum class ErrorCode {
    Ok = 0,
    NetError,
    ServerError,
    UnAuthorized,
    RequestTooMany,
    RequestFailed,
    InputTextLengthInvalid,
    TaskTimeout,
    GenerateImageFailed,
    GenerateImageBlocked,
    ParamError,
    InputTextInvalid,
    FileNotFound,
    IntentServerError,
    NlpServerError,
    VisionServerError,
    InputTextBlocked,
    OutputTextBlocked,
    SystemOperationFailed,
    IntentionRecognitionFailed,
    NlpModelNotFound,
    VisionModelNotFound,
    FileExceedSizeLimit,
    FileParseFailed,
    CreateScheduleFailed,
    CreateToDoListFailed,
    IntentNlpModelNotFound,
    FreeAllocatedQuotaExceeded,
    LocalNlpModelNotLoaded,
    SslCertificateError,
    FileDatabaseCorrupted,
    ExtractImageContentsFailed,
    InitNotCompleted,  // 该错误码在助手sdk使用，此处仅做占位，以便兼容
    VisionImageGenerationInsufficientLicense,
    UnknownError = 10000,
};

class Error {
public:
    ErrorCode code = ErrorCode::Ok;
    std::string message{};
    explicit operator bool() const { return code != ErrorCode::Ok; }

    static ErrorCode getErrorCodeFromNlpEngine(int engineErrorCode);
    static ErrorCode getErrorCodeFromVisionEngine(int engineErrorCode);
    static ErrorCode getErrorCodeFromDialogueControl(int dialogueErrorCode);
    static ErrorCode getErrorCodeFromFileKnowledgeBase(int engineErrorCode);
    static ErrorCode getErrorCodeFromAdvancedKnowledgeBase(int errorCode);
};

}  // namespace assistant

#endif
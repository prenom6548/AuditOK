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

#ifndef __KYLIN_AI_PRIVATE_OS_ASSISTANT_ERROR_H__
#define __KYLIN_AI_PRIVATE_OS_ASSISTANT_ERROR_H__

#include <string>

#include "kyaiexport.h"

namespace kyai {
namespace assistant {

enum class ErrorCode {
    Ok = 0,
    NetError,
    ServerError,
    UnAuthorized,
    RequestTooMany,
    RequestFailed,
    InputTextLengthInvaild,
    TaskTimeout,
    GenerateImageFailed,
    GenerateImageBlocked,
    ParamError,
    InputTextInvaild,
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
    UnknownError = 10000,
};

class KYAI_ASSISTANT_EXPORT Error {
public:
    ErrorCode code = ErrorCode::Ok;
    std::string message{};
    explicit operator bool() const { return code != ErrorCode::Ok; }

    static ErrorCode getCodeFromNlpError(int errorCode);
    static ErrorCode getCodeFromVisionError(int errorCode);
};

}  // namespace assistant
}  // namespace kyai

#endif  // __KYLIN_AI_PRIVATE_OS_ASSISTANT_ERROR_H__

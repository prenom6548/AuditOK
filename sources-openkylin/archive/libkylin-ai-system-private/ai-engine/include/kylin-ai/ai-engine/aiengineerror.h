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

#ifndef AIENGINEERROR_H
#define AIENGINEERROR_H

#include <string>

namespace ai_engine {
// 引擎相关错误分类
enum class EngineErrorCategory {
    Invalid = -1,
    Initialization,
    InputError,
    OutputError
};

#define PLUGIN_COMMON_ERROR_CODE                                          \
    /*通用初始化相关错误*/                                                \
    NomoreMemory,                                                         \
                                                                          \
    /*本地模型初始化相关错误*/                                            \
    FileNotFound, VersionMismatched, FilePermissionDenied,                \
                                                                          \
    /*云端模型初始化相关错误*/                                            \
    NetworkDisconnected, NetworkLatencyTooLong, FailedToConnectServer,    \
    ServerNoResponse, NoServerService, SslCertificateError, Unauthorized, \
                                                                          \
    /*输入相关错误*/                                                      \
    TooLittleParams, TooManyParams, InvalidParamType, TooLittleData,      \
    TooManyData, TooFrequentRequest, TooManyRequest, UnknownError = 1000,

// NLP引擎相关错误码
enum class NlpEngineErrorCode {
    PLUGIN_COMMON_ERROR_CODE

    // 输出相关错误
    FailedToAnswer,
    InvalidWords,
    InputTextBlocked,
    OutputTextBlocked,
    FreeAllocatedQuotaExceeded,
    LocalNlpModelNotLoaded,
};

// 语音引擎相关错误码
enum class SpeechEngineErrorCode {
    PLUGIN_COMMON_ERROR_CODE

    // 输出相关错误
    UnsupportedLanguage,
    UnsupportedScript,
    InvalidWakeupWords
};

// 图像引擎相关错误码
enum class VisionEngineErrorCode {
    PLUGIN_COMMON_ERROR_CODE

    // 输出相关错误
    Invalidwords,
    ImageGenerationFailed,
    ImageCensorshipFailed,
    OcrModelError,
    ImageFileReadError,
    ModelEarlyLoaded,
    OcrTimeout,
    MattingModelError,
    DdcpModelError,
    ImageNumberInvalid,
    ImageSizeInvalid
};

class EngineError {
public:
    EngineError() = default;
    EngineError(int code, const std::string& message)
        : code_(code), message_(message) {}

    const std::string& getModule() const { return module_; }
    int getCode() const { return code_; }
    const std::string& getMessage() const { return message_; }

private:
    std::string module_{"AI Engine"};
    int code_{-1};
    std::string message_;
};
}  // namespace ai_engine

#endif

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


#include "osassistant/error.h"

// #include "nlp.h"
// #include "vision.h"
namespace kyai {
namespace assistant {

enum NlpResult {
    NLP_SUCCESS = 0,
    NLP_SESSION_ERROR,     // session 错误
    NLP_NET_ERROR,         // 网络错误
    NLP_SERVER_ERROR,      // 服务错误或者不可用
    NLP_UNAUTHORIZED,      // 权限限制，鉴权失败，无权访问
    NLP_REQUEST_TOO_MANY,  // 请求频率过高
    NLP_REQUEST_FAILED,    // 请求失败
    NLP_INPUT_TEXT_LENGTH_INVAILD,  // 输入文本长度超限
    NLP_PARAM_ERROR,                // 参数错误
};

ErrorCode Error::getCodeFromNlpError(int errorCode) {
    switch (errorCode) {
        case NLP_SUCCESS:
            return ErrorCode::Ok;
        case NLP_SESSION_ERROR:
            return ErrorCode::ServerError;
        case NLP_NET_ERROR:
            return ErrorCode::NetError;
        case NLP_SERVER_ERROR:
            return ErrorCode::ServerError;
        case NLP_UNAUTHORIZED:
            return ErrorCode::UnAuthorized;
        case NLP_REQUEST_TOO_MANY:
            return ErrorCode::RequestTooMany;
        case NLP_REQUEST_FAILED:
            return ErrorCode::RequestFailed;
        case NLP_INPUT_TEXT_LENGTH_INVAILD:
            return ErrorCode::InputTextLengthInvaild;
        case NLP_PARAM_ERROR:
            return ErrorCode::ParamError;
        default:
            return ErrorCode::Ok;
    }
    return ErrorCode::Ok;
}

enum VisionResult {
    VISION_SUCCESS = 0,
    VISION_SESSION_ERROR,
    VISION_NET_ERROR,         // 网络错误
    VISION_SERVER_ERROR,      // 服务错误或者不可用
    VISION_UNAUTHORIZED,      // 权限限制，鉴权失败，无权访问
    VISION_REQUEST_TOO_MANY,  // 请求频率过高
    VISION_REQUEST_FAILED,    // 请求失败
    VISION_TASK_TIMEOUT,      // 任务超时
    VISION_INPUT_TEXT_LENGTH_INVAILD,  // 输入文本长度超限
    VISION_GENERATE_IMAGE_FAILED,      // 生成图片失败
    VISION_GENERATE_IMAGE_BLOCKED,     // 图片审核失败
    VISION_PARAM_ERROR,                // 参数错误
};

ErrorCode Error::getCodeFromVisionError(int errorCode) {
    switch (errorCode) {
        case VISION_SUCCESS:
            return ErrorCode::Ok;
        case VISION_SESSION_ERROR:
            return ErrorCode::ServerError;
        case VISION_NET_ERROR:
            return ErrorCode::NetError;
        case VISION_SERVER_ERROR:
            return ErrorCode::ServerError;
        case VISION_UNAUTHORIZED:
            return ErrorCode::UnAuthorized;
        case VISION_REQUEST_TOO_MANY:
            return ErrorCode::RequestTooMany;
        case VISION_REQUEST_FAILED:
            return ErrorCode::RequestFailed;
        case VISION_TASK_TIMEOUT:
            return ErrorCode::TaskTimeout;
        case VISION_INPUT_TEXT_LENGTH_INVAILD:
            return ErrorCode::InputTextLengthInvaild;
        case VISION_GENERATE_IMAGE_FAILED:
            return ErrorCode::GenerateImageFailed;
        case VISION_GENERATE_IMAGE_BLOCKED:
            return ErrorCode::GenerateImageBlocked;
        default:
            return ErrorCode::Ok;
    }
    return ErrorCode::Ok;
}

}  // namespace assistant
}  // namespace kyai

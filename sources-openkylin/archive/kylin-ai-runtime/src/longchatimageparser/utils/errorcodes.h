/*
 * Copyright (C) 2025 Kylin Software Co., Ltd.
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

#pragma once

namespace utils {

/**
 * @brief OCR处理系统错误码定义
 *
 * 错误码分组规则：
 * - 0: 成功
 * - 1-99: 通用错误
 * - 100-199: 图像切片相关错误
 * - 200-299: X裁剪计算相关错误
 * - 300-399: OCR处理相关错误
 * - 400-499: 去重处理相关错误
 * - 500-599: 平台检测相关错误
 * - 600-699: 内容标记相关错误
 * - 700-799: 聊天分析相关错误
 * - 800-899: 结果导出相关错误
 * - 900-999: 总结创建相关错误
 */
enum class ErrorCode : int {
    // ========== 成功 ==========
    SUCCESS = 0,

    // ========== 通用错误 (1-99) ==========
    UNKNOWN_ERROR = 1,          // 未知错误
    INVALID_PARAMETER = 2,      // 无效参数
    FILE_NOT_FOUND = 3,         // 文件未找到
    INITIALIZATION_FAILED = 6,  // 初始化失败

    // ========== 图像切片错误 (100-199) ==========
    IMAGE_SLICE_FAILED = 100,  // 图像切片失败

    // ========== X裁剪计算错误 (200-299) ==========
    XCROP_CALC_FAILED = 200,  // X裁剪值计算失败

    // ========== OCR处理错误 (300-399) ==========
    OCR_PROCESS_FAILED = 300,      // OCR处理失败
    OCR_ENGINE_INIT_FAILED = 301,  // OCR引擎初始化失败
    OCR_SDK_NOT_AVAILABLE = 302,   // OCR SDK不可用
    OCR_RECOGNITION_FAILED = 303,  // OCR识别失败

    // ========== 去重处理错误 (400-499) ==========
    DEDUP_PROCESS_FAILED = 400,  // 去重处理失败

    // ========== 平台检测错误 (500-599) ==========
    AVATAR_DETECTION_EMPTY = 500,  // 头像检测结果为空

    // ========== 内容标记错误 (600-699) ==========
    CONTENT_MARK_FAILED = 600,  // 内容标记失败

    // ========== 聊天分析错误 (700-799) ==========
    CHAT_ANALYZE_FAILED = 700  // 聊天分析失败
};

/**
 * @brief 获取错误码描述
 * @param code 错误码
 * @return 错误码的中文描述
 */
inline const char* getErrorDescription(ErrorCode code) {
    switch (code) {
        // 成功
        case ErrorCode::SUCCESS:
            return "处理成功";

        // 通用错误
        case ErrorCode::UNKNOWN_ERROR:
            return "未知错误";
        case ErrorCode::INVALID_PARAMETER:
            return "无效参数";
        case ErrorCode::FILE_NOT_FOUND:
            return "文件未找到";
        case ErrorCode::INITIALIZATION_FAILED:
            return "初始化失败";

        // 图像切片错误
        case ErrorCode::IMAGE_SLICE_FAILED:
            return "图像切片失败";

        // X裁剪计算错误
        case ErrorCode::XCROP_CALC_FAILED:
            return "X裁剪值计算失败";

        // OCR处理错误
        case ErrorCode::OCR_PROCESS_FAILED:
            return "OCR处理失败";
        case ErrorCode::OCR_ENGINE_INIT_FAILED:
            return "OCR引擎初始化失败";
        case ErrorCode::OCR_SDK_NOT_AVAILABLE:
            return "OCR SDK不可用";
        case ErrorCode::OCR_RECOGNITION_FAILED:
            return "OCR识别失败";

        // 去重处理错误
        case ErrorCode::DEDUP_PROCESS_FAILED:
            return "去重处理失败";

        // 平台检测错误
        case ErrorCode::AVATAR_DETECTION_EMPTY:
            return "头像检测结果为空";

        // 内容标记错误
        case ErrorCode::CONTENT_MARK_FAILED:
            return "内容标记失败";

        // 聊天分析错误
        case ErrorCode::CHAT_ANALYZE_FAILED:
            return "聊天分析失败";

        default:
            return "未定义的错误";
    }
}

/**
 * @brief 获取处理步骤名称
 * @param code 错误码
 * @return 对应的处理步骤名称
 */
inline const char* getProcessStepName(ErrorCode code) {
    int codeValue = static_cast<int>(code);

    if (codeValue == 0) return "成功";
    if (codeValue > 0 && codeValue < 100) return "通用处理";
    if (codeValue >= 100 && codeValue < 200) return "图像切片";
    if (codeValue >= 200 && codeValue < 300) return "X裁剪计算";
    if (codeValue >= 300 && codeValue < 400) return "OCR处理";
    if (codeValue >= 400 && codeValue < 500) return "去重处理";
    if (codeValue >= 500 && codeValue < 600) return "平台检测";
    if (codeValue >= 600 && codeValue < 700) return "内容标记";
    if (codeValue >= 700 && codeValue < 800) return "聊天分析";
    if (codeValue >= 800 && codeValue < 900) return "结果导出";
    if (codeValue >= 900 && codeValue < 1000) return "总结创建";

    return "未知步骤";
}

/**
 * @brief 判断错误码是否表示严重错误
 * @param code 错误码
 * @return true表示严重错误，false表示可恢复的错误
 */
inline bool isCriticalError(ErrorCode code) {
    switch (code) {
        case ErrorCode::INITIALIZATION_FAILED:
        case ErrorCode::OCR_ENGINE_INIT_FAILED:
            return true;
        default:
            return false;
    }
}

}  // namespace utils

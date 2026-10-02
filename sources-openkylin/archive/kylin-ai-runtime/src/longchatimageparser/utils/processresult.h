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

#include <chrono>
#include <optional>
#include <string>

#include "errorcodes.h"

namespace utils {

/**
 * @brief OCR处理结果结构体
 *
 * 封装处理结果，包含错误码、错误信息、结果数据等
 * 支持成功和失败两种状态的表示
 */
struct ProcessResult {
    // ========== 核心字段 ==========
    ErrorCode error_code;               // 错误码
    std::string error_message;          // 详细错误信息
    std::optional<std::string> result;  // 成功时的结果数据

    // ========== 附加信息 ==========
    std::string failed_step;                    // 失败的步骤名称
    std::string exception_type;                 // 异常类型（如果有）
    std::chrono::milliseconds processing_time;  // 处理耗时

    // ========== 构造函数 ==========
    /**
     * @brief 默认构造函数，初始化为成功状态
     */
    ProcessResult() : error_code(ErrorCode::SUCCESS), processing_time(0) {}

    /**
     * @brief 带错误码的构造函数
     */
    ProcessResult(ErrorCode code) : error_code(code), failed_step(getProcessStepName(code)), processing_time(0) {}

    // ========== 静态工厂方法 ==========
    /**
     * @brief 创建成功结果
     * @param result_str 处理成功的结果数据
     * @return ProcessResult 成功状态的结果对象
     */
    static ProcessResult success(const std::string& result_str) {
        ProcessResult res;
        res.error_code = ErrorCode::SUCCESS;
        res.result = result_str;
        res.failed_step = "";
        return res;
    }

    /**
     * @brief 创建失败结果
     * @param code 错误码
     * @param message 详细错误信息
     * @return ProcessResult 失败状态的结果对象
     */
    static ProcessResult failure(ErrorCode code, const std::string& message) {
        ProcessResult res;
        res.error_code = code;
        res.error_message = message;
        res.failed_step = getProcessStepName(code);
        return res;
    }

    /**
     * @brief 创建带异常信息的失败结果
     * @param code 错误码
     * @param message 错误信息
     * @param exception_type 异常类型名称
     * @return ProcessResult 失败状态的结果对象
     */
    static ProcessResult failureWithException(ErrorCode code, const std::string& message,
                                              const std::string& exception_type) {
        ProcessResult res = failure(code, message);
        res.exception_type = exception_type;
        return res;
    }

    // ========== 状态判断方法 ==========
    /**
     * @brief 判断处理是否成功
     * @return true表示成功，false表示失败
     */
    bool isSuccess() const { return error_code == ErrorCode::SUCCESS; }

    /**
     * @brief 判断是否有结果数据
     * @return true表示有结果数据
     */
    bool hasResult() const { return result.has_value(); }

    /**
     * @brief 判断是否为严重错误
     * @return true表示严重错误
     */
    bool isCritical() const { return isCriticalError(error_code); }

    // ========== 数据访问方法 ==========
    /**
     * @brief 获取错误码的整数值（用于C接口或返回值）
     * @return 错误码的整数值
     */
    int getErrorCodeInt() const { return static_cast<int>(error_code); }

    /**
     * @brief 获取结果字符串，如果没有结果则返回空字符串
     * @return 结果字符串
     */
    std::string getResultString() const { return result.value_or(""); }

    /**
     * @brief 获取错误描述（组合错误码描述和详细信息）
     * @return 完整的错误描述
     */
    std::string getFullErrorDescription() const {
        if (isSuccess()) {
            return "成功";
        }

        std::string desc = "[错误码 " + std::to_string(getErrorCodeInt()) + "] ";
        desc += getErrorDescription(error_code);

        if (!failed_step.empty()) {
            desc += " (步骤: " + failed_step + ")";
        }

        if (!error_message.empty()) {
            desc += " - " + error_message;
        }

        if (!exception_type.empty()) {
            desc += " [异常类型: " + exception_type + "]";
        }

        return desc;
    }

    /**
     * @brief 设置处理耗时
     * @param time_ms 处理耗时（毫秒）
     */
    void setProcessingTime(std::chrono::milliseconds time_ms) { processing_time = time_ms; }

    /**
     * @brief 获取处理耗时（毫秒）
     * @return 处理耗时
     */
    long getProcessingTimeMs() const { return processing_time.count(); }
};

}  // namespace utils

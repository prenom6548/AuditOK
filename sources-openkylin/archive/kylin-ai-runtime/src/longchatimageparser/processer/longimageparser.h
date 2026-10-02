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
#include <kylin-ai/aiengine.h>

#include <map>
#include <memory>
#include <opencv2/opencv.hpp>
#include <optional>
#include <string>
#include <vector>

#include "../utils/chatmessage.h"
#include "../utils/config.h"
#include "../utils/ocravatartypes.h"
#include "../utils/processresult.h"
#include "../utils/sliceinfo.h"

namespace parser {

// 实现细节类，使用Pimpl idiom
class LongImageParserImpl;

/**
 * 长图解析器
 *
 * 这是整个系统的核心控制器，负责：
 * - 协调9个主要处理步骤的执行
 * - 管理各个模块之间的数据流
 * - 提供完整的错误处理机制
 * - 生成详细的处理结果摘要
 */
class LongImageParser {
public:
    /**
     * 构造函数
     * 使用硬编码的默认配置初始化所有组件
     */
    LongImageParser(std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> visionEngine);

    /**
     * 析构函数
     */
    ~LongImageParser();

    /**
     * 提取图像内容的完整流程
     *
     * 执行9个主要步骤：
     * 1. 切分图像
     * 2. 计算x_crop值
     * 3. 处理切片（OCR+头像检测）
     * 4. 去重处理
     * 5. 平台类型判断
     * 6. 内容标记
     * 7. 分析聊天消息
     * 8. 导出结果
     * 9. 创建处理总结
     *
     * 不抛出异常，所有错误通过返回值中的错误码表示
     *
     * @param image_path 图像文件路径
     * @return ProcessResult 包含错误码、错误信息和处理结果的结构体
     */
    utils::ProcessResult extractImageContents(const std::string& image_path);

private:
    // 使用Pimpl idiom隐藏实现细节
    std::unique_ptr<LongImageParserImpl> pimpl_;
};

}  // namespace parser
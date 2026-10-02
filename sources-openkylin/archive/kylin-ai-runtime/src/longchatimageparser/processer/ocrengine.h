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

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <optional>
#include <string>
#include <vector>

#include "../utils/config.h"
#include "../utils/ocravatartypes.h"
#include "../utils/sliceinfo.h"

#ifdef DEBUG_MODE
namespace export_ns {
class ChineseTextRenderer;  // 前向声明避免头文件依赖
}  // namespace export_ns
#endif

namespace parser {

// TextLine结构体
using ai_engine::core_ai::vision::PixelPoint;
using ai_engine::core_ai::vision::TextLine;
using ai_engine::core_ai::vision::TextRecognitionResult;

/**
 * @brief OCR引擎封装类
 */
class OcrEngine {
private:
    utils::Config& config_;

    // 麒麟AI engine
    std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> visionEngine_;

    // 异步回调同步化所需成员
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> result_ready_{false};
    std::vector<TextLine> ocr_results_;
    int error_code_{0};
    std::string error_message_;

#ifdef DEBUG_MODE
    // 中文字体渲染器
    std::unique_ptr<export_ns::ChineseTextRenderer> chinese_renderer_;
#endif

    float text_score_threshold_;
    bool is_initialized_{false};

    /**
     * @brief 将切片坐标转换为原图坐标
     * @param points 返回的像素点坐标
     * @param start_y 切片的Y轴偏移量
     * @param x_offset X轴偏移量（默认为0，当图像被裁剪时使用）
     */
    std::vector<std::vector<float>> adjustBoxToOriginal(const std::vector<PixelPoint>& points, int start_y,
                                                        int x_offset = 0);

#ifdef DEBUG_MODE
    /**
     * @brief 基于最终result可视化OCR结果（仅Debug模式）
     */
    void visualizeWithScores(const cv::Mat& image, const utils::SliceOcrResult& result,
                             const utils::SliceInfo& slice_info, const std::string& save_path);
#endif

    /**
     * @brief 执行OCR识别
     * @param image 待识别的图像
     * @return OCR识别结果（TextLine列表）
     */
    std::vector<TextLine> performOCRRecognition(const cv::Mat& image);

    /**
     * @brief 回调处理函数
     */
    void handleOCRResult(const TextRecognitionResult& result);

public:
    /**
     * @brief 构造函数
     * @param config 配置对象
     * @param visionEngine 文本识别引擎
     */
    explicit OcrEngine(const utils::Config& config,
                       std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> visionEngine);

    /**
     * @brief 析构函数
     */
    ~OcrEngine();

    /**
     * @brief 处理单个切片的OCR识别 - 对应Python版本的process_slice
     * @param slice_info 切片信息
     * @param save_visualization 是否保存可视化结果
     * @param x_crop X坐标裁剪值，过滤掉中心X坐标小于此值的OCR结果
     * @return 切片OCR结果
     */
    utils::SliceOcrResult processocrSlice(const utils::SliceInfo& slice_info, bool save_visualization = true,
                                          std::optional<int> x_crop = std::nullopt);
};

}  // namespace parser

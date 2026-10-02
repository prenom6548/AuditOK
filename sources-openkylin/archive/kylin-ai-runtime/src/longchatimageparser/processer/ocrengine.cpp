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

#include "ocrengine.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <set>
#include <sstream>
#include <stdexcept>

#include "../../utils/logger.h"
#ifdef DEBUG_MODE
#include "../utils/chinesetextrenderer.h"
#endif

namespace parser {

using namespace utils;
#ifdef DEBUG_MODE
using namespace export_ns;
#endif

// Base64编码函数
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static std::string base64_encode(const unsigned char* bytes_to_encode, size_t in_len) {
    std::string ret;
    int val = 0, valb = -6;
    for (size_t i = 0; i < in_len; i++) {
        unsigned char c = bytes_to_encode[i];
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            ret.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        ret.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (ret.size() % 4) {
        ret.push_back('=');
    }
    return ret;
}

// OCREngine实现

OcrEngine::OcrEngine(const utils::Config& config,
                     std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> visionEngine)
    : config_(const_cast<utils::Config&>(config)) {
    try {
        // 从配置中获取文本置信度阈值
        text_score_threshold_ = config_.ocr.text_score_threshold;
        // 保存 visionEngine 为成员变量
        visionEngine_ = std::move(visionEngine);
        // 直接初始化麒麟AI engine
        auto result = visionEngine_->init();
        if (!result.success) {
            LOGE("Failed to init vision engine: {} {}", result.error.getCode(), result.error.getMessage());
            throw std::runtime_error("Failed to init vision engine: " + result.error.getMessage());
        }
        // 设置回调函数
        visionEngine_->setTextRecognitionResultCallback(
            [this](const ai_engine::core_ai::vision::TextRecognitionResult& result) { handleOCRResult(result); });

        is_initialized_ = true;
        LOGD("[OcrEngine] KylinAI engine initialized successfully");

#ifdef DEBUG_MODE
#ifdef USE_FREETYPE
        // 初始化中文字体渲染器（仅在Debug模式下）
        std::string font_path = "../shangshoushisongti.ttf";
        chinese_renderer_ = std::make_unique<export_ns::ChineseTextRenderer>(font_path, 16);

        if (!chinese_renderer_->isInitialized()) {
            LOGD("[OcrEngine] Warning: Chinese font renderer init failed, using system default");
        } else {
            LOGD("[OcrEngine] Chinese font renderer initialized successfully");
        }
#else
        chinese_renderer_ = nullptr;
#endif
#endif

        LOGD("[OcrEngine] Init complete, text confidence threshold: {}", text_score_threshold_);

    } catch (const std::exception& e) {
        LOGE("[OcrEngine] Initialization failed: {}", e.what());
        throw;
    }
}

OcrEngine::~OcrEngine() {
    if (visionEngine_) {
        visionEngine_->destroy();
        visionEngine_.reset();
    }
}

void OcrEngine::handleOCRResult(const TextRecognitionResult& result) {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        // 直接存储返回的TextLine结果
        ocr_results_.clear();
        ocr_results_.reserve(result.lines.size());
        for (const auto& line : result.lines) {
            ocr_results_.push_back(line);
        }
        error_code_ = 0;
    } catch (const std::exception& e) {
        error_code_ = -1;
        error_message_ = e.what();
        ocr_results_.clear();
    }

    result_ready_ = true;
    cv_.notify_one();
}

std::vector<TextLine> OcrEngine::performOCRRecognition(const cv::Mat& image) {
    if (!is_initialized_) {
        LOGE("[OcrEngine] Engine not initialized");
        return {};
    }

    // 将图像编码为PNG格式的缓冲区
    std::vector<uchar> image_buffer;
    if (!cv::imencode(".png", image, image_buffer)) {
        LOGE("[OcrEngine] Image encoding failed");
        return {};
    }

    // 转换为Base64字符串
    std::string base64_data = base64_encode(image_buffer.data(), image_buffer.size());

    // 重置结果状态
    {
        std::lock_guard<std::mutex> lock(mutex_);
        result_ready_ = false;
        ocr_results_.clear();
        error_message_.clear();
        error_code_ = 0;
    }

    // 调用异步OCR识别
    auto result = visionEngine_->recognizeTextFromBase64ImageData(base64_data);
    if (!result.success) {
        LOGE("[OcrEngine] OCR recognition call failed: {} {}", result.error.getCode(), result.error.getMessage());
        return {};
    }

    // 等待结果（最长30秒超时）
    std::unique_lock<std::mutex> lock(mutex_);
    bool success = cv_.wait_for(lock, std::chrono::seconds(30), [this] { return result_ready_.load(); });

    if (!success) {
        LOGE("[OcrEngine] OCR recognition timeout");
        return {};
    }

    if (error_code_ != 0) {
        LOGE("[OcrEngine] OCR recognition failed: {}", error_message_);
        return {};
    }

#ifdef DEBUG_MODE
    LOGD("[OcrEngine] OCR returned {} recognition results", ocr_results_.size());
#endif
    return ocr_results_;
}

utils::SliceOcrResult OcrEngine::processocrSlice(const utils::SliceInfo& slice_info, bool save_visualization,
                                                 std::optional<int> x_crop) {
    LOGD("[OcrEngine] Processing slice {}...", slice_info.slice_index);

    utils::SliceOcrResult result(slice_info.slice_index, slice_info.start_y, slice_info.end_y);

    try {
        cv::Mat slice_image = slice_info.image;

        if (slice_image.empty()) {
            LOGD("[OcrEngine] Warning: Slice {} image is empty", slice_info.slice_index);
            return result;
        }

        // 如果指定了x_crop，裁剪图像
        cv::Mat cropped_image = slice_image;
        int x_offset = 0;
        if (x_crop.has_value() && x_crop.value() > 0 && x_crop.value() < slice_image.cols) {
            cv::Rect roi(x_crop.value(), 0, slice_image.cols - x_crop.value(), slice_image.rows);
            cropped_image = slice_image(roi);
            x_offset = x_crop.value();

            LOGD("[OcrEngine] Cropping image: original_width={}, x_crop={}, cropped_width={}", slice_image.cols,
                 x_crop.value(), cropped_image.cols);
        }

        LOGD("[OcrEngine] Starting OCR inference, image size: {}x{} channels={}", cropped_image.cols,
             cropped_image.rows, cropped_image.channels());

        // 直接获取TextLine结果
        std::vector<TextLine> ocr_results = performOCRRecognition(cropped_image);

        LOGD("[OcrEngine] OCR returned {} detection results", ocr_results.size());

        // 打印前几个结果用于调试
        for (size_t i = 0; i < std::min(size_t(3), ocr_results.size()); i++) {
            LOGD("  Result{}: text='{}' conf={} points_size={}", i, ocr_results[i].text, ocr_results[i].conf,
                 ocr_results[i].points.size());
        }

        for (const auto& line : ocr_results) {
            try {
                // 验证数据有效性
                if (line.points.size() != 4) {
                    LOGD("[OcrEngine] Warning: Skipping invalid polygon (points != 4)");
                    continue;
                }

                if (line.text.empty()) {
                    LOGD("[OcrEngine] Warning: Skipping empty text");
                    continue;
                }

                // 使用engine返回的置信度进行过滤
                if (line.conf < text_score_threshold_) {
                    continue;
                }

                // 转换坐标到原图坐标系
                std::vector<std::vector<float>> adjusted_box =
                    adjustBoxToOriginal(line.points, slice_info.start_y, x_offset);

                // 添加OCR项，使用SDK返回的实际置信度
                result.add_ocr_item(line.text, adjusted_box, line.conf);

            } catch (const std::exception& e) {
                LOGD("[OcrEngine] Warning: Error processing OCR item: {}", e.what());
                continue;
            }
        }

        LOGD("[OcrEngine] Slice {} kept {} texts after filtering", slice_info.slice_index, result.ocr_items.size());

        // 排序结果
        result.sort_by_y();

        // 保存可视化结果（仅在Debug模式下）
#ifdef DEBUG_MODE
        if (save_visualization && config_.shouldSaveOutputFiles()) {
            try {
                std::string vis_path = config_.output.output_images_dir + "/slice_ocr_result_" +
                                       std::to_string(slice_info.slice_index) + ".jpg";
                visualizeWithScores(slice_image, result, slice_info, vis_path);

            } catch (const std::exception& viz_error) {
                LOGD("[OcrEngine] Warning: Slice {} visualization save failed: {}", slice_info.slice_index,
                     viz_error.what());
            }
        }
#endif

        LOGD("[OcrEngine] Slice {} processing completed, detected {} texts", slice_info.slice_index,
             result.ocr_items.size());

    } catch (const std::exception& e) {
        LOGE("[OcrEngine] OCR processing failed - Slice {}: {}", slice_info.slice_index, e.what());
        LOGD("[OcrEngine] Slice {} skipping OCR, returning empty result", slice_info.slice_index);
    }

    return result;
}

// 私有方法实现

std::vector<std::vector<float>> OcrEngine::adjustBoxToOriginal(const std::vector<PixelPoint>& points, int start_y,
                                                               int x_offset) {
    std::vector<std::vector<float>> adjusted_box;

    if (points.size() == 4) {
        // 找到4个角点：左上、右上、右下、左下
        std::vector<PixelPoint> sorted_points = points;

        // 初始化：假设第0个点是所有角
        int top_left_idx = 0, top_right_idx = 0, bottom_right_idx = 0, bottom_left_idx = 0;
        int min_sum = sorted_points[0].x + sorted_points[0].y;   // 左上角：最小 x+y
        int max_sum = min_sum;                                   // 右下角：最大 x+y
        int min_diff = sorted_points[0].y - sorted_points[0].x;  // 右上角：最小 y-x
        int max_diff = min_diff;                                 // 左下角：最大 y-x

        // 一次遍历，同时找出4个角
        for (int i = 1; i < 4; i++) {
            int sum = sorted_points[i].x + sorted_points[i].y;
            int diff = sorted_points[i].y - sorted_points[i].x;

            if (sum < min_sum) {
                min_sum = sum;
                top_left_idx = i;
            }
            if (sum > max_sum) {
                max_sum = sum;
                bottom_right_idx = i;
            }
            if (diff < min_diff) {
                min_diff = diff;
                top_right_idx = i;
            }
            if (diff > max_diff) {
                max_diff = diff;
                bottom_left_idx = i;
            }
        }

        // 按顺序构造：[左上, 右上, 右下, 左下]
        std::vector<int> order = {top_left_idx, top_right_idx, bottom_right_idx, bottom_left_idx};

        for (int idx : order) {
            std::vector<float> adjusted_point = {
                static_cast<float>(sorted_points[idx].x + x_offset),  // 添加x偏移
                static_cast<float>(sorted_points[idx].y + start_y)    // 添加y偏移
            };
            adjusted_box.emplace_back(std::move(adjusted_point));
        }
    } else {
        // 如果不是4个点，直接转换
        for (const auto& point : points) {
            std::vector<float> adjusted_point = {static_cast<float>(point.x + x_offset),
                                                 static_cast<float>(point.y + start_y)};
            adjusted_box.emplace_back(std::move(adjusted_point));
        }
    }

    return adjusted_box;
}

#ifdef DEBUG_MODE
void OcrEngine::visualizeWithScores(const cv::Mat& image, const utils::SliceOcrResult& result,
                                    const utils::SliceInfo& slice_info, const std::string& save_path) {
    cv::Mat vis_image = image.clone();

    // 添加调试标记
    cv::rectangle(vis_image, cv::Point(5, 5), cv::Point(200, 40), cv::Scalar(0, 0, 255), -1);
    cv::putText(vis_image, "C++ OCR FINAL", cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255),
                1);

    for (size_t i = 0; i < result.ocr_items.size(); i++) {
        const auto& ocr_item = result.ocr_items[i];

        std::vector<cv::Point> pts;
        for (const auto& point : ocr_item.box) {
            int slice_x = static_cast<int>(point[0]);
            int slice_y = static_cast<int>(point[1] - slice_info.start_y);
            pts.push_back(cv::Point(slice_x, slice_y));
        }

        if (pts.size() == 4) {
            std::vector<std::vector<cv::Point>> contours = {pts};
            cv::polylines(vis_image, contours, true, cv::Scalar(0, 255, 0), 3);

            std::stringstream ss;
            ss << "T" << (i + 1) << " (" << std::fixed << std::setprecision(2) << ocr_item.score
               << "): " << ocr_item.text;
            std::string box_label = ss.str();

            int baseline = 0;
            double font_scale = 0.7;
            cv::Size text_size = cv::getTextSize(box_label, cv::FONT_HERSHEY_SIMPLEX, font_scale, 2, &baseline);

            cv::Point text_pos(pts[0].x, pts[0].y - 8);
            text_pos.x = std::max(5, text_pos.x);
            text_pos.y = std::max(text_size.height + 8, text_pos.y);

            cv::rectangle(vis_image, cv::Point(text_pos.x - 3, text_pos.y - text_size.height - 3),
                          cv::Point(text_pos.x + text_size.width + 3, text_pos.y + 3), cv::Scalar(255, 255, 255), -1);

            if (chinese_renderer_ && chinese_renderer_->isInitialized()) {
                chinese_renderer_->putText(vis_image, box_label, text_pos, cv::Scalar(0, 0, 255), 1);
            } else {
                std::stringstream ss_fallback;
                ss_fallback << "T" << (i + 1) << " (" << std::fixed << std::setprecision(2) << ocr_item.score << ")";
                std::string fallback_label = ss_fallback.str();
                cv::putText(vis_image, fallback_label, text_pos, cv::FONT_HERSHEY_SIMPLEX, font_scale,
                            cv::Scalar(0, 0, 255), 2);
            }

            if (slice_info.slice_index == 0) {
                LOGD("[OcrEngine] T{}: {}", (i + 1), ocr_item.text);
            }
        }
    }

    bool save_success = cv::imwrite(save_path, vis_image);
    LOGD("[OcrEngine] Visualization image saved: {} (items={}, save_success={})", save_path, result.ocr_items.size(),
         save_success);
}
#endif  // DEBUG_MODE

}  // namespace parser

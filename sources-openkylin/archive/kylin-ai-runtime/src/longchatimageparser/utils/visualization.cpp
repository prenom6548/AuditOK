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

#include "visualization.h"

#include <algorithm>

#include "../../utils/logger.h"

#ifdef DEBUG_MODE

namespace export_ns {

Visualizer::Visualizer(const utils::Config& config) : config_(config) {
    output_dir_ = std::filesystem::path(config_.output.output_images_dir);
    // 仅在Debug模式下创建输出目录
    if (config_.shouldSaveOutputFiles()) {
        init_output_directory();
    }
}

void Visualizer::init_output_directory() {
    try {
        std::filesystem::create_directories(output_dir_);
        LOGD("Output directory created/confirmed: {}", output_dir_.string());
    } catch (const std::exception& e) {
        LOGE("Failed to create output directory: {}", e.what());
    }
}

void Visualizer::createProcessSummaryImage(const cv::Mat& original_image,
                                           const std::vector<utils::SliceInfo>& slice_infos, int ocr_count,
                                           int avatar_count, const std::string& output_file) {
    int h = original_image.rows;
    int w = original_image.cols;

    // 创建缩略图
    double scale = std::min(800.0 / w, 1.0);
    int new_w = static_cast<int>(w * scale);
    int new_h = static_cast<int>(h * scale);

    cv::Mat thumbnail;
    cv::resize(original_image, thumbnail, cv::Size(new_w, new_h));

    // 在缩略图上绘制切片边界
    for (const auto& slice_info : slice_infos) {
        int y1 = static_cast<int>(slice_info.start_y * scale);
        int y2 = static_cast<int>(slice_info.end_y * scale);

        cv::line(thumbnail, cv::Point(0, y1), cv::Point(new_w, y1), cv::Scalar(0, 255, 0), 2);

        std::string slice_label = "Slice " + std::to_string(slice_info.slice_index);
        cv::putText(thumbnail, slice_label, cv::Point(10, y1 + 20), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    cv::Scalar(0, 255, 0), 1);
    }

    // 添加统计信息
    int info_height = 100;
    cv::Mat result = cv::Mat::ones(new_h + info_height, new_w, CV_8UC3) * 255;

    // 复制缩略图到结果图像上部
    thumbnail.copyTo(result(cv::Rect(0, 0, new_w, new_h)));

    // 添加文本信息
    int y_offset = new_h + 30;

    std::string total_slices = "Total Slices: " + std::to_string(slice_infos.size());
    cv::putText(result, total_slices, cv::Point(10, y_offset), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 1);

    std::string ocr_results = "OCR Results: " + std::to_string(ocr_count);
    cv::putText(result, ocr_results, cv::Point(10, y_offset + 25), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0),
                1);

    std::string avatars_detected = "Avatars Detected: " + std::to_string(avatar_count);
    cv::putText(result, avatars_detected, cv::Point(10, y_offset + 50), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                cv::Scalar(0, 0, 0), 1);

    if (config_.shouldSaveOutputFiles()) {
        std::filesystem::path output_path = output_dir_ / output_file;
        cv::imwrite(output_path.string(), result);
        LOGD("[DEBUG] Processing summary image saved to: {}", output_path.string());
    }
}

}  // namespace export_ns

#endif  // DEBUG_MODE

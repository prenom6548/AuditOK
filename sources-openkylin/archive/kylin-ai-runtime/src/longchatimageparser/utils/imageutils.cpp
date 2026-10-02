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

#include "imageutils.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

#include "../../utils/logger.h"

namespace utils {
namespace ImageUtils {

bool saveDebugImage(const cv::Mat& image, const std::string& filename, const std::string& output_dir) {
    try {
        // 创建输出目录（如果不存在）
        std::filesystem::path output_path(output_dir);
        std::filesystem::create_directories(output_path);

        // 构建完整路径
        std::filesystem::path full_path = output_path / filename;

        // 保存图像
        bool success = cv::imwrite(full_path.string(), image);

        if (!success) {
            LOGE("Failed to save image: {}", full_path.string());
        }

        return success;

    } catch (const std::exception& e) {
        LOGE("Save debug image exception: {}", e.what());
        return false;
    }
}

cv::Mat draw_boxes_on_image(const cv::Mat& image, const std::vector<BoundingBox>& boxes, const cv::Scalar& color,
                            int thickness) {
    // 创建图像副本
    cv::Mat result_img = image.clone();

    for (const auto& box : boxes) {
        if (std::holds_alternative<OCRBox>(box)) {
            // OCR格式: 4个点的多边形
            const OCRBox& ocr_box = std::get<OCRBox>(box);

            if (ocr_box.size() >= 4) {
                // 转换为OpenCV points数组
                std::vector<cv::Point> points;
                for (const auto& point : ocr_box) {
                    points.push_back(point);
                }

                // 绘制多边形
                const cv::Point* pts = points.data();
                int npts = static_cast<int>(points.size());
                cv::polylines(result_img, &pts, &npts, 1, true, color, thickness);
            }

        } else if (std::holds_alternative<RectBox>(box)) {
            // 矩形格式: (x, y, w, h)
            const RectBox& rect_box = std::get<RectBox>(box);

            cv::Point pt1(rect_box.x, rect_box.y);
            cv::Point pt2(rect_box.x + rect_box.width, rect_box.y + rect_box.height);
            cv::rectangle(result_img, pt1, pt2, color, thickness);
        }
    }

    return result_img;
}

cv::Mat crop_image_region(const cv::Mat& image, int x, int y, int w, int h) {
    // 确保坐标在图像范围内
    x = std::max(0, x);
    y = std::max(0, y);
    int x2 = std::min(image.cols, x + w);
    int y2 = std::min(image.rows, y + h);

    // 计算实际的裁剪区域
    int actual_w = x2 - x;
    int actual_h = y2 - y;

    // 如果区域无效，返回空图像
    if (actual_w <= 0 || actual_h <= 0) {
        return cv::Mat();
    }

    // 使用ROI裁剪
    cv::Rect roi(x, y, actual_w, actual_h);
    return image(roi).clone();
}

cv::Mat merge_images_vertically(const std::vector<cv::Mat>& images, int gap) {
    if (images.empty()) {
        return cv::Mat();
    }

    // 获取最大宽度
    int max_width = 0;
    for (const auto& img : images) {
        max_width = std::max(max_width, img.cols);
    }

    // 计算总高度
    int total_height = 0;
    for (const auto& img : images) {
        total_height += img.rows;
    }
    total_height += gap * (static_cast<int>(images.size()) - 1);

    // 检查图像通道数（确保一致性）
    int channels = images[0].channels();
    int type = images[0].type();

    // 创建结果图像（白色背景）
    cv::Mat result;
    if (channels == 3) {
        result = cv::Mat::ones(total_height, max_width, type) * 255;
    } else if (channels == 1) {
        result = cv::Mat::ones(total_height, max_width, type) * 255;
    } else {
        result = cv::Mat::zeros(total_height, max_width, type);
    }

    // 合并图像
    int current_y = 0;
    for (const auto& img : images) {
        int h = img.rows;
        int w = img.cols;

        // 居中放置
        int x_offset = (max_width - w) / 2;

        // 确保不会越界
        x_offset = std::max(0, x_offset);
        int copy_width = std::min(w, max_width - x_offset);
        int copy_height = std::min(h, result.rows - current_y);

        if (copy_width > 0 && copy_height > 0) {
            cv::Rect src_rect(0, 0, copy_width, copy_height);
            cv::Rect dst_rect(x_offset, current_y, copy_width, copy_height);

            img(src_rect).copyTo(result(dst_rect));
        }

        current_y += h + gap;
    }

    return result;
}

}  // namespace ImageUtils
}  // namespace utils

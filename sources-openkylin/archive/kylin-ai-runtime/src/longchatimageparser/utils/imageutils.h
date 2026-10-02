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

#include <opencv2/opencv.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace utils {

/**
 * 图像处理工具函数
 * 提供图像读写、处理、绘制等功能
 */
namespace ImageUtils {

/**
 * 边界框类型
 * - OCRBox: OCR格式的4个点坐标 [[x1,y1], [x2,y2], [x3,y3], [x4,y4]]
 * - RectBox: 矩形格式 (x, y, w, h)
 */
using OCRBox = std::vector<cv::Point2i>;  // 4个点的OCR边界框
using RectBox = cv::Rect;                 // 矩形边界框
using BoundingBox = std::variant<OCRBox, RectBox>;

/**
 * 保存调试图像
 *
 * @param image 要保存的图像
 * @param filename 文件名
 * @param output_dir 输出目录，默认为 "./output_images/debug"
 * @return 是否成功保存
 */
bool saveDebugImage(const cv::Mat& image, const std::string& filename,
                    const std::string& output_dir = "./output_images/debug");

/**
 * 在图像上绘制边界框
 *
 * @param image 原始图像
 * @param boxes 边界框列表（支持OCR格式和矩形格式混合）
 * @param color 绘制颜色 (B, G, R)，默认为绿色
 * @param thickness 线条粗细，默认为2
 * @return 绘制了边界框的图像副本
 */
cv::Mat draw_boxes_on_image(const cv::Mat& image, const std::vector<BoundingBox>& boxes,
                            const cv::Scalar& color = cv::Scalar(0, 255, 0), int thickness = 2);

/**
 * 裁剪图像区域
 * 自动处理边界检查，确保不会超出图像范围
 *
 * @param image 原始图像
 * @param x 左上角x坐标
 * @param y 左上角y坐标
 * @param w 宽度
 * @param h 高度
 * @return 裁剪后的图像区域
 */
cv::Mat crop_image_region(const cv::Mat& image, int x, int y, int w, int h);

/**
 * 垂直合并多个图像
 *
 * @param images 图像列表
 * @param gap 图像之间的间隔（像素），默认为10
 * @return 合并后的图像
 */
cv::Mat merge_images_vertically(const std::vector<cv::Mat>& images, int gap = 10);

}  // namespace ImageUtils

}  // namespace utils

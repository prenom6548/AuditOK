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

/*
 * Avatar detection器模块实现
 * 负责检测聊天界面中的头像位置
 */

#include "avatardetector.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "../../utils/logger.h"

namespace detection {

// 默认合并阈值（像素），当无法从检测框计算时使用
constexpr double DEFAULT_MERGE_THRESHOLD = 20.0;

AvatarDetector::AvatarDetector(const utils::Config& config) : config_(config) {
    // 初始化调试目录
    if (config.shouldSaveOutputFiles()) {
        debug_dir_ = config.output.debug_dir;
        if (!debug_dir_.empty()) {
            std::filesystem::create_directories(debug_dir_);
        }
    } else {
        debug_dir_ = "";  // Release模式下清空调试目录路径
    }

    // 初始化Avatar detection参数
    binary_threshold_ = config.image.binary_threshold;
    gaussian_blur_size_ =
        std::make_pair(std::get<0>(config.image.gaussian_blur_size), std::get<1>(config.image.gaussian_blur_size));
    strict_square_ratio_min_ = config.avatar.strict_square_ratio_min;
    strict_square_ratio_max_ = config.avatar.strict_square_ratio_max;
    iou_threshold_ = config.avatar.iou_threshold;
    x_crop_offset_ = config.avatar.x_crop_offset;
}

std::vector<utils::AvatarItem> AvatarDetector::detectAvatars(const utils::SliceInfo& slice_info,
                                                             std::optional<int> x_crop) {
    LOGD("[AvatarDetector] Start detecting slice index: {} ", slice_info.slice_index);

    try {
        // 根据x_crop裁剪图像
        cv::Mat slice_img;
        if (x_crop.has_value()) {
            int crop_width = std::min(x_crop.value(), slice_info.image.cols);
            slice_img = slice_info.image(cv::Rect(0, 0, crop_width, slice_info.image.rows));
            //     "进行裁剪"
            LOGD("[AvatarDetector] Using x_crop={} for cropping", x_crop.value());
        } else {
            slice_img = slice_info.image;
            LOGD("[AvatarDetector] No x cropping, using original image");
        }

        // 预处理图像
        cv::Mat binary = preprocessImage(slice_img);

        // 提取轮廓和外接矩形
        auto rects = extractContoursAndRects(binary, slice_img);

        // 应用NMS去除重叠框
        auto nms_rects = applyNms(rects);

        // 合并相邻的框
        double merge_threshold = calculateMergeThreshold(rects);
        auto merged_rects = mergeNearbyBoxes(nms_rects, merge_threshold);

#ifdef DEBUG_MODE
        // 保存调试图像
        if (!debug_dir_.empty()) {
            saveDebugImage(slice_img, merged_rects, slice_info.slice_index);
        }
#endif

        // 转换为AvatarItem对象
        std::vector<utils::AvatarItem> avatar_items;
        for (const auto& rect : merged_rects) {
            // 将Y坐标转换为原图坐标
            auto adjusted_box =
                std::make_tuple(static_cast<float>(rect.x), static_cast<float>(rect.y + slice_info.start_y),
                                static_cast<float>(rect.width), static_cast<float>(rect.height));
            utils::AvatarItem avatar_item(adjusted_box, slice_info.slice_index);
            avatar_items.push_back(avatar_item);
        }

        //     detected "
        LOGD("[C++Avatar detection] Slice {} detected {} avatars", slice_info.slice_index, avatar_items.size());
        return avatar_items;

    } catch (const std::exception& e) {
        LOGE("[C++Avatar detection] Avatar detection failed - Slice {}：{}", slice_info.slice_index, e.what());
        LOGD("[C++Avatar detection] Slice {} Skipping avatar detection, returning empty avatar list",
             slice_info.slice_index);
        return {};
    }
}

std::optional<int> AvatarDetector::calculateXCrop(const std::vector<utils::SliceInfo>& slice_infos) {
    // 根据Slice数量决定处理哪些Slice
    auto slices_to_process = getSlicesToProcess(slice_infos);

    // 处理选定的Slice，收集target_box
    std::vector<BoundingBoxWithSlice> all_boxes;
    for (const auto& slice_info : slices_to_process) {
        auto target_box = findTargetBoxForSlice(slice_info);
        if (target_box.has_value()) {
            const auto& box = target_box.value();
            // 将坐标转换为原图坐标
            BoundingBoxWithSlice adjusted_box(box.x, box.y + slice_info.start_y, box.width, box.height,
                                              slice_info.slice_index);
            all_boxes.push_back(adjusted_box);
        }
    }

    // 找到最合适的target_box
    auto selected_box = findBestTargetBox(all_boxes);

    if (selected_box.has_value() && !slice_infos.empty()) {
        const auto& box = selected_box.value();
        int image_width = slice_infos[0].image.cols;
        int x_crop = std::min(box.x + box.width + x_crop_offset_, image_width - 1);
        LOGD("[C++Avatar detection] Found suitable target_box, calculated x_crop value: {} (image_width={})", x_crop,
             image_width);
        return x_crop;
    } else {
        LOGD("[C++Avatar detection] No suitable target_box found");
        return std::nullopt;
    }
}

cv::Mat AvatarDetector::preprocessImage(const cv::Mat& image) {
    // 转灰度
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // 高斯模糊去噪
    cv::Mat blurred;
    cv::Size blur_size(gaussian_blur_size_.first, gaussian_blur_size_.second);
    cv::GaussianBlur(gray, blurred, blur_size, 0);

    // 二值化
    cv::Mat binary;
    cv::threshold(blurred, binary, binary_threshold_, 255, cv::THRESH_BINARY);

    // 反转颜色（头像通常是深色背景）
    binary = 255 - binary;

    return binary;
}

std::vector<BoundingBox> AvatarDetector::extractContoursAndRects(const cv::Mat& binary_img,
                                                                 const cv::Mat& original_img) {
    // 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 计算所有轮廓的外接矩形
    std::vector<BoundingBox> rects;
    for (const auto& cnt : contours) {
        cv::Rect bounding_rect = cv::boundingRect(cnt);
        rects.emplace_back(bounding_rect.x, bounding_rect.y, bounding_rect.width, bounding_rect.height);
    }

    // 按面积从大到小排序
    std::sort(rects.begin(), rects.end(), [](const auto& a, const auto& b) { return a.area() > b.area(); });

    return rects;
}

std::optional<BoundingBox> AvatarDetector::findTargetBoxForSlice(const utils::SliceInfo& slice_info) {
    // 预处理图像
    cv::Mat binary = preprocessImage(slice_info.image);

    // 提取轮廓和外接矩形
    auto rects = extractContoursAndRects(binary, slice_info.image);

    if (rects.empty()) {
        return std::nullopt;
    }

    // 使用改进的算法找到最合适的框
    return selectTargetBox(rects);
}

std::optional<BoundingBox> AvatarDetector::selectTargetBox(const std::vector<BoundingBox>& rects) {
    if (rects.empty()) {
        return std::nullopt;
    }

    // 获取前三个不同的x坐标值
    std::vector<int> x_list;
    for (const auto& rect : rects) {
        x_list.push_back(rect.x);
    }
    std::sort(x_list.begin(), x_list.end());
    x_list.erase(std::unique(x_list.begin(), x_list.end()), x_list.end());

    std::vector<int> top_3_x(x_list.begin(), x_list.begin() + std::min(3, static_cast<int>(x_list.size())));

    // 挑选左侧前三且严格趋于正方形的框
    std::vector<BoundingBox> square_rects;
    for (const auto& rect : rects) {
        if (std::find(top_3_x.begin(), top_3_x.end(), rect.x) != top_3_x.end() && isStrictSquare(rect)) {
            square_rects.push_back(rect);
        }
    }

    // 在剩余框中选择面积最大的作为target_box
    if (!square_rects.empty()) {
        auto max_rect = *std::max_element(square_rects.begin(), square_rects.end(),
                                          [](const auto& a, const auto& b) { return a.area() < b.area(); });
        return max_rect;
    } else {
        // 降级处理：选择最左侧的框
        auto min_rect =
            *std::min_element(rects.begin(), rects.end(), [](const auto& a, const auto& b) { return a.x < b.x; });
        return min_rect;
    }
}

std::optional<BoundingBoxWithSlice> AvatarDetector::findBestTargetBox(
    const std::vector<BoundingBoxWithSlice>& all_boxes) {
    if (all_boxes.empty()) {
        return std::nullopt;
    }

    // 按x坐标排序
    auto sorted_boxes = all_boxes;
    std::sort(sorted_boxes.begin(), sorted_boxes.end(), [](const auto& a, const auto& b) { return a.x < b.x; });

    // 对最左侧前20%的box进行操作
    int left_20_percent_count = std::max(1, static_cast<int>(sorted_boxes.size() * 0.2));
    auto left_boxes =
        std::vector<BoundingBoxWithSlice>(sorted_boxes.begin(), sorted_boxes.begin() + left_20_percent_count);

    // 找到符合要求的框
    for (const auto& box : left_boxes) {
        // 判断是否严格趋近于正方形
        double aspect_ratio = (box.height > 0) ? static_cast<double>(box.width) / box.height : 0;
        bool is_square_like = (aspect_ratio >= strict_square_ratio_min_ && aspect_ratio <= strict_square_ratio_max_);

        if (is_square_like) {
            return box;
        }
    }

    return std::nullopt;
}

std::vector<utils::SliceInfo> AvatarDetector::getSlicesToProcess(const std::vector<utils::SliceInfo>& slice_infos) {
    int total_slices = static_cast<int>(slice_infos.size());
    if (total_slices == 1) {
        return slice_infos;
    } else if (total_slices == 2) {
        return std::vector<utils::SliceInfo>(slice_infos.begin(), slice_infos.begin() + 1);
    } else {
        return std::vector<utils::SliceInfo>(slice_infos.begin() + 1, slice_infos.end() - 1);
    }
}

std::vector<BoundingBox> AvatarDetector::applyNms(const std::vector<BoundingBox>& rects) {
    std::vector<BoundingBox> keep_rects;
    for (const auto& rect : rects) {
        bool keep = true;
        for (const auto& kept_rect : keep_rects) {
            if (calculateIou(rect, kept_rect) > iou_threshold_) {
                keep = false;
                break;
            }
        }
        if (keep) {
            keep_rects.push_back(rect);
        }
    }
    return keep_rects;
}

double AvatarDetector::calculateIou(const BoundingBox& box1, const BoundingBox& box2) {
    // 计算交集
    int xi1 = std::max(box1.x, box2.x);
    int yi1 = std::max(box1.y, box2.y);
    int xi2 = std::min(box1.right(), box2.right());
    int yi2 = std::min(box1.bottom(), box2.bottom());

    int inter_width = std::max(0, xi2 - xi1);
    int inter_height = std::max(0, yi2 - yi1);
    int inter_area = inter_width * inter_height;

    // 计算并集
    int box1_area = box1.area();
    int box2_area = box2.area();
    int union_area = box1_area + box2_area - inter_area;

    if (union_area == 0) {
        return 0.0;
    }

    return static_cast<double>(inter_area) / union_area;
}

double AvatarDetector::calculateMergeThreshold(const std::vector<BoundingBox>& rects) {
    if (!rects.empty()) {
        const auto& box = rects[0];                     // 面积最大的框
        int max_dim = std::max(box.width, box.height);  // 最长边
        return max_dim * config_.image.merge_distance_factor;
    } else {
        return DEFAULT_MERGE_THRESHOLD;
    }
}

std::vector<BoundingBox> AvatarDetector::mergeNearbyBoxes(const std::vector<BoundingBox>& rects,
                                                          double merge_threshold) {
    std::vector<BoundingBox> merged_rects;
    std::vector<bool> used(rects.size(), false);

    for (size_t i = 0; i < rects.size(); ++i) {
        if (used[i]) {
            continue;
        }

        // 找到所有需要与当前框合并的框
        std::vector<BoundingBox> group;
        group.push_back(rects[i]);
        used[i] = true;

        for (size_t j = i + 1; j < rects.size(); ++j) {
            if (used[j]) {
                continue;
            }

            // 检查是否与组中任何一个框相邻
            bool should_add = false;
            for (const auto& box_in_group : group) {
                if (shouldMerge(box_in_group, rects[j], merge_threshold)) {
                    should_add = true;
                    break;
                }
            }

            if (should_add) {
                group.push_back(rects[j]);
                used[j] = true;
            }
        }

        // 合并这一组框
        auto merged_box = mergeBoxes(group);
        if (merged_box.has_value()) {
            merged_rects.push_back(merged_box.value());
        }
    }

    return merged_rects;
}

bool AvatarDetector::shouldMerge(const BoundingBox& box1, const BoundingBox& box2, double distance_threshold) {
    // 计算两个框中心点之间的距离
    double center1_x = box1.x + box1.width / 2.0;
    double center1_y = box1.y + box1.height / 2.0;
    double center2_x = box2.x + box2.width / 2.0;
    double center2_y = box2.y + box2.height / 2.0;
    double distance = std::sqrt((center1_x - center2_x) * (center1_x - center2_x) +
                                (center1_y - center2_y) * (center1_y - center2_y));

    // 如果距离小于阈值或IOU大于0.01，则合并
    return distance < distance_threshold || calculateIou(box1, box2) > 0.01;
}

std::optional<BoundingBox> AvatarDetector::mergeBoxes(const std::vector<BoundingBox>& boxes) {
    if (boxes.empty()) {
        return std::nullopt;
    }

    int min_x = boxes[0].x;
    int min_y = boxes[0].y;
    int max_x = boxes[0].right();
    int max_y = boxes[0].bottom();

    for (const auto& box : boxes) {
        min_x = std::min(min_x, box.x);
        min_y = std::min(min_y, box.y);
        max_x = std::max(max_x, box.right());
        max_y = std::max(max_y, box.bottom());
    }

    return BoundingBox(min_x, min_y, max_x - min_x, max_y - min_y);
}

#ifdef DEBUG_MODE
void AvatarDetector::saveDebugImage(const cv::Mat& image, const std::vector<BoundingBox>& rects, int slice_index) {
    // 仅在Debug模式下保存调试图像
    if (!config_.shouldSaveOutputFiles() || debug_dir_.empty()) {
        // Release模式下，只在Debug配置时输出日志
        LOGD("[DEBUG] Skipping save avatar debug image avatars_slice_:{}", slice_index);
        return;
    }

    cv::Mat debug_img = image.clone();
    for (const auto& rect : rects) {
        cv::rectangle(debug_img, cv::Point(rect.x, rect.y), cv::Point(rect.right(), rect.bottom()),
                      cv::Scalar(0, 255, 0), 2);
    }

    std::string debug_path = debug_dir_ + "/avatars_slice_" + std::to_string(slice_index) + ".jpg";
    bool save_success = cv::imwrite(debug_path, debug_img);
    if (save_success) {
        LOGD("[DEBUG] Avatar debug image saved: {}", debug_path);
    } else {
        LOGE("[DEBUG] Failed to save avatar debug image: {} ", debug_path);
    }
}
#endif

bool AvatarDetector::isStrictSquare(const BoundingBox& rect) {
    if (rect.height == 0 || rect.width == 0) return false;

    double ratio = static_cast<double>(rect.width) / rect.height;
    return (ratio >= strict_square_ratio_min_ && ratio <= strict_square_ratio_max_) ||
           ((1.0 / ratio) >= strict_square_ratio_min_ && (1.0 / ratio) <= strict_square_ratio_max_);
}

}  // namespace detection

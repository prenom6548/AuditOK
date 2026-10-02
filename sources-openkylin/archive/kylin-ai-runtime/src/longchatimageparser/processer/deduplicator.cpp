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

#include "deduplicator.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "../../utils/logger.h"

namespace detection {

Deduplicator::Deduplicator(const utils::Config& config) : config_(config) {
    ocr_iou_threshold_ = config.deduplication.ocr_iou_threshold;
    avatar_iou_threshold_ = config.deduplication.avatar_iou_threshold;

    LOGD("[Deduplicator] Init complete, OCR threshold: {}, avatar threshold: {}", ocr_iou_threshold_,
         avatar_iou_threshold_);
}

std::pair<std::vector<utils::OCRItem>, std::vector<utils::AvatarItem>> Deduplicator::deduplicate(
    const std::vector<utils::OCRItem>& ocr_items, const std::vector<utils::AvatarItem>& avatar_items) {
    LOGD("[Deduplicator] Starting deduplication...");

    // OCR识别结果去重
    std::vector<utils::OCRItem> deduplicated_ocr = deduplicate_ocr(ocr_items);
    LOGD("[Deduplicator] OCR dedup: {} -> {}", ocr_items.size(), deduplicated_ocr.size());

    // 头像检测结果去重
    std::vector<utils::AvatarItem> deduplicated_avatars = deduplicate_avatars(avatar_items);
    //     "
    LOGD("[Deduplicator] Avatar dedup: {} -> {}", avatar_items.size(), deduplicated_avatars.size());

    return std::make_pair(deduplicated_ocr, deduplicated_avatars);
}

std::vector<utils::OCRItem> Deduplicator::deduplicate_ocr(const std::vector<utils::OCRItem>& ocr_items) {
    if (ocr_items.empty()) {
        return {};
    }

    std::vector<utils::OCRItem> deduplicated;

    for (const auto& current_item : ocr_items) {
        bool is_duplicate = false;

        // 与已添加的项比较
        for (size_t i = 0; i < deduplicated.size(); ++i) {
            float iou = calculateBoxIou(current_item.box, deduplicated[i].box);

            if (iou > ocr_iou_threshold_) {
                is_duplicate = true;
                // 保留置信度更高的结果
                if (current_item.score > deduplicated[i].score) {
                    deduplicated[i] = current_item;
                }
                break;
            }
        }

        if (!is_duplicate) {
            deduplicated.push_back(current_item);
        }
    }

    return deduplicated;
}

std::vector<utils::AvatarItem> Deduplicator::deduplicate_avatars(const std::vector<utils::AvatarItem>& avatar_items) {
    if (avatar_items.empty()) {
        return {};
    }

    std::vector<utils::AvatarItem> deduplicated;

    for (const auto& current_item : avatar_items) {
        bool is_duplicate = false;

        // 检查是否与已存在的头像重复
        for (size_t i = 0; i < deduplicated.size(); ++i) {
            float iou = calculateBoxIou(current_item.box, deduplicated[i].box);

            if (iou > avatar_iou_threshold_) {
                is_duplicate = true;

                // 保留面积更大的头像
                if (current_item.area() > deduplicated[i].area()) {
                    deduplicated[i] = current_item;
                }
                break;
            }
        }

        if (!is_duplicate) {
            deduplicated.push_back(current_item);
        }
    }

    return deduplicated;
}

float Deduplicator::calculateIouFromBounds(const BoundingBoxMinMax& b1, const BoundingBoxMinMax& b2) {
    // 计算交集
    float inter_x_min = std::max(b1.min_x, b2.min_x);
    float inter_y_min = std::max(b1.min_y, b2.min_y);
    float inter_x_max = std::min(b1.max_x, b2.max_x);
    float inter_y_max = std::min(b1.max_y, b2.max_y);

    if (inter_x_max <= inter_x_min || inter_y_max <= inter_y_min) {
        return 0.0f;
    }

    float inter_area = (inter_x_max - inter_x_min + 1) * (inter_y_max - inter_y_min + 1);

    // 计算并集
    float area1 = (b1.max_x - b1.min_x + 1) * (b1.max_y - b1.min_y + 1);
    float area2 = (b2.max_x - b2.min_x + 1) * (b2.max_y - b2.min_y + 1);
    float union_area = area1 + area2 - inter_area;

    return union_area > 0 ? inter_area / union_area : 0.0f;
}

float Deduplicator::calculateBoxIou(const std::vector<std::vector<float>>& box1,
                                    const std::vector<std::vector<float>>& box2) {
    try {
        return calculateIouFromBounds(getBoxBounds(box1), getBoxBounds(box2));
    } catch (const std::exception& e) {
        LOGE("[Error] Error calculating IOU: {}", e.what());
        return 0.0f;
    }
}

float Deduplicator::calculateBoxIou(const std::tuple<float, float, float, float>& box1,
                                    const std::tuple<float, float, float, float>& box2) {
    try {
        return calculateIouFromBounds(getBoxBounds(box1), getBoxBounds(box2));
    } catch (const std::exception& e) {
        LOGE("[Error] Error calculating IOU: {}", e.what());
        return 0.0f;
    }
}

BoundingBoxMinMax Deduplicator::getBoxBounds(const std::vector<std::vector<float>>& box) {
    // OCR格式: [[x1,y1], [x2,y2], [x3,y3], [x4,y4]]
    std::vector<float> x_coords, y_coords;

    for (const auto& point : box) {
        if (point.size() >= 2) {
            x_coords.push_back(point[0]);
            y_coords.push_back(point[1]);
        }
    }

    float min_x = *std::min_element(x_coords.begin(), x_coords.end());
    float min_y = *std::min_element(y_coords.begin(), y_coords.end());
    float max_x = *std::max_element(x_coords.begin(), x_coords.end());
    float max_y = *std::max_element(y_coords.begin(), y_coords.end());

    return BoundingBoxMinMax(min_x, min_y, max_x, max_y);
}

BoundingBoxMinMax Deduplicator::getBoxBounds(const std::tuple<float, float, float, float>& box) {
    // xywh格式: (x, y, w, h)
    float x = std::get<0>(box);
    float y = std::get<1>(box);
    float w = std::get<2>(box);
    float h = std::get<3>(box);

    // 转换为闭区间：max = x + w - 1（w像素从x开始，到x+w-1结束）
    return BoundingBoxMinMax(x, y, x + w - 1, y + h - 1);
}

}  // namespace detection

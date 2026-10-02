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

#include <tuple>
#include <vector>

#include "../utils/config.h"
#include "../utils/ocravatartypes.h"

namespace detection {

/**
 * 边界框结构，用于IOU计算(使用min/max坐标格式)
 */
struct BoundingBoxMinMax {
    float min_x;
    float min_y;
    float max_x;
    float max_y;

    BoundingBoxMinMax(float min_x, float min_y, float max_x, float max_y)
        : min_x(min_x), min_y(min_y), max_x(max_x), max_y(max_y) {}
};

/**
 * 去重处理器类
 */
class Deduplicator {
private:
    const utils::Config& config_;  // 配置引用
    float ocr_iou_threshold_;      // OCR去重的IOU阈值
    float avatar_iou_threshold_;   // 头像去重的IOU阈值

    /**
     * 计算两个OCR边界框的IOU
     * @param box1 边界框1 (OCR格式: [[x1,y1], [x2,y2], [x3,y3], [x4,y4]])
     * @param box2 边界框2 (OCR格式: [[x1,y1], [x2,y2], [x3,y3], [x4,y4]])
     * @return IOU值 (0-1)
     */
    float calculateBoxIou(const std::vector<std::vector<float>>& box1, const std::vector<std::vector<float>>& box2);

    /**
     * 计算两个Avatar边界框的IOU
     * @param box1 边界框1 (xywh格式: (x, y, w, h))
     * @param box2 边界框2 (xywh格式: (x, y, w, h))
     * @return IOU值 (0-1)
     */
    float calculateBoxIou(const std::tuple<float, float, float, float>& box1,
                          const std::tuple<float, float, float, float>& box2);

    /**
     * 从BoundingBoxMinMax计算IOU（内部实现）
     * @param b1 边界框1
     * @param b2 边界框2
     * @return IOU值 (0-1)
     */
    float calculateIouFromBounds(const BoundingBoxMinMax& b1, const BoundingBoxMinMax& b2);

    /**
     * 从OCR格式的边界框获取边界值
     * @param box OCR格式边界框 [[x1,y1], [x2,y2], [x3,y3], [x4,y4]]
     * @return 边界框结构
     */
    BoundingBoxMinMax getBoxBounds(const std::vector<std::vector<float>>& box);

    /**
     * 从xywh格式的边界框获取边界值
     * @param box xywh格式边界框 (x, y, w, h)
     * @return 边界框结构
     */
    BoundingBoxMinMax getBoxBounds(const std::tuple<float, float, float, float>& box);

    /**
     * OCR结果去重
     * @param ocr_items OCR项列表
     * @return 去重后的OCR项列表
     */
    std::vector<utils::OCRItem> deduplicate_ocr(const std::vector<utils::OCRItem>& ocr_items);

    /**
     * 头像位置去重
     * @param avatar_items 头像项列表
     * @return 去重后的头像项列表
     */
    std::vector<utils::AvatarItem> deduplicate_avatars(const std::vector<utils::AvatarItem>& avatar_items);

public:
    /**
     * 构造函数
     * @param config 配置对象
     */
    explicit Deduplicator(const utils::Config& config);

    /**
     * 对OCR结果和头像位置进行去重
     * @param ocr_items OCR识别项列表
     * @param avatar_items 头像项列表
     * @return std::pair<去重后的OCR项列表, 去重后的头像项列表>
     */
    std::pair<std::vector<utils::OCRItem>, std::vector<utils::AvatarItem>> deduplicate(
        const std::vector<utils::OCRItem>& ocr_items, const std::vector<utils::AvatarItem>& avatar_items);
};

}  // namespace detection

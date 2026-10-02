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

#include <jsoncpp/json/json.h>

#include <memory>
#include <string>
#include <tuple>
#include <vector>

/**
 * OCR结果数据模型
 */

namespace utils {

/**
 * 单个OCR识别
 */
struct OCRItem {
    std::string text;
    std::vector<std::vector<float>> box;  // [[x1,y1], [x2,y2], [x3,y3], [x4,y4]]
    double score;
    int slice_index;
    std::string original_text;
    bool is_virtual;

    // 构造函数
    OCRItem() : score(0.0), slice_index(0), original_text(""), is_virtual(false) {}

    OCRItem(const std::string& text, const std::vector<std::vector<float>>& box, double score, int slice_index,
            const std::string& original_text = "", bool is_virtual = false)
        : text(text),
          box(box),
          score(score),
          slice_index(slice_index),
          original_text(original_text),
          is_virtual(is_virtual) {}

    double get_center_y() const;
    double get_min_y() const;
    double get_max_y() const;
    double get_min_x() const;
    double get_max_x() const;

    Json::Value to_dict() const;
};

/**
 * 头像检测项
 */
struct AvatarItem {
    std::tuple<float, float, float, float> box;
    int slice_index;

    // 构造函数
    AvatarItem() : box(std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f)), slice_index(0) {}

    AvatarItem(const std::tuple<float, float, float, float>& box, int slice_index)
        : box(box), slice_index(slice_index) {}

    float x() const { return std::get<0>(box); }
    float y() const { return std::get<1>(box); }
    float width() const { return std::get<2>(box); }
    float height() const { return std::get<3>(box); }
    float center_x() const { return x() + width() / 2; }
    float center_y() const { return y() + height() / 2; }
    float area() const { return width() * height(); }

    Json::Value to_dict() const;
};

/**
 * 切片OCR处理结果
 */
class SliceOcrResult {
public:
    int slice_index;
    int start_y;
    int end_y;
    std::vector<OCRItem> ocr_items;
    std::vector<AvatarItem> avatar_items;

    // 构造函数
    SliceOcrResult() : slice_index(0), start_y(0), end_y(0) {}

    SliceOcrResult(int slice_index, int start_y, int end_y)
        : slice_index(slice_index), start_y(start_y), end_y(end_y) {}

    OCRItem& add_ocr_item(const std::string& text, const std::vector<std::vector<float>>& box, double score,
                          bool is_virtual = false);

    AvatarItem& add_avatar_item(const std::tuple<float, float, float, float>& box);

    void sort_by_y();  // 按Y坐标排序

    Json::Value to_dict() const;
};

}  // namespace utils

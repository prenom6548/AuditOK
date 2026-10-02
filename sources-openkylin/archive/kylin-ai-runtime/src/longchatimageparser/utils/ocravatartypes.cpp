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

#include "ocravatartypes.h"

#include <algorithm>
#include <numeric>

namespace utils {

// OCRItem 实现

double OCRItem::get_center_y() const {
    /**
     * 获取中心Y坐标
     */
    double sum_y = 0.0;
    for (const auto& pt : box) {
        if (pt.size() > 1) {
            sum_y += pt[1];
        }
    }
    return sum_y / 4.0;
}

double OCRItem::get_min_y() const {
    /**
     * 获取最小Y坐标
     */
    if (box.size() > 3 && box[3].size() > 1) {
        return box[3][1];
    }
    return 0.0;
}

double OCRItem::get_max_y() const {
    /**
     * 获取最大Y坐标
     */
    double max_y = 0.0;
    for (const auto& pt : box) {
        if (pt.size() > 1) {
            max_y = std::max(max_y, static_cast<double>(pt[1]));
        }
    }
    return max_y;
}

double OCRItem::get_min_x() const {
    /**
     * 获取最小X坐标
     */
    if (box.empty() || box[0].empty()) return 0.0;

    double min_x = box[0][0];
    for (const auto& pt : box) {
        if (pt.size() > 0) {
            min_x = std::min(min_x, static_cast<double>(pt[0]));
        }
    }
    return min_x;
}

double OCRItem::get_max_x() const {
    /**
     * 获取最大X坐标
     */
    double max_x = 0.0;
    for (const auto& pt : box) {
        if (pt.size() > 0) {
            max_x = std::max(max_x, static_cast<double>(pt[0]));
        }
    }
    return max_x;
}

Json::Value OCRItem::to_dict() const {
    /**
     * 转换为字典格式
     */
    Json::Value j;
    j["text"] = text;

    // 转换box到JSON数组
    Json::Value box_json(Json::arrayValue);
    for (const auto& row : box) {
        Json::Value row_json(Json::arrayValue);
        for (float val : row) {
            row_json.append(val);
        }
        box_json.append(row_json);
    }
    j["box"] = box_json;

    j["score"] = score;
    j["slice_index"] = slice_index;
    if (!original_text.empty()) {
        j["original_text"] = original_text;
    } else {
        j["original_text"] = Json::Value(Json::nullValue);
    }
    j["virtual"] = is_virtual;
    return j;
}

// AvatarItem 实现

Json::Value AvatarItem::to_dict() const {
    /**
     * 转换为字典格式
     */
    Json::Value j;
    // 将tuple转换为数组格式
    Json::Value box_array(Json::arrayValue);
    box_array.append(std::get<0>(box));
    box_array.append(std::get<1>(box));
    box_array.append(std::get<2>(box));
    box_array.append(std::get<3>(box));
    j["box"] = box_array;
    j["slice_index"] = slice_index;
    j["center_x"] = center_x();
    j["center_y"] = center_y();
    return j;
}

// SliceOcrResult 实现

OCRItem& SliceOcrResult::add_ocr_item(const std::string& text, const std::vector<std::vector<float>>& box, double score,
                                      bool is_virtual) {
    /**
     * 添加OCR项
     */
    OCRItem item(text, box, score, slice_index, "", is_virtual);
    ocr_items.push_back(item);
    return ocr_items.back();
}

AvatarItem& SliceOcrResult::add_avatar_item(const std::tuple<float, float, float, float>& box) {
    /**
     * 添加头像项
     */
    AvatarItem item(box, slice_index);
    avatar_items.push_back(item);
    return avatar_items.back();
}

void SliceOcrResult::sort_by_y() {
    /**
     * 按Y坐标排序 - 使用中心Y坐标
     */
    std::sort(ocr_items.begin(), ocr_items.end(),
              [](const OCRItem& a, const OCRItem& b) { return a.get_center_y() < b.get_center_y(); });

    std::sort(avatar_items.begin(), avatar_items.end(),
              [](const AvatarItem& a, const AvatarItem& b) { return a.center_y() < b.center_y(); });
}

Json::Value SliceOcrResult::to_dict() const {
    /**
     * 转换为字典格式
     */
    Json::Value j;
    j["slice_index"] = slice_index;
    j["start_y"] = start_y;
    j["end_y"] = end_y;

    // 转换ocr_items
    Json::Value ocr_items_json = Json::Value(Json::arrayValue);
    for (const auto& item : ocr_items) {
        ocr_items_json.append(item.to_dict());
    }
    j["ocr_items"] = ocr_items_json;

    // 转换avatar_items
    Json::Value avatar_items_json = Json::Value(Json::arrayValue);
    for (const auto& item : avatar_items) {
        avatar_items_json.append(item.to_dict());
    }
    j["avatar_items"] = avatar_items_json;

    return j;
}

}  // namespace utils

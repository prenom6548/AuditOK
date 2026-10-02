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

#include <opencv2/opencv.hpp>
#include <tuple>
#include <vector>

namespace utils {

/**
 * SliceInfo - 切片信息数据模型
 */
class SliceInfo {
public:
    int slice_index;  // 切片索引
    int start_y;      // 起始Y坐标
    int end_y;        // 结束Y坐标

    // 图像数据：使用OpenCV Mat
    cv::Mat image;

    // 构造函数
    SliceInfo() = default;

    SliceInfo(int slice_index, int start_y, int end_y, const cv::Mat& image = cv::Mat());

    /**
     * 获取切片高度
     * @return 切片高度 (end_y - start_y)
     */
    int height() const;

    /**
     * 获取图像形状
     * @return tuple<height, width, channels>
     */
    std::tuple<int, int, int> shape() const;

    /**
     * 判断Y坐标是否在切片范围内
     * @param y Y坐标
     * @return 是否在范围内
     */
    bool contains_y(double y) const;

    /**
     * 转换为字典格式（不包含图像数据）
     * @return JSON对象
     */
    Json::Value to_dict() const;

    // 检查是否有图像数据
    bool has_image_data() const;
};

}  // namespace utils

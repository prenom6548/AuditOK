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

#include "sliceinfo.h"

namespace utils {

// SliceInfo 实现

SliceInfo::SliceInfo(int slice_index, int start_y, int end_y, const cv::Mat& image)
    : slice_index(slice_index), start_y(start_y), end_y(end_y), image(image) {}

int SliceInfo::height() const { return end_y - start_y; }

std::tuple<int, int, int> SliceInfo::shape() const {
    if (image.empty()) {
        return std::make_tuple(0, 0, 0);
    }

    int height = image.rows;
    int width = image.cols;
    int channels = image.channels();

    return std::make_tuple(height, width, channels);
}

bool SliceInfo::contains_y(double y) const { return start_y <= y && y <= end_y; }

Json::Value SliceInfo::to_dict() const {
    Json::Value result;
    result["slice_index"] = slice_index;
    result["start_y"] = start_y;
    result["end_y"] = end_y;
    result["height"] = height();

    return result;
}

bool SliceInfo::has_image_data() const { return !image.empty(); }

}  // namespace utils

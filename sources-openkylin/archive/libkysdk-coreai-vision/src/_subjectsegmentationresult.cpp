/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include "_subjectsegmentationresult.h"

#include "subjectsegmentationresult.h"

// Get request id
const char *subject_segmentation_result_get_request_id(const SubjectSegmentationResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    if (result->requestId.empty()) {
        return nullptr;
    }
    return result->requestId.c_str();
}

// 获取前景掩码数据
const SegmentationMaskData *subject_segmentation_result_get_mask_data(const SubjectSegmentationResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    if (result->maskDataPtr == nullptr) {
        return nullptr;
    }
    return result->maskDataPtr.get();
}

// 获取前景图像数据
const unsigned char *subject_segmentation_result_get_foreground_image_data(const SubjectSegmentationResult *result,
                                                                           int *width, int *height,
                                                                           size_t *data_length) {
    if (result == nullptr) {
        return nullptr;
    }
    if (width == nullptr || height == nullptr || data_length == nullptr) {
        return nullptr;
    }
    if (result->imageData.empty()) {
        return nullptr;
    }
    *width = result->width;
    *height = result->height;
    *data_length = result->imageData.size();
    return result->imageData.data();
}

// 获取掩码格式
const char *subject_segmentation_result_get_foreground_image_format(const SubjectSegmentationResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    if (result->format.empty()) {
        return nullptr;
    }
    return result->format.c_str();
}

// 获取边缘点组
const SubjectSegmentationPointGroup *subject_segmentation_result_get_edge_point_groups(
    const SubjectSegmentationResult *result, int *group_counts) {
    if (result == nullptr) {
        return nullptr;
    }
    if (group_counts == nullptr) {
        return nullptr;
    }
    if (result->pointGroups.empty()) {
        return nullptr;
    }
    *group_counts = result->pointGroups.size();
    return result->pointGroups.data();
}

// 获取错误码
int subject_segmentation_result_get_error_code(const SubjectSegmentationResult *result) {
    if (result == nullptr) {
        return -1;
    }
    return result->errorCode;
}

// 获取错误信息
const char *subject_segmentation_result_get_error_message(const SubjectSegmentationResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->errorMessage.c_str();
}

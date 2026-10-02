/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "_segmentationresult.h"

#include <kylin-ai/coreai/vision/error.h>

#include <cstdint>
#include <string>
#include <vector>

#include "segmentationresult.h"

const SegmentationMaskData *segmentation_result_get_mask_data(SegmentationResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    result->mask_.mask_data = result->data_.data();
    result->mask_.mask_length = result->data_.size();
    return &result->mask_;
}

int segmentation_result_get_error_code(SegmentationResult *result) {
    if (result == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    return result->errorCode;
}

const char *segmentation_result_get_error_message(SegmentationResult *result) {
    if (result == nullptr) {
        return nullptr;
    }
    return result->errorMessage.c_str();
}

/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#include "_imageresult.h"

#include "imageresult.h"

ImageFormat image_result_get_format(VisionImageResult *image_result) {
    if (image_result == nullptr) {
        return ImageFormat::IMAGE_FORMAT_UNKNOWN;
    }
    return image_result->format;
}

ImageSize image_result_get_size(VisionImageResult *image_result) {
    if (image_result == nullptr) {
        return ImageSize{-1, -1};
    }
    return ImageSize{image_result->width, image_result->height};
}

int image_result_get_total(VisionImageResult *image_result) {
    if (image_result == nullptr) {
        return -1;
    }
    return image_result->total;
}

int image_result_get_index(VisionImageResult *image_result) {
    if (image_result == nullptr) {
        return -1;
    }
    return image_result->index;
}

const uint8_t *image_result_get_data(VisionImageResult *image_result, int *data_length) {
    if (image_result == nullptr || data_length == nullptr) {
        return nullptr;
    }
    *data_length = image_result->data.size();
    return image_result->data.data();
}

int image_result_get_error_code(VisionImageResult *image_result) {
    if (image_result == nullptr) {
        return -1;
    }
    return image_result->errorCode;
}

const char *image_result_get_error_message(VisionImageResult *image_result) {
    if (image_result == nullptr) {
        return nullptr;
    }
    return image_result->errorMessage.c_str();
}

const char *image_result_get_request_id(VisionImageResult *image_result) {
    if (image_result == nullptr) {
        return nullptr;
    }
    return image_result->requestId.c_str();
}

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
#include "segmentation.h"

#include "_segmentationsession.h"
#include "logger.h"
#include "util.h"

namespace {
using SegmentationSessionPtr = kyai::core::vision::_SegmentationSession *;
}

SegmentationSession *segmentation_create_session() {
    return reinterpret_cast<SegmentationSession *>(new kyai::core::vision::_SegmentationSession());
}

void segmentation_destroy_session(SegmentationSession **session) {
    if (session == nullptr) {
        return;
    }
    if (*session == nullptr) {
        return;
    }
    SegmentationSessionPtr(*session)->destroy();
    delete SegmentationSessionPtr(*session);
    *session = nullptr;
}

void segmentation_init_session_async(SegmentationSession *session, SegmentationInitCallback callback, void *user_data) {
    if (session == nullptr) {
        if (callback) {
            callback(AI_COMMON_INVALID_SESSION, user_data);
        }
        return;
    }
    SegmentationSessionPtr(session)->initAsync(callback, user_data);
}

void segmentation_result_set_callback(SegmentationSession *session, SegmentationResultCallback callback,
                                      void *user_data) {
    if (session == nullptr) {
        LOGE("Session is null.");
        return;
    }
    SegmentationSessionPtr(session)->setSegmentationResultCallback(callback, user_data);
}

void segmentation_encode_image_file_async(SegmentationSession *session, const char *image_file,
                                          SegmentationEncodeCallback callback, void *user_data) {
    if (session == nullptr) {
        callback(AI_COMMON_INVALID_SESSION, user_data);
        LOGE("Session is null.");
        return;
    }
    if (image_file == nullptr || !util::fileExists(image_file)) {
        callback(AI_COMMON_FILE_NOT_FOUND, user_data);
        LOGE("The image file is empty or does not exist.");
        return;
    }
    if (!util::hasReadPermission(image_file)) {
        callback(AI_COMMON_PERMISSION_DENIED, user_data);
        LOGE("The image file does not have read permission.");
        return;
    }
    SegmentationSessionPtr(session)->encodeImageFileAsync(image_file, callback, user_data);
}

void segmentation_encode_image_data_async(SegmentationSession *session, const char *image_data,
                                          size_t image_data_length, SegmentationEncodeCallback callback,
                                          void *user_data) {
    if (session == nullptr) {
        callback(AI_COMMON_INVALID_SESSION, user_data);
        LOGE("Session is null.");
        return;
    }
    if (image_data == nullptr || image_data_length == 0) {
        callback(AI_COMMON_INVALID_PARAMETER, user_data);
        LOGE("Image file is null.");
        return;
    }
    std::vector<uint8_t> imageData(image_data, image_data + image_data_length);
    SegmentationSessionPtr(session)->encodeImageDataAsync(imageData, callback, user_data);
}

int segmentation_clear_cache(SegmentationSession *session) {
    if (session == nullptr) {
        LOGE("Session is null.");
        return AI_COMMON_INVALID_SESSION;
    }
    return SegmentationSessionPtr(session)->clearCache();
}

void segmentation_run_async(SegmentationSession *session, const SegmentationPixelSelection *pixel_points,
                            int min_mask_region_area) {
    if (session == nullptr) {
        LOGE("Session is null.");
        return;
    }
    if (min_mask_region_area < 0) {
        min_mask_region_area = 0;
    }
    std::vector<PixelPoint> points{};
    if (pixel_points != nullptr) {
        points = {pixel_points->points, pixel_points->points + pixel_points->count};
    }
    SegmentationSessionPtr(session)->runAsync(points, min_mask_region_area);
}

void segmentation_enable_internal_event_loop(SegmentationSession *session, bool enable) {
    if (session == nullptr) {
        LOGE("Session is null.");
        return;
    }
    SegmentationSessionPtr(session)->enableInternalEventLoop(enable);
}

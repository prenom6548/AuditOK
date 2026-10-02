/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
 later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

//
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
//
#include <opencv2/opencv.hpp>
//
#include <kylin-ai/common/error.h>
//
#include "logger.h"
#include "segmentation.h"

void segmentationResultCallback(SegmentationResult *result, void *user_data) {
    if (result == nullptr) {
        return;
    }
    int errorCode = segmentation_result_get_error_code(result);
    std::string errorMessage = segmentation_result_get_error_message(result);
    if (errorCode != AI_COMMON_SUCCESS) {
        LOGE("error info:", errorCode, errorMessage);
        return;
    }
    const SegmentationMaskData *data = segmentation_result_get_mask_data(result);
    if (data == nullptr) {
        return;
    }
    if (data->mask_height == 0) {
        return;
    }
    if (data->mask_width == 0) {
        return;
    }
    if (data->mask_data == nullptr) {
        return;
    }
    if (data->mask_length == 0) {
        return;
    }
    if (data->mask_width * data->mask_height != data->mask_length) {
        return;
    }
    LOGD(data->mask_length, data->mask_height, data->mask_width);
    cv::Mat mask(data->mask_height, data->mask_width, CV_8UC1, data->mask_data);
    bool success = cv::imwrite("mask_output.png", mask);
    if (!success) {
        LOGE("Failed to save PNG!");
        return;
    }
    LOGI("Succeeded to save PNG!");
}

void segmentationEncodeCallback(int error_code, void *user_data) { LOGI("encode error code", error_code); }

void segmentationInitCallback(int error_code, void *user_data) { LOGI("init error code", error_code); }

void testSegmentationFromImageFile() {
    namespace fs = std::filesystem;
    fs::path path = fs::path(__FILE__).parent_path() / "test2.png";
    SegmentationSession *session = segmentation_create_session();
    segmentation_enable_internal_event_loop(session, true);

    segmentation_init_session_async(session, segmentationInitCallback, nullptr);
    LOGI("init");
    while (std::getchar() != '\n');

    segmentation_result_set_callback(session, segmentationResultCallback, nullptr);
    segmentation_encode_image_file_async(session, path.c_str(), segmentationEncodeCallback, nullptr);
    while (std::getchar() != '\n');
    std::vector<PixelPoint> points{{546, 218}, {832, 207}, {739, 219}, {876, 255}};
    SegmentationPixelSelection selection{points.data(), points.size()};
    segmentation_run_async(session, &selection, 0);
    LOGI("run async");
    while (std::getchar() != '\n');

    points = {{669, 390}, {815, 356}, {588, 518}, {769, 616}};
    selection = {points.data(), points.size()};
    segmentation_run_async(session, &selection, 0);
    LOGI("run async");
    while (std::getchar() != '\n');

    int errorCode = segmentation_clear_cache(session);
    LOGI("clear", errorCode);
    while (std::getchar() != '\n');

    points = {{546, 218}, {832, 207}, {739, 219}, {876, 255}};
    selection = {points.data(), points.size()};
    segmentation_run_async(session, &selection, 0);
    LOGI("run async");
    while (std::getchar() != '\n');

    segmentation_destroy_session(&session);
    LOGI("destroy session");
}

std::vector<char> readAudioData(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file:", filePath);
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> audioData(fileSize);
    file.read(audioData.data(), fileSize);
    return audioData;
}

void testSegmentationFromImageData() {
    namespace fs = std::filesystem;
    fs::path path = fs::path(__FILE__).parent_path() / "test2.png";
    std::vector<char> imageData = readAudioData(path.string());
    SegmentationSession *session = segmentation_create_session();
    segmentation_enable_internal_event_loop(session, true);

    segmentation_init_session_async(session, segmentationInitCallback, nullptr);
    LOGI("init");
    while (std::getchar() != '\n');

    segmentation_result_set_callback(session, segmentationResultCallback, nullptr);
    segmentation_encode_image_data_async(session, imageData.data(), imageData.size(), segmentationEncodeCallback,
                                         nullptr);
    while (std::getchar() != '\n');
    std::vector<PixelPoint> points{{546, 218}, {832, 207}, {739, 219}, {876, 255}};
    SegmentationPixelSelection selection{points.data(), points.size()};
    segmentation_run_async(session, &selection, 0);
    LOGI("run async");
    while (std::getchar() != '\n');

    points = {{669, 390}, {815, 356}, {588, 518}, {769, 616}};
    selection = {points.data(), points.size()};
    segmentation_run_async(session, &selection, 0);
    LOGI("run async");
    while (std::getchar() != '\n');

    int errorCode = segmentation_clear_cache(session);
    LOGI("clear", errorCode);
    while (std::getchar() != '\n');

    points = {{546, 218}, {832, 207}, {739, 219}, {876, 255}};
    selection = {points.data(), points.size()};
    segmentation_run_async(session, &selection, 0);
    LOGI("run async");
    while (std::getchar() != '\n');

    segmentation_destroy_session(&session);
    LOGI("destroy session");
}

int main() {
    testSegmentationFromImageFile();
    testSegmentationFromImageData();
    return 0;
}

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

#include "imageslicer.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "../../utils/logger.h"

namespace parser {

namespace {
cv::Mat applyMildBackgroundBlur(const cv::Mat& input) {
    if (input.empty()) {
        return input;
    }

    cv::Mat input_bgr;
    cv::Mat alpha_channel;
    if (input.channels() == 4) {
        cv::cvtColor(input, input_bgr, cv::COLOR_BGRA2BGR);
        std::vector<cv::Mat> channels;
        cv::split(input, channels);
        alpha_channel = channels[3];
    } else {
        input_bgr = input.clone();
    }

    cv::Mat gray;
    cv::cvtColor(input_bgr, gray, cv::COLOR_BGR2GRAY);

    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);

    cv::Mat dilated;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9));
    cv::dilate(edges, dilated, kernel);

    cv::Mat mask;
    cv::GaussianBlur(dilated, mask, cv::Size(15, 15), 0);

    cv::Mat blurred;
    cv::GaussianBlur(input_bgr, blurred, cv::Size(13, 13), 2.2);

    cv::Mat alpha;
    mask.convertTo(alpha, CV_32F, 1.0 / 255.0);
    std::vector<cv::Mat> alpha_channels(3, alpha);
    cv::Mat alpha3;
    cv::merge(alpha_channels, alpha3);

    cv::Mat blurred_f;
    cv::Mat input_f;
    blurred.convertTo(blurred_f, CV_32F);
    input_bgr.convertTo(input_f, CV_32F);

    cv::Mat inv_alpha3;
    cv::subtract(cv::Scalar(1.0f, 1.0f, 1.0f), alpha3, inv_alpha3);

    cv::Mat blended_f;
    cv::multiply(blurred_f, inv_alpha3, blurred_f);
    cv::multiply(input_f, alpha3, input_f);
    cv::add(blurred_f, input_f, blended_f);
    cv::Mat out_bgr;
    blended_f.convertTo(out_bgr, input_bgr.type());

    if (input.channels() == 4 && !alpha_channel.empty()) {
        cv::Mat out_bgra;
        cv::cvtColor(out_bgr, out_bgra, cv::COLOR_BGR2BGRA);
        std::vector<cv::Mat> out_channels;
        cv::split(out_bgra, out_channels);
        out_channels[3] = alpha_channel;
        cv::merge(out_channels, out_bgra);
        return out_bgra;
    }

    return out_bgr;
}

}  // namespace

ImageSlicer::ImageSlicer(const utils::Config& config)
    : config_(config),
      slice_height_(config.image.slice_height),
      overlap_(config.image.overlap),
      output_dir_(config.output.output_images_dir) {
// 创建输出目录（仅在Debug模式下）
#ifdef DEBUG_MODE
    if (config_.shouldSaveOutputFiles()) {
        try {
            if (!std::filesystem::exists(output_dir_)) {
                std::filesystem::create_directories(output_dir_);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            LOGE("Warning: Failed to create output directory: {}", e.what());
        }
    }
#endif
}

std::tuple<cv::Mat, std::vector<utils::SliceInfo>> ImageSlicer::sliceImage(const std::string& image_path) {
    // 读取原始图像
    cv::Mat original_image = cv::imread(image_path, cv::IMREAD_COLOR);
    if (original_image.empty()) {
        std::string error_msg = diagnoseImageReadFailure(image_path);
        LOGE("Cannot read image: {}, reason: {}", image_path, error_msg);
        throw std::runtime_error("Cannot read image: " + image_path + ". " + error_msg);
    }

    // Apply a mild background blur while preserving foreground edges before slicing.
    original_image = applyMildBackgroundBlur(original_image);

    int height = original_image.rows;
    int width = original_image.cols;
    LOGD("Original image size: {} x {}", width, height);

    // 如果图像高度小于等于切片高度，不需要切分
    if (height <= slice_height_) {
        utils::SliceInfo slice_info(0, 0, height, original_image);
#ifdef DEBUG_MODE
        saveSlice(slice_info);
#endif
        return std::make_tuple(original_image, std::vector<utils::SliceInfo>{slice_info});
    }

    // 切分图像
    std::vector<utils::SliceInfo> slice_infos = performSlicing(original_image);

    LOGD("Total slices {} slices", slice_infos.size());
    return std::make_tuple(original_image, slice_infos);
}

std::vector<utils::SliceInfo> ImageSlicer::performSlicing(const cv::Mat& image) {
    int height = image.rows;
    int width = image.cols;
    std::vector<utils::SliceInfo> slice_infos;
    int current_y = 0;
    int slice_index = 0;

    while (current_y < height) {
        // 计算当前切片的结束位置
        int end_y = std::min(current_y + slice_height_, height);

        // 提取切片
        cv::Mat slice_img = image(cv::Rect(0, current_y, width, end_y - current_y)).clone();

        // 创建切片信息
        utils::SliceInfo slice_info(slice_index, current_y, end_y, slice_img);
        slice_infos.push_back(slice_info);
#ifdef DEBUG_MODE
        // Saved slice图像
        saveSlice(slice_info);
#endif
        // 计算下一slices的起始位置
        if (end_y >= height) {
            break;
        }
        current_y = end_y - overlap_;
        slice_index++;
    }

    return slice_infos;
}
#ifdef DEBUG_MODE
void ImageSlicer::saveSlice(const utils::SliceInfo& slice_info) {
    // 仅在Debug模式下Saved slice图像文件
    if (!config_.shouldSaveOutputFiles()) {
        // Release模式下不保存文件
        return;
    }

    // 生成切片文件名，使用3位数格式（001, 002, ...）
    char filename[20];
    std::sprintf(filename, "slice_%03d.jpg", slice_info.slice_index);

    std::filesystem::path slice_path = output_dir_ / filename;

    // 现在SliceInfo.image直接是cv::Mat格式
    const cv::Mat& slice_mat = slice_info.image;

    if (!cv::imwrite(slice_path.string(), slice_mat)) {
        LOGE("Warning: Failed to save slice {} to {}", slice_info.slice_index, slice_path.string());
    } else {
        LOGD("[DEBUG] Saved slice {}: {}", slice_info.slice_index, slice_path.string());
    }
}
#endif

std::vector<utils::SliceInfo> ImageSlicer::getSlicesToProcess(const std::vector<utils::SliceInfo>& slice_infos) {
    size_t total_slices = slice_infos.size();

    if (total_slices == 1) {
        // 只有一slices，处理所有切片
        LOGD("Only one slice, processing all");
        return slice_infos;
    } else if (total_slices == 2) {
        // 两slices，选择第一slices
        LOGD("2 slices, processing first one only");
        return std::vector<utils::SliceInfo>(slice_infos.begin(), slice_infos.begin() + 1);
    } else {
        // 大于等于3slices，排除开始和结束的切片，只处理中间切片
        std::vector<utils::SliceInfo> middle_slices(slice_infos.begin() + 1, slice_infos.end() - 1);
        //     middle_slices.size()
        LOGD("Total {} slices, processing {} middle slices", total_slices, middle_slices.size());
        return middle_slices;
    }
}

std::string ImageSlicer::diagnoseImageReadFailure(const std::string& image_path) {
    // 检查文件是否存在
    if (!std::filesystem::exists(image_path)) {
        return "File does not exist, please check the path";
    }

    // 检查是否是文件(不是目录)
    if (!std::filesystem::is_regular_file(image_path)) {
        return "Path does not point to a file";
    }

    // 检查文件大小
    std::error_code ec;
    auto file_size = std::filesystem::file_size(image_path, ec);
    if (ec || file_size == 0) {
        return "File is empty";
    }

    // 检查文件扩展名
    std::filesystem::path path_obj(image_path);
    std::string file_ext = path_obj.extension().string();
    std::transform(file_ext.begin(), file_ext.end(), file_ext.begin(), ::tolower);

    std::vector<std::string> supported_formats = {".png", ".jpg", ".jpeg", ".bmp", ".tiff", ".tif"};
    if (std::find(supported_formats.begin(), supported_formats.end(), file_ext) == supported_formats.end()) {
        return "Unsupported file format: " + file_ext + ", supported formats: .png, .jpg, .jpeg, .bmp, .tiff, .tif";
    }

    // 检查文件是否损坏(尝试读取文件头)
    try {
        std::ifstream file(image_path, std::ios::binary);
        if (!file.is_open()) {
            return "Cannot open file for reading";
        }

        char header[10];
        file.read(header, sizeof(header));
        if (file.gcount() < 4) {
            return "File header incomplete, file may be corrupted";
        }
        file.close();
    } catch (const std::exception& e) {
        return "Cannot read file: " + std::string(e.what());
    }

    return "File format may not be supported by OpenCV or file is corrupted";
}

}  // namespace parser

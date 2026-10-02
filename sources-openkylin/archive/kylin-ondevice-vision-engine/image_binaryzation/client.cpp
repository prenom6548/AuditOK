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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "client.h"

#include <nlohmann/json.hpp>

#include "universal_image_reader.h"

using namespace std;
namespace nl = nlohmann;
namespace tc = triton::client;

namespace ai_engine::core_ai::vision {

BinaryImage::BinaryImage() { binarizeImageRunning_.store(false, std::memory_order_release); }

void BinaryImage::setImageBinaryResultCallback(ImageResultCallback& callback) {
    imageBinaryzationResultCallback_ = std::move(callback);
}

result::AiEngineBaseResult BinaryImage::binaryImageRequestByPath(const string& imagePath) {
    std::lock_guard<std::mutex> lock(binImageMutex_);
    binarizeImageRunning_.store(true, std::memory_order_release);
    result::AiEngineBaseResult error;
    error.success = true;
    ImgBase64Result res;

    // 使用 ifstream 检查文件是否存在
    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        // 如果文件无法打开，返回错误信息
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_VISION_IMAGE_SIZE_INVALID,
                                             "[BinaryImage]: 文件为空。");
        binarizeImageRunning_.store(false, std::memory_order_release);
        return error;
    }

    std::vector<uchar> data = readFile(imagePath);
    std::string imgFormat = getFormatFromData(data);
    cv::Mat src_img = loadImageFromData(data, imgFormat);
    res = binaryImageRequestByImage(src_img, imgFormat);
    if (res.errorCode != -1) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_VISION_IMAGE_SIZE_INVALID,
                                             "[BinaryImage]: 文件为空。");
        binarizeImageRunning_.store(false, std::memory_order_release);
        return error;
    }
    ImageResult result;
    result.success = true;
    result.imageFormat = res.format;
    result.imageBase64Data = res.base64Result;
    result.modelName = modelName_;
    if (binarizeImageRunning_.load(std::memory_order_acquire) == true) {
        if (imageBinaryzationResultCallback_ != nullptr) {
            imageBinaryzationResultCallback_(result);
        }
    }
    binarizeImageRunning_.store(false, std::memory_order_release);

    return error;
}

result::AiEngineBaseResult BinaryImage::binaryImageRequestByBase64(const std::string& imageData) {
    std::lock_guard<std::mutex> lock(binImageMutex_);
    binarizeImageRunning_.store(true, std::memory_order_release);
    result::AiEngineBaseResult error;
    error.success = true;
    ImgBase64Result res;

    if (imageData.empty()) {
        res.errorCode = 1;
        res.errorMessage = "base64_string size error";
        return error;
    }
    std::string decoded_data = base64_decode(imageData.c_str());
    if (decoded_data.size() < 8) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_VISION_IMAGE_SIZE_INVALID,
                                             "[BinaryImage]: 文件为空。");
        binarizeImageRunning_.store(false, std::memory_order_release);
        return error;
    }
    std::vector<uchar> data(decoded_data.begin(), decoded_data.end());
    std::string imgFormat = getFormatFromData(data);
    cv::Mat image = loadImageFromData(data, imgFormat);
    res = binaryImageRequestByImage(image, imgFormat);
    if (res.errorCode != -1) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_VISION_IMAGE_SIZE_INVALID,
                                             "[BinaryImage]: 文件为空。");
        binarizeImageRunning_.store(false, std::memory_order_release);
        return error;
    }
    ImageResult result;
    result.success = true;
    result.imageFormat = res.format;
    result.imageBase64Data = res.base64Result;
    result.modelName = modelName_;
    if (binarizeImageRunning_.load(std::memory_order_acquire) == true) {
        if (imageBinaryzationResultCallback_ != nullptr) {
            imageBinaryzationResultCallback_(result);
        }
    }
    binarizeImageRunning_.store(false, std::memory_order_release);

    return error;
}

std::string BinaryImage::getConfigValue(const char* key, const char* engine) { return ""; }

ImgBase64Result BinaryImage::binaryImageRequestByImage(const cv::Mat& Image, const std::string& imgFormat) {
    ImgBase64Result res;
    if (Image.empty()) {
        res.errorCode = 2;
        res.errorMessage = "Image is empty";
        // 可以执行后续的图像处理操作
        std::cout << __LINE__ << " res.errorMessage: " << res.errorMessage << std::endl;
        return res;
    }

    cv::Mat opMat = Image.clone();
    // 灰度
    cv::Mat grayMat = opMat.clone();
    if (opMat.channels() != 1) {
        cvtColor(opMat, grayMat, cv::COLOR_BGR2GRAY);
    }
    // 双边滤波只支持8位无符号整型（CV_8U）和32位浮点型（CV_32F）的图像格式
    if (grayMat.type() != CV_8U || grayMat.type() != CV_32F) {
        grayMat.convertTo(grayMat, CV_8U);
    }
    // 双边滤波-降噪
    cv::Mat filtMat = grayMat.clone();
    bilateralFilter(grayMat, filtMat, 10, 10, 10);  // src和dst需不同

    // 黑白二值图像
    cv::Mat binaryMat = filtMat.clone();
    double maxValue = 255;
    cv::adaptiveThreshold(filtMat, binaryMat, maxValue,
                          cv::ADAPTIVE_THRESH_GAUSSIAN_C,  // 自动阈值计算
                          cv::THRESH_BINARY,
                          25,  // blockSize 必须是奇数
                          9);  // -C 用于微调阈值

    std::string res_base64 = MatToBase64(binaryMat);
    res.base64Result = res_base64;
    res.format = imgFormat;  // 图片format
    res.errorCode = -1;      // 假设成功时 errorCode 为 -1
    res.errorMessage.clear();

    return res;
}

void BinaryImage::stopImageBinarization() {
    if (binarizeImageRunning_.load(std::memory_order_acquire)) {
        binarizeImageRunning_.store(false, std::memory_order_release);
        ImageResult result;
        result.success = true;
        result.imageFormat = "";
        result.imageBase64Data = "";
        result.modelName = modelName_;
        if (imageBinaryzationResultCallback_ != nullptr) {
            imageBinaryzationResultCallback_(result);
        }
    }
}
}  // namespace ai_engine::core_ai::vision
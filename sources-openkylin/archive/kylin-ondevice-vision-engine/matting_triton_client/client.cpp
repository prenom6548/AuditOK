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

using namespace std;
namespace tc = triton::client;

namespace ai_engine::core_ai::vision {

BaseMattingClient::BaseMattingClient(const std::string& url, const std::string& modelCapability, int inpWidth,
                                     int inpHeight)
    : modelCapability_(modelCapability), inpWidth_(inpWidth), inpHeight_(inpHeight) {
    FAIL_IF_ERR(tc::InferenceServerHttpClient::Create(&client_, url, false), "Failed to create Triton client: ");
    MattingRunning_.store(false, std::memory_order_release);
}

void BaseMattingClient::preprocess(const cv::Mat& mattingSrcImg) {
    if (mattingSrcImg.empty()) {
        throw std::runtime_error("Input image is empty");
    }

    cv::Mat cv_dst;
    cv::resize(mattingSrcImg, cv_dst, cv::Size(this->inpWidth_, this->inpHeight_), cv::INTER_LINEAR);

    int row = cv_dst.rows;
    int col = cv_dst.cols;

    this->inputImage_.resize(row * col * cv_dst.channels());
    for (int c = 0; c < 3; c++) {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                float pix = cv_dst.ptr<uchar>(i)[j * 3 + 2 - c];
                this->inputImage_[c * row * col + i * col + j] = pix / 255.0f;
            }
        }
    }
}

cv::Mat BaseMattingClient::replaceBG(const cv::Mat cvSrc, cv::Mat& alpha, std::vector<int>& bgColor) {
    if (cvSrc.empty() || alpha.empty()) {
        throw std::runtime_error("Input image or alpha mask is empty");
    }

    int width = cvSrc.cols;
    int height = cvSrc.rows;

    cv::Mat cv_matting = cv::Mat::zeros(cv::Size(width, height), CV_8UC4);

    float* alpha_data = (float*)alpha.data;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float alpha = alpha_data[i * width + j];
            cv_matting.at<cv::Vec4b>(i, j)[0] = cvSrc.at<cv::Vec3b>(i, j)[0] * alpha + (1 - alpha) * bgColor[0];  // B
            cv_matting.at<cv::Vec4b>(i, j)[1] = cvSrc.at<cv::Vec3b>(i, j)[1] * alpha + (1 - alpha) * bgColor[1];  // G
            cv_matting.at<cv::Vec4b>(i, j)[2] = cvSrc.at<cv::Vec3b>(i, j)[2] * alpha + (1 - alpha) * bgColor[2];  // R
            cv_matting.at<cv::Vec4b>(i, j)[3] = static_cast<uchar>(alpha * 255);
        }
    }

    return cv_matting;
}

result::AiEngineBaseResult BaseMattingClient::mattingRequestByPath(const std::string& imagePath) {
    std::lock_guard<std::mutex> lock(MattingMutex_);
    MattingRunning_.store(true, std::memory_order_release);
    result::AiEngineBaseResult error;
    error.success = true;
    ImgBase64Result res;

    // 使用 ifstream 检查文件是否存在
    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        // 如果文件无法打开，返回错误信息
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_FILE_NOT_FOUND,
                                             "[MattingTriton]: 文件无法打开。");
        MattingRunning_.store(false, std::memory_order_release);
        return error;
    }

    std::vector<uchar> data = readFile(imagePath);
    std::string img_format = getFormatFromData(data);
    cv::Mat src_img = loadImageFromData(data, img_format);

    if (src_img.empty()) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_IMAGE_DATA_INVALID,
                                             "[MattingTriton]: 图片文件解析异常。");
        MattingRunning_.store(false, std::memory_order_release);
        return error;
    }

    cv::Mat bgr = ensureBGR(src_img);

    mattingRequestByImage(bgr, img_format, res);

    ImageResult result;
    result.success = true;
    result.imageFormat = res.format;
    result.imageBase64Data = res.base64Result;
    result.edgePointsData = res.edgePointsData;
    result.height = res.height;
    result.width = res.width;
    result.maskBase64Data = res.maskBase64Data;
    result.modelName = modelName_;
    if (MattingRunning_.load(std::memory_order_acquire) == true) {
        if (mattingResultCallback_ != nullptr) {
            mattingResultCallback_(result);
        }
    }
    MattingRunning_.store(false, std::memory_order_release);

    return error;
}

result::AiEngineBaseResult BaseMattingClient::mattingRequestByBase64(const std::string& imageData) {
    std::lock_guard<std::mutex> lock(MattingMutex_);
    MattingRunning_.store(true, std::memory_order_release);
    result::AiEngineBaseResult error;
    error.success = true;
    ImgBase64Result res;
    if (imageData.empty()) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_VISION_IMAGE_SIZE_INVALID,
                                             "[MattingTriton]: 文件为空。");
        MattingRunning_.store(false, std::memory_order_release);
        return error;
    }
    std::string decoded_data = base64_decode(imageData.c_str());
    if (decoded_data.size() < 8) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_VISION_IMAGE_SIZE_INVALID,
                                             "[MattingTriton]: 文件为空。");
        MattingRunning_.store(false, std::memory_order_release);
        return error;
    }
    std::vector<uchar> data(decoded_data.begin(), decoded_data.end());
    std::string img_format = getFormatFromData(data);
    cv::Mat src_img = loadImageFromData(data, img_format);
    if (src_img.empty()) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_IMAGE_DATA_INVALID,
                                             "[MattingTriton]: 图片文件解析异常。");
        MattingRunning_.store(false, std::memory_order_release);
        return error;
    }
    cv::Mat bgr = ensureBGR(src_img);
    mattingRequestByImage(bgr, img_format, res);

    ImageResult result;
    result.success = true;
    result.imageFormat = res.format;
    result.imageBase64Data = res.base64Result;
    result.edgePointsData = res.edgePointsData;
    result.height = res.height;
    result.width = res.width;
    result.maskBase64Data = res.maskBase64Data;
    result.modelName = modelName_;
    if (MattingRunning_.load(std::memory_order_acquire) == true) {
        if (mattingResultCallback_ != nullptr) {
            mattingResultCallback_(result);
        }
    }
    MattingRunning_.store(false, std::memory_order_release);

    return error;
}

std::string BaseMattingClient::getConfigValue(const char* key, const char* engine) {
    std::string model_name = "model_bank";
    tc::Error err = client_->LoadModel(model_name);
    tc::InferOptions options(model_name);
    options.model_version_ = "";
    options.priority_ = 1;

    std::vector<std::string> input_key_data{std::string(key)};
    std::vector<std::string> input_engine_data{std::string(engine)};

    tc::InferInput *input0, *input1;
    std::vector<int64_t> shape{1};
    tc::InferInput::Create(&input0, "key", shape, "BYTES");
    tc::InferInput::Create(&input1, "engine", shape, "BYTES");
    std::shared_ptr<tc::InferInput> input0_ptr, input1_ptr;
    input0_ptr.reset(input0);
    input1_ptr.reset(input1);
    input0_ptr->AppendFromString(input_key_data);
    input1_ptr->AppendFromString(input_engine_data);
    std::vector<tc::InferInput*> inputs = {input0_ptr.get(), input1_ptr.get()};

    tc::InferRequestedOutput* output0;
    tc::Error err_out0 = tc::InferRequestedOutput::Create(&output0, "value");
    std::shared_ptr<tc::InferRequestedOutput> output0_ptr;
    output0_ptr.reset(output0);
    std::vector<const tc::InferRequestedOutput*> outputs = {output0_ptr.get()};

    tc::InferResult* results;
    tc::Headers http_headers;
    tc::Error err_infer = client_->Infer(&results, options, inputs, outputs, http_headers);
    if (!err_infer.IsOk()) {
        std::cout << __LINE__ << " err_infer.Message(): " << err_infer.Message() << std::endl;
        return "";
    }

    std::shared_ptr<tc::InferResult> results_ptr;
    results_ptr.reset(results);

    char* output0_data_ptr;
    size_t recv_output0_byte_size;

    std::vector<std::string> text_output_data;
    tc::Error err_paser = results_ptr->StringData("value", &text_output_data);
    if (!err_paser.IsOk()) {
        std::cout << __LINE__ << " err_paser.Message(): " << err_infer.Message() << std::endl;
        return "";
    }
    return text_output_data[0];
}

ContourResult BaseMattingClient::extractEdgePoints(const cv::Mat& mask) {
    ContourResult result;
    if (mask.empty()) return result;

    cv::Mat binary_mask;
    mask.convertTo(binary_mask, CV_8UC1, 255.0);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(binary_mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        stEdgePointInfo edge_info;
        for (const auto& pt : contour) {
            stEdgePoint edge_point;
            edge_point.x = pt.x;
            edge_point.y = pt.y;
            edge_info.edge_points.push_back(edge_point);
        }
        edge_info.iPointCount = static_cast<int>(edge_info.edge_points.size());
        result.edge_point_infos.push_back(edge_info);
    }
    result.iContourCount = static_cast<int>(result.edge_point_infos.size());
    return result;
}

void BaseMattingClient::setMattingResultCallback(ImageResultCallback callback) {
    mattingResultCallback_ = std::move(callback);
}

result::AiEngineBaseResult BaseMattingClient::initMattingsModule() {
    result::AiEngineBaseResult error;
    error.success = true;
    modelName_ = getConfigValue(modelCapability_.c_str(), modelCapability_.c_str());
    if (modelName_.size() < 1) {
        printf("line:%d model_name.size():%ld\n", __LINE__, modelName_.size());
    }
    return error;
}

result::AiEngineBaseResult BaseMattingClient::destroyMattingsModule() {
    result::AiEngineBaseResult error;
    error.success = true;
    return error;
}

void BaseMattingClient::stopMattingProcess() {
    if (MattingRunning_.load(std::memory_order_acquire) == true) {
        MattingRunning_.store(false, std::memory_order_release);
        ImageResult result;
        result.success = true;
        result.imageFormat = "";
        result.imageBase64Data = "";
        result.edgePointsData = "";
        result.modelName = modelName_;
        if (mattingResultCallback_ != nullptr) {
            mattingResultCallback_(result);
        }
    }
}
}  // namespace ai_engine::core_ai::vision
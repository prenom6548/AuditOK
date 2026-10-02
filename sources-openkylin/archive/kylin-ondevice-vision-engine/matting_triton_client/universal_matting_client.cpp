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

#include "universal_matting_client.h"

#include <nlohmann/json.hpp>

using namespace std;
namespace tc = triton::client;

namespace ai_engine::core_ai::vision {

void UniversalMattingTritonClient::mattingRequestByImage(const cv::Mat& image, const std::string& imgFormat,
                                                         ImgBase64Result& res) {
    res.format = 13;     // 图片format
    res.errorCode = -1;  // 假设成功时 errorCode 为 -1
    res.errorMessage.clear();
    cv::Mat mask = detect(image, res);

    std::vector<int> color{255, 255, 255};
    cv::Mat dstimg = replaceBG(image, mask, color);
    // 提取边缘点坐标
    ContourResult result = extractEdgePoints(mask);

    std::vector<uchar> buf;
    cv::imencode("." + imgFormat, dstimg, buf);
    std::string res_base64 = base64_encode(buf.data(), buf.size());
    res.base64Result = res_base64;
    // 转换为 JSON
    nlohmann::json j_array = nlohmann::json::array();

    for (const auto& info : result.edge_point_infos) {
        nlohmann::json j_points = nlohmann::json::array();
        for (const auto& point : info.edge_points) {
            j_points.push_back(nlohmann::json{{"x", point.x}, {"y", point.y}});
        }
        j_array.push_back(j_points);
    }

    res.edgePointsData = j_array.dump();
    res.height = image.rows;
    res.width = image.cols;

    cv::Mat binary_mask;
    mask.convertTo(binary_mask, CV_8UC1, 255.0);
    std::vector<uchar> raw_pixels(binary_mask.begin<uchar>(), binary_mask.end<uchar>());
    std::string mask_buf_base64 = base64_encode(raw_pixels.data(), raw_pixels.size());
    res.maskBase64Data = mask_buf_base64;

    res.format = imgFormat;  // 图片format
    res.errorCode = -1;      // 成功时 errorCode 为 -1
    res.errorMessage.clear();

    return;
}

cv::Mat UniversalMattingTritonClient::detect(const cv::Mat& mattingSrcImg, ImgBase64Result& res) {
    bool ready = false;
    tc::Error error = client_->IsModelReady(&ready, modelName_);
    if (!error.IsOk()) {
        std::cerr << "[UniversalMattingTritonClient]:Triton error: unable to get model "
                     "ready status,"
                  << "error message:" << error.Message() << std::endl;
        return cv::Mat();
    }
    if (!ready) {
        std::cerr << "[PortraitMattingTritonClient]Model error: model is not ready, we have to LoadModel "
                     "model : "
                  << modelName_ << std::endl;
        error = client_->LoadModel(modelName_);
        if (!error.IsOk()) {
            std::cerr << "[UniversalMattingTritonClient]:Triton error: unable to LoadModel model." << modelName_
                      << "error message:" << error.Message() << std::endl;
            return cv::Mat();
        }
    }
    this->preprocess(mattingSrcImg);

    const int out_h = 1024;
    const int out_w = 1024;

    // 准备输入数据
    std::vector<int64_t> input_shape = {1, 3, inpHeight_, inpWidth_};

    std::vector<uint8_t> input_data(inputImage_.size() * sizeof(float));
    std::memcpy(input_data.data(), inputImage_.data(), inputImage_.size() * sizeof(float));
    tc::InferInput* input0;

    tc::Error err_ino = tc::InferInput::Create(&input0, "input", input_shape, "FP32");
    if (!err_ino.IsOk()) {
        res.errorCode = 3;
        res.errorMessage = err_ino.Message();
        // model error "unable to get product_reviews"
        std::cout << __LINE__ << " res.errorMessage: " << res.errorMessage << std::endl;
        return cv::Mat();
    }

    std::shared_ptr<tc::InferInput> input0_ptr;
    input0_ptr.reset(input0);
    input0_ptr->SetBinaryData(true);
    input0_ptr->AppendRaw(input_data.data(), input_data.size());

    tc::InferRequestedOutput* output0;

    tc::Error err_out0 = tc::InferRequestedOutput::Create(&output0, "output");
    if (!err_out0.IsOk()) {
        res.errorCode = 4;
        res.errorMessage = err_out0.Message();
        // req error "unable to get unnorm_text_features"
        std::cout << __LINE__ << " res.errorMessage: " << res.errorMessage << std::endl;
        return cv::Mat();
    }

    std::shared_ptr<tc::InferRequestedOutput> output0_ptr;
    output0_ptr.reset(output0);

    std::string model_version = "";
    // The inference settings. Will be using default for now.
    tc::InferOptions options(modelName_);
    options.model_version_ = model_version;
    options.priority_ = 1;

    std::vector<tc::InferInput*> inputs = {input0_ptr.get()};
    std::vector<const tc::InferRequestedOutput*> outputs = {output0_ptr.get()};

    tc::InferResult* results;
    tc::Headers http_headers;
    tc::Error err_infer = client_->Infer(&results, options, inputs, outputs, http_headers);
    if (!err_infer.IsOk()) {
        res.errorCode = 5;
        res.errorMessage = err_infer.Message();
        // Infer error "unable to run model"
        std::cout << __LINE__ << " res.errorMessage: " << res.errorMessage << std::endl;
        return cv::Mat();
    }
    std::shared_ptr<tc::InferResult> results_ptr;
    results_ptr.reset(results);

    float* mask_ptr;
    size_t recv_output0_byte_size;
    results_ptr->RawData("output", (const uint8_t**)&mask_ptr, &recv_output0_byte_size);

    cv::Mat cv_map;
    cv::Mat cv_mask_out(out_h, out_w, CV_32FC1, mask_ptr);
    cv::resize(cv_mask_out, cv_map, cv::Size(mattingSrcImg.cols, mattingSrcImg.rows));

    // 创建二值化mask用于轮廓检测
    cv::Mat binary_mask;
    cv_map.convertTo(binary_mask, CV_8UC1, 255.0);

    // 找到所有轮廓
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(binary_mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 计算最大面积
    float max_area = 0.0f;
    for (const auto& contour : contours) {
        float area = static_cast<float>(cv::contourArea(contour));
        if (area > max_area) {
            max_area = area;
        }
    }

    // 创建过滤后的mask
    cv::Mat filtered_mask = cv::Mat::zeros(cv_map.size(), CV_8UC1);
    float area_threshold = max_area * 0.1f;

    // 只保留面积大于阈值的轮廓
    for (const auto& contour : contours) {
        float area = static_cast<float>(cv::contourArea(contour));
        if (area >= area_threshold) {
            cv::fillPoly(filtered_mask, std::vector<std::vector<cv::Point>>{contour}, cv::Scalar(255));
        }
    }

    // 将过滤后的mask转换为浮点数格式并应用到原始mask
    filtered_mask.convertTo(filtered_mask, CV_32FC1, 1.0f / 255.0f);
    cv::multiply(cv_map, filtered_mask, cv_map);

    return cv_map;
}
}  // namespace ai_engine::core_ai::vision
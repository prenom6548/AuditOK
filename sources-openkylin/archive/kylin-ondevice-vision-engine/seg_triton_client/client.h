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
#pragma once

#include <dirent.h>
#include <kylin-ai/ai-engine/aiengine.h>
#include <kylin-ai/ai-engine/imageprocessengine.h>
#include <kylin-ai/common/error.h>
#include <triton/client/http_client.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

#include "base64_transformer.h"
#include "common.h"

namespace tc = triton::client;
namespace vision = ai_engine::core_ai::vision;
#define FAIL_IF_ERR(X, MSG)                                              \
    {                                                                    \
        tc::Error err = (X);                                             \
        if (!err.IsOk()) {                                               \
            std::cerr << "error: " << (MSG) << ": " << err << std::endl; \
            exit(1);                                                     \
        }                                                                \
    }

namespace ai_engine::core_ai::vision {

struct MatInfo {
    cv::Mat mask;           ///< 二值掩码图像（uint8，0/255）
    float iou_pred;         ///< 预测的IoU值（置信度）
    cv::Mat low_res_mask;   ///< 低分辨率掩码logits（256x256，float类型，用于稳定性分数计算）
    float stability_score;  ///< 稳定性分数，范围[0.0, 1.0]，值越高表示掩码越稳定
};

class SegTritonClient {
public:
    SegTritonClient(const std::string& url);
    void setSegmentationResultCallback(const SegmentationResultCallback& callback);
    result::AiEngineBaseResult segEncodeRequestByPath(const std::string& imagePath);
    result::AiEngineBaseResult segEncodeRequestByBase64(const std::string& imageData);
    result::AiEngineBaseResult segDecodeRequest(const std::vector<std::tuple<int, int, bool>>& points,
                                                uint32_t minArea);
    vision::ImageSegmentationResult segEncodeRequestByImage(const cv::Mat& image, const std::string& imgFormat);
    void init();
    std::string getConfigValue(const char* key, const char* engine);
    void segmentationClearCache();
    result::AiEngineBaseResult stopSegmentation();
    std::string currentSegmentationModel() const { return modelName_; };
    void setCurrentSegmentationModel(const std::string& model) {
        modelName_ = model;
        return;
    };

private:
    vision::ImageSegmentationResult encode(const cv::Mat& srcImg, std::string& mask_ptr);
    vision::ImageSegmentationResult decode(const std::vector<std::tuple<int, int, bool>>& point, uint32_t minArea);
    bool loadModelIfNotAvailable(const std::string& modelName);
    int inputWidth = 992;
    int inputHeight = 992;
    std::vector<nlohmann::json> maskJsonResult_;
    std::string segEncoderData_;
    std::string imgFormat_;
    std::string modelName_;
    std::unique_ptr<tc::InferenceServerHttpClient> client_;  // Triton client as a member variable
    std::atomic<bool> SegRunning_;
    std::mutex segMutex_;  // 用于保护并发访问的互斥锁
    SegmentationResultCallback segResultCallback_ = nullptr;
};
}  // namespace ai_engine::core_ai::vision
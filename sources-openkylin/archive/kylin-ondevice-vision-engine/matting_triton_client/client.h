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
#include <opencv2/opencv.hpp>

#include "base64_transformer.h"
#include "common.h"
#include "universal_image_reader.h"

namespace tc = triton::client;
namespace ai_engine::core_ai::vision {

#define FAIL_IF_ERR(X, MSG)                                              \
    {                                                                    \
        tc::Error err = (X);                                             \
        if (!err.IsOk()) {                                               \
            std::cerr << "error: " << (MSG) << ": " << err << std::endl; \
            exit(1);                                                     \
        }                                                                \
    }

// 边缘点结构体
struct stEdgePoint {
    int x;
    int y;
};

struct stEdgePointInfo {
    std::vector<stEdgePoint> edge_points;
    int iPointCount;
};
// 边缘轮廓结果结构体
struct ContourResult {
    std::vector<stEdgePointInfo> edge_point_infos;
    int iContourCount;
};

class BaseMattingClient {
public:
    // 构造函数
    BaseMattingClient(const std::string& url, const std::string& modelCapability, int inpWidth, int inpHeight);

    // 预处理、检测和后处理方法声明
    virtual cv::Mat detect(const cv::Mat& mattingSrcImg, ImgBase64Result& res) = 0;
    virtual void mattingRequestByImage(const cv::Mat& image, const std::string& imgFormat, ImgBase64Result& res) = 0;

    virtual void preprocess(const cv::Mat& mattingSrcImg);
    virtual cv::Mat replaceBG(const cv::Mat cvSrc, cv::Mat& alpha, std::vector<int>& bgColor);

    // 共同实现的方法
    result::AiEngineBaseResult mattingRequestByPath(const std::string& imagePath);
    result::AiEngineBaseResult mattingRequestByBase64(const std::string& imageData);
    ContourResult extractEdgePoints(const cv::Mat& mask);
    std::string getConfigValue(const char* key, const char* engine);

    void setMattingResultCallback(ImageResultCallback callback);
    result::AiEngineBaseResult initMattingsModule();
    result::AiEngineBaseResult destroyMattingsModule();
    void stopMattingProcess();

    std::string currentModel() const { return modelName_; }
    void setCurrentModel(const std::string& model) {
        modelName_ = model;
        return;
    }

protected:
    int inpWidth_ = 512;
    int inpHeight_ = 512;
    std::vector<float> inputImage_;
    std::unique_ptr<tc::InferenceServerHttpClient> client_;  // 维护的Triton客户端
    std::string modelCapability_;
    std::string modelName_;
    ImageResultCallback mattingResultCallback_ = nullptr;

private:
    std::atomic<bool> MattingRunning_;
    std::mutex MattingMutex_;  // 用于保护并发访问的互斥锁
};
}  // namespace ai_engine::core_ai::vision
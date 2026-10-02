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

namespace tc = triton::client;

#define FAIL_IF_ERR(X, MSG)                                              \
    {                                                                    \
        tc::Error err = (X);                                             \
        if (!err.IsOk()) {                                               \
            std::cerr << "error: " << (MSG) << ": " << err << std::endl; \
            exit(1);                                                     \
        }                                                                \
    }

namespace ai_engine::core_ai::vision {
class BinaryImage {
public:
    BinaryImage();
    void setImageBinaryResultCallback(ImageResultCallback& callback);
    result::AiEngineBaseResult binaryImageRequestByPath(const std::string& imagePath);
    result::AiEngineBaseResult binaryImageRequestByBase64(const std::string& imageData);
    ImgBase64Result binaryImageRequestByImage(const cv::Mat& image, const std::string& imgFormat);
    std::string getConfigValue(const char* key, const char* engine);
    void stopImageBinarization();

private:
    std::string modelName_;
    std::atomic<bool> binarizeImageRunning_;
    ImageResultCallback imageBinaryzationResultCallback_ = nullptr;
    std::mutex binImageMutex_;  // 用于保护并发访问的互斥锁
};
}  // namespace ai_engine::core_ai::vision
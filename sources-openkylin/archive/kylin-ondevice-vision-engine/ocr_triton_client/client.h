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

#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <triton/client/http_client.h>
#include <atomic>
#include <dirent.h>
#include <fstream>
#include <chrono>
#include "base64_transformer.h"
#include <kylin-ai/ai-engine/textrecognitionengine.h>
#include <kylin-ai/ai-engine/aiengine.h>

namespace tc = triton::client;

namespace vision = ai_engine::core_ai::vision;
#define FAIL_IF_ERR(X, MSG)                                        \
  {                                                                \
    tc::Error err = (X);                                           \
    if (!err.IsOk()) {                                             \
      std::cerr << "error: " << (MSG) << ": " << err << std::endl; \
      exit(1);                                                     \
    }                                                              \
  }
namespace ai_engine::core_ai::vision {

class OcrTritonClient {
public:
    OcrTritonClient(const std::string& url);
    void setTextRecognitionResultCallback(TextRecognitionResultCallback callback);
    result::AiEngineBaseResult OcrRequestByPath(std::string ImagePath);
    result::AiEngineBaseResult OcrRequestByBase64(const char *ImageData);
    vision::TextRecognitionResult OcrRequestByImage(cv::Mat Image);
    void init();
    std::string GetConfigValue(const char* key, const char* engine);

    result::AiEngineBaseResult stopTextRecognition();

    std::string currentTextRecognitionModel() const { return model_name_; }
    void setCurrentTextRecognitionModel(const std::string& model) { model_name_ = model; return; }

private:
    std::atomic<bool> OcrRunning_;
    std::unique_ptr<tc::InferenceServerHttpClient> client_;  // Triton client as a member variable
    std::string model_name_;
    std::mutex ocrMutex_;       // 用于保护并发访问的互斥锁
    std::mutex callBackMutex_;  // 用于保护回调函数变量的互斥锁
    TextRecognitionResultCallback ocrResultCallback_ = nullptr;
};
}

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
#include <triton/client/http_client.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "base64_transformer.h"
#include "client.h"
#include "common.h"

namespace tc = triton::client;

namespace ai_engine::core_ai::vision {

class PortraitMattingTritonClient : public BaseMattingClient {
public:
    PortraitMattingTritonClient(const std::string& url) : BaseMattingClient(url, "matting_portrait", 512, 512) {}
    cv::Mat detect(const cv::Mat& mattingSrcImg, ImgBase64Result& res) override;
    void mattingRequestByImage(const cv::Mat& image, const std::string& imgFormat, ImgBase64Result& res) override;

private:
    const float mean_[3] = {0.485, 0.456, 0.406};
    const float std_[3] = {0.229, 0.224, 0.225};
};
}
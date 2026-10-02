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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

#include "baiduresponseprocessor.h"
#include "baiduvisionengine.h"
#include "logger.h"
#include "util.h"

static void writeBinaryDataToFile(const std::string &filename, const std::vector<char> &data) {
    std::ofstream outputFile(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!outputFile.is_open()) {
        LOGE("Failed to open file for writing.");
        return;
    }

    // 写入数据到文件
    outputFile.write(data.data(), data.size());

    // 检查是否写入成功
    if (!outputFile.good()) {
        LOGE("Error occurred while writing to file.");
    } else {
        LOGI("Binary data has been written to file:", filename);
    }

    // 关闭文件
    outputFile.close();
}

void getModelInfo() {
    BaiduVisionEngine baiduVisionEngine;
    LOGD("Engine name is", baiduVisionEngine.engineName());
    LOGD("Engine current model name is", baiduVisionEngine.currentModel());
    LOGD("Engine model info is", baiduVisionEngine.modelInfo());
}

std::string getConfig() {
    return R"(
        {
            "appId" : "",
            "apiKey" : "",
            "secretKey" : ""
        }
    )";
}

void testVision() {
    namespace fs = std::filesystem;

    ai_engine::EngineError error;

    BaiduVisionEngine baiduVisionEngine;
    if (baiduVisionEngine.isCloud()) {
        LOGI("Baidu vision engine is cloud.");
    }

    baiduVisionEngine.setConfig(getConfig());
    baiduVisionEngine.setImageSize(512, 512);
    baiduVisionEngine.setImageNumber(1);
    baiduVisionEngine.setImageStyle("写实风格");

    LOGD("Supported ImageStyles", baiduVisionEngine.supportedImageStyles().data());
    LOGD("Supported ImageNumber", baiduVisionEngine.supportedImageNumber());
    LOGD("Supported ImageSizes", baiduVisionEngine.supportedImageSizes().data());

    baiduVisionEngine.setGenerationResultCallback(
        [](const ai_engine::gen_ai::vision::ImageGenerationResult &imageData) {
            std::string imageDataString{baidu_vision_util::base64Decode(imageData.imageBase64Data)};
            std::vector<char> imagedata{imageDataString.begin(), imageDataString.end()};
            writeBinaryDataToFile("./test_output.png", imagedata);
            LOGD(imageData.imageBase64Data.size(), imageData.format, imageData.height, imageData.index, imageData.total,
                 imageData.error.getCode(), imageData.error.getMessage());
        });
    LOGI("Generating image.");
    auto result = baiduVisionEngine.generateImage("生成一张小猫的图片");
    if (!result.success) {
        LOGE(result.error.getCode(), result.error.getMessage());
        return;
    }
    LOGI("Generate image success.");

    while (std::getchar() != '\n') {
    }
    baiduVisionEngine.stopImageGeneration();
}

int main() {
    getModelInfo();
    testVision();
    return 0;
}
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

#include "logger.h"
#include "util.h"
#include "xunfeivisionengine.h"
static void writeBinaryDataToFile(const std::string &filename, const std::vector<char> &data) {
    std::ofstream outputFile(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    // 写入数据到文件
    outputFile.write(data.data(), data.size());

    // 检查是否写入成功
    if (!outputFile.good()) {
        std::cerr << "Error occurred while writing to file." << std::endl;
    } else {
        std::cout << "Binary data has been written to file: " << filename << std::endl;
    }

    // 关闭文件
    outputFile.close();
}

void getmodelinfo() {
    XunfeiVisionEngine xunfeiVisionEngine;
    LOGD("engine name is", xunfeiVisionEngine.engineName());
    LOGD("engine current model name is", xunfeiVisionEngine.currentModel());
    LOGD("engine model info is", xunfeiVisionEngine.modelInfo());
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
void testvision() {
    namespace fs = std::filesystem;

    ai_engine::EngineError error;

    XunfeiVisionEngine xunfeiVisionEngine;
    if (xunfeiVisionEngine.isCloud()) {
        LOGDF("有网络");
    }

    xunfeiVisionEngine.setConfig(getConfig());
    xunfeiVisionEngine.setImageSize(512, 512);
    xunfeiVisionEngine.setImageNumber(1);
    xunfeiVisionEngine.setImageStyle("写实风格");

    LOGD("Supported ImageStyles", xunfeiVisionEngine.supportedImageStyles().data());
    LOGD("Supported ImageNumber", xunfeiVisionEngine.supportedImageNumber());
    LOGD("Supported ImageSizes", xunfeiVisionEngine.supportedImageSizes().data());

    xunfeiVisionEngine.setGenerationResultCallback(
        [](const ai_engine::gen_ai::vision::ImageGenerationResult &imageData) {
            std::string imageDataString{xunfei_vision_util::base64Decode(imageData.imageBase64Data)};
            std::vector<char> imagedata{imageDataString.begin(), imageDataString.end()};
            writeBinaryDataToFile("./test3.png", imagedata);
            LOGD(imageData.imageBase64Data.size(), imageData.format, imageData.height, imageData.index, imageData.total,
                 imageData.error.getCode(), imageData.error.getMessage());
        });
    std::thread generateThread([&xunfeiVisionEngine]() {
        auto result = xunfeiVisionEngine.generateImage("生成一张小狗的图片");
        if (!result.success) {
            LOGI("generate image failed", result.error.getCode(), result.error.getMessage());
        }
    });
    while (std::getchar() != '\n') {
        break;
    }
    xunfeiVisionEngine.stopImageGeneration();
    if (generateThread.joinable()) {
        generateThread.join();
    }
}
int main() {
    getmodelinfo();
    testvision();
    return 0;
}
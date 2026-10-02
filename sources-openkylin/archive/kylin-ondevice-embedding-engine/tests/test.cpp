/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "ondeviceembeddingengine.h"

using json = nlohmann::json;

std::string base64Encode(const std::vector<uint8_t>& buffer);
std::vector<uint8_t> readFile(const std::string& filePath);
int main() {
    ai_engine::core_ai::embedding::OndeviceEmbeddingEngine engine;
    ai_engine::result::AiEngineBaseResult res = engine.init();

    // std::cout << "modelInfo:" << engine.modelInfo() << std::endl;
    std::string s = engine.modelInfo();
    std::cout << "modelInfo:" << s << std::endl;
    auto items = json::parse(s);
    for (auto& item : items["models"]) {
        std::cout << "name:" << item["name"] << std::endl;
        std::cout << "dim:" << item["dim"] << std::endl;
        std::cout << "model_capability:" << item["model_capability"]
                  << std::endl;
        std::cout << "ondevice:" << item["ondevice"] << std::endl;
        std::cout << "priority:" << item["priority"] << std::endl;
    }
    engine.setCurrentImageEmbeddingModel(
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image");
    engine.setCurrentTextEmbeddingModel("ensemble-embd_gte-base_uint8-text");
    std::cout << "ImageEmbedding model name:"
              << engine.currentImageEmbeddingModel() << std::endl;
    std::cout << "TextEmbedding model name:"
              << engine.currentTextEmbeddingModel() << std::endl;

    // 向量化任务
    bool done = false;
    std::vector<float> result;
    std::mutex mtx;
    std::condition_variable cv;
    int errorCode;
    std::string errorMessage;
    std::string engineNamge;
    auto callback =
        [&](const ai_engine::core_ai::embedding::EmbeddingResult& res) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                result = res.embeddings;
                errorCode = res.error.getCode();
                errorMessage = res.error.getMessage();
                engineNamge = res.modelName;

                done = true;
            }

            cv.notify_all();  // 通知主线程结果已准备好
        };
    engine.setEmbeddingResultCallback(callback);
    std::string longText =
        "在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件：在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件在使用 Visual Studio Code (VSCode) "
        "编写代码时，确实有一些插件可以帮助你检测当前文件是否用到了某个库。这些"
        "插件通常会分析代码，找出未使用的导入语句，并提供删除建议。以下是一些常"
        "用的插件";

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "微信图片_20240709181353.jpg";

// 测试不同向量化任务的接口
#if 1
    if (res.success)
        engine.embeddingText(longText);
    else
        std::cout << "init text module error:" << res.error.getMessage()
                  << std::endl;
#elif 0
    ai_engine::result::AiEngineBaseResult res =
        engine.initImageEmbeddingModule();
    if (res.success)
        engine.embeddingTextWithImageModel(longText);
    else
        std::cout << "init image module error:" << res.error.getMessage()
                  << std::endl;
    engine.destroyImageEmbeddingModule();
#elif 0
    ai_engine::result::AiEngineBaseResult res =
        engine.initImageEmbeddingModule();
    if (res.success)
        engine.embeddingImageFromImageFile(path.string());
    else
        std::cout << "init image module error:" << res.error.getMessage()
                  << std::endl;
    engine.destroyImageEmbeddingModule();
#else
    ai_engine::result::AiEngineBaseResult res =
        engine.initImageEmbeddingModule();
    if (res.success) {
        std::vector<uint8_t> imageData = readFile(path.string());
        std::string base64Data = base64Encode(imageData);
        engine.embeddingImageFromBase64ImageData(base64Data);
    } else
        std::cout << "init image module error:" << res.error.getMessage()
                  << std::endl;
    engine.destroyImageEmbeddingModule();
#endif
    std::cout << "Doing other work while waiting for the result..."
              << std::endl;

    // 等待结果准备好
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&done]() { return done; });
    }

    std::cout << std::endl;
    std::cout << "cout--------------------------------------------result"
              << std::endl;
    std::cout << " Embedding result:" << "engineName:" << engineNamge
              << ",vector size is:" << result.size()
              << ",errorCode:" << errorCode << ",errorMessage:" << errorMessage
              << std::endl;
    std::cout << " vector:";
    for (float f : result) {
        std::cout << f << ", ";
    }

    std::cout << std::endl;
    std::cout << "cout--------------------------------------------result"
              << std::endl;
    std::cout << std::endl;

    std::cout << "Hello World!" << std::endl;
}

std::vector<uint8_t> readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    } else {
        throw std::runtime_error("Failed to read file");
    }
}
const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string base64Encode(const std::vector<uint8_t>& buffer) {
    std::string encodedData;
    int i = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];

    while (i < buffer.size()) {
        char_array_3[0] = buffer[i++];
        char_array_3[1] = (i < buffer.size()) ? buffer[i++] : 0;
        char_array_3[2] = (i < buffer.size()) ? buffer[i++] : 0;

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = (char_array_3[2] & 0x3f);

        for (int j = 0; (j < 4); ++j) {
            encodedData += base64_chars[char_array_4[j]];
        }
    }

    while ((encodedData.size() % 4) != 0) {
        encodedData += '=';
    }

    return encodedData;
}
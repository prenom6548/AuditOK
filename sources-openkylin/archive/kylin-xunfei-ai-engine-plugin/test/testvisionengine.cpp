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

#include <assert.h>
#include <fstream>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include "xunfeiengine.h"
std::string base64Decode(const std::string &input) {
    BIO *bio{nullptr};
    BIO *b64{nullptr};
    char *buffer = new char[input.size()];
    std::copy(input.begin(), input.end(), buffer);

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(buffer, static_cast<int>(input.size()));
    bio = BIO_push(b64, bio);

    int decodedLength = BIO_read(bio, buffer, static_cast<int>(input.size()));
    std::string result(buffer, decodedLength);
    BIO_free_all(bio);
    delete[] buffer;

    return result;
}
void writeBinaryDataToFile(const std::string &filename,
                           const std::vector<char> &data) {
    std::ofstream outputFile(filename,
                             std::ios::out | std::ios::binary | std::ios::app);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    outputFile.write(data.data(), data.size());

    if (!outputFile.good()) {
        std::cerr << "Error occurred while writing to file." << std::endl;
    } else {
        std::cout << "Binary data has been written to file: " << filename
                  << std::endl;
    }

    outputFile.close();
}

void testVision() {
    XunfeiAiEngine xunfeiAiEngine;
    auto engine = xunfeiAiEngine.createImageGenerationEngine();

    const std::string configJson =
        u8R"({"appId":"fakeAppId", "apiKey":"fakeApiKey", "secretKey":"fakeSecretKey"})";
    engine->setConfig(configJson);

    engine->setImageNumber(1);
    engine->setImageStyle("写实风格");
    engine->setImageSize(1024, 1024);
    int imageCount = 0;
    engine->setGenerationResultCallback(
        [&imageCount](
            ai_engine::gen_ai::vision::ImageGenerationResult imageData)
            -> void {
            std::cout << "image format: " << imageData.format << std::endl;
            std::cout << "image height: " << imageData.height << std::endl;
            std::cout << "image width: " << imageData.width << std::endl;
            std::cout << "image index: " << imageData.index << std::endl;
            std::cout << "image total: " << imageData.total << std::endl;
            std::cout << "image data size: " << imageData.imageBase64Data.size()
                      << std::endl;
            std::string imageBase64 = base64Decode(imageData.imageBase64Data);
            writeBinaryDataToFile(
                "test" + std::to_string(imageCount++) + ".jpg",
                std::vector<char>(imageBase64.begin(), imageBase64.end()));
            std::cout << "image error message: " << imageData.error.getMessage()
                      << std::endl;
        });

    ai_engine::EngineError error;
    auto ret = engine->generateImage("骑马");
    std::cout << "prompt2Image result: " << error.getMessage() << std::endl;
    assert(ret.success == true);
}

int main(int argc, char *argv[]) {
    testVision();
    return 0;
}
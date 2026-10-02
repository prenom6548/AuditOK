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
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <thread>
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

std::vector<uint8_t> readAudioData(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> audioData(fileSize);
    file.read(reinterpret_cast<std::istream::char_type *>(audioData.data()),
              fileSize);
    return audioData;
}

std::vector<std::vector<uint8_t>>
splitAudioData(const std::vector<uint8_t> &audioData, size_t chunkSize) {
    std::vector<std::vector<uint8_t>> chunks;
    size_t dataSize = audioData.size();
    size_t numChunks = dataSize / chunkSize;
    size_t remainder = dataSize % chunkSize;

    for (size_t i = 0; i < numChunks; ++i) {
        chunks.push_back(
            std::vector<uint8_t>(audioData.begin() + i * chunkSize,
                                 audioData.begin() + (i + 1) * chunkSize));
    }

    if (remainder != 0) {
        chunks.push_back(
            std::vector<uint8_t>(audioData.end() - remainder, audioData.end()));
    }

    return chunks;
}

void testSpeech() {
    XunfeiAiEngine xunfeiAiEngine;
    auto engine = xunfeiAiEngine.createSpeechEngine();

    const std::string configJson = u8R"(
            {"appId":"fakeAppId", "apiKeyRealTime":"fakeApiKeyRealTime",
             "apiKeyContinuous":"fakeApiKeyContinuous", "secretKeyContinuous":"fakeSecretKeyContinuous"}
            )";
    engine->setConfig(configJson);

    auto audioData = readAudioData("16k_10.pcm");

    ai_engine::EngineError error;
    auto ret = engine->initRecognizeOnceModule();
    assert(ret.success == true);

    engine->setRecognitionResultCallback(
        [](ai_engine::core_ai::speech::RecognitionResult result) {
            std::cout << "MID_TEXT: " << result.text << std::endl;
        });

    ret = engine->recognizeOnce("\{\"rate\": 16000}", audioData);
    std::cout << "recongnize once result: " << error.getMessage() << std::endl;
    assert(ret.success == true);
    ret = engine->destroyRecognizeOnceModule();

    ret = engine->initContinuousRecognitionModule();
    assert(ret.success == true);

    ret = engine->startContinuousRecognition("\{\"rate\": 16000}");
    assert(ret.success == true);
    auto audioDatas = splitAudioData(audioData, 2560);
    for (auto &data : audioDatas) {
        std::vector<uint8_t> theAudioData(data.begin(), data.end());
        ret = engine->writeContinuousRecognitionAudioData(theAudioData);
        assert(ret.success == true);
        usleep(80 * 1000);
    }
    sleep(5);
    engine->stopContinuousRecognition();
    engine->destroyContinuousRecognitionModule();

    ret = engine->initSynthesizeModule();
    assert(ret.success == true);
    std::string text =
        "看起来像是随机敲击键盘产生的字符组合，没有明确的含义或上下文。\
        它可能是一个错误输入、代码片段的一部分，或者仅仅是随机文本。\
        如果您能提供更多的上下文或信息，我可能能更好地理解它的含义或用途。";

    engine->setSynthesisResultCallback(
        [](ai_engine::core_ai::speech::SynthesisResult result) {
            std::cout << result.base64AudioData.size() << std::endl;
            std::string audioData = base64Decode(result.base64AudioData);
            writeBinaryDataToFile(
                "test.mp3",
                std::vector<char>(audioData.begin(), audioData.end()));
        });

    ret = engine->synthesizeText("\{\"speed\": 5}", text);
    std::cout << "synthesize once result: " << error.getMessage() << std::endl;
    assert(ret.success == true);
    ret = engine->destroySynthesizeModule();
    assert(ret.success == true);
}

int main(int argc, char *argv[]) {
    testSpeech();
    return 0;
}
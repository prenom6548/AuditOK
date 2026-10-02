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

#include <kylin-ai/aiengine.h>

#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

#include "engine/aienginepluginmanager.h"

std::vector<char> readAudioData(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> audioData(fileSize);
    file.read(audioData.data(), fileSize);
    return audioData;
}

static void testSpeechOfBaidu() {
    ai_engine::AiEnginePluginManager aepm;
    auto baiduEngine = aepm.createSpeechEngine("baidu");

    ai_engine::speech::AbstractCloudSpeechEngine *speechEnginePtr(
        dynamic_cast<ai_engine::speech::AbstractCloudSpeechEngine *>(
            baiduEngine.release()));

    std::unique_ptr<ai_engine::speech::AbstractCloudSpeechEngine>
        speechEngine(speechEnginePtr);

    speechEngine->setRecognizingCallback(
        [](ai_engine::speech::RecognitionResult result) {
            std::cout << result.text << std::endl;
        });
    speechEngine->setRecognizedCallback(
        [](ai_engine::speech::RecognitionResult type) {
            std::cout << "Recognize finished..." << std::endl;
        });

    speechEngine->setSynthesizingCallback(
        [](ai_engine::speech::SynthesisResult result) {
            std::cout << result.audioData.size() << std::endl;
        });
    speechEngine->setSynthesizedCallback(
        [](ai_engine::speech::ResultType type) {
            std::cout << "Synthesize finished..." << std::endl;
        });

    speechEngine->setConfig(R"(
        {
            "appId":"",
            "apiKey":"",
            "secretKey":"",
        }
    )");

    ai_engine::EngineError error;
    auto audioData = readAudioData("16k.pcm");
    ASSERT_FALSE(audioData.empty());
    auto ret =
        speechEngine->recognizeOnce("\{\"rate\": 1600}", audioData, error);
    std::cout << "recongnize once: " << (int)ret << std::endl;
    ASSERT_TRUE(ret);

    std::string text = "麒麟软件，风行天下";
    ret = speechEngine->synthesizeOnce("\{\"speed\": 5}", text, error);
    std::cout << "synthesize once: " << (int)ret << std::endl;
    ASSERT_TRUE(ret);

    audioData = readAudioData("16k.pcm");
    ASSERT_FALSE(audioData.empty());
    ret = speechEngine->recognizeOnce("\{\"rate\": 1600}", audioData, error);
    std::cout << "recongnize once: " << (int)ret << std::endl;
}

static void testNlpOfBaidu() {
    ai_engine::EngineError error;
    ai_engine::AiEnginePluginManager aepm;
    auto baiduEngine = aepm.createTextGenerationEngine("baidu");

    ai_engine::nlp::AbstractCloudNlpEngine *nlpEnginePtr(
        dynamic_cast<ai_engine::nlp::AbstractCloudNlpEngine *>(
            baiduEngine.release()));

    std::unique_ptr<ai_engine::nlp::AbstractCloudNlpEngine> nlpEngine(
        nlpEnginePtr);

    nlpEngine->setConfig(R"(
        {
            "appId":"",
            "apiKey":"",
            "secretKey":"",
        }
    )");
    nlpEngine->setChatResultCallback(
        [](const ai_engine::nlp::ChatResult &result) -> void {
            std::cout << result.text << std::endl;
        });

    auto ret = nlpEngine->chat("我叫麒麟软件", error);
    std::cout << "chat: " << (int)ret << std::endl;
    ASSERT_TRUE(ret);

    ret = nlpEngine->chat("我是谁", error);
    std::cout << "chat: " << (int)ret << std::endl;
    ASSERT_TRUE(ret);
}

static void testVisionOfBaidu() {
    ai_engine::EngineError error;
    ai_engine::AiEnginePluginManager aepm;
    auto baiduEngine = aepm.createImageGenerationEngine("baidu");

    ai_engine::vision::AbstractCloudVisionEngine *visionEnginePtr(
        dynamic_cast<ai_engine::vision::AbstractCloudVisionEngine *>(
            baiduEngine.release()));

    std::unique_ptr<ai_engine::vision::AbstractCloudVisionEngine>
        visionEngine(visionEnginePtr);

    visionEngine->setConfig(R"(
        {
            "appId":"",
            "apiKey":"",
            "secretKey":"",
        }
    )");
    visionEngine->setPrompt2ImageNumber(2);
    visionEngine->setPrompt2ImageCallback(
        [](ai_engine::vision::ImageData imageData) -> void {
            std::cout << "format: " << imageData.format << std::endl;
            std::cout << "height: " << imageData.height << std::endl;
            std::cout << "width: " << imageData.width << std::endl;
            std::cout << "index: " << imageData.index << std::endl;
            std::cout << "total: " << imageData.total << std::endl;
            std::cout << "image data size: " << imageData.imageData.size()
                      << std::endl;
        });

    auto ret = visionEngine->prompt2Image(
        "黄金比例构成，一颗冰晶小苍兰，微距摄影，超详细刻画", error);
    std::cout << "vision: " << (int)ret << std::endl;
    ASSERT_TRUE(ret);
}

TEST(EngineManagerTest, BaiduEngineIntegration) {
    testSpeechOfBaidu();
    testNlpOfBaidu();
    testVisionOfBaidu();
}

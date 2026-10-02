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
#include <vector>

#include "audioplayer.h"
#include "audiorecorder.h"
#include "baiduspeechengine_p.h"
#include "logger.h"
#include "util.h"

std::string getConfig() {
#define USE_THE_CORRECT_KEY 1
#if USE_THE_CORRECT_KEY
    // 正确的key
    return R"(
        {
	        "appId" : "",
			"apiKey" : "",
	        "secretKey" : ""
        }
    )";
#else
    // 错误的key
    return R"(
        {
	        "appId" : "appId",
			"apiKey" : "apiKey",
	        "secretKey" : "secretKey"
        }
    )";
#endif
}

std::vector<uint8_t> readAudioData(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file:", filePath);
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> audioData(fileSize);
    file.read(reinterpret_cast<char *>(audioData.data()), fileSize);
    return audioData;
}

std::vector<uint8_t> getData() {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "16k.pcm";
    if (not fs::exists(path)) {
        LOGE("error: file not exists");
        return {};
    }
    return readAudioData(path.string());
}

void testContinuousRecognition() {
    using namespace std::chrono_literals;

    BaiduSpeechEnginePrivate engine{};
    if (engine.isCloud()) {
        LOGI("Engine is cloud.");
    }
    engine.setConfig(getConfig());
    engine.setRecognitionResultCallback([](const ai_engine::core_ai::speech::RecognitionResult &result) {
        LOGI("===========================================================");
        LOGI("text:", result.text);
        LOGI("error code:", result.error.getCode());
        LOGI("error message:", result.error.getMessage());
        LOGI("reason:", int(result.reason));
        LOGI("speaker id:", result.speakerId);
    });

    auto result = engine.initContinuousRecognitionModule();
    if (!result.success) {
        LOGE("Init continuous recognition module failed, error info:", result.error.getCode(),
             result.error.getMessage(), ".");
        return;
    }

    while (true) {
        std::atomic_bool isRunning = true;
        AudioRecorder audioRecorder{};
        audioRecorder.start();

        std::thread thread([&engine, &isRunning, &audioRecorder]() {
            auto result = engine.startContinuousRecognition(R"({"format": "", "rate": 16000, "channel": 1})");
            if (!result.success) {
                LOGE("Start continuous recognition failed, error info:", result.error.getCode(),
                     result.error.getMessage(), ".");
                return;
            }
            audioRecorder.start();
            while (isRunning) {
                using namespace std::chrono_literals;
                std::chrono::time_point<std::chrono::steady_clock> timePoint = std::chrono::steady_clock::now() + 40ms;
                std::vector<uint8_t> audioData = audioRecorder.getSingleFrameAudioData();
                if (audioData.empty()) {
                    std::this_thread::sleep_for(10ms);
                    continue;
                }
                result = engine.writeContinuousRecognitionAudioData(audioData);
                if (!result.success) {
                    LOGE("Write continuous recognition audio data failed, "
                         "error info : ",
                         result.error.getCode(), result.error.getMessage(), ".");
                    break;
                }
                std::this_thread::sleep_until(timePoint);
            }
        });
        while (std::getchar() != '\n') {
        }

        isRunning = false;

        result = engine.stopContinuousRecognition();
        thread.join();
    }

    result = engine.destroyContinuousRecognitionModule();
}

void testOnceRecognition() {

    std::vector<uint8_t> audioData = getData();

    ai_engine::EngineError error;

    BaiduSpeechEnginePrivate engine;
    if (engine.isCloud()) {
        LOGI("云端模型");
    }
    engine.setConfig(getConfig());
    auto result = engine.initRecognizeOnceModule();
    if (!result.success) {
        LOGE("Baidu speech engine init error:", result.error.getCode(), result.error.getMessage());
        return;
    }

    engine.setRecognitionResultCallback([](const ai_engine::core_ai::speech::RecognitionResult &result) {
        LOGI(result.text, result.error.getCode(), result.error.getMessage(), int(result.reason), result.speakerId);
    });
    engine.recognizeOnce("", audioData);

    while (std::getchar() != '\n') {
    }
    engine.destroyRecognizeOnceModule();
}

void testOnceSynthesize() {
    namespace fs = std::filesystem;

    ai_engine::EngineError error;

    BaiduSpeechEnginePrivate baiduSpeechEngine;

    if (baiduSpeechEngine.isCloud()) {
        LOGI("云端模型");
    }

    baiduSpeechEngine.setConfig(getConfig());

    auto result = baiduSpeechEngine.initSynthesizeModule();
    if (!result.success) {
        LOGE("Baidu speech engine init error:", result.error.getCode(), result.error.getMessage());
        return;
    }

    AudioPlayer audioPlayer;
    int i = 0;
    baiduSpeechEngine.setSynthesisResultCallback(
        [&audioPlayer, &i](const ai_engine::core_ai::speech::SynthesisResult &result) {
            auto data = baidu_speech_util::base64Decode(result.base64AudioData);
            audioPlayer.playing(data.c_str(), data.size());
            ++i;
            LOGI(i, int(result.reason), result.base64AudioData.c_str(), result.error.getCode(),
                 result.error.getMessage().c_str());
        });
    std::string message;
    for (int i = 0; i < 2500; i++) {
        message += "你好，";
    }

    result = baiduSpeechEngine.synthesizeText(
        R"(
            {
                "speed": 50,
                "volume": 50,
                "pitch": 50
            }
        )",
        message);
    if (!result.success) {
        LOGE(result.error.getCode(), result.error.getMessage());
    }
    while (std::getchar() != '\n') {
        break;
    }

    baiduSpeechEngine.destroySynthesizeModule();
}

void getModelInfo() {
    BaiduSpeechEnginePrivate baiduSpeechEngine;
    LOGD("engine name is", baiduSpeechEngine.engineName());
    LOGD("engine current model name is", baiduSpeechEngine.currentModel());
    LOGD("engine modelinfo is", baiduSpeechEngine.modelInfo());
}

int main() {
    getModelInfo();
    testContinuousRecognition();
    testOnceRecognition();
    testOnceSynthesize();
    return 0;
}
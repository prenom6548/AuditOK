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

#include <unistd.h>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include "audioplayer.h"
#include "audiorecorder.h"
#include "logger.h"
#include "ondevicespeechengine.h"
#include "util.h"

namespace ai_engine::core_ai::speech {

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
    file.read(reinterpret_cast<std::istream::char_type *>(audioData.data()), fileSize);
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
    OndeviceSpeechEngine engine{};
    if (!engine.isCloud()) {
        LOGI("Engine is ondevice.");
    }
    engine.setRecognitionResultCallback([](const ai_engine::core_ai::speech::RecognitionResult &result) {
        LOGI("===========================================================");
        LOGI("text:", result.text);
        LOGI("error code:", result.error.getCode());
        LOGI("error message:", result.error.getMessage());
        LOGI("reason:", int(result.reason));
        LOGI("speaker id:", result.speakerId);
    });

    auto start = std::chrono::high_resolution_clock::now();
    auto result = engine.initContinuousRecognitionModule();
    if (!result.success) {
        LOGE("Init continuous recognition module failed, error info:", result.error.getCode(),
             result.error.getMessage(), ".");
        return;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "函数耗时: " << duration.count() << " 毫秒" << std::endl;

    std::atomic_bool isRunning = true;
    AudioRecorder audioRecorder{};
    std::thread thread([&engine, &isRunning, &audioRecorder]() {
        auto result = engine.startContinuousRecognition(R"({"format": "", "rate": 16000, "channel": 1})");
        if (!result.success) {
            LOGE("Start continuous recognition failed, error info:", result.error.getCode(), result.error.getMessage(),
                 ".");
            return;
        }
        while (isRunning) {
            std::vector<uint8_t> audioData;
            audioRecorder.recording(audioData);
            result = engine.writeContinuousRecognitionAudioData(audioData);
            if (!result.success) {
                LOGE(
                    "Write continuous recognition audio data failed, error "
                    "info:",
                    result.error.getCode(), result.error.getMessage(), ".");
                break;
            }
            usleep(40 * 1000);
        }
    });
    while (std::getchar() != '\n') {
    }

    isRunning = false;

    result = engine.stopContinuousRecognition();
    thread.join();

    result = engine.destroyContinuousRecognitionModule();
}

void testOnceRecognition() {
    std::vector<uint8_t> audioData = getData();

    ai_engine::EngineError error;

    OndeviceSpeechEngine engine;
    if (engine.isCloud()) {
        LOGI("云端模型");
    }

    engine.setRecognitionResultCallback([](const ai_engine::core_ai::speech::RecognitionResult &result) {
        LOGI(result.text, result.error.getCode(), result.error.getMessage(), int(result.reason), result.speakerId);
    });

    auto result = engine.initRecognizeOnceModule();
    if (!result.success) {
        LOGE("init recognize once module failed", error.getCode(), error.getMessage());
        return;
    }

    result = engine.recognizeOnce("", audioData);
    if (!result.success) {
        LOGE("recognize once failed");
        return;
    }

    while (std::getchar() != '\n') {
    }
    engine.destroyRecognizeOnceModule();
}

void testOnceSynthesize() {
    ai_engine::EngineError error;

    OndeviceSpeechEngine engine;

    if (engine.isCloud()) {
        LOGI("云端模型");
    }

    engine.setSynthesisResultCallback([](const ai_engine::core_ai::speech::SynthesisResult &result) {
        LOGD("Synthesis callback received:");
        LOGD("  Success:", (result.success ? "true" : "false"));
        LOGD("  Model name:", result.modelName);
        LOGD("  Audio format:", result.audioFormat);
        LOGD("  Audio channel:", result.audioChannel);
        LOGD("  Audio rate:", result.audioRate);

        if (result.success) {
            // 使用util中的base64解码器解码音频数据并保存为PCM文件
            try {
                std::string decoded_audio = ondevice_speech_util::base64Decode(result.base64AudioData);
                LOGD("Decoded audio data size:", decoded_audio.size(), "bytes");

                // 将解码后的音频转换为vector
                std::vector<uint8_t> audioData(decoded_audio.begin(), decoded_audio.end());

                // 播放音频
                AudioPlayer player;
                player.playing(audioData);

                // 保存重采样后的PCM数据到文件
                std::ofstream pcmFile("output_audio_16k.pcm", std::ios::binary);
                if (pcmFile.is_open()) {
                    pcmFile.write(reinterpret_cast<const char *>(audioData.data()), audioData.size());
                    pcmFile.close();
                    LOGI("16kHz PCM audio data saved to output_audio_16k.pcm");
                } else {
                    LOGE("Failed to open PCM file for writing");
                }

            } catch (const std::exception &e) {
                LOGE("Error decoding base64 audio data: ", e.what());
            }

            LOGD("Base64 audio data length: ", result.base64AudioData.length(), "characters");

        } else {
            LOGE("Synthesis failed: ");
        }
    });

    // 初始化合成模块
    auto initResult = engine.initSynthesizeModule();
    if (!initResult.success) {
        LOGE("Failed to initialize synthesis module: ", initResult.error.getCode(), initResult.error.getMessage());
    }

    LOGI("Synthesis module initialized successfully");

    // 准备合成参数（根据实际需要调整）
    std::string params = "{speaker:0}";  // 空参数对象，根据实际API要求可能需要特定格式

    // 要合成的文本
    // std::string textToSynthesize = "今天1天气真nice。";
    std::string textToSynthesize =
        "在这个快节奏的时代，人们常常被各种信息和任务包围，难以静下心来感受生活的本真。我们追逐效率，却忽略了内心的节奏"
        "；我们渴望成功，却忘记了幸福往往藏在微小的细节里。清晨的一缕阳光、午后的一杯热茶、夜晚与家人的一句闲聊，这些看"
        "似平凡的瞬间，其实构成了生命中最珍贵的部分。学会放慢脚步，并不是懈怠，而是为了更好地前行。当我们不再一味地向外"
        "索求，而是向内探寻，便能发现内心早已拥有的富足与安宁。真正的成长，不只是能力的提升，更是心灵的成熟。它体现在面"
        "对挫折时的从容，对待他人时的善意，以及对自我价值的坚定认同。生活不会永远一帆风顺，但只要心中有光，就能在黑暗中"
        "找到方向。愿我们都能在喧嚣中守住内心的宁静，在纷繁中保持清醒的头脑，用温柔而坚定的态度，走过每一个平凡却不平庸"
        "的日子。";

    LOGD("Synthesizing text:", textToSynthesize);

    // 执行语音合成
    auto synthesisResult = engine.synthesizeText(params, textToSynthesize);

    if (synthesisResult.success) {
        LOGI("Text synthesis completed successfully");
    } else {
        LOGE("Text synthesis failed: ");
    }

    // 销毁合成模块
    auto destroyResult = engine.destroySynthesizeModule();
    if (!destroyResult.success) {
        LOGE("Failed to destroy synthesis module: ");
    } else {
        LOGI("Synthesis module destroyed successfully");
    }
}

void testAsyncSynthesize() {
    ai_engine::EngineError error;

    OndeviceSpeechEngine engine;

    if (engine.isCloud()) {
        LOGI("云端模型");
    }

    // 用于等待异步操作完成的标志和锁
    std::mutex mtx;
    std::condition_variable cv;
    bool synthesisCompleted = false;

    // 用于存储音频片段的队列
    std::vector<std::vector<uint8_t>> audioQueue;
    std::mutex queueMutex;

    engine.setSynthesisResultCallback([&](const ai_engine::core_ai::speech::SynthesisResult &result) {
        LOGD("Async Synthesis callback received:");
        LOGD("  Success:", (result.success ? "true" : "false"));
        LOGD("  Model name:", result.modelName);
        LOGD("  Audio format:", result.audioFormat);
        LOGD("  Audio channel:", result.audioChannel);
        LOGD("  Audio rate:", result.audioRate);
        LOGD("  Reason:", static_cast<int>(result.reason));

        // 添加详细的reason打印
        switch (result.reason) {
            case ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesisStarted:
                LOGI("  Reason detail: SpeechSynthesisStarted");
                break;
            case ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesizing:
                LOGI("  Reason detail: SpeechSynthesizing");
                break;
            case ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesisCompleted:
                LOGI("  Reason detail: SpeechSynthesisCompleted");
                break;
            default:
                LOGI("  Reason detail: Unknown reason code:", static_cast<int>(result.reason));
                break;
        }

        if (result.success) {
            // 使用util中的base64解码器解码音频数据
            try {
                std::string decoded_audio = ondevice_speech_util::base64Decode(result.base64AudioData);
                LOGD("Decoded audio data size:", decoded_audio.size(), "bytes");

                // 将解码后的音频转换为vector
                std::vector<uint8_t> audioData(decoded_audio.begin(), decoded_audio.end());

                // 添加到音频队列中
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    audioQueue.push_back(audioData);
                }

                // 播放当前音频片段
                AudioPlayer player;
                player.playing(audioData);

                LOGD("Base64 audio data length: ", result.base64AudioData.length(), "characters");

            } catch (const std::exception &e) {
                LOGE("Error decoding base64 audio data: ", e.what());
            }
        } else {
            LOGE("Synthesis failed: ");
        }

        // 当合成完成时通知主线程
        if (result.reason == ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesisCompleted) {
            LOGI("Received SpeechSynthesisCompleted signal, processing final audio data");

            // 保存完整的音频数据到文件
            std::lock_guard<std::mutex> lock(queueMutex);
            std::vector<uint8_t> completeAudioData;

            // 合并所有音频片段
            LOGI("Total audio segments received:", audioQueue.size());
            for (size_t i = 0; i < audioQueue.size(); ++i) {
                LOGI("Processing segment", i, "with size:", audioQueue[i].size(), "bytes");
                completeAudioData.insert(completeAudioData.end(), audioQueue[i].begin(), audioQueue[i].end());
            }

            // 保存完整的PCM数据到文件
            std::ofstream pcmFile("output_audio_async_16k.pcm", std::ios::binary);
            if (pcmFile.is_open()) {
                pcmFile.write(reinterpret_cast<const char *>(completeAudioData.data()), completeAudioData.size());
                pcmFile.close();
                LOGI("Complete 16kHz PCM audio data saved to output_audio_async_16k.pcm, total size:",
                     completeAudioData.size(), "bytes");
            } else {
                LOGE("Failed to open PCM file for writing");
            }

            // 通知主线程合成完成
            std::lock_guard<std::mutex> lock2(mtx);
            synthesisCompleted = true;
            cv.notify_one();
        }
    });

    // 初始化合成模块
    auto initResult = engine.initSynthesizeModule();
    if (!initResult.success) {
        LOGE("Failed to initialize synthesis module: ", initResult.error.getCode(), initResult.error.getMessage());
        return;
    }

    LOGI("Synthesis module initialized successfully");

    // 准备合成参数（根据实际需要调整）
    std::string params = "{speaker:0}";  // 空参数对象，根据实际API要求可能需要特定格式

    // 要合成的文本
    std::string textToSynthesize =
        "我是麒麟AI助手，由麒麟软件开发的桌面系统级智能助手。基于多种主流大模型的能力，为您提供文本问答、语音对话、文本"
        "辅助创作、整理周报、会议助手等多种人工智能助理服务。";

    LOGD("Synthesizing text:", textToSynthesize);

    // 执行语音合成（异步）
    auto synthesisResult = engine.synthesizeText(params, textToSynthesize);

    if (synthesisResult.success) {
        LOGI("Text synthesis request sent successfully, waiting for completion...");
    } else {
        LOGE("Text synthesis failed: ");
        return;
    }

    // 等待异步操作完成（增加超时时间）
    std::unique_lock<std::mutex> lock(mtx);
    if (!cv.wait_for(lock, std::chrono::seconds(60), [&] { return synthesisCompleted; })) {
        LOGE("Synthesis timed out after 60 seconds");
    }

    LOGI("Synthesis process completed or timed out");

    // 销毁合成模块
    // auto destroyResult = engine.destroySynthesizeModule();
    // if (!destroyResult.success) {
    //     LOGE("Failed to destroy synthesis module: ");
    // } else {
    //     LOGI("Synthesis module destroyed successfully");
    // }
}

}  // namespace ai_engine::core_ai::speech

int main(int argc, char *argv[]) {
    // ai_engine::core_ai::speech::testContinuousRecognition();
    // ai_engine::core_ai::speech::testOnceRecognition();
    // ai_engine::core_ai::speech::testOnceSynthesize();
    ai_engine::core_ai::speech::testAsyncSynthesize();
    return 0;
}
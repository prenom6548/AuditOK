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

#include <atomic>
#include <chrono>
#include <cmath>  // 添加数学库头文件
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
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

std::vector<uint8_t> readAudioData(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file:", filePath);
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> audioData(fileSize);
    file.read(reinterpret_cast<std::istream::char_type*>(audioData.data()), fileSize);
    return audioData;
}

std::vector<uint8_t> getTestAudioData() {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "16k.pcm";
    if (not fs::exists(path)) {
        LOGW("Warning: test audio file not exists, using simulated voice data");

        // 创建模拟的语音数据，包含明显的语音活动模式
        std::vector<uint8_t> audioData(128000);

        // 生成基础噪声（低能量背景）
        for (size_t i = 0; i < audioData.size(); ++i) {
            // 基础白噪声，范围在较小值
            audioData[i] = static_cast<uint8_t>(rand() % 30);
        }

        // 添加几个语音段落（高能量区域）
        // 第一个语音段：位置 20000-30000 (大约1.25-1.875秒)
        for (size_t i = 20000; i < 30000 && i < audioData.size(); ++i) {
            // 正弦波模拟语音，叠加一些随机噪声
            double t = (i - 20000) * 0.001;                                   // 时间变量
            int amplitude = static_cast<int>(100 * sin(2 * M_PI * 200 * t));  // 200Hz频率
            audioData[i] = static_cast<uint8_t>(std::max(0, std::min(255, 128 + amplitude + (rand() % 40 - 20))));
        }

        // 第二个语音段：位置 50000-65000 (大约3.125-4.0625秒)
        for (size_t i = 50000; i < 65000 && i < audioData.size(); ++i) {
            double t = (i - 50000) * 0.0015;                                 // 不同频率
            int amplitude = static_cast<int>(80 * sin(2 * M_PI * 300 * t));  // 300Hz频率
            audioData[i] = static_cast<uint8_t>(std::max(0, std::min(255, 128 + amplitude + (rand() % 30 - 15))));
        }

        // 第三个语音段：位置 80000-95000 (大约5-5.9375秒)
        for (size_t i = 80000; i < 95000 && i < audioData.size(); ++i) {
            double t = (i - 80000) * 0.0012;                                 // 另一种频率模式
            int amplitude = static_cast<int>(90 * sin(2 * M_PI * 250 * t));  // 250Hz频率
            audioData[i] = static_cast<uint8_t>(std::max(0, std::min(255, 128 + amplitude + (rand() % 35 - 17))));
        }

        // 在语音段之间保持静音（低能量）
        LOGI("Generated simulated audio data with 3 voice segments");
        LOGI("Segment 1: 20000-30000 (1.25-1.875s)");
        LOGI("Segment 2: 50000-65000 (3.125-4.0625s)");
        LOGI("Segment 3: 80000-95000 (5-5.9375s)");

        return audioData;
    }
    return readAudioData(path.string());
}

void testSingleEngineCycle(int iteration) {
    LOGI("=== Starting test iteration", iteration, "===");

    // 构造引擎
    OndeviceSpeechEngine engine{};

    LOGI("Engine constructed in iteration", iteration);

    // 检查引擎属性
    if (!engine.isCloud()) {
        LOGI("Engine is ondevice in iteration", iteration);
    }

    LOGI("Engine name:", engine.engineName());
    LOGI("Current model:", engine.currentModel());

    engine.setCurrentContinuousRecognitionModel("paraformer-online");

    // 设置回调函数
    engine.setRecognitionResultCallback([iteration](const RecognitionResult& result) {
        LOGI("Iteration", iteration, "- Recognition result:");
        LOGI("  Text:", result.text);
        LOGI("  Error code:", result.error.getCode());
        LOGI("  Error message:", result.error.getMessage());
        LOGI("  Reason:", int(result.reason));
        LOGI("  Speaker ID:", result.speakerId);
    });

    // 初始化连续识别模块
    auto startTime = std::chrono::high_resolution_clock::now();
    auto result = engine.initContinuousRecognitionModule();
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    if (!result.success) {
        LOGE("Iteration", iteration, "- Init continuous recognition module failed:");
        LOGE("  Error code:", result.error.getCode());
        LOGE("  Error message:", result.error.getMessage());
        return;
    }

    LOGI("Iteration", iteration, "- Continuous recognition module initialized successfully");
    LOGI("  Initialization time:", duration.count(), "ms");

    // 启动连续识别
    result = engine.startContinuousRecognition(R"({"format": "", "rate": 16000, "channel": 1})");
    if (!result.success) {
        LOGE("Iteration", iteration, "- Start continuous recognition failed:");
        LOGE("  Error code:", result.error.getCode());
        LOGE("  Error message:", result.error.getMessage());
        engine.destroyContinuousRecognitionModule();
        return;
    }

    LOGI("Iteration", iteration, "- Continuous recognition started successfully");

    // 写入音频数据
    std::vector<uint8_t> audioData = getTestAudioData();
    if (!audioData.empty()) {
        // 分批写入音频数据，模拟实时录音
        size_t chunkSize = 1280;
        size_t totalWritten = 0;

        for (size_t i = 0; i < audioData.size(); i += chunkSize) {
            size_t currentChunkSize = std::min(chunkSize, audioData.size() - i);
            std::vector<uint8_t> chunk(audioData.begin() + i, audioData.begin() + i + currentChunkSize);

            result = engine.writeContinuousRecognitionAudioData(chunk);
            if (!result.success) {
                LOGE("Iteration", iteration, "- Write audio data failed at byte", i);
                LOGE("  Error code:", result.error.getCode());
                LOGE("  Error message:", result.error.getMessage());
                break;
            }

            totalWritten += currentChunkSize;
            // usleep(10 * 1000);  // 10ms delay between chunks
        }

        LOGI("Iteration", iteration, "- Wrote", totalWritten, "bytes of audio data");
    } else {
        LOGW("Iteration", iteration, "- No audio data available, skipping write test");
    }

    // 停止连续识别
    result = engine.stopContinuousRecognition();
    if (!result.success) {
        LOGE("Iteration", iteration, "- Stop continuous recognition failed:");
        LOGE("  Error code:", result.error.getCode());
        LOGE("  Error message:", result.error.getMessage());
    } else {
        LOGI("Iteration", iteration, "- Continuous recognition stopped successfully");
    }

    // 销毁连续识别模块
    result = engine.destroyContinuousRecognitionModule();
    if (!result.success) {
        LOGE("Iteration", iteration, "- Destroy continuous recognition module failed:");
        LOGE("  Error code:", result.error.getCode());
        LOGE("  Error message:", result.error.getMessage());
    } else {
        LOGI("Iteration", iteration, "- Continuous recognition module destroyed successfully");
    }

    LOGI("=== Completed test iteration", iteration, "===");
}

void testMultipleConstructionDestruction(int numIterations = 10) {
    LOGI("Starting continuous construction/destruction test with", numIterations, "iterations");

    auto testStartTime = std::chrono::high_resolution_clock::now();

    for (int i = 1; i <= numIterations; ++i) {
        auto iterationStartTime = std::chrono::high_resolution_clock::now();

        try {
            testSingleEngineCycle(i);
        } catch (const std::exception& e) {
            LOGE("Exception in iteration", i, ":", e.what());
        } catch (...) {
            LOGE("Unknown exception in iteration", i);
        }

        auto iterationEndTime = std::chrono::high_resolution_clock::now();
        auto iterationDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(iterationEndTime - iterationStartTime);

        LOGI("Iteration", i, "completed in", iterationDuration.count(), "ms");

        // 短暂延迟避免过于频繁的构造销毁
        if (i < numIterations) {
            usleep(100 * 1000);  // 100ms delay between iterations
        }
    }

    auto testEndTime = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(testEndTime - testStartTime);

    LOGI("=== Test Summary ===");
    LOGI("Total iterations:", numIterations);
    LOGI("Total test time:", totalDuration.count(), "ms");
    LOGI("Average time per iteration:", totalDuration.count() / numIterations, "ms");
    LOGI("===================");
}

void testConcurrentConstructionDestruction() {
    LOGI("Starting concurrent construction/destruction test");

    const int numThreads = 3;
    const int iterationsPerThread = 5;

    std::vector<std::thread> threads;
    std::atomic<int> completedIterations{0};

    auto startTime = std::chrono::high_resolution_clock::now();

    // 创建多个线程同时进行测试
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([t, iterationsPerThread, &completedIterations]() {
            LOGI("Thread", t, "started");

            for (int i = 1; i <= iterationsPerThread; ++i) {
                int globalIteration = t * iterationsPerThread + i;

                try {
                    testSingleEngineCycle(globalIteration);
                    completedIterations++;
                } catch (const std::exception& e) {
                    LOGE("Thread", t, "exception in iteration", i, ":", e.what());
                } catch (...) {
                    LOGE("Thread", t, "unknown exception in iteration", i);
                }

                if (i < iterationsPerThread) {
                    usleep(50 * 1000);  // 50ms delay between iterations
                }
            }

            LOGI("Thread", t, "completed");
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    LOGI("=== Concurrent Test Summary ===");
    LOGI("Number of threads:", numThreads);
    LOGI("Iterations per thread:", iterationsPerThread);
    LOGI("Total iterations:", numThreads * iterationsPerThread);
    LOGI("Completed iterations:", completedIterations.load());
    LOGI("Total test time:", totalDuration.count(), "ms");
    LOGI("Average time per iteration:", totalDuration.count() / (numThreads * iterationsPerThread), "ms");
    LOGI("=============================");
}

}  // namespace ai_engine::core_ai::speech

int main(int argc, char* argv[]) {
    LOGI("OndeviceSpeechEngine Continuous Construction/Destruction Test");
    LOGI("============================================================");

    int testType = 0;
    int iterations = 10;

    // 解析命令行参数
    if (argc > 1) {
        testType = std::atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = std::atoi(argv[2]);
    }

    switch (testType) {
        case 1:
            LOGI("Running sequential multiple construction/destruction test");
            ai_engine::core_ai::speech::testMultipleConstructionDestruction(iterations);
            break;
        case 2:
            LOGI("Running concurrent construction/destruction test");
            ai_engine::core_ai::speech::testConcurrentConstructionDestruction();
            break;
        default:
            LOGI("Running default sequential test");
            ai_engine::core_ai::speech::testMultipleConstructionDestruction(iterations);
            break;
    }

    LOGI("Test completed successfully");
    return 0;
}
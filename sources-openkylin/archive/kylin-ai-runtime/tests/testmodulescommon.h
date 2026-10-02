/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#ifndef TESTMODULES_COMMON_H
#define TESTMODULES_COMMON_H

#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "kylin-ai/ai-engine/imagegenerationengine.h"
#include "kylin-ai/ai-engine/imageprocessengine.h"
#include "kylin-ai/ai-engine/speechengine.h"
#include "kylin-ai/ai-engine/textrecognitionengine.h"
#include "kylin-ai/common/error.h"

#define private public
#include "taskmanager/taskrunner.h"
#undef private

class ScopedTaskConfigOverride {
public:
    ScopedTaskConfigOverride(const std::string& content, bool writeFile) {
        auto dir =
            std::filesystem::temp_directory_path() / ("kylin-ai-runtime-testmodules-" + std::to_string(getpid()));
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        configPath_ = dir / "task.json";

        task::setTaskRunnerConfigPathForTesting(configPath_);

        if (writeFile) {
            std::ofstream ofs(configPath_);
            ofs << content;
            ofs.close();
        } else {
            std::error_code removeEc;
            std::filesystem::remove(configPath_, removeEc);
        }
    }

    ~ScopedTaskConfigOverride() {
        task::clearTaskRunnerConfigPathForTesting();
        std::error_code ec;
        std::filesystem::remove(configPath_, ec);
        if (!configPath_.parent_path().empty()) {
            std::filesystem::remove(configPath_.parent_path(), ec);
        }
    }

private:
    std::filesystem::path configPath_;
};

class ScopedPendingLimits {
public:
    ScopedPendingLimits(size_t maxTotal, size_t maxPerType) {
        task::TaskRunner::getInstance().setPendingLimitsForTesting(maxTotal, maxPerType);
    }
    ~ScopedPendingLimits() { task::TaskRunner::getInstance().setPendingLimitsForTesting(10000, 2000); }
};

struct CallbackState {
    std::mutex mutex;
    std::condition_variable cv;
    int finished{0};
    int results{0};
    int lastError{0};
};

inline bool waitFor(std::condition_variable& cv, std::mutex& mutex, const std::function<bool()>& predicate,
                    std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex);
    return cv.wait_for(lock, timeout, predicate);
}

inline std::string makeTimeoutConfig(const std::string& taskType, int64_t timeoutSec) {
    std::ostringstream oss;
    oss << "{\n"
        << "  \"taskTimeoutSec\": {\n"
        << "    \"" << taskType << "\": " << timeoutSec << "\n"
        << "  }\n"
        << "}\n";
    return oss.str();
}

inline std::string makeRawAudioConfigJson() {
    std::ostringstream oss;
    oss << "{\n"
        << "  \"input_source\": 3,\n"
        << "  \"raw_data\": \"AA==\",\n"
        << "  \"sample_rate\": 16000,\n"
        << "  \"output_target\": 0\n"
        << "}\n";
    return oss.str();
}

class FakeImageGenerationEngine : public ai_engine::gen_ai::vision::AbstractImageGenerationEngine {
public:
    FakeImageGenerationEngine(bool success, bool block, int sleepMs, bool emitCallback, int errorCode)
        : success_(success), block_(block), sleepMs_(sleepMs), emitCallback_(emitCallback), errorCode_(errorCode) {}

    std::string engineName() const override { return "fake-image-engine"; }
    bool isCloud() const override { return false; }
    bool isBuiltInEngine() const override { return false; }
    std::string modelInfo() const override { return "{}"; }
    std::string currentModel() const override { return "fake-image-model"; }
    std::string currentModelName() const override { return "fake-image-model"; }

    void setGenerationResultCallback(ai_engine::gen_ai::vision::ImageGenerationCallback callback) override {
        callback_ = std::move(callback);
    }
    ai_engine::result::AiEngineBaseResult init() override { return {true, ai_engine::EngineError()}; }
    ai_engine::result::AiEngineBaseResult destroy() override { return {true, ai_engine::EngineError()}; }

    std::vector<std::string> supportedImageStyles() const override { return {"default"}; }
    int supportedImageNumber() const override { return 1; }
    std::vector<std::pair<int, int>> supportedImageSizes() const override { return {{256, 256}}; }
    void setImageStyle(const std::string&) override {}
    void setImageNumber(int) override {}
    void setImageSize(int, int) override {}

    ai_engine::result::AiEngineBaseResult generateImage(const std::string&) override {
        if (block_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() { return stopRequested_.load(); });
        } else if (sleepMs_ > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs_));
        }
        if (emitCallback_ && callback_) {
            ai_engine::gen_ai::vision::ImageGenerationResult result{};
            result.success = true;
            result.modelName = "fake-image-model";
            result.format = "png";
            result.width = 256;
            result.height = 256;
            result.total = 1;
            result.index = 0;
            result.imageBase64Data = "data";
            callback_(result);
        }
        if (!success_) {
            return {false, ai_engine::EngineError(errorCode_, "image failed")};
        }
        return {true, ai_engine::EngineError()};
    }

    void stopImageGeneration() override {
        stopRequested_.store(true);
        cv_.notify_all();
    }

private:
    ai_engine::gen_ai::vision::ImageGenerationCallback callback_{};
    bool success_{true};
    bool block_{false};
    int sleepMs_{0};
    bool emitCallback_{false};
    int errorCode_{-1};
    std::atomic_bool stopRequested_{false};
    std::mutex mutex_;
    std::condition_variable cv_;
};

class FakeTextRecognitionEngine : public ai_engine::core_ai::vision::AbstractTextRecognitionEngine {
public:
    FakeTextRecognitionEngine(bool success, int sleepMs, bool emitCallback, int errorCode)
        : success_(success), sleepMs_(sleepMs), emitCallback_(emitCallback), errorCode_(errorCode) {}

    std::string engineName() const override { return "fake-ocr-engine"; }
    bool isCloud() const override { return false; }
    bool isBuiltInEngine() const override { return false; }
    std::string modelInfo() const override { return "{}"; }
    std::string currentModel() const override { return currentModel_; }
    std::string currentModelName() const override { return currentModel_; }
    void setCurrentTextRecognitionModel(const std::string& model) override { currentModel_ = model; }

    void setTextRecognitionResultCallback(ai_engine::core_ai::vision::TextRecognitionResultCallback callback) override {
        callback_ = std::move(callback);
    }
    ai_engine::result::AiEngineBaseResult init() override { return {true, ai_engine::EngineError()}; }
    ai_engine::result::AiEngineBaseResult destroy() override { return {true, ai_engine::EngineError()}; }

    ai_engine::result::AiEngineBaseResult recognizeTextFromFile(const std::string&) override { return run(); }
    ai_engine::result::AiEngineBaseResult recognizeTextFromBase64ImageData(const std::string&) override {
        return run();
    }
    void stopTextRecognition() override { stopRequested_.store(true); }

private:
    ai_engine::result::AiEngineBaseResult run() {
        if (sleepMs_ > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs_));
        }
        if (stopRequested_.load()) {
            return {true, ai_engine::EngineError()};
        }
        if (emitCallback_ && callback_) {
            ai_engine::core_ai::vision::TextRecognitionResult result{};
            result.success = true;
            result.allText = "ok";
            callback_(result);
        }
        if (!success_) {
            return {false, ai_engine::EngineError(errorCode_, "ocr failed")};
        }
        return {true, ai_engine::EngineError()};
    }

    ai_engine::core_ai::vision::TextRecognitionResultCallback callback_{};
    bool success_{true};
    int sleepMs_{0};
    bool emitCallback_{false};
    int errorCode_{-1};
    std::atomic_bool stopRequested_{false};
    std::string currentModel_{"fake-ocr-model"};
};

class FakeImageProcessEngine : public ai_engine::core_ai::vision::AbstractImageProcessEngine {
public:
    FakeImageProcessEngine(bool success, int sleepMs, bool emitCallback, int errorCode)
        : success_(success), sleepMs_(sleepMs), emitCallback_(emitCallback), errorCode_(errorCode) {}

    std::string engineName() const override { return "fake-matting-engine"; }
    bool isCloud() const override { return false; }
    bool isBuiltInEngine() const override { return false; }
    std::string modelInfo() const override { return "{}"; }
    std::string currentModel() const override { return currentModel_; }
    std::string currentModelName() const override { return currentModel_; }
    void setCurrentPortraitSegmentationModel(const std::string& model) override { portraitSegmentationModel_ = model; }
    void setCurrentGeneralSegmentationModel(const std::string& model) override { generalSegmentationModel_ = model; }

    ai_engine::result::AiEngineBaseResult initMattingsModule() override { return {true, ai_engine::EngineError()}; }
    ai_engine::result::AiEngineBaseResult destroyMattingsModule() override { return {true, ai_engine::EngineError()}; }
    void setMattingResultCallback(ai_engine::core_ai::vision::ImageResultCallback callback) override {
        callback_ = std::move(callback);
    }

    ai_engine::result::AiEngineBaseResult mattingFromBase64ImageData(const std::string&,
                                                                     ai_engine::core_ai::vision::MattingMode) override {
        return run();
    }
    ai_engine::result::AiEngineBaseResult mattingFromImageFile(const std::string&,
                                                               ai_engine::core_ai::vision::MattingMode) override {
        return run();
    }
    void stopMattingProcess() override {
        stopRequested_.store(true);
        cv_.notify_all();
    }

    ai_engine::result::AiEngineBaseResult initSegmentationModule() override { return {true, ai_engine::EngineError()}; }
    ai_engine::result::AiEngineBaseResult destroySegmentationModule() override {
        return {true, ai_engine::EngineError()};
    }
    void setSegmentationResultCallback(
        const ai_engine::core_ai::vision::SegmentationResultCallback& callback) override {
        segmentationCallback_ = callback;
    }
    ai_engine::result::AiEngineBaseResult encodeSegmentationFromBase64ImageData(const std::string&) override {
        return runSegmentation();
    }
    ai_engine::result::AiEngineBaseResult encodeSegmentationFromImageFile(const std::string&) override {
        return runSegmentation();
    }
    ai_engine::result::AiEngineBaseResult decodeSegmentation(
        const std::vector<ai_engine::core_ai::vision::SegmentationPoint>&, uint32_t) override {
        return runSegmentation();
    }
    void clearSegmentationCache() override {}
    ai_engine::result::AiEngineBaseResult stopSegmentation() override {
        stopRequested_.store(true);
        cv_.notify_all();
        return {true, ai_engine::EngineError()};
    }
    void setCurrentSegmentationModel(const std::string& model) override { currentModel_ = model; }

    ai_engine::result::AiEngineBaseResult initImageBinaryModule() override { return {true, ai_engine::EngineError()}; }
    ai_engine::result::AiEngineBaseResult destroyImageBinaryModule() override {
        return {true, ai_engine::EngineError()};
    }
    void setImageBinaryResultCallback(ai_engine::core_ai::vision::ImageResultCallback) override {}
    ai_engine::result::AiEngineBaseResult binarizeImageFromBase64ImageData(const std::string&) override {
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult binarizeImageFromImageFile(const std::string&) override {
        return {true, ai_engine::EngineError()};
    }
    void stopImageBinarization() override {}

private:
    ai_engine::result::AiEngineBaseResult run() {
        if (sleepMs_ > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs_));
        }
        if (emitCallback_ && callback_) {
            ai_engine::core_ai::vision::ImageResult result{};
            result.success = true;
            result.imageFormat = "png";
            result.imageBase64Data = "data";
            result.maskBase64Data = "mask";
            result.width = 1;
            result.height = 1;
            callback_(result);
        }
        if (!success_) {
            return {false, ai_engine::EngineError(errorCode_, "matting failed")};
        }
        if (stopRequested_.load()) {
            return {true, ai_engine::EngineError()};
        }
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult runSegmentation() {
        if (sleepMs_ > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs_));
        }
        if (!success_) {
            return {false, ai_engine::EngineError(errorCode_, "segmentation failed")};
        }
        if (stopRequested_.load()) {
            return {true, ai_engine::EngineError()};
        }
        return {true, ai_engine::EngineError()};
    }

    ai_engine::core_ai::vision::ImageResultCallback callback_{};
    ai_engine::core_ai::vision::SegmentationResultCallback segmentationCallback_{};
    bool success_{true};
    int sleepMs_{0};
    bool emitCallback_{false};
    int errorCode_{-1};
    std::atomic_bool stopRequested_{false};
    std::string currentModel_{"fake-matting-model"};
    std::string portraitSegmentationModel_{"fake-portrait-segmentation-model"};
    std::string generalSegmentationModel_{"fake-general-segmentation-model"};
    std::mutex mutex_;
    std::condition_variable cv_;
};

class FakeSpeechEngine : public ai_engine::core_ai::speech::AbstractSpeechEngine {
public:
    FakeSpeechEngine(bool recognizeSuccess, bool synthSuccess, int sleepMs, bool emitCallback, int errorCode)
        : recognizeSuccess_(recognizeSuccess),
          synthSuccess_(synthSuccess),
          sleepMs_(sleepMs),
          emitCallback_(emitCallback),
          errorCode_(errorCode) {}

    std::string engineName() const override { return "fake-speech-engine"; }
    bool isCloud() const override { return false; }
    bool isBuiltInEngine() const override { return false; }
    std::string modelInfo() const override { return "{}"; }
    bool setCurrentModel(const std::string& model) override {
        currentModel_ = model;
        return true;
    }
    std::string currentModel() const override { return currentModel_; }
    std::string currentModelName() const override { return currentModel_; }
    void setConfig(const std::string& config) override { config_ = config; }

    void setRecognitionResultCallback(ai_engine::core_ai::speech::RecognitionResultCallback callback) override {
        recognitionCallback_ = std::move(callback);
    }
    void setSynthesisResultCallback(ai_engine::core_ai::speech::SynthesisResultCallback callback) override {
        synthesisCallback_ = std::move(callback);
    }

    ai_engine::result::AiEngineBaseResult initContinuousRecognitionModule() override {
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult destroyContinuousRecognitionModule() override {
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult startContinuousRecognition(const std::string&) override {
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult writeContinuousRecognitionAudioData(const std::vector<uint8_t>&) override {
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult stopContinuousRecognition() override {
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult setCurrentContinuousRecognitionModel(const std::string& model) override {
        continuousRecognitionModel_ = model;
        return {true, ai_engine::EngineError()};
    }
    std::string currentContinuousRecognitionModel() const override { return continuousRecognitionModel_; }
    ai_engine::result::AiEngineBaseResult initRecognizeOnceModule() override {
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult destroyRecognizeOnceModule() override {
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult recognizeOnce(const std::string&, const std::vector<uint8_t>&) override {
        if (sleepMs_ > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs_));
        }
        if (emitCallback_ && recognitionCallback_) {
            ai_engine::core_ai::speech::RecognitionResult result{};
            result.success = recognizeSuccess_;
            result.text = "ok";
            result.speakerId = 1;
            result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognized;
            result.modelName = recognizeOnceModel_;
            if (!recognizeSuccess_) {
                result.error = ai_engine::EngineError(errorCode_, "recognize failed");
            }
            recognitionCallback_(result);
        }
        if (!recognizeSuccess_) {
            return {false, ai_engine::EngineError(errorCode_, "recognize failed")};
        }
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult stopRecognizeOnce() override { return {true, ai_engine::EngineError()}; }
    ai_engine::result::AiEngineBaseResult setCurrentRecognizeOnceModel(const std::string& model) override {
        recognizeOnceModel_ = model;
        return {true, ai_engine::EngineError()};
    }
    std::string currentRecognizeOnceModel() const override { return recognizeOnceModel_; }
    ai_engine::result::AiEngineBaseResult initSynthesizeModule() override { return {true, ai_engine::EngineError()}; }
    ai_engine::result::AiEngineBaseResult destroySynthesizeModule() override {
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult synthesizeText(const std::string&, const std::string&) override {
        if (sleepMs_ > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs_));
        }
        if (emitCallback_ && synthesisCallback_) {
            ai_engine::core_ai::speech::SynthesisResult result{};
            result.success = synthSuccess_;
            result.audioFormat = "pcm";
            result.audioRate = 16000;
            result.audioChannel = 1;
            result.base64AudioData = "data";
            result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesisCompleted;
            result.modelName = synthesizeModel_;
            if (!synthSuccess_) {
                result.error = ai_engine::EngineError(errorCode_, "synth failed");
            }
            synthesisCallback_(result);
        }
        if (!synthSuccess_) {
            return {false, ai_engine::EngineError(errorCode_, "synth failed")};
        }
        return {true, ai_engine::EngineError()};
    }
    ai_engine::result::AiEngineBaseResult stopSynthesize() override { return {true, ai_engine::EngineError()}; }
    ai_engine::result::AiEngineBaseResult setCurrentSynthesizeModel(const std::string& model) override {
        synthesizeModel_ = model;
        return {true, ai_engine::EngineError()};
    }
    std::string currentSynthesizeModel() const override { return synthesizeModel_; }

private:
    ai_engine::core_ai::speech::RecognitionResultCallback recognitionCallback_{};
    ai_engine::core_ai::speech::SynthesisResultCallback synthesisCallback_{};
    bool recognizeSuccess_{true};
    bool synthSuccess_{true};
    int sleepMs_{0};
    bool emitCallback_{false};
    int errorCode_{-1};
    std::string currentModel_{"fake-speech-model"};
    std::string continuousRecognitionModel_{"fake-speech-model"};
    std::string recognizeOnceModel_{"fake-speech-model"};
    std::string synthesizeModel_{"fake-speech-model"};
    std::string config_{};
};

#endif  // TESTMODULES_COMMON_H

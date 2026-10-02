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

#include "speechsynthesizer.h"

#include <jsoncpp/json/json.h>
#include <kylin-ai/common/error.h>

#include <chrono>
#include <filesystem>
#include <fstream>

#include "engineselector/engineselector.h"
#include "modelconfig/modelconfigmanager.h"
#include "modelconfig/modelstatustracker.h"
#include "taskmanager/taskrunner.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace {
constexpr int64_t kDefaultSpeechSynthesisTimeoutMs = 60LL * 60 * 1000;
constexpr size_t kDefaultSpeechSynthesisRequestQueueMaxSize = 10;

int64_t loadSpeechSynthesisTimeoutMs() {
    const auto taskTimeout = task::loadTaskTimeoutMs(task::TaskType::OnceTextToSpeech);
    if (taskTimeout.has_value()) {
        return *taskTimeout;
    }
    LOGW("Speech synthesis task timeout not found in task config, use default.");
    return kDefaultSpeechSynthesisTimeoutMs;
}

size_t loadSpeechSynthesisRequestQueueMaxSize() {
    const auto queueMaxSize = task::loadTaskQueueMaxSize(task::TaskType::OnceTextToSpeech);
    if (queueMaxSize.has_value()) {
        return static_cast<size_t>(*queueMaxSize);
    }
    LOGW("Speech synthesis request queue max size not found in task config, use default.");
    return kDefaultSpeechSynthesisRequestQueueMaxSize;
}

bool hasError(int errorCode) { return errorCode != AI_COMMON_SUCCESS && errorCode != -1; }

}  // namespace
SpeechSynthesizer::SpeechSynthesizer(bool isDefaultMode) : isDefaultMode_{isDefaultMode} {}

SpeechSynthesizer::~SpeechSynthesizer() {
    destroy();
    LOGD("~SpeechSynthesizer");
}

bool SpeechSynthesizer::init(int &errorCode, const ISpeechEnginePtr &speechEngine) {
    ISpeechEnginePtr initEngine{};
    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        speechEngine_ = speechEngine;
        initEngine = speechEngine_;
    }
    if (initEngine == nullptr) {
        errorCode = AI_COMMON_INVALID_PARAMETER;
        return false;
    }

    ModelStatusTracker modelStatusTracker(initEngine->currentModelName());
    auto result = initEngine->initSynthesizeModule();
    if (!result.success) {
        LOGE("Init synthesize once module engine error: {} {}", result.error.getCode(), result.error.getMessage());
        if (!isDefaultMode_) {
            // 非默认模式，直接报错
            errorCode = result.error.getCode();
            return false;
        }
    }

    bool needToReturnError{false};
    if (!result.success) {
        EngineSelector engineSelector{ai_engine::AiEnginePluginManager::getInstance(),
                                      ModelConfigManager::getInstance()};
        EngineSelector::CoreAiSpeechEnginePredicate predicate =
            [](const std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> &engine) {
                auto ondeviceResult = engine->initSynthesizeModule();
                return ondeviceResult.success;
            };
        auto fallbackEngineUnique =
            engineSelector.selectNextPriorityCoreAiSpeechEngine(currentEngineDeployType(), predicate);
        ISpeechEnginePtr fallbackEngine{std::move(fallbackEngineUnique)};

        {
            std::lock_guard<std::mutex> lock(engineMutex_);
            fallbackSpeechEngine_ = fallbackEngine;
            if (fallbackSpeechEngine_ == nullptr) {
                needToReturnError = true;
            }
        }
    }

    if (!result.success && needToReturnError) {
        errorCode = result.error.getCode();
        return false;
    }
    return true;
}

void SpeechSynthesizer::destroy() {
    if (destroying_.exchange(true)) {
        return;
    }
    stopSpeaking();
    synthesisResultCallback_ = nullptr;
    rejectPendingRequests(AI_COMMON_TASK_CANCELLED, "Speech synthesis task has been cancelled.");

    ISpeechEnginePtr speechEngine{};
    ISpeechEnginePtr fallbackSpeechEngine{};
    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        speechEngine = speechEngine_;
        fallbackSpeechEngine = fallbackSpeechEngine_;
        speechEngine_ = nullptr;
        fallbackSpeechEngine_ = nullptr;
    }

    if (speechEngine == nullptr) {
        return;
    }
    speechEngine->setSynthesisResultCallback(nullptr);
    auto result = speechEngine->destroySynthesizeModule();
    if (!result.success) {
        LOGE("Failed to destroy synthesize module: {}", result.error.getMessage());
    }
    if (fallbackSpeechEngine == nullptr) {
        return;
    }
    fallbackSpeechEngine->setSynthesisResultCallback(nullptr);
    result = fallbackSpeechEngine->destroySynthesizeModule();
    if (!result.success) {
        LOGE("Failed to destroy synthesize module: {}", result.error.getMessage());
    }
}

void SpeechSynthesizer::setSynthesisResultCallback(const SynthesisResultCallback &callback) {
    synthesisResultCallback_ = callback;
}

void SpeechSynthesizer::setAudioConfig(const AudioConfigInfo &audioConfigInfo) {
    outputAudioInfo_ = audioConfigInfo.outputAudioInfo();
}

void SpeechSynthesizer::synthesizeTextOnce(const std::string &text) { synthesizeTextOnce(text, ""); }

void SpeechSynthesizer::synthesizeTextOnce(const std::string &text, const std::string &requestId) {
    if (destroying_.load()) {
        notifyRequestFailed(AI_COMMON_TASK_CANCELLED, "Speech synthesizer is being destroyed.", requestId);
        return;
    }
    TtsParam ttsParam;
    ttsParam.outputAudioInfo = outputAudioInfo_;
    ttsParam.text = text;
    ttsParam.requestId = requestId;
    if (!tryEnqueueRequest(std::move(ttsParam))) {
        return;
    }
    onceTtsAsync();
}

void SpeechSynthesizer::stopSpeaking() {
    LOGD("SpeechSynthesizer stopSpeaking!");
    audioPlayer_.setPlaying(false);
}

void SpeechSynthesizer::sendSynthesisResultWithError(const ai_engine::EngineError &error,
                                                     const std::string &requestId) const {
    SpeechSynthesisResult result{};
    result.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
    result.audioFormat = "";
    result.audioRate = 0;
    result.audioChannel = 0;
    result.base64AudioData.clear();
    result.errorCode = error.getCode();
    result.errorMessage = error.getMessage();
    result.requestId = requestId;
    if (synthesisResultCallback_ != nullptr) {
        synthesisResultCallback_(result);
    }
}

void SpeechSynthesizer::writePcmToFile(const std::vector<uint8_t> &audioData) {
    std::filesystem::path filename = outputAudioInfo_.outputFilePath;
    std::filesystem::path directory = filename.parent_path();
    if (!std::filesystem::exists(directory)) {
        if (!std::filesystem::create_directories(directory)) {
            LOGE("Failed to create directory: {}", directory.string());
            return;
        }
    }
    if (!std::filesystem::exists(filename)) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            LOGE("Failed to create file {}", filename.string());
            return;
        }

        file.write(reinterpret_cast<const char *>(audioData.data()), audioData.size());
        file.close();
        return;
    }
    if (isFirstWrite_) {
        std::ofstream file(filename, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            LOGE("Failed to open file {}", filename.string());
            return;
        }

        file.write(reinterpret_cast<const char *>(audioData.data()), audioData.size());
        file.close();
    } else {
        std::ofstream file(filename, std::ios::binary | std::ios::app);
        if (!file.is_open()) {
            LOGE("Failed to open file {}", filename.string());
            return;
        }

        file.write(reinterpret_cast<const char *>(audioData.data()), audioData.size());
        file.close();
    }
}

void SpeechSynthesizer::handleSynthesisResult(const ai_engine::core_ai::speech::SynthesisResult &result,
                                              const TtsParam &ttsParam) {
    if (timedOut_.load()) {
        return;
    }
    SpeechSynthesisResult synthesisResult{};
    synthesisResult.reason = int(result.reason);
    synthesisResult.audioFormat = result.audioFormat;
    synthesisResult.audioRate = result.audioRate;
    synthesisResult.audioChannel = result.audioChannel;
    synthesisResult.base64AudioData = result.base64AudioData;
    synthesisResult.errorCode = result.error.getCode();
    synthesisResult.errorMessage = result.error.getMessage();
    synthesisResult.requestId = ttsParam.requestId;
    synthesisResult.modelName = result.modelName;

    if (ttsParam.outputAudioInfo.outputTarget == OutputAudioInfo::OutputTarget::DEFAULT_SPEAKER) {
        std::vector<uint8_t> audioData = util::base64Decode(result.base64AudioData);
        audioPlayer_.playing(audioData);
        if (synthesisResultCallback_ != nullptr) {
            synthesisResultCallback_(synthesisResult);
        }
    } else if (ttsParam.outputAudioInfo.outputTarget == OutputAudioInfo::OutputTarget::PCM_OUTPUT_FILE) {
        std::vector<uint8_t> audioData = util::base64Decode(result.base64AudioData);
        writePcmToFile(audioData);
        if (isFirstWrite_) {
            isFirstWrite_ = false;
        }
        if (synthesisResultCallback_ != nullptr) {
            synthesisResultCallback_(synthesisResult);
        }
    } else if (ttsParam.outputAudioInfo.outputTarget == OutputAudioInfo::OutputTarget::PCM_DATA) {
        if (synthesisResultCallback_ != nullptr) {
            synthesisResultCallback_(synthesisResult);
        }
    }
}

void SpeechSynthesizer::onceTtsAsync() {
    if (destroying_.load()) {
        rejectPendingRequests(AI_COMMON_TASK_CANCELLED, "Speech synthesis task has been cancelled.");
        return;
    }
    bool expected = false;
    if (!taskRunning_.compare_exchange_strong(expected, true)) {
        return;
    }
    TtsParam nextParam{};
    if (!ttsParamQueue_.try_dequeue(nextParam)) {
        taskRunning_.store(false);
        return;
    }
    decrementRequestQueueSize();
    std::shared_ptr sharedObject = getSharedObject();
    auto enqueueResult = task::TaskRunner::getInstance().addTask(
        {task::generateTaskId(), task::TaskType::OnceTextToSpeech, [sharedObject, nextParam]() mutable {
             sharedObject->onceTtsTask(std::move(nextParam));
             sharedObject->taskRunning_.store(false);
             sharedObject->onceTtsAsync();
         }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue OnceTextToSpeech task.");
        taskRunning_.store(false);
        notifyRequestFailed(AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                            "Too many requests, please try again later.", nextParam.requestId);
        rejectPendingRequests(AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                              "Too many requests, please try again later.");
    }
}

bool SpeechSynthesizer::tryEnqueueRequest(TtsParam ttsParam) {
    size_t pending = requestQueueSize_.fetch_add(1, std::memory_order_relaxed);
    if (pending >= loadSpeechSynthesisRequestQueueMaxSize()) {
        requestQueueSize_.fetch_sub(1, std::memory_order_relaxed);
        LOGW("Speech synthesis request queue is full, reject request.");
        notifyRequestFailed(AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                            "Too many requests, please try again later.", ttsParam.requestId);
        return false;
    }
    ttsParamQueue_.enqueue(std::move(ttsParam));
    return true;
}

void SpeechSynthesizer::decrementRequestQueueSize() {
    size_t current = requestQueueSize_.fetch_sub(1, std::memory_order_relaxed);
    if (current == 0) {
        requestQueueSize_.store(0, std::memory_order_relaxed);
    }
}

void SpeechSynthesizer::onceTtsTask(TtsParam ttsParam) {
    if (destroying_.load()) {
        notifyRequestFailed(AI_COMMON_TASK_CANCELLED, "Speech synthesis task has been cancelled.", ttsParam.requestId);
        return;
    }
    LOGI("TtsParam: {}", ttsParam.text);
    timedOut_.store(false);
    util::Watchdog watchdog;
    std::weak_ptr<SpeechSynthesizer> weakSelf = weak_from_this();
    std::string requestId = ttsParam.requestId;
    watchdog.Start(loadSpeechSynthesisTimeoutMs(), [weakSelf, requestId]() {
        auto self = weakSelf.lock();
        if (self == nullptr || self->destroying_.load()) {
            return;
        }
        self->timedOut_.store(true);
        self->notifyRequestFailed(AISDK_MODEL_RUN_TIME_OUT, "Speech synthesis timed out.", requestId);
    });
    Json::Value value;
    value["speed"] = ttsParam.outputAudioInfo.speed;
    value["volume"] = ttsParam.outputAudioInfo.volume;
    value["pitch"] = ttsParam.outputAudioInfo.pitch;
    audioPlayer_.setPlaying(true);
    auto callback = [weakSelf, ttsParam](const ai_engine::core_ai::speech::SynthesisResult &result) {
        auto self = weakSelf.lock();
        if (self == nullptr || self->destroying_.load()) {
            return;
        }
        self->handleSynthesisResult(result, ttsParam);
    };

    ISpeechEnginePtr speechEngine{};
    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        speechEngine = speechEngine_;
    }
    if (speechEngine == nullptr) {
        notifyRequestFailed(AI_COMMON_TASK_CANCELLED, "Speech synthesizer is not available.", ttsParam.requestId);
        watchdog.TryFinish();
        return;
    }

    bool shouldNotifyPrimaryDestroying = false;
    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        if (destroying_.load() || speechEngine_ == nullptr) {
            shouldNotifyPrimaryDestroying = true;
        } else {
            speechEngine->setSynthesisResultCallback(callback);
        }
    }
    if (shouldNotifyPrimaryDestroying) {
        notifyRequestFailed(AI_COMMON_TASK_CANCELLED, "Speech synthesizer is being destroyed.", ttsParam.requestId);
        watchdog.TryFinish();
        return;
    }

    ModelStatusTracker modelStatusTracker(speechEngine->currentModelName());
    engineInUse_ = true;
    auto result = speechEngine->synthesizeText(util::jsonToString(value), ttsParam.text);
    engineInUse_ = false;
    if (result.success) {
        watchdog.TryFinish();
        return;
    }
    if (!isDefaultMode_) {
        // 非默认模式下，直接返回错误
        LOGE("Failed to speech synthesize text: {} {}.", result.error.getCode(), result.error.getMessage());
        sendSynthesisResultWithError(result.error, ttsParam.requestId);
        watchdog.TryFinish();
        return;
    }

    ISpeechEnginePtr fallbackSpeechEngine{};
    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        fallbackSpeechEngine = fallbackSpeechEngine_;
    }
    if (fallbackSpeechEngine == nullptr) {
        EngineSelector engineSelector{ai_engine::AiEnginePluginManager::getInstance(),
                                      ModelConfigManager::getInstance()};
        EngineSelector::CoreAiSpeechEnginePredicate predicate =
            [](const std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine> &engine) {
                auto backupResult = engine->initSynthesizeModule();
                return backupResult.success;
            };
        auto fallbackEngineUnique =
            engineSelector.selectNextPriorityCoreAiSpeechEngine(currentEngineDeployType(), predicate);
        ISpeechEnginePtr fallbackEngine{std::move(fallbackEngineUnique)};
        if (fallbackEngine == nullptr) {
            sendSynthesisResultWithError(result.error, ttsParam.requestId);
            watchdog.TryFinish();
            return;
        }

        bool shouldNotifyFallbackDestroying = false;
        {
            std::lock_guard<std::mutex> lock(engineMutex_);
            if (destroying_.load()) {
                shouldNotifyFallbackDestroying = true;
            } else {
                if (fallbackSpeechEngine_ == nullptr) {
                    fallbackSpeechEngine_ = fallbackEngine;
                }
                fallbackSpeechEngine = fallbackSpeechEngine_;
            }
        }
        if (shouldNotifyFallbackDestroying) {
            notifyRequestFailed(AI_COMMON_TASK_CANCELLED, "Speech synthesizer is being destroyed.", ttsParam.requestId);
            watchdog.TryFinish();
            return;
        }
    }

    if (fallbackSpeechEngine == nullptr) {
        notifyRequestFailed(AI_COMMON_TASK_CANCELLED, "Speech synthesizer is not available.", ttsParam.requestId);
        watchdog.TryFinish();
        return;
    }

    bool shouldNotifyFinalDestroying = false;
    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        if (destroying_.load() || fallbackSpeechEngine_ == nullptr) {
            shouldNotifyFinalDestroying = true;
        } else {
            fallbackSpeechEngine->setSynthesisResultCallback(callback);
        }
    }
    if (shouldNotifyFinalDestroying) {
        notifyRequestFailed(AI_COMMON_TASK_CANCELLED, "Speech synthesizer is being destroyed.", ttsParam.requestId);
        watchdog.TryFinish();
        return;
    }

    ModelStatusTracker backupModelStatusTracker(fallbackSpeechEngine->currentModelName());
    engineInUse_ = true;
    auto backupResult = fallbackSpeechEngine->synthesizeText(util::jsonToString(value), ttsParam.text);
    engineInUse_ = false;
    if (!backupResult.success) {
        sendSynthesisResultWithError(backupResult.error, ttsParam.requestId);
    }
    watchdog.TryFinish();
}

void SpeechSynthesizer::notifyRequestFailed(int errorCode, const std::string &message, const std::string &requestId) {
    SpeechSynthesisResult result{};
    result.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
    result.audioFormat = "";
    result.audioRate = 0;
    result.audioChannel = 0;
    result.base64AudioData.clear();
    result.errorCode = errorCode;
    result.errorMessage = message;
    result.requestId = requestId;
    if (synthesisResultCallback_ != nullptr) {
        synthesisResultCallback_(result);
    }
}

void SpeechSynthesizer::rejectPendingRequests(int errorCode, const std::string &message) {
    TtsParam dropped{};
    while (ttsParamQueue_.try_dequeue(dropped)) {
        decrementRequestQueueSize();
        notifyRequestFailed(errorCode, message, dropped.requestId);
    }
}

DeployType SpeechSynthesizer::currentEngineDeployType() const {
    DeployType type{DeployType::Unknown};
    ISpeechEnginePtr speechEngine{};
    {
        std::lock_guard<std::mutex> lock(engineMutex_);
        speechEngine = speechEngine_;
    }
    if (speechEngine == nullptr) {
        return type;
    }
    if (speechEngine->isCustomModelEngine()) {
        type = DeployType::Custom;
    } else if (speechEngine->isCloud()) {
        type = DeployType::PublicCloud;
    } else {
        type = DeployType::Local;
    }
    return type;
}

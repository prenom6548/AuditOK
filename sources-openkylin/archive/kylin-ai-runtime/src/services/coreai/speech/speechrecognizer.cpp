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

#include "speechrecognizer.h"

#include <jsoncpp/json/json.h>
#include <kylin-ai/common/error.h>
#include <modelconfig/modelconfigmanager.h>

#include <chrono>
#include <fstream>
#include <mutex>
#include <utility>

#include "engineselector/engineselector.h"
#include "taskmanager/taskrunner.h"
#include "utils/logger.h"
#include "utils/util.h"
#include "utils/watchdog.h"

namespace {
constexpr int64_t kDefaultSpeechRecognitionTimeoutMs = 60LL * 60 * 1000;
constexpr size_t kDefaultSpeechRecognitionRequestQueueMaxSize = 10;

int64_t loadSpeechRecognitionTimeoutMs() {
    const auto taskTimeout = task::loadTaskTimeoutMs(task::TaskType::OnceSpeechRecognition);
    if (taskTimeout.has_value()) {
        return *taskTimeout;
    }
    LOGW("Speech recognition task timeout not found in task config, use default.");
    return kDefaultSpeechRecognitionTimeoutMs;
}

size_t loadSpeechRecognitionRequestQueueMaxSize() {
    const auto queueMaxSize = task::loadTaskQueueMaxSize(task::TaskType::OnceSpeechRecognition);
    if (queueMaxSize.has_value()) {
        return static_cast<size_t>(*queueMaxSize);
    }
    LOGW("Speech recognition request queue max size not found in task config, use default.");
    return kDefaultSpeechRecognitionRequestQueueMaxSize;
}

ai_engine::EngineError getFirstError(const std::vector<ai_engine::EngineError>& engineErrors) {
    for (const auto& engineError : engineErrors) {
        if (engineError.getCode() != AI_COMMON_SUCCESS && engineError.getCode() != -1) {
            return engineError;
        }
    }
    return {AI_COMMON_SUCCESS, ""};
}
}  // namespace

SpeechRecognizer::SpeechRecognizer(bool isDefaultMode) : isDefaultMode_(isDefaultMode) {}

SpeechRecognizer::~SpeechRecognizer() {
    destroy();
    LOGD("Speech recognizer destructor execution completed.");
}

void SpeechRecognizer::setRecognitionResultCallback(const RecognitionResultCallback& callback) {
    recognitionResultCallback_ = callback;
}

void SpeechRecognizer::setAudioConfig(const AudioConfigInfo& configInfo) {
    inputAudioInfo_ = configInfo.inputAudioInfo();
}

void SpeechRecognizer::startContinuousRecognition() {
    engineInUse_ = true;

    {
        std::lock_guard<std::mutex> lock(continuousModelStatusTrackerkMutex_);
        continuousModelStatusTracker_ = std::make_unique<ModelStatusTracker>(speechEngine_->currentModelName());
    }

    switch (inputAudioInfo_.inputSource) {
        case InputAudioInfo::InputSource::DEFAULT_MIC: {
            startContinuousRecognitionFromDefaultMic();
            break;
        }
        case InputAudioInfo::InputSource::STREAM: {
            startContinuousRecognitionFromAudioStream();
            break;
        }
        case InputAudioInfo::InputSource::PCM_FILE: {
            startContinuousRecognitionFromPcmFile(inputAudioInfo_.inputFilePath);
            break;
        }
        case InputAudioInfo::InputSource::RAW_DATA: {
            startContinuousRecognitionFromRawData(inputAudioInfo_.audioData);
            break;
        }
        default: {
            break;
        }
    }
}

void SpeechRecognizer::stopContinuousRecognition() {
    {
        std::make_unique<ModelStatusTracker>(speechEngine_->currentModelName());
        continuousModelStatusTracker_.reset();
    }
    if (inputAudioInfo_.inputSource == InputAudioInfo::InputSource::DEFAULT_MIC) {
        isRunningDefaultMic_ = false;
    }

    if (speechEngine_ != nullptr) {
        auto result = speechEngine_->stopContinuousRecognition();
        if (!result.success) {
            LOGE("Failed to call stopContinuousRecognition: {} {}.", result.error.getCode(), result.error.getMessage());
            callRecognitionResultCallback(makeResultFromEngineError(result.error));
        }
    }
    if (fallbackSpeechEngine_ != nullptr) {
        auto result = fallbackSpeechEngine_->stopContinuousRecognition();
        if (!result.success) {
            LOGE("Failed to call stopContinuousRecognition: {} {}.", result.error.getCode(), result.error.getMessage());
        }
    }
    engineInUse_ = false;
}

bool SpeechRecognizer::init(int& errorCode, const ISpeechEnginePtr& speechEngine) {
    speechEngine_ = speechEngine;
    ModelStatusTracker modelStatusTracker(speechEngine_->currentModelName());
    auto resultContinuous = speechEngine_->initContinuousRecognitionModule();
    if (!resultContinuous.success) {
        LOGE("Failed to call initContinuousRecognitionModule: {} {}.", resultContinuous.error.getCode(),
             resultContinuous.error.getMessage());
    }

    auto resultOnce = speechEngine_->initRecognizeOnceModule();
    if (!resultOnce.success) {
        LOGE("Failed to call initRecognizeOnceModule: {} {}.", resultOnce.error.getCode(),
             resultOnce.error.getMessage());
    }
    std::shared_ptr sharedObject = getSharedObject();
    if (sharedObject == nullptr) {
        errorCode = AISDK_RUNTIME_ERROR;
        return false;
    }
    auto callback = [sharedObject](const ai_engine::core_ai::speech::RecognitionResult& result) {
        if (sharedObject->timedOut_.load()) {
            return;
        }
        SpeechRecognitionResult recognitionResult{};
        recognitionResult.reason = int(result.reason);
        recognitionResult.text = result.text;
        recognitionResult.speakerId = result.speakerId;
        recognitionResult.errorCode = result.error.getCode() == -1 ? AI_COMMON_SUCCESS : result.error.getCode();
        recognitionResult.errorMessage = result.error.getMessage();
        recognitionResult.requestId = sharedObject->currentRequestId_;
        recognitionResult.modelName = result.modelName;
        if (!sharedObject->isDefaultMode_ || recognitionResult.errorCode == AI_COMMON_SUCCESS) {
            sharedObject->callRecognitionResultCallback(recognitionResult);
        } else {
            // 留到循环检查时，再决定是否返回错误，重新构造，避免errorCode为-1的情况
            std::lock_guard locker(sharedObject->callbackMutex_);
            sharedObject->callbackError_ = {recognitionResult.errorCode, recognitionResult.errorMessage};
        }
        LOGD("Current model name: {}.", result.modelName);
        if (recognitionResult.errorCode != AISDK_NO_ERROR) {
            sharedObject->errorOccurred_ = true;
        }
    };

    LOGD("isDefaultMode_ {}", isDefaultMode_);

    bool needToReturnError{false};
    if (!resultContinuous.success || !resultOnce.success) {
        if (!isDefaultMode_) {
            needToReturnError = true;
        } else {
            EngineSelector engineSelector{ai_engine::AiEnginePluginManager::getInstance(),
                                          ModelConfigManager::getInstance()};
            EngineSelector::CoreAiSpeechEnginePredicate predicate =
                [](const std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine>& engine) {
                    auto ondeviceResultContinuous = engine->initContinuousRecognitionModule();
                    auto ondeviceResultOnce = engine->initRecognizeOnceModule();
                    return ondeviceResultContinuous.success && ondeviceResultOnce.success;
                };
            fallbackSpeechEngine_ =
                engineSelector.selectNextPriorityCoreAiSpeechEngine(currentEngineDeployType(), predicate);
            if (fallbackSpeechEngine_ == nullptr) {
                needToReturnError = true;
            } else {
                fallbackSpeechEngine_->setRecognitionResultCallback(callback);
            }
        }
    }

    if (!resultContinuous.success && needToReturnError) {
        errorCode = resultContinuous.error.getCode();
        callRecognitionResultCallback(makeResultFromEngineError(resultContinuous.error));
        return false;
    }

    if (!resultOnce.success && needToReturnError) {
        errorCode = resultOnce.error.getCode();
        callRecognitionResultCallback(makeResultFromEngineError(resultOnce.error));
        return false;
    }

    // 初始化回调函数
    speechEngine_->setRecognitionResultCallback(callback);
    return true;
}

void SpeechRecognizer::destroy() {
    stopContinuousRecognition();
    // 清理回调函数资源
    recognitionResultCallback_ = nullptr;
    if (speechEngine_ == nullptr) {
        return;
    }
    speechEngine_->setRecognitionResultCallback(nullptr);
    auto result = speechEngine_->destroyContinuousRecognitionModule();
    if (!result.success) {
        LOGE("Failed to destroy continuous recognition module: {} {}.", result.error.getCode(),
             result.error.getMessage());
    }
    result = speechEngine_->destroyRecognizeOnceModule();
    if (!result.success) {
        LOGE("Failed to destroy recognize once module: {} {}.", result.error.getCode(), result.error.getMessage());
    }
    if (fallbackSpeechEngine_ == nullptr) {
        return;
    }
    fallbackSpeechEngine_->setRecognitionResultCallback(nullptr);
    result = fallbackSpeechEngine_->destroyContinuousRecognitionModule();
    if (!result.success) {
        LOGE("Failed to destroy continuous recognition module: {} {}.", result.error.getCode(),
             result.error.getMessage());
    }
    result = fallbackSpeechEngine_->destroyRecognizeOnceModule();
    if (!result.success) {
        LOGE("Failed to destroy recognize once module: {} {}.", result.error.getCode(), result.error.getMessage());
    }
    fallbackSpeechEngine_ = nullptr;
}

SpeechRecognitionResult SpeechRecognizer::makeResultFromEngineError(const ai_engine::EngineError& error) const {
    SpeechRecognitionResult result{};
    result.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
    result.text = "";
    result.speakerId = -1;
    result.errorCode = error.getCode();
    result.errorMessage = error.getMessage();
    result.requestId = currentRequestId_;
    return result;
}

void SpeechRecognizer::callRecognitionResultCallback(const SpeechRecognitionResult& result) const {
    if (recognitionResultCallback_ != nullptr) {
        recognitionResultCallback_(result);
    }

    if (result.errorCode > 0) {
        std::lock_guard<std::mutex> lock(continuousModelStatusTrackerkMutex_);
        if (continuousModelStatusTracker_) {
            continuousModelStatusTracker_.reset();
        }
    }
}

static std::vector<uint8_t> readAudioData(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file: {}.", filePath);
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> audioData(fileSize);
    file.read(audioData.data(), fileSize);
    return {audioData.begin(), audioData.end()};
}

void SpeechRecognizer::startContinuousRecognitionFromDefaultMic() {
    errorOccurred_ = false;
    std::string deviceName;
    bool mute{true};
    AudioRecorder::getMicrophoneInfo(deviceName, mute);
    if (deviceName.empty()) {
        SpeechRecognitionResult result;
        result.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
        result.text = "";
        result.speakerId = -1;
        result.errorCode = AI_COMMON_SPEECH_DEFAULT_MICROPHONE_INVALID;
        result.errorMessage = "Default microphone invalid.";
        callRecognitionResultCallback(result);
        return;
    }
    if (mute) {
        SpeechRecognitionResult result;
        result.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
        result.text = "";
        result.speakerId = -1;
        result.errorCode = AI_COMMON_SPEECH_DEFAULT_MICROPHONE_MUTE;
        result.errorMessage = "Default microphone mute.";
        callRecognitionResultCallback(result);
        return;
    }

    isRunningDefaultMic_ = true;
    std::shared_ptr audioRecorderPtr{std::make_shared<AudioRecorder>()};
    std::shared_ptr sharedObject = getSharedObject();

    auto result = task::TaskRunner::getInstance().addTask(
        {task::generateTaskId(), task::TaskType::ContinuousSpeechRecognition,
         [sharedObject, audioRecorderPtr{std::move(audioRecorderPtr)}]() {
             sharedObject->runTaskForContinuousRecognitionFromDefaultMic(audioRecorderPtr);
         }});
    if (result != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue ContinuousSpeechRecognition task.");
        if (result == task::TaskRunner::EnqueueResult::RejectedQueueFull) {
            isRunningDefaultMic_ = false;
            SpeechRecognitionResult callbackResult{};
            callbackResult.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
            callbackResult.text = "";
            callbackResult.speakerId = -1;
            callbackResult.errorCode = AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS;
            callbackResult.errorMessage = "Too many requests, please try again later.";
            callRecognitionResultCallback(callbackResult);
        }
    }
}

void SpeechRecognizer::runTaskForContinuousRecognitionFromDefaultMic(
    const std::shared_ptr<AudioRecorder>& audioRecorderPtr) {
    auto result = speechEngine_->startContinuousRecognition(R"({"format": "", "rate": 16000, "channel": 1})");
    LOGD("result.success {}", result.success);
    if (result.success) {
        bool hasAuthenticationError{false};
        continuousAsrFromDefaultMic(audioRecorderPtr, speechEngine_, hasAuthenticationError);
        if (!hasAuthenticationError) {
            return;
        }
    }
    ai_engine::EngineError firstError;
    {
        std::lock_guard locker(callbackMutex_);
        firstError = getFirstError({result.error, callbackError_});
    }
    if (!isDefaultMode_) {
        callRecognitionResultCallback(makeResultFromEngineError(firstError));
        return;
    }
    if (fallbackSpeechEngine_ == nullptr) {
        EngineSelector engineSelector{ai_engine::AiEnginePluginManager::getInstance(),
                                      ModelConfigManager::getInstance()};
        EngineSelector::CoreAiSpeechEnginePredicate predicate =
            [](const std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine>& engine) {
                auto ondeviceResultContinuous = engine->initContinuousRecognitionModule();
                return ondeviceResultContinuous.success;
            };
        fallbackSpeechEngine_ =
            engineSelector.selectNextPriorityCoreAiSpeechEngine(currentEngineDeployType(), predicate);
    }
    // 使用本地引擎失败也直接报错
    if (fallbackSpeechEngine_ == nullptr) {
        callRecognitionResultCallback(makeResultFromEngineError(firstError));
        return;
    }
    std::shared_ptr sharedObject = getSharedObject();
    if (sharedObject == nullptr) {
        callRecognitionResultCallback(makeResultFromEngineError(firstError));
        return;
    }
    auto callback = [sharedObject](const ai_engine::core_ai::speech::RecognitionResult& result) {
        if (sharedObject->timedOut_.load()) {
            return;
        }
        SpeechRecognitionResult recognitionResult{};
        recognitionResult.reason = int(result.reason);
        recognitionResult.text = result.text;
        recognitionResult.speakerId = result.speakerId;
        recognitionResult.errorCode = result.error.getCode() == -1 ? AI_COMMON_SUCCESS : result.error.getCode();
        recognitionResult.errorMessage = result.error.getMessage();
        recognitionResult.requestId = sharedObject->currentRequestId_;
        recognitionResult.modelName = result.modelName;
        sharedObject->callRecognitionResultCallback(recognitionResult);
        if (recognitionResult.errorCode != AI_COMMON_SUCCESS) {
            sharedObject->errorOccurred_ = true;
        }
    };
    // 重置错误标志
    errorOccurred_ = false;
    fallbackSpeechEngine_->setRecognitionResultCallback(callback);
    auto startResult =
        fallbackSpeechEngine_->startContinuousRecognition(R"({"format": "", "rate": 16000, "channel": 1})");
    if (!startResult.success) {
        callRecognitionResultCallback(makeResultFromEngineError(firstError));
        return;
    }
    bool unusedAuthenticationError{false};  // 不必判断
    continuousAsrFromDefaultMic(audioRecorderPtr, fallbackSpeechEngine_, unusedAuthenticationError);
}

void SpeechRecognizer::continuousAsrFromDefaultMic(const std::shared_ptr<AudioRecorder>& audioRecorderPtr,
                                                   const ISpeechEnginePtr& speechEnginePtr,
                                                   bool& hasAuthenticationError) {
    // 本地已经运行的情况下，报错本地引擎相关错误
    audioRecorderPtr->start();
    while (isRunningDefaultMic_) {
        using namespace std::chrono_literals;
        std::chrono::time_point<std::chrono::steady_clock> timePoint = std::chrono::steady_clock::now() + 40ms;
        if (errorOccurred_) {
            // 回调中报错，停止识别
            // 如果是非默认模式，直接退出即可，也不必检查错误类型
            if (!isDefaultMode_) {
                break;
            }
            // 如果是默认模式，如果不是鉴权错误，直接报错
            {
                std::lock_guard locker(callbackMutex_);
                if (callbackError_.getCode() != AI_COMMON_AUTHENTICATION_FAILURE &&
                    callbackError_.getCode() != AI_COMMON_SUCCESS) {
                    callRecognitionResultCallback(makeResultFromEngineError(callbackError_));
                    break;
                }
            }
            // 如果是鉴权错误，需要返回该错误
            hasAuthenticationError = true;
            break;
        }
        std::string deviceName;
        bool mute{true};
        AudioRecorder::getMicrophoneInfo(deviceName, mute);
        if (deviceName.empty()) {
            SpeechRecognitionResult asrResult;
            asrResult.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
            asrResult.text = "";
            asrResult.speakerId = -1;
            asrResult.errorCode = AI_COMMON_SPEECH_DEFAULT_MICROPHONE_INVALID;
            asrResult.errorMessage = "Default microphone invalid.";
            callRecognitionResultCallback(asrResult);
            speechEnginePtr->stopContinuousRecognition();
            return;
        }
        if (mute) {
            SpeechRecognitionResult asrResult;
            asrResult.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
            asrResult.text = "";
            asrResult.speakerId = -1;
            asrResult.errorCode = AI_COMMON_SPEECH_DEFAULT_MICROPHONE_MUTE;
            asrResult.errorMessage = "Default microphone mute.";
            callRecognitionResultCallback(asrResult);
            speechEnginePtr->stopContinuousRecognition();
            return;
        }

        std::vector<std::vector<uint8_t> > audioDatas = audioRecorderPtr->getAudioDatas();
        if (audioDatas.empty()) {
            std::this_thread::sleep_for(1ms);
            continue;
        }
        bool errorOccurred{false};
        for (const auto& audioData : audioDatas) {
            auto result = speechEnginePtr->writeContinuousRecognitionAudioData(audioData);
            if (!result.success) {
                LOGE("Failed to write audio data: {} {}.", result.error.getCode(), result.error.getMessage());
                callRecognitionResultCallback(makeResultFromEngineError(result.error));
                errorOccurred = true;
                break;
            }
        }
        if (errorOccurred) {
            break;
        }
        std::this_thread::sleep_until(timePoint);
    }
    // 清理callbackError，以便后续调用
    {
        std::lock_guard locker(callbackMutex_);
        callbackError_ = {AI_COMMON_SUCCESS, ""};
    }
}

void SpeechRecognizer::startContinuousRecognitionFromRawData(const std::vector<uint8_t>& audioData) {
    if (audioData.size() < SINGLE_FRAME_LIMIT) {
        LOGE("Func startContinuousRecognitionFromRawData params is abnormal!");
        ai_engine::EngineError error(AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID,
                                     "Input raw data is too little!");
        callRecognitionResultCallback(makeResultFromEngineError(error));
        return;
    }
    std::shared_ptr sharedObject = getSharedObject();

    auto enqueueResult = task::TaskRunner::getInstance().addTask(
        {task::generateTaskId(), task::TaskType::ContinuousSpeechRecognition,
         [sharedObject, audioData]() { sharedObject->runTaskForContinuousRecognitionFromRawData(audioData); }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue ContinuousSpeechRecognition task.");
        if (enqueueResult == task::TaskRunner::EnqueueResult::RejectedQueueFull) {
            SpeechRecognitionResult callbackResult{};
            callbackResult.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
            callbackResult.text = "";
            callbackResult.speakerId = -1;
            callbackResult.errorCode = AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS;
            callbackResult.errorMessage = "Too many requests, please try again later.";
            callRecognitionResultCallback(callbackResult);
        }
    }
}

void SpeechRecognizer::runTaskForContinuousRecognitionFromRawData(const std::vector<uint8_t>& audioData) const {
    auto result = speechEngine_->startContinuousRecognition(R"({"format": "", "rate": 16000, "channel": 1})");

    if (!result.success) {
        LOGE("Failed to call startContinuousRecognition: {} {}.", result.error.getCode(), result.error.getMessage());
        callRecognitionResultCallback(makeResultFromEngineError(result.error));
    }
    for (int i = 0; i < audioData.size(); i += 1280) {
        LOGD("write audio data: {}.", i);
        auto begin = audioData.begin() + i;
        auto end = audioData.begin() + i + 1280;
        if (end > audioData.end()) {
            end = audioData.end();
        }
        result = speechEngine_->writeContinuousRecognitionAudioData(std::vector(begin, end));
        if (!result.success) {
            LOGE("Failed to write audio data: {} {}.", result.error.getCode(), result.error.getMessage());
            callRecognitionResultCallback(makeResultFromEngineError(result.error));
            break;
        }
    }
    result = speechEngine_->stopContinuousRecognition();
    if (!result.success) {
        LOGE("Failed to call stopContinuousRecognition: {} {}.", result.error.getCode(), result.error.getMessage());
        callRecognitionResultCallback(makeResultFromEngineError(result.error));
    }
}

void SpeechRecognizer::startContinuousRecognitionFromAudioStream() const {
    auto result = speechEngine_->startContinuousRecognition(R"({"format": "", "rate": 16000, "channel": 1})");

    if (!result.success) {
        LOGE("Failed to call startContinuousRecognition: {} {}.", result.error.getCode(), result.error.getMessage());
        callRecognitionResultCallback(makeResultFromEngineError(result.error));
    }
}

void SpeechRecognizer::writeContinuousRecognitionFromAudioStream(const std::vector<uint8_t>& audioData) {
    std::shared_ptr sharedObject = getSharedObject();

    auto enqueueResult = task::TaskRunner::getInstance().addTask(
        {task::generateTaskId(), task::TaskType::ContinuousSpeechRecognition, [sharedObject, audioData]() {
             auto result = sharedObject->speechEngine_->writeContinuousRecognitionAudioData(audioData);

             if (!result.success) {
                 LOGE("Failed to write audio data: {} {}.", result.error.getCode(), result.error.getMessage());
                 sharedObject->callRecognitionResultCallback(sharedObject->makeResultFromEngineError(result.error));
             }
         }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue ContinuousSpeechRecognition task.");
        if (enqueueResult == task::TaskRunner::EnqueueResult::RejectedQueueFull) {
            SpeechRecognitionResult callbackResult{};
            callbackResult.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
            callbackResult.text = "";
            callbackResult.speakerId = -1;
            callbackResult.errorCode = AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS;
            callbackResult.errorMessage = "Too many requests, please try again later.";
            callRecognitionResultCallback(callbackResult);
        }
    }
}

bool SpeechRecognizer::isRegularPcmFile(const std::filesystem::path& path) const {
    if (not std::filesystem::exists(path)) {
        LOGE("{} is not exists.", path.string());
        ai_engine::EngineError error(AI_COMMON_SPEECH_PARAM_INVALID,
                                     fmt::format("Input path {} is not exists!", path.string()));
        callRecognitionResultCallback(makeResultFromEngineError(error));
        return false;
    }
    if (not std::filesystem::is_regular_file(path)) {
        LOGE("{} is not regular file.", path.string());
        ai_engine::EngineError error(AI_COMMON_SPEECH_PARAM_INVALID,
                                     fmt::format("Input path {} is not exists!", path.string()));
        callRecognitionResultCallback(makeResultFromEngineError(error));
    }

    if (std::filesystem::file_size(path) < SINGLE_FRAME_LIMIT) {
        LOGE("file[{}] size < {}, it is too small.", path.string(), SINGLE_FRAME_LIMIT);
        ai_engine::EngineError error(AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID,
                                     fmt::format("Input path {} data is too little!", path.string()));
        callRecognitionResultCallback(makeResultFromEngineError(error));
        return false;
    }
    return true;
}

void SpeechRecognizer::startContinuousRecognitionFromPcmFile(const std::filesystem::path& path) {
    if (not isRegularPcmFile(path)) {
        return;
    }
    // TODO 这里需要优化一下，不会一次性读取
    startContinuousRecognitionFromRawData(readAudioData(path.string()));
}

void SpeechRecognizer::recognizeOnce() { recognizeOnce(""); }

void SpeechRecognizer::recognizeOnce(const std::string& requestId) {
    InputAudioInfo audioInfo = inputAudioInfo_;
    audioInfo.requestId = requestId;
    if (!tryEnqueueRequest(std::move(audioInfo))) {
        return;
    }
    onceAsrAsync();
}

void SpeechRecognizer::onceAsrAsync() {
    bool expected = false;
    if (!taskRunning_.compare_exchange_strong(expected, true)) {
        return;
    }
    InputAudioInfo nextInfo{};
    if (!audioInfoQueue_.try_dequeue(nextInfo)) {
        taskRunning_.store(false);
        return;
    }
    decrementRequestQueueSize();
    std::shared_ptr sharedObject = getSharedObject();
    auto enqueueResult = task::TaskRunner::getInstance().addTask(
        {task::generateTaskId(), task::TaskType::OnceSpeechRecognition, [sharedObject, nextInfo]() mutable {
             sharedObject->onceAsrTask(nextInfo);
             sharedObject->taskRunning_.store(false);
             sharedObject->onceAsrAsync();
         }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue OnceSpeechRecognition task.");
        taskRunning_.store(false);
        notifyRequestFailed(AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                            "Too many requests, please try again later.", nextInfo.requestId);
        rejectPendingRequests(AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                              "Too many requests, please try again later.");
    }
}

bool SpeechRecognizer::tryEnqueueRequest(InputAudioInfo inputAudioInfo) {
    size_t pending = requestQueueSize_.fetch_add(1, std::memory_order_relaxed);
    if (pending >= loadSpeechRecognitionRequestQueueMaxSize()) {
        requestQueueSize_.fetch_sub(1, std::memory_order_relaxed);
        LOGW("Speech recognition request queue is full, reject request.");
        notifyRequestFailed(AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                            "Too many requests, please try again later.", inputAudioInfo.requestId);
        return false;
    }
    audioInfoQueue_.enqueue(std::move(inputAudioInfo));
    return true;
}

void SpeechRecognizer::decrementRequestQueueSize() {
    size_t current = requestQueueSize_.fetch_sub(1, std::memory_order_relaxed);
    if (current == 0) {
        requestQueueSize_.store(0, std::memory_order_relaxed);
    }
}

void SpeechRecognizer::onceAsrTask(const InputAudioInfo& inputAudioInfo) {
    timedOut_.store(false);
    currentRequestId_ = inputAudioInfo.requestId;
    util::Watchdog watchdog;
    watchdog.Start(loadSpeechRecognitionTimeoutMs(), [this]() {
        timedOut_.store(true);
        notifyRequestFailed(AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT, "Speech recognition timed out.",
                            currentRequestId_);
    });
    Json::Value params;
    params["format"] = inputAudioInfo.getFormaString();
    params["rate"] = inputAudioInfo.rate;
    params["channel"] = inputAudioInfo.channel;
    std::vector<uint8_t> audioData;
    if (inputAudioInfo.inputSource == InputAudioInfo::InputSource::RAW_DATA) {
        audioData = inputAudioInfo.audioData;
    } else if (inputAudioInfo.inputSource == InputAudioInfo::InputSource::PCM_FILE) {
        audioData = readAudioData(inputAudioInfo.inputFilePath.string());
    } else {
        ai_engine::EngineError error(AI_COMMON_SPEECH_PARAM_INVALID, "Recognize once only support file and raw data.");
        if (watchdog.TryFinish()) {
            callRecognitionResultCallback(makeResultFromEngineError(error));
        }
        watchdog.Finish();
        return;
    }
    ModelStatusTracker modelStatusTracker(speechEngine_->currentModelName());
    engineInUse_ = true;
    auto result = speechEngine_->recognizeOnce(util::jsonToString(params), audioData);
    engineInUse_ = false;
    if (!result.success) {
        LOGE("Failed to speech recognize once: {} {}.", result.error.getCode(), result.error.getMessage());
        if (watchdog.TryFinish()) {
            callRecognitionResultCallback(makeResultFromEngineError(result.error));
        }
        watchdog.Finish();
        return;
    }
    watchdog.TryFinish();
    watchdog.Finish();
}

void SpeechRecognizer::notifyRequestFailed(int errorCode, const std::string& message,
                                           const std::string& requestId) const {
    SpeechRecognitionResult callbackResult{};
    callbackResult.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
    callbackResult.text = "";
    callbackResult.speakerId = -1;
    callbackResult.errorCode = errorCode;
    callbackResult.errorMessage = message;
    callbackResult.requestId = requestId;
    callRecognitionResultCallback(callbackResult);
}

void SpeechRecognizer::rejectPendingRequests(int errorCode, const std::string& message) {
    InputAudioInfo dropped{};
    while (audioInfoQueue_.try_dequeue(dropped)) {
        decrementRequestQueueSize();
        notifyRequestFailed(errorCode, message, dropped.requestId);
    }
}

DeployType SpeechRecognizer::currentEngineDeployType() const {
    DeployType type{DeployType::Unknown};
    if (speechEngine_ == nullptr) {
        return type;
    }
    if (speechEngine_->isCustomModelEngine()) {
        type = DeployType::Custom;
    } else if (speechEngine_->isCloud()) {
        type = DeployType::PublicCloud;
    } else {
        type = DeployType::Local;
    }
    return type;
}

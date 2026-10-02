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

#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include <kylin-ai/aiengine.h>
#include <kylin-ai/common/error.h>

#include <atomic>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <mutex>

#include "audioconfiginfo.h"
#include "concurrentqueue/blockingconcurrentqueue.h"
#include "modelconfig/modelconfigenums.h"
#include "modelconfig/modelstatustracker.h"
#include "taskmanager/taskrunner.h"
#include "utils/audiorecorder.h"
#include "utils/logger.h"

struct SpeechRecognitionResult {
    int reason;
    std::string text;
    int speakerId{0};
    int errorCode{AI_COMMON_SUCCESS};
    std::string errorMessage;
    std::string requestId;
    std::string modelName;
};

class SpeechRecognizer : public std::enable_shared_from_this<SpeechRecognizer> {
public:
    static constexpr std::size_t SINGLE_FRAME_LIMIT = 1280;

    using ISpeechEngine = ai_engine::core_ai::speech::AbstractSpeechEngine;
    using ISpeechEnginePtr = std::shared_ptr<ISpeechEngine>;

    using RecognitionResultCallback = std::function<void(SpeechRecognitionResult)>;

public:
    explicit SpeechRecognizer(bool isDefaultMode);

    ~SpeechRecognizer();

    bool init(int& errorCode, const ISpeechEnginePtr& speechEngine);

    void destroy();

    void setRecognitionResultCallback(const RecognitionResultCallback& callback);

    void setAudioConfig(const AudioConfigInfo& configInfo);

    void startContinuousRecognition();
    void stopContinuousRecognition();

    void writeContinuousRecognitionFromAudioStream(const std::vector<uint8_t>& audioData);

    void recognizeOnce();
    void recognizeOnce(const std::string& requestId);

    bool engineInUse() const { return engineInUse_; }

private:
    std::shared_ptr<SpeechRecognizer> getSharedObject() {
        try {
            return shared_from_this();
        } catch (std::exception& except) {
            LOGE("{}", except.what());
            return nullptr;
        }
    }

    SpeechRecognitionResult makeResultFromEngineError(const ai_engine::EngineError& error) const;

    void callRecognitionResultCallback(const SpeechRecognitionResult& result) const;

    void startContinuousRecognitionFromDefaultMic();
    void runTaskForContinuousRecognitionFromDefaultMic(const std::shared_ptr<AudioRecorder>& audioRecorderPtr);
    void continuousAsrFromDefaultMic(const std::shared_ptr<AudioRecorder>& audioRecorderPtr,
                                     const ISpeechEnginePtr& speechEnginePtr, bool& hasAuthenticationError);

    void startContinuousRecognitionFromRawData(const std::vector<uint8_t>& audioData);
    void runTaskForContinuousRecognitionFromRawData(const std::vector<uint8_t>& audioData) const;

    void startContinuousRecognitionFromAudioStream() const;

    bool isRegularPcmFile(const std::filesystem::path& path) const;
    void startContinuousRecognitionFromPcmFile(const std::filesystem::path& path);

    void onceAsrAsync();

    void onceAsrTask(const InputAudioInfo& inputAudioInfo);

    bool tryEnqueueRequest(InputAudioInfo inputAudioInfo);
    void decrementRequestQueueSize();

    void notifyRequestFailed(int errorCode, const std::string& message, const std::string& requestId) const;
    void rejectPendingRequests(int errorCode, const std::string& message);

    DeployType currentEngineDeployType() const;

private:
    InputAudioInfo inputAudioInfo_{};
    ISpeechEnginePtr speechEngine_{nullptr};
    ISpeechEnginePtr fallbackSpeechEngine_{nullptr};
    RecognitionResultCallback recognitionResultCallback_{nullptr};

    // 以下为语音识别的标志位
    std::atomic_bool isRunningDefaultMic_{false};
    mutable std::mutex continuousModelStatusTrackerkMutex_;
    mutable std::unique_ptr<ModelStatusTracker> continuousModelStatusTracker_{nullptr};

    std::atomic_bool engineInUse_{false};
    std::atomic_bool taskRunning_{false};
    std::atomic_bool timedOut_{false};
    moodycamel::BlockingConcurrentQueue<InputAudioInfo> audioInfoQueue_;
    std::atomic<size_t> requestQueueSize_{0};
    std::atomic_bool errorOccurred_{false};  // 流式语音识别使用
    std::string currentRequestId_;           // 当前单次识别任务的requestId
    bool isDefaultMode_{false};

    std::mutex callbackMutex_;
    ai_engine::EngineError callbackError_{AI_COMMON_SUCCESS, ""};
};

#endif

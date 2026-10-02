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

#ifndef SPEECH_SYNTHESIZER_H
#define SPEECH_SYNTHESIZER_H

#include <kylin-ai/aiengine.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>

#include "audioconfiginfo.h"
#include "concurrentqueue/blockingconcurrentqueue.h"
#include "modelconfig/modelconfigenums.h"
#include "taskmanager/taskrunner.h"
#include "utils/audioplayer.h"
#include "utils/watchdog.h"

struct SpeechSynthesisResult {
    int reason;
    std::string audioFormat;
    int audioRate;
    int audioChannel;
    std::string base64AudioData;
    int errorCode;
    std::string errorMessage;
    std::string requestId;
    std::string modelName;
};

class SpeechSynthesizer : public std::enable_shared_from_this<SpeechSynthesizer> {
public:
    struct TtsParam {
        OutputAudioInfo outputAudioInfo;
        std::string text;
        std::string requestId;
    };

    using ISpeechEngine = ai_engine::core_ai::speech::AbstractSpeechEngine;
    using ISpeechEnginePtr = std::shared_ptr<ISpeechEngine>;

    using SynthesisResultCallback = std::function<void(SpeechSynthesisResult)>;

public:
    explicit SpeechSynthesizer(bool isDefaultMode);

    ~SpeechSynthesizer();

    bool init(int& errorCode, const ISpeechEnginePtr& speechEngine);

    void destroy();

    void setSynthesisResultCallback(const SynthesisResultCallback& callback);

    void setAudioConfig(const AudioConfigInfo& audioConfigInfo);

    void synthesizeTextOnce(const std::string& text);
    void synthesizeTextOnce(const std::string& text, const std::string& requestId);
    void stopSpeaking();
    bool engineInUse() const { return engineInUse_; }

private:
    std::shared_ptr<SpeechSynthesizer> getSharedObject() { return shared_from_this(); }

    void sendSynthesisResultWithError(const ai_engine::EngineError& error, const std::string& requestId = "") const;
    void writePcmToFile(const std::vector<uint8_t>& audioData);

    void handleSynthesisResult(const ai_engine::core_ai::speech::SynthesisResult& result, const TtsParam& ttsParam);

    void onceTtsAsync();

    void onceTtsTask(TtsParam ttsParam);

    bool tryEnqueueRequest(TtsParam ttsParam);
    void decrementRequestQueueSize();

    void notifyRequestFailed(int errorCode, const std::string& message, const std::string& requestId = "");
    void rejectPendingRequests(int errorCode, const std::string& message);

    DeployType currentEngineDeployType() const;

private:
    OutputAudioInfo outputAudioInfo_;
    ISpeechEnginePtr speechEngine_{nullptr};
    ISpeechEnginePtr fallbackSpeechEngine_{nullptr};
    SynthesisResultCallback synthesisResultCallback_{nullptr};
    bool isFirstWrite_ = true;
    std::atomic_bool engineInUse_{false};
    std::atomic_bool taskRunning_{false};
    std::atomic_bool timedOut_{false};
    std::atomic_bool destroying_{false};
    std::atomic<size_t> requestQueueSize_{0};
    mutable std::mutex engineMutex_;
    moodycamel::BlockingConcurrentQueue<TtsParam> ttsParamQueue_;
    AudioPlayer audioPlayer_{};
    bool isDefaultMode_{true};  // 默认模式要进行自动模型切换
};

#endif

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

#ifndef XUNFEISPEECHENGINE_P_H
#define XUNFEISPEECHENGINE_P_H

#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/speechengine.h>
#include <kylin-ai/common/error.h>

#include "WebSocketClient.h"

// 开始：XunfeiWebSocketClient
class XunfeiWebSocketClient : public hv::WebSocketClient {
public:
    XunfeiWebSocketClient(hv::EventLoopPtr loop = nullptr) : hv::WebSocketClient(loop) {}

    ~XunfeiWebSocketClient() override = default;

    void setConnectTimeoutSec(int sec) const {
        if (channel) {
            channel->setConnectTimeout(sec * 1000);
        }
    }

    void setSendTimeoutSec(int sec) const {
        if (channel) {
            channel->setWriteTimeout(sec * 1000);
        }
    }

    void setRecvTimeoutSec(int sec) const {
        if (channel) {
            channel->setReadTimeout(sec * 1000);
        }
    }

    void setCloseTimeoutSec(int sec) const {
        if (channel) {
            channel->setCloseTimeout(sec * 1000);
        }
    }

private:
    hv::EventLoopPtr loop_{nullptr};
};
// 结束：XunfeiWebSocketClient

class XunfeiSpeechEnginePrivate {
public:
    XunfeiSpeechEnginePrivate();
    ~XunfeiSpeechEnginePrivate();
    std::string engineName() const;
    bool isCloud() const { return true; }
    bool isBuiltInEngine() const { return false; }
    bool isCustomModelEngine() const { return true; }
    std::string currentModel() const;
    std::string currentModelName() const;

    void setConfig(const std::string &config);
    bool setCurrentModel(const std::string &model);
    std::string modelInfo() const;

    void setRecognitionResultCallback(ai_engine::core_ai::speech::RecognitionResultCallback callback);

    void setSynthesisResultCallback(ai_engine::core_ai::speech::SynthesisResultCallback callback);

    // 流式语音识别
    ai_engine::result::AiEngineBaseResult initContinuousRecognitionModule();
    ai_engine::result::AiEngineBaseResult startContinuousRecognition(const std::string &params);
    ai_engine::result::AiEngineBaseResult writeContinuousRecognitionAudioData(const std::vector<uint8_t> &audioData);
    ai_engine::result::AiEngineBaseResult stopContinuousRecognition();
    ai_engine::result::AiEngineBaseResult destroyContinuousRecognitionModule();
    ai_engine::result::AiEngineBaseResult setCurrentContinuousRecognitionModel(const std::string &model) { return {}; }
    std::string currentContinuousRecognitionModel() const { return ""; };

    // 非流式语音识别
    ai_engine::result::AiEngineBaseResult initRecognizeOnceModule();
    ai_engine::result::AiEngineBaseResult recognizeOnce(const std::string &params,
                                                        const std::vector<uint8_t> &audioData);
    ai_engine::result::AiEngineBaseResult destroyRecognizeOnceModule();
    ai_engine::result::AiEngineBaseResult stopRecognizeOnce() { return {}; }
    ai_engine::result::AiEngineBaseResult setCurrentRecognizeOnceModel(const std::string &model) { return {}; }
    std::string currentRecognizeOnceModel() const { return ""; }

    // 一次语音合成
    ai_engine::result::AiEngineBaseResult initSynthesizeModule();
    ai_engine::result::AiEngineBaseResult synthesizeText(const std::string &params, const std::string &text);
    ai_engine::result::AiEngineBaseResult destroySynthesizeModule();
    ai_engine::result::AiEngineBaseResult stopSynthesize() { return {}; }
    ai_engine::result::AiEngineBaseResult setCurrentSynthesizeModel(const std::string &model) { return {}; }
    std::string currentSynthesizeModel() const { return ""; }

private:
    using EngineError = ai_engine::EngineError;

    void setRecognitionParams(const std::string &params);

    void setSynthesisParams(const std::string &params);

    // 开始 - 流式语音识别私有函数
    std::string continuousAsrBuildUrl();
    void handleRecognitionResult(const Json::Value &jsonData);
    bool isRecognizingResult(const Json::Value &jsonData);
    using RecognitionResultList = std::vector<ai_engine::core_ai::speech::RecognitionResult>;
    RecognitionResultList getContinuousRecognitionResult(const Json::Value &jsonData);

    void handleMessage(const std::string &message);
    // 结束 - 流式语音识别私有函数

    // 开始 - 一次语音识别私有函数
    bool onceAsrParamsCheck(const std::vector<uint8_t> &audioData, EngineError &error);
    std::string onceAsrBuildData(const std::string &params, const std::vector<uint8_t> &audioData);
    std::string onceAsrBuildUrl();
    bool onceAsrOnMessageCheck(const std::string &message);
    void onceAsrOnMessage(const std::string &message, std::atomic_bool &isFinish);
    // 结束 - 一次语音识别私有函数

    // 开始 - 一次语音合成私有函数
    bool onceTtsParamsCheck(const std::string &text, EngineError &error);
    std::string onceTtsBuildData(const std::string &params, const std::string &text);
    std::string onceTtsBuildUrl();
    bool onceTtsOnMessageCheck(const std::string &message);
    void onceTtsOnMessage(const std::string &message, std::atomic_bool &isFinish);
    // 结束 - 一次语音合成私有函数

private:
    std::string realtimeAsrHostUrl_{"wss://rtasr.xfyun.cn/v1/ws"};
    std::string asrHostUrl_{"wss://iat-api.xfyun.cn/v2/iat"};
    std::string ttsHostUrl_{"wss://tts-api.xfyun.cn/v2/tts"};

    std::string apiKeyRealTime_;
    std::string appIdRealTime_;
    std::string apiKeyContinuous_;
    std::string secretKeyContinuous_;
    std::string appIdContinuous_;

    std::string currentModel_{"讯飞-语音大模型"};
    std::string currentModelName_{"xunfei-speech-model"};
    std::string currentDisplayName_{"讯飞-语音大模型"};

    ai_engine::core_ai::speech::RecognitionResultCallback recognitionCallback_;
    ai_engine::core_ai::speech::SynthesisResultCallback synthesisCallback_;

    std::string format_;
    int rate_{16000};
    int channel_{1};

    int speed_{50};
    int volume_{50};
    int pitch_{50};

    // 流式语音识别私有变量
    int currentRole_{1};
    std::atomic_bool continuousAsrOpened_{false};
    std::atomic_bool continuousAsrFinished_{false};
    XunfeiWebSocketClient wssClient_{};

    std::mutex mutex_;

    std::atomic_bool onceAsrFinished_{false};
    std::atomic_bool onceTtsFinished_{false};

    std::atomic_bool continuousAsrStarting_{false};
    std::mutex continuousAsrMutex_;
    ai_engine::EngineError continuousAsrError_{AI_COMMON_SUCCESS, ""};
    std::atomic_bool onceTtsClosed_{false};

    std::mutex onceTtsMutex_;
    ai_engine::EngineError onceTtsError_{AI_COMMON_SUCCESS, ""};
};

#endif

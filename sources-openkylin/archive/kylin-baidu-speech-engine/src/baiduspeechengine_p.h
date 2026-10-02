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

#ifndef BAIDUSPEECHENGINE_P_H
#define BAIDUSPEECHENGINE_P_H

#include <jsoncpp/json/value.h>
#include <kylin-ai/ai-engine/speechengine.h>
#include <kylin-ai/common/error.h>

#include "cpr/cpr.h"

#include "WebSocketClient.h"

// 开始：BaiduWebSocketClient
class BaiduWebSocketClient : public hv::WebSocketClient {
public:
    BaiduWebSocketClient(hv::EventLoopPtr loop = nullptr) : hv::WebSocketClient(loop) {}

    ~BaiduWebSocketClient() override = default;

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
// 结束：BaiduWebSocketClient

class BaiduSpeechEnginePrivate {
public:
    BaiduSpeechEnginePrivate();
    ~BaiduSpeechEnginePrivate();
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
    ai_engine::result::AiEngineBaseResult setCurrentContinuousRecognitionModel(const std::string &model) const {
        return {};
    }
    std::string currentContinuousRecognitionModel() const { return ""; };

    // 非流式语音识别
    ai_engine::result::AiEngineBaseResult initRecognizeOnceModule();
    ai_engine::result::AiEngineBaseResult recognizeOnce(const std::string &params,
                                                        const std::vector<uint8_t> &audioData);
    ai_engine::result::AiEngineBaseResult destroyRecognizeOnceModule();
    ai_engine::result::AiEngineBaseResult stopRecognizeOnce() const { return {}; }
    ai_engine::result::AiEngineBaseResult setCurrentRecognizeOnceModel(const std::string &model) const { return {}; }
    std::string currentRecognizeOnceModel() const { return ""; }

    // 一次语音合成
    ai_engine::result::AiEngineBaseResult initSynthesizeModule();
    ai_engine::result::AiEngineBaseResult synthesizeText(const std::string &params, const std::string &text);
    ai_engine::result::AiEngineBaseResult destroySynthesizeModule();
    ai_engine::result::AiEngineBaseResult stopSynthesize() const { return {}; }
    ai_engine::result::AiEngineBaseResult setCurrentSynthesizeModel(const std::string &model) const { return {}; }
    std::string currentSynthesizeModel() const { return ""; }

private:
    // 回调函数相关
    void callRecognitionCallback(const ai_engine::core_ai::speech::RecognitionResult &result) const;

    void callSynthesisCallback(const ai_engine::core_ai::speech::SynthesisResult &result) const;

private:
    enum class OperationType {
        RecognizeOnce,
        ContinuousRecognition,
        Synthesize,
    };

    std::string &getAccessToken();
    bool generateAccessToken();

    void setRecognitionParams(const std::string &params);
    bool writeOnceRecognitionResult(const std::string &data);

    std::string buildAuthParamsData();

    void setSynthesisParams(const std::string &params);
    ai_engine::result::AiEngineBaseResult doRequestSynthesize(const std::string &text, OperationType type);
    bool writeSynthesisResult(const std::string &data, OperationType type);

    ai_engine::result::AiEngineBaseResult processCprResponse(cpr::Response &response, OperationType type);

    void runCallbackWithError(const ai_engine::EngineError &error, OperationType type);

    void handleRecognitionResult(const std::string &message);

private:
    bool refreshAccessTokenRequired_{false};
    bool refreshAccessTokenCompleted_{false};

    std::string appId_;
    std::string apiKey_;
    std::string secretKey_;
    std::string accessToken_;
    std::string currentModel_{"百度-语音大模型"};
    std::string currentModelName_{"baidu-speech-model"};
    std::string currentDisplayName_{"百度-语音大模型"};

    ai_engine::core_ai::speech::RecognitionResultCallback recognitionCallback_;
    ai_engine::core_ai::speech::SynthesisResultCallback synthesisCallback_;

    std::string format_;
    int rate_{0};
    int channel_{0};

    const int maxWebDelay_{1000 * 10};
    int currentWebDelay_{0};
    bool isPinging_{false};

    int speed_{5};
    int volume_{5};
    int pitch_{5};

    ai_engine::EngineError currentError_;

    // 流式语音识别私有变量
    std::atomic_bool isOpen_{false};
    std::atomic_bool isFinish_{false};
    std::atomic_bool isErrorOccurred_{false};
    BaiduWebSocketClient wssClient_{};
    std::mutex mutex_;

    std::string token_{};

    std::string timestamp_;

    std::atomic_bool continuousAsrStarting_{false};
    std::mutex continuousAsrMutex_;
    ai_engine::EngineError continuousAsrError_{AI_COMMON_SUCCESS, ""};
};

#endif

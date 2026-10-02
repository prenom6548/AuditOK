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

#include "baiduspeechengine_p.h"
#include <unistd.h>
#include <kylin-ai/common/error.h>

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include "HttpClient.h"

#include "logger.h"
#include "servererror.h"
#include "token.h"
#include "util.h"

namespace {
unsigned long long getTimestampUs() {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * (unsigned long long)(1000000) + tv.tv_usec;
}
} // namespace

BaiduSpeechEnginePrivate::BaiduSpeechEnginePrivate() {
#ifdef RUN_IN_DEBUG
    hlog_set_format("[%y-%m-%d %H:%M:%S.%z] %L %s");
    hlog_set_handler([](int loglevel, const char *buf, int len) { printf("%.*s", len, buf); });
#else
    hlog_set_level(LOG_LEVEL_SILENT);
#endif
}

BaiduSpeechEnginePrivate::~BaiduSpeechEnginePrivate() {}

std::string BaiduSpeechEnginePrivate::engineName() const { return "baidu"; }

std::string BaiduSpeechEnginePrivate::currentModel() const { return currentModel_; }

std::string BaiduSpeechEnginePrivate::currentModelName() const { return currentModelName_; }

bool BaiduSpeechEnginePrivate::setCurrentModel(const std::string &model) {
    Json::Value modelInfoJson = baidu_speech_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            currentModel_ = model;
            return true;
        }
    }
    fprintf(stderr, "Invalid current model name: %s\n", model.c_str());
    return false;
}

void BaiduSpeechEnginePrivate::setConfig(const std::string &config) {
    // config格式要求如下：
    //  {
    //      "appId" : "",
    //      "apiKey" : "",
    //      "secretKey" : ""
    //  }
    Json::Value configJson = baidu_speech_util::formatJsonFromString(config);
    if (configJson.isNull()) {
        LOGE("Invalid config for baidu speech engine, json is NULL", config);
        return;
    }
    if (!configJson.isMember("appId") || !configJson["appId"].isString()) {
        LOGE("Invalid config [appId] for baidu speech engine:", config);
        return;
    }
    if (!configJson.isMember("apiKey") || !configJson["apiKey"].isString()) {
        LOGE("Invalid config [apiKey] for baidu speech engine:", config);
        return;
    }
    if (!configJson.isMember("secretKey") || !configJson["secretKey"].isString()) {
        LOGE("Invalid config [secretKey] for baidu speech engine:", config);
        return;
    }
    appId_ = configJson["appId"].asString();
    apiKey_ = configJson["apiKey"].asString();
    secretKey_ = configJson["secretKey"].asString();
}

std::string BaiduSpeechEnginePrivate::modelInfo() const {
    const char *info = R"(
    {
        "vendor": "百度",
        "models": [
            {
                "modelName": "baidu-speech-model",
                "displayName": "百度-语音大模型",
                "model": "百度-语音大模型",
                "brand": "baidu",
                "subConfig": [
                    {
                        "name": "语音识别&语音合成",
                        "config": [
                            {
                                "friendlyName": "APPID",
                                "configName": "appId"
                            },
                            {
                                "friendlyName": "APIKey",
                                "configName": "apiKey"
                            },
                            {
                                "friendlyName": "APISecret",
                                "configName": "secretKey"
                            }
                        ]
                    }
                ],
                "is3rdModel": true
            }
        ]
    }
    )";

    return info;
}

void BaiduSpeechEnginePrivate::setRecognitionResultCallback(
    ai_engine::core_ai::speech::RecognitionResultCallback callback) {
    recognitionCallback_ = std::move(callback);
}

void BaiduSpeechEnginePrivate::setSynthesisResultCallback(
    ai_engine::core_ai::speech::SynthesisResultCallback callback) {
    synthesisCallback_ = std::move(callback);
}

void BaiduSpeechEnginePrivate::handleRecognitionResult(const std::string &message) {
    LOGI(message);
    Json::Value json = baidu_speech_util::stringToJson(message);
    if (json == Json::nullValue) {
        // 不完整的json数据
        LOGE("Invalid recognition result:", message, ".");
        isErrorOccurred_ = true;
        ai_engine::core_ai::speech::RecognitionResult result{};
        result.success = false;
        result.error = ai_engine::EngineError{AI_COMMON_SERVICE_ERROR, "Parse json message failed."};
        result.modelName = currentDisplayName_;
        result.text = "";
        result.speakerId = 0;
        result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred;
        if (continuousAsrStarting_) {
            std::lock_guard locker(continuousAsrMutex_);
            continuousAsrError_ =
                ai_engine::EngineError(AI_COMMON_NET_ERROR, "Continuous asr: on message: message is incomplete.");
        } else {
            if (recognitionCallback_) {
                recognitionCallback_(result);
            }
        }
        return;
    }
    int errorCode = baidu_speech_server_error::parseErrorCode(message);
    if (errorCode == 0) {
        // 正常情况下
        ai_engine::core_ai::speech::SpeechResultReason reason{
            ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred};
        if (json.isMember("type") && json["type"].isString()) {
            if (json["type"].asString() == "FIN_TEXT") {
                reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognized;
            } else if (json["type"].asString() == "MID_TEXT") {
                reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognizing;
            } else if (json["type"].asString() == "HEARTBEAT") {
                return;
            }
        }
        std::string recognitionMessage;
        if (json.isMember("result") && json["result"].isString()) {
            recognitionMessage = json["result"].asString();
        }

        ai_engine::core_ai::speech::RecognitionResult result{};
        result.success = true;
        result.error = ai_engine::EngineError{};
        result.modelName = currentDisplayName_;
        result.speakerId = 0;
        result.text = recognitionMessage;
        result.reason = reason;
        if (recognitionCallback_) {
            recognitionCallback_(result);
        }
    } else {
        // 出错情况下
        if (errorCode == -3005) {
            // 音频质量问题，无效音频，这一句话报错，不影响后续识别
            return;
        }
        isErrorOccurred_ = true;
        int theErrorCode = baidu_speech_server_error::realtimeAsrErrorCode2speechResult(errorCode);

        std::string errorMessage;
        if (json.isMember("err_msg") && json["err_msg"].isString()) {
            errorMessage = json["err_msg"].asString();
        }
        ai_engine::core_ai::speech::RecognitionResult result{};
        result.success = false;
        result.error = ai_engine::EngineError{theErrorCode, errorMessage};
        result.modelName = currentDisplayName_;
        result.speakerId = 0;
        result.text = "";
        result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred;
        if (continuousAsrStarting_) {
            std::lock_guard locker(continuousAsrMutex_);
            continuousAsrError_ = ai_engine::EngineError{theErrorCode, errorMessage};
        } else {
            if (recognitionCallback_) {
                recognitionCallback_(result);
            }
        }
    }
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::initContinuousRecognitionModule() {
    // 因为百度语音识别鉴权成功时不返回且和语音识别结果处于同一信道，所以此处不用实现
    return {true, ai_engine::EngineError{}};
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::startContinuousRecognition(const std::string &params) {
    std::lock_guard locker(mutex_);
    continuousAsrStarting_ = true;
    continuousAsrError_ = ai_engine::EngineError{AI_COMMON_SUCCESS, ""};
    ai_engine::result::AiEngineBaseResult result{false, ai_engine::EngineError{}};
    setRecognitionParams(params);
    wssClient_.onopen = [this]() {
        LOGI("onopen");
        isOpen_ = true;
    };
    wssClient_.onmessage = [this](const std::string &message) { handleRecognitionResult(message); };
    wssClient_.onclose = []() { LOGI("onclose"); };
    wssClient_.setPingInterval(1000);

    isOpen_ = false;
    isErrorOccurred_ = false;
    timestamp_ = std::to_string(getTimestampUs());
    std::string url = "wss://vop.baidu.com/realtime_asr?sn=kylin-ai-subsystem" + timestamp_;
    int ret = wssClient_.open(url.c_str());
    if (ret != 0) {
        LOGI("Open failed.");
        result.success = false;
        result.error = ai_engine::EngineError(AI_COMMON_NET_ERROR, "Network disconnected.");
        continuousAsrStarting_ = false;
        return result;
    }
    // 设置超时时间函数必须放到open函数之后，否则会失效
    wssClient_.setSendTimeoutSec(3);
    wssClient_.setRecvTimeoutSec(3);
    // 需求要求5秒
    for (int i = 0; i < 30; ++i) {
        // 100ms
        baidu_speech_util::sleepForMs(100);
        if (isOpen_) {
            break;
        }
    }
    if (isOpen_) {
        if (baidu_speech_util::isAllDigits(appId_)) {
            result.success = true;
            isFinish_ = false;
            std::string authParamsData = buildAuthParamsData();
            wssClient_.send(authParamsData);
        } else {
            // appId 不是数字，则直接报鉴权错误
            result.success = false;
            result.error = ai_engine::EngineError(AI_COMMON_AUTHENTICATION_FAILURE, "Authentication failed.");
        }
    } else {
        isFinish_ = false;
        result.success = false;
        result.error = ai_engine::EngineError(AI_COMMON_NET_ERROR, "Network disconnected.");
    }
    if (result.success) {
        // 上述情况下未报错，再检查回调中是否有错误
        std::lock_guard continuousAsrLocker(continuousAsrMutex_);
        if (continuousAsrError_.getCode() != AI_COMMON_SUCCESS && continuousAsrError_.getCode() != -1) {
            wssClient_.close();
            result.success = false;
            result.error = continuousAsrError_;
            continuousAsrStarting_ = false;
            return result;
        }
        ai_engine::core_ai::speech::RecognitionResult recognitionResult{};
        recognitionResult.success = true;
        recognitionResult.error = {};
        recognitionResult.modelName = currentDisplayName_;
        recognitionResult.text = "";
        recognitionResult.speakerId = 0;
        recognitionResult.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognitionStarted;
        recognitionCallback_(recognitionResult);
    }
    continuousAsrStarting_ = false;
    return result;
}

ai_engine::result::AiEngineBaseResult
BaiduSpeechEnginePrivate::writeContinuousRecognitionAudioData(const std::vector<uint8_t> &audioData) {
    std::lock_guard locker(mutex_);
    ai_engine::result::AiEngineBaseResult result{false, ai_engine::EngineError{}};
    if (isFinish_) {
        // 主动关闭
        result.success = true;
        return result;
    }
    if (isErrorOccurred_) {
        // 主动关闭，回调中出现错误后，再次调用write函数直接返回，不报错，防止出现两个错误
        result.success = true;
        return result;
    }
    if (!isOpen_) {
        result.success = false;
        result.error = ai_engine::EngineError(AI_COMMON_SERVICE_ERROR, "Wss client is not inited.");
        return result;
    }

    if (audioData.empty()) {
        result.success = false;
        result.error =
            ai_engine::EngineError(AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID, "Audio data is empty!");
        return result;
    }

    int sent = wssClient_.send(reinterpret_cast<const char *>(audioData.data()), int(audioData.size()),
                               ws_opcode::WS_OPCODE_BINARY);
    if (sent < 0) {
        LOGI("Send failed.");
        result.success = false;
        result.error = ai_engine::EngineError(AI_COMMON_NET_ERROR, "Network disconnected.");
        return result;
    }
    result.success = true;
    return result;
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::stopContinuousRecognition() {
    std::lock_guard locker(mutex_);
    wssClient_.send(R"({"type": "FINISH"})");
    isFinish_ = true;
    return {true, ai_engine::EngineError{}};
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::destroyContinuousRecognitionModule() {
    wssClient_.close();
    return {true, ai_engine::EngineError{}};
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::initRecognizeOnceModule() {
    currentError_ = ai_engine::EngineError();
    if (!generateAccessToken()) {
        return {false, currentError_};
    }
    refreshAccessTokenCompleted_ = false;
    refreshAccessTokenRequired_ = false;
    return {true, ai_engine::EngineError()};
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::recognizeOnce(const std::string &params,
                                                                              const std::vector<uint8_t> &audioData) {
    currentError_ = ai_engine::EngineError();

    if (!recognitionCallback_) {
        return {false, currentError_};
    }
    if (audioData.empty()) {
        currentError_ =
            ai_engine::EngineError(AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID, "Audio data is empty!");
        return {false, currentError_};
    }

    setRecognitionParams(params);

    Json::Value postJson;
    postJson["format"] = format_;
    postJson["rate"] = 16000;
    postJson["channel"] = 1;
    postJson["cuid"] = "kylin-ai-subsystem";
    postJson["token"] = getAccessToken();
    postJson["dev_pid"] = 80001;
    postJson["speech"] = baidu_speech_util::base64Encode(std::string(audioData.begin(), audioData.end()));
    postJson["len"] = (int)audioData.size();
    postJson["type"] = "FINISH";
    cpr::Url url{"https://vop.baidu.com/pro_api"};
    cpr::Header header{{"Content-Type", "application/json"}};
    cpr::Body body{{postJson.toStyledString()}};
    cpr::Response response =
        cpr::Post(url, header, body, cpr::WriteCallback{[this](const std::string &data, intptr_t userdata) {
                      return writeOnceRecognitionResult(data);
                  }});

    if (refreshAccessTokenRequired_) {
        refreshAccessTokenCompleted_ = true;
        refreshAccessTokenRequired_ = false;
        generateAccessToken();
        return recognizeOnce(params, audioData);
    }

    return processCprResponse(response, OperationType::RecognizeOnce);
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::destroyRecognizeOnceModule() {
    return {true, ai_engine::EngineError()};
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::initSynthesizeModule() {
    return baidu_speech_token::getToken(apiKey_, secretKey_, token_);
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::synthesizeText(const std::string &params,
                                                                               const std::string &text) {
    if (synthesisCallback_ == nullptr) {
        return {false, {AI_COMMON_SPEECH_SYNTHESIS_TEXT_LENGTH_INVALID, "Callback is nullptr."}};
    }
    if (text.empty()) {
        return {false, {AI_COMMON_SPEECH_SYNTHESIS_TEXT_LENGTH_INVALID, "Text is empty."}};
    }

    ai_engine::core_ai::speech::SynthesisResult result;
    result.success = true;
    result.error = {};
    result.modelName = currentDisplayName_;
    result.audioFormat = {};
    result.audioRate = {};
    result.audioChannel = {};
    result.base64AudioData = {};
    result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesisStarted;
    callSynthesisCallback(result);

    setSynthesisParams(params);

    auto texts = baidu_speech_util::segmentString(text);
    int continueCounter = 0;
    for (int i = 0; i < texts.size(); i++) {
        const std::string &segment = texts[i];
        LOGI(i, segment);
        LOGI("====================================================");
        std::string body;
        body += "tex=" + baidu_speech_util::urlEncode(baidu_speech_util::urlEncode(segment));
        body += "&tok=" + token_;
        body += "&cuid=kylin-ai-subsystem";
        body += "&ctp=1";
        body += "&lan=zh";
        body += "&spd=" + std::to_string(speed_);
        body += "&pit=" + std::to_string(pitch_);
        body += "&vol=" + std::to_string(volume_);
        body += "&per=0"; // 参考：https://ai.baidu.com/ai-doc/SPEECH/Rluv3uq3d
        body += "&aue=4";

        HttpRequest request;
        request.method = HTTP_POST;
        request.url = "https://tsn.baidu.com/text2audio";
        request.headers["accept"] = "*/*";
        request.headers["Content-Type"] = "application/x-www-form-urlencoded";
        request.body = body;
        request.timeout = 10;

        HttpResponse response;
        hv::HttpClient client;
        int ret = client.send(&request, &response);
        if (ret != 0 || response.status_code != 200) {
            if (response.status_code == 400) {
                // 网络未认证时，这里返回400，先将400映射到网络未认证错误
                return {false, {AI_COMMON_SSL_CERTIFICATE_ERROR, "Network Unauthenticated Error."}};
            } else {
                LOGI("request failed or response failed, ", ret, response.status_code);
                i -= 1;
                if (continueCounter >= 1) {
                    return {false, {AI_COMMON_NET_ERROR, "Request failed."}};
                }
                continueCounter += 1;
                baidu_speech_util::sleepForSec(1);
                continue;
            }
        }

        if (response.ContentType() == AUDIO_BASIC) {
            std::string format;
            int rate;
            int channel;
            baidu_speech_util::parseAudioBasicContentTypeMessage(response.GetHeader("Content-Type"), format, rate,
                                                                 channel);
            result.success = true;
            result.error = {};
            result.modelName = currentDisplayName_;
            result.audioFormat = format;
            result.audioRate = rate;
            result.audioChannel = channel;
            result.base64AudioData = baidu_speech_util::base64Encode(response.body);
            result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesizing;
            callSynthesisCallback(result);
        } else if (response.ContentType() == APPLICATION_JSON) {
            int errorCode{};
            int subErrorCode{};
            std::string errorMessage;
            baidu_speech_server_error::parseErrorInfo(response.body, errorCode, subErrorCode, errorMessage);
            // 参考文档：https://ai.baidu.com/ai-doc/SPEECH/Zlbxew2qk
            if (errorCode == 502 && (subErrorCode == 110 || subErrorCode == 111)) {
                // token过期重新获取一个
                auto getTokenResult = baidu_speech_token::getToken(apiKey_, secretKey_, token_);
                if (!getTokenResult.success) {
                    return {false, getTokenResult.error};
                }
                continue;
            } else {
                return {false, {baidu_speech_server_error::ttsErrorCode2speechResult(errorCode), errorMessage}};
            }
        } else {
            // 未知响应
            return {false, {AI_COMMON_UNKNOWN_ERROR, "Response content type is not supported."}};
        }
    }
    result.success = true;
    result.error = {};
    result.modelName = currentDisplayName_;
    result.audioFormat = {};
    result.audioRate = {};
    result.audioChannel = {};
    result.base64AudioData.clear();
    result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesisCompleted;
    callSynthesisCallback(result);
    continueCounter = 0; // 重置重试计数器
    return {true, {}};
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::destroySynthesizeModule() {
    return {true, ai_engine::EngineError()};
}

void BaiduSpeechEnginePrivate::callRecognitionCallback(
    const ai_engine::core_ai::speech::RecognitionResult &result) const {
    if (recognitionCallback_ != nullptr) {
        recognitionCallback_(result);
    }
}

void BaiduSpeechEnginePrivate::callSynthesisCallback(const ai_engine::core_ai::speech::SynthesisResult &result) const {
    if (synthesisCallback_ != nullptr) {
        synthesisCallback_(result);
    }
}

std::string &BaiduSpeechEnginePrivate::getAccessToken() {
    if (accessToken_.empty()) {
        generateAccessToken();
    }
    return accessToken_;
}

bool BaiduSpeechEnginePrivate::generateAccessToken() {
    int errorCode{int(cpr::ErrorCode::OK)};
    std::string errorMessage;
    LOGI("apiKey_, secretKey_", apiKey_, secretKey_);
    accessToken_ = baidu_speech_token::getBaiduSpeechToken(apiKey_, secretKey_, errorCode, errorMessage);
    if (accessToken_.empty()) {
        fprintf(stderr, "get baidu token failed: %s\n", errorMessage.c_str());
        if (errorCode != int(cpr::ErrorCode::OK)) {
            if (errorCode == int(cpr::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR)) {
                currentError_ = ai_engine::EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, errorMessage);
            } else {
                currentError_ = ai_engine::EngineError(AI_COMMON_NET_ERROR, errorMessage);
            }
        } else {
            currentError_ = ai_engine::EngineError(AI_COMMON_AUTHENTICATION_FAILURE, errorMessage);
        }
        return false;
    }
    return true;
}

void BaiduSpeechEnginePrivate::setRecognitionParams(const std::string &params) {
    Json::Value paramsJson = baidu_speech_util::formatJsonFromString(params);
    format_ = paramsJson["format"].asString();
    rate_ = paramsJson["rate"].asInt();
    channel_ = paramsJson["channel"].asInt();
    if (format_.empty()) {
        format_ = "pcm";
    }
    if (rate_ == 0) {
        rate_ = 16000;
    }
    if (channel_ == 0) {
        channel_ = 1;
    }
}

bool BaiduSpeechEnginePrivate::writeOnceRecognitionResult(const std::string &data) {

    Json::Value resultJson = baidu_speech_util::formatJsonFromString(data);
    if (baidu_speech_token::isBaiduTokenExpiredByResult(resultJson)) {
        if (!refreshAccessTokenCompleted_) {
            refreshAccessTokenRequired_ = true;
            return false;
        }
    }

    int errorCode = baidu_speech_server_error::parseErrorCode(data);
    if (errorCode == 3307) {
        fprintf(stderr, "baidu recognize failed: %s\n", "原始音频质量过差");
        resultJson["result"][0] = "";
    } else if (errorCode > 0) {
        fprintf(stderr, "baidu recognize failed: %s\n", data.c_str());
        currentError_ = ai_engine::EngineError(baidu_speech_server_error::asrErrorCode2speechResult(errorCode), data);
        runCallbackWithError(currentError_, BaiduSpeechEnginePrivate::OperationType::RecognizeOnce);
        return false;
    }

    ai_engine::core_ai::speech::RecognitionResult result;
    ai_engine::EngineError error;
    result.success = true;
    result.error = error;
    result.modelName = currentDisplayName_;
    result.text = resultJson["result"][0].asString();
    result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognitionCompleted;
    result.speakerId = 0;
    refreshAccessTokenCompleted_ = false;
    refreshAccessTokenRequired_ = false;
    callRecognitionCallback(result);
    return true;
}

std::string BaiduSpeechEnginePrivate::buildAuthParamsData() {
    Json::Value bodyJson;
    bodyJson["data"]["appid"] = atoi(appId_.c_str());
    bodyJson["data"]["appkey"] = apiKey_;
    bodyJson["data"]["dev_pid"] = 15372;
    bodyJson["data"]["cuid"] = "kylin-ai-subsystem" + timestamp_;
    bodyJson["data"]["format"] = "pcm";
    bodyJson["data"]["sample"] = 16000;
    bodyJson["type"] = "START";
    return bodyJson.toStyledString();
}

void BaiduSpeechEnginePrivate::setSynthesisParams(const std::string &params) {
    if (params.empty()) {
        speed_ = 5;
        volume_ = 5;
        pitch_ = 5;
        return;
    }
    Json::Value paramsJson = baidu_speech_util::formatJsonFromString(params);
    speed_ = paramsJson["speed"].asInt();
    volume_ = paramsJson["volume"].asInt();
    pitch_ = paramsJson["pitch"].asInt();
    if (speed_ < 0 or speed_ > 100) {
        speed_ = 50;
    }
    if (volume_ < 0 or volume_ > 100) {
        volume_ = 50;
    }
    if (pitch_ < 0 or pitch_ > 100) {
        pitch_ = 50;
    }
    speed_ /= 10;
    volume_ /= 10;
    pitch_ /= 10;
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::doRequestSynthesize(const std::string &text,
                                                                                    OperationType type) {
    std::string postString;
    postString += ("tex=" + baidu_speech_util::urlEncode(baidu_speech_util::urlEncode(text)));
    postString += ("&tok=" + getAccessToken());
    postString += ("&cuid=kylin-ai-subsystem");
    postString += ("&ctp=1");
    postString += ("&lan=zh");
    postString += ("&spd=" + std::to_string(speed_));
    postString += ("&pit=" + std::to_string(pitch_));
    postString += ("&vol=" + std::to_string(volume_));
    postString += ("&per=1");
    postString += ("&aue=4");
    cpr::Url url{"https://tsn.baidu.com/text2audio"};
    cpr::Header header{{"accept", "*/*"}, {"Content-Type", "application/x-www-form-urlencoded"}};
    cpr::Body body{postString};

    cpr::Response response =
        cpr::Post(url, header, body, cpr::WriteCallback{[this, type](const std::string &data, intptr_t userdata) {
                      return writeSynthesisResult(data, type);
                  }},
                  cpr::LowSpeed(100, 30));

    if (refreshAccessTokenRequired_) {
        refreshAccessTokenCompleted_ = true;
        refreshAccessTokenRequired_ = false;
        generateAccessToken();
        return doRequestSynthesize(text, type);
    }

    return processCprResponse(response, type);
}

bool BaiduSpeechEnginePrivate::writeSynthesisResult(const std::string &data, OperationType type) {
    Json::Value resultJson = baidu_speech_util::formatJsonFromString(data);
    if (baidu_speech_token::isBaiduTokenExpiredByResult(resultJson)) {
        if (!refreshAccessTokenCompleted_) {
            refreshAccessTokenRequired_ = true;
            return false;
        }
    }

    if (int errorCode = baidu_speech_server_error::parseErrorCode(data)) {
        LOGE("baidu synthesize failed:", data);
        currentError_ = ai_engine::EngineError(baidu_speech_server_error::ttsErrorCode2speechResult(errorCode), data);
        runCallbackWithError(currentError_, type);
        return false;
    }

    ai_engine::core_ai::speech::SynthesisResult result;
    result.success = true;
    result.error = ai_engine::EngineError();
    result.modelName = currentDisplayName_;
    result.audioFormat = "pcm";
    result.audioRate = 16000;
    result.audioChannel = 1;
    result.base64AudioData = baidu_speech_util::base64Encode(data);
    result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesizing;
    refreshAccessTokenCompleted_ = false;
    refreshAccessTokenRequired_ = false;
    callSynthesisCallback(result);
    return true;
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEnginePrivate::processCprResponse(cpr::Response &response,
                                                                                   OperationType type) {
    if (response.error.code == cpr::ErrorCode::REQUEST_CANCELLED) {
        return {false, ai_engine::EngineError()};
    }

    if (response.error) {
        fprintf(stderr, "net error: %s\n", response.error.message.c_str());
        currentError_ = ai_engine::EngineError(AI_COMMON_NET_ERROR, response.error.message);
        return {false, currentError_};
    }

    return {true, ai_engine::EngineError()};
}

void BaiduSpeechEnginePrivate::runCallbackWithError(const ai_engine::EngineError &error, OperationType type) {
    if (type == OperationType::RecognizeOnce) {
        ai_engine::core_ai::speech::RecognitionResult result{
            false,         error, currentDisplayName_,
            std::string(), 0,     ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred};
        callRecognitionCallback(result);
    } else if (type == OperationType::ContinuousRecognition) {
        ai_engine::core_ai::speech::RecognitionResult result{
            false,         error, currentDisplayName_,
            std::string(), 0,     ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred};
        callRecognitionCallback(result);
    } else if (type == OperationType::Synthesize) {
        ai_engine::core_ai::speech::SynthesisResult result{
            false,
            error,
            currentDisplayName_,
            {},
            {},
            {},
            {},
            ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred};
        callSynthesisCallback(result);
    }
}

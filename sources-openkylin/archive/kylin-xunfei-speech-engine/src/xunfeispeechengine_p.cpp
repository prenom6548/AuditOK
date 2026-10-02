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

#include "xunfeispeechengine_p.h"

#include <kylin-ai/common/error.h>
#include <unistd.h>

#include <charconv>
#include <cstring>
#include <string>

#include "logger.h"
#include "servererror.h"
#include "util.h"

namespace {
bool stringContains(const std::string &str, const std::string &sub) { return str.find(sub) != std::string::npos; }

}  // namespace

XunfeiSpeechEnginePrivate::XunfeiSpeechEnginePrivate() {
#ifdef RUN_IN_DEBUG
    hlog_set_format("[%y-%m-%d %H:%M:%S.%z] %L %s");
    hlog_set_handler([](int loglevel, const char *buf, int len) { printf("%.*s", len, buf); });
#else
    hlog_set_level(LOG_LEVEL_SILENT);
#endif
}

XunfeiSpeechEnginePrivate::~XunfeiSpeechEnginePrivate() {}

std::string XunfeiSpeechEnginePrivate::engineName() const { return "xunfei"; }

std::string XunfeiSpeechEnginePrivate::currentModel() const { return currentModel_; }

std::string XunfeiSpeechEnginePrivate::currentModelName() const { return currentModelName_; }

void XunfeiSpeechEnginePrivate::setConfig(const std::string &config) {
    Json::Value configJson = xunfei_speech_util::formatJsonFromString(config);

    if (!configJson.isNull() && configJson.isMember("apiKeyRealTime") && configJson.isMember("appIdRealTime") &&
        configJson["appIdRealTime"].isString() && configJson["apiKeyRealTime"].isString()) {
        appIdRealTime_ = configJson["appIdRealTime"].asString();
        apiKeyRealTime_ = configJson["apiKeyRealTime"].asString();
    } else {
        LOGE("Invalid config for xunfei realtime asr engine:", config, ".");
    }

    if (!configJson.isNull() && configJson.isMember("apiKeyContinuous") && configJson.isMember("secretKeyContinuous") &&
        configJson.isMember("appIdContinuous") && configJson["apiKeyContinuous"].isString() &&
        configJson["secretKeyContinuous"].isString() && configJson["appIdContinuous"].isString()) {
        appIdContinuous_ = configJson["appIdContinuous"].asString();
        apiKeyContinuous_ = configJson["apiKeyContinuous"].asString();
        secretKeyContinuous_ = configJson["secretKeyContinuous"].asString();
    } else {
        LOGE("Invalid config for xunfei iat&tts engine:", config, ".");
    }
}

bool XunfeiSpeechEnginePrivate::setCurrentModel(const std::string &model) {
    Json::Value modelInfoJson = xunfei_speech_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            currentModel_ = model;
            return true;
        }
    }
    LOGE("Invalid current model name:", model, ".");
    return false;
}

std::string XunfeiSpeechEnginePrivate::modelInfo() const {
    const char *info = R"(
    {
        "vendor": "讯飞",
        "models": [
            {
                "modelName": "xunfei-speech-model",
                "displayName": "讯飞-语音大模型",
                "model": "讯飞-语音大模型",
                "brand": "xunfei",
                "subConfig": [
                    {
                        "name": "语音听写&语音合成",
                        "config": [
                            {
                                "friendlyName": "APPID",
                                "configName": "appIdContinuous"
                            },
                            {
                                "friendlyName": "APIKey",
                                "configName": "apiKeyContinuous"
                            },
                            {
                                "friendlyName": "APISecret",
                                "configName": "secretKeyContinuous"
                            }
                        ]
                    },
                    {
                        "name": "实时语音转写",
                        "config": [
                            {
                                "friendlyName": "APPID",
                                "configName": "appIdRealTime"
                            },
                            {
                                "friendlyName": "APIKey",
                                "configName": "apiKeyRealTime"
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

void XunfeiSpeechEnginePrivate::setRecognitionResultCallback(
    ai_engine::core_ai::speech::RecognitionResultCallback callback) {
    std::lock_guard locker(mutex_);
    recognitionCallback_ = std::move(callback);
}

void XunfeiSpeechEnginePrivate::setSynthesisResultCallback(
    ai_engine::core_ai::speech::SynthesisResultCallback callback) {
    std::lock_guard locker(mutex_);
    synthesisCallback_ = std::move(callback);
}

void XunfeiSpeechEnginePrivate::setRecognitionParams(const std::string &params) {
    Json::Value paramsJson = xunfei_speech_util::formatJsonFromString(params);
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

void XunfeiSpeechEnginePrivate::setSynthesisParams(const std::string &params) {
    Json::Value paramsJson = xunfei_speech_util::formatJsonFromString(params);
    if (paramsJson.isMember("speed") && paramsJson["speed"].isInt()) {
        speed_ = paramsJson["speed"].asInt();
    }
    if (paramsJson.isMember("volume") && paramsJson["volume"].isInt()) {
        volume_ = paramsJson["volume"].asInt();
    }
    if (paramsJson.isMember("pitch") && paramsJson["pitch"].isInt()) {
        pitch_ = paramsJson["pitch"].asInt();
    }
    if (speed_ < 0 || speed_ > 100) {
        speed_ = 50;
    }
    if (volume_ < 0 || volume_ > 100) {
        volume_ = 50;
    }
    if (pitch_ < 0 || pitch_ > 100) {
        pitch_ = 50;
    }
}

std::string XunfeiSpeechEnginePrivate::continuousAsrBuildUrl() {
    std::string timestampString = xunfei_speech_util::getCurrentUnixTimestamp();
    std::string signature = appIdRealTime_ + timestampString;
    signature = xunfei_speech_util::calculateMD5(signature);
    signature = xunfei_speech_util::hmacSha1Encode(signature, apiKeyRealTime_);
    std::string url = std::string(realtimeAsrHostUrl_) + "?" + "appid=" + appIdRealTime_ +
                      "&ts=" + xunfei_speech_util::urlEncode(timestampString) +
                      "&signa=" + xunfei_speech_util::urlEncode(signature) + "&roleType=2";
    return url;
}

void XunfeiSpeechEnginePrivate::handleRecognitionResult(const Json::Value &jsonData) {
    RecognitionResultList resultList = getContinuousRecognitionResult(jsonData);

    if (recognitionCallback_) {
        for (const auto &result : resultList) {
            recognitionCallback_(result);
        }
    }
}

bool XunfeiSpeechEnginePrivate::isRecognizingResult(const Json::Value &jsonData) {
    return jsonData["cn"]["st"]["type"].asString() == "1";
}

XunfeiSpeechEnginePrivate::RecognitionResultList XunfeiSpeechEnginePrivate::getContinuousRecognitionResult(
    const Json::Value &jsonData) {
    RecognitionResultList resultList;
    for (int k = 0; k < jsonData["cn"]["st"]["rt"].size(); k++) {
        for (int i = 0; i < jsonData["cn"]["st"]["rt"][k]["ws"].size(); i++) {
            for (int j = 0; j < jsonData["cn"]["st"]["rt"][k]["ws"][i]["cw"].size(); j++) {
                const std::string resultText = jsonData["cn"]["st"]["rt"][k]["ws"][i]["cw"][j]["w"].asString();
                // 该值只有角色切换时才会变化，其余时值为0。例如角色A开始说话
                // rl=1，后面角色A说话rl都是0，等到角色B开始说话时，rl=2，角色
                // B继续说话rl又变回0。
                const std::string roleText = jsonData["cn"]["st"]["rt"][k]["ws"][i]["cw"][j]["rl"].asString();
                ai_engine::core_ai::speech::RecognitionResult result{};
                result.success = true;
                result.error = EngineError{};
                result.modelName = currentDisplayName_;
                result.text = resultText;
                result.speakerId = 1;
                if (isRecognizingResult(jsonData)) {
                    result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognizing;
                } else {
                    result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognized;
                }

                int role = atoi(roleText.c_str());
                if (role != currentRole_ && role != 0) {
                    result.speakerId = role;
                    currentRole_ = role;
                    resultList.push_back(result);
                    continue;
                }

                if (resultList.empty()) {
                    resultList.push_back(result);
                } else {
                    resultList.back().text += resultText;
                }
            }
        }
    }
    return resultList;
}

void XunfeiSpeechEnginePrivate::handleMessage(const std::string &message) {
    LOGI("Continuous asr: on message", message);
    if (message.empty()) {
        return;
    }
    int code = xunfei_speech_server_error::parseErrorCode(message);
    if (code == -1) {
        // 回复消息体不完整，不是json结构
        ai_engine::core_ai::speech::RecognitionResult result{};
        result.success = false;
        result.error = EngineError(AI_COMMON_NET_ERROR, "Continuous asr: on message: message is incomplete.");
        result.modelName = currentDisplayName_;
        result.text = "";
        result.speakerId = -1;
        result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred;
        if (continuousAsrStarting_) {
            std::lock_guard locker(continuousAsrMutex_);
            continuousAsrError_ =
                EngineError(AI_COMMON_NET_ERROR, "Continuous asr: on message: message is incomplete.");
        } else {
            if (recognitionCallback_) {
                recognitionCallback_(result);
            }
        }

    } else if (code == 0) {
        Json::Value resultJson = xunfei_speech_util::stringToJson(message);
        if (resultJson.isMember("action") && resultJson["action"].isString()) {
            std::string action = resultJson["action"].asString();
            if (action == "started") {
                LOGI("started");
            } else if (action == "result") {
                Json::Value dataJson = xunfei_speech_util::formatJsonFromString(resultJson["data"].asString());
                handleRecognitionResult(dataJson);
            }
        }
    } else {
        ai_engine::core_ai::speech::RecognitionResult result{};
        result.success = false;
        int errorCode = xunfei_speech_server_error::realtimeAsrErrorCode2speechResult(code);
        Json::Value value = xunfei_speech_util::stringToJson(message);
        std::string errorMessage;
        if (value.isMember("desc") && value["desc"].isString()) {
            errorMessage = value["desc"].asString();
        }
        // 返回10700错误码(引擎错误)时包含'超过15秒没有向服务端发送音频数据'的情况
        // 会在报错信息中显示, 所以需要单独判断一下，返回正确的错误码
        if (errorMessage.find("37005") != std::string::npos) {
            errorCode = AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID;
        }
        result.error = EngineError(errorCode, errorMessage);
        result.modelName = currentDisplayName_;
        result.text = "";
        result.speakerId = -1;
        result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred;
        if (continuousAsrStarting_) {
            std::lock_guard locker(continuousAsrMutex_);
            continuousAsrError_ = EngineError(errorCode, errorMessage);
        } else {
            if (recognitionCallback_) {
                recognitionCallback_(result);
            }
        }
    }
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::initContinuousRecognitionModule() {
    ai_engine::result::AiEngineBaseResult result{false, EngineError{}};
    if (appIdRealTime_.empty() || apiKeyRealTime_.empty()) {
        result.success = false;
        result.error = EngineError{AI_COMMON_MODEL_NOT_FOUND, "Init continuous asr: appId or apiKey is empty."};
        return result;
    }
    std::atomic_bool isOpen{false};
    std::atomic_bool isMessaged{false};
    XunfeiWebSocketClient client{};
    client.onopen = [&isOpen]() {
        LOGI("Init continuous asr: on open.");
        isOpen = true;
    };
    client.onmessage = [&isMessaged, &result](const std::string &message) {
        LOGI("Init continuous asr: on message:", message, ".");
        // {
        //     "action":"started",
        //     "code":"0",
        //     "data":"",
        //     "desc":"success",
        //     "sid":"rta13b4a589@dx2f5f1b49a270000100"
        // }
        int code = xunfei_speech_server_error::parseErrorCode(message);
        if (code == -1) {
            // 回复消息体不完整
            result.success = false;
            result.error = EngineError(AI_COMMON_NET_ERROR, "Init continuous asr on message error");
        } else if (code == 0) {
            result.success = true;
        } else {
            result.success = false;
            int errorCode = xunfei_speech_server_error::realtimeAsrErrorCode2speechResult(code);
            Json::Value value = xunfei_speech_util::stringToJson(message);
            std::string errorMessage;
            if (value.isMember("desc") && value["desc"].isString()) {
                errorMessage = value["desc"].asString();
                if (stringContains(errorMessage, "ts expired")) {
                    errorCode = AI_COMMON_SSL_CERTIFICATE_ERROR;
                }
            }
            result.error = EngineError(errorCode, errorMessage);
        }

        isMessaged = true;
    };
    client.onclose = []() { LOGI("Init continuous asr: on close."); };
    client.setPingInterval(1000);
    std::string url = continuousAsrBuildUrl();
    if (url.empty()) {
        // url为空，是由于时间服务器连不上，所以获取不到，报网络断开错误
        // SslCertificateError的错误信息包含"请求时间验证失败"
        result.success = false;
        result.error = EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, "Get NTP server error.");
        return result;
    }
    int ret = client.open(url.c_str());
    if (ret != 0) {
        LOGI("Init continuous asr: open failed.");
        result.success = false;
        result.error = EngineError(AI_COMMON_NET_ERROR, "Init continuous asr: open failed.");
        return result;
    }
    client.setSendTimeoutSec(3);
    client.setRecvTimeoutSec(3);
    for (int i = 0; i < 30; ++i) {
        // 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (isOpen && isMessaged) {
            break;
        }
    }
    if (!result.success && result.error.getCode() == -1) {
        // 不成功且错误码为默认值时
        int httpStatus = client.getHttpResponse()->status_code;
        if (httpStatus == 302) {
            result.error = EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, "Redirect.");
        } else {
            result.error = EngineError(AI_COMMON_NET_ERROR, "Init continuous asr: time out.");
        }
    }
    return result;
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::startContinuousRecognition(const std::string &params) {
    std::lock_guard locker(mutex_);
    continuousAsrStarting_ = true;
    continuousAsrError_ = ai_engine::EngineError{AI_COMMON_SUCCESS, ""};
    LOGD("Continuous asr: start.");
    ai_engine::result::AiEngineBaseResult result{false, EngineError{}};
    if (appIdRealTime_.empty() || apiKeyRealTime_.empty()) {
        result.success = false;
        result.error = EngineError{AI_COMMON_MODEL_NOT_FOUND, "Init continuous asr: appId or apiKey is empty."};
        continuousAsrStarting_ = false;
        return result;
    }
    setRecognitionParams(params);
    wssClient_.onopen = [this]() {
        LOGI("Continuous asr: on open.");
        continuousAsrOpened_ = true;
    };
    wssClient_.onmessage = [this](const std::string &message) { handleMessage(message); };
    wssClient_.onclose = []() { LOGI("Continuous asr: on close."); };
    wssClient_.setPingInterval(1000);
    std::string url = continuousAsrBuildUrl();
    LOGD(url);
    if (url.empty()) {
        // url为空，是由于时间服务器连不上，所以获取不到，报网络断开错误
        // SslCertificateError的错误信息包含"请求时间验证失败"
        result.success = false;
        result.error = EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, "Get NTP server error.");
        continuousAsrStarting_ = false;
        return result;
    }
    continuousAsrOpened_ = false;
    int ret = wssClient_.open(url.c_str());
    if (ret != 0) {
        LOGI("Continuous asr: open failed.");
        result.success = false;
        result.error = EngineError(AI_COMMON_NET_ERROR, "Continuous asr: open failed.");
        continuousAsrStarting_ = false;
        return result;
    }

    wssClient_.setSendTimeoutSec(3);
    wssClient_.setRecvTimeoutSec(3);

    // 需求要求5秒
    for (int i = 0; i < 30; ++i) {
        // 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (continuousAsrOpened_) {
            break;
        }
    }

    if (!continuousAsrOpened_) {
        result.success = false;
        result.error = EngineError(AI_COMMON_NET_ERROR, "Init continuous asr: time out.");
        continuousAsrStarting_ = false;
        return result;
    }
    {
        std::lock_guard continuousAsrLocker(continuousAsrMutex_);
        if (continuousAsrError_.getCode() != AI_COMMON_SUCCESS && continuousAsrError_.getCode() != -1) {
            result.success = false;
            result.error = continuousAsrError_;
            continuousAsrStarting_ = false;
            return result;
        }
    }

    result.success = true;
    continuousAsrFinished_ = false;

    ai_engine::core_ai::speech::RecognitionResult recognitionResult{};
    recognitionResult.success = true;
    recognitionResult.error = {};
    recognitionResult.modelName = currentDisplayName_;
    recognitionResult.text = "";
    recognitionResult.speakerId = 0;
    recognitionResult.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognitionStarted;
    if (recognitionCallback_) {
        recognitionCallback_(recognitionResult);
    }
    continuousAsrStarting_ = false;
    return result;
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::writeContinuousRecognitionAudioData(
    const std::vector<uint8_t> &audioData) {
    std::lock_guard locker(mutex_);
    ai_engine::result::AiEngineBaseResult result{false, EngineError{}};
    if (continuousAsrFinished_) {
        // 主动关闭
        result.success = true;
        return result;
    }
    if (!continuousAsrOpened_) {
        result.success = false;
        result.error = EngineError(AI_COMMON_NET_ERROR, "Continuous asr: wss client is not inited.");
        return result;
    }

    if (audioData.empty()) {
        result.success = false;
        result.error =
            EngineError(AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID, "Continuous asr: audio data is empty.");
        return result;
    }

    int sent = wssClient_.send(reinterpret_cast<const char *>(audioData.data()), int(audioData.size()),
                               ws_opcode::WS_OPCODE_BINARY);
    if (sent < 0) {
        LOGI("Continuous asr: send failed.");
        result.success = false;
        result.error = EngineError(AI_COMMON_NET_ERROR, "Continuous asr: send failed.");
        return result;
    }
    result.success = true;
    return result;
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::stopContinuousRecognition() {
    std::lock_guard locker(mutex_);
    wssClient_.send(R"({"end": true})");
    continuousAsrFinished_ = true;
    return {true, EngineError{}};
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::destroyContinuousRecognitionModule() {
    wssClient_.close();
    return {true, EngineError{}};
}

// 开始 - 一次语音识别私有函数
bool XunfeiSpeechEnginePrivate::onceAsrParamsCheck(const std::vector<uint8_t> &audioData, EngineError &error) {
    if (!recognitionCallback_) {
        error = EngineError(AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID, "Once Asr: callback is nullptr.");
        return false;
    }
    if (audioData.empty()) {
        // 此处不用调用回调函数通知
        error = EngineError(AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID, "Once Asr: the audio data is empty.");
        return false;
    }
    return true;
}

std::string XunfeiSpeechEnginePrivate::onceAsrBuildData(const std::string &params,
                                                        const std::vector<uint8_t> &audioData) {
    setRecognitionParams(params);
    Json::Value bodyJson;
    bodyJson["common"]["app_id"] = appIdContinuous_;
    bodyJson["business"]["language"] = "zh_cn";
    bodyJson["business"]["domain"] = "iat";
    bodyJson["business"]["accent"] = "mandarin";
    bodyJson["data"]["status"] = 2;
    bodyJson["data"]["format"] = "audio/L16;rate=" + std::to_string(rate_);
    bodyJson["data"]["encoding"] = "raw";
    bodyJson["data"]["audio"] = xunfei_speech_util::base64Encode(std::string(audioData.begin(), audioData.end()));
    return bodyJson.toStyledString();
}

std::string XunfeiSpeechEnginePrivate::onceAsrBuildUrl() {
    auto urlData = xunfei_speech_util::parseURL(asrHostUrl_);

    std::string date = xunfei_speech_util::getGmtTime();

    std::string originSignature;
    originSignature += "host: " + urlData["Host"] + "\n";
    originSignature += "date: " + date + "\n";
    originSignature += "GET " + urlData["Path"] + " HTTP/1.1";

    std::string shaSignature = xunfei_speech_util::hmacSha256Encode(originSignature, secretKeyContinuous_);

    std::string authorization;
    authorization += R"(hmac username=")" + apiKeyContinuous_ + R"(", )";
    authorization += R"(algorithm="hmac-sha256", )";
    authorization += R"(headers="host date request-line", )";
    authorization += R"(signature=")" + shaSignature + R"(")";
    authorization = xunfei_speech_util::base64Encode(authorization);

    std::string url;
    url += std::string(asrHostUrl_);
    url += "?";
    url += "authorization=" + xunfei_speech_util::urlEncode(authorization);
    url += "&";
    url += "date=" + xunfei_speech_util::urlEncode(date);
    url += "&";
    url += "host=" + urlData["Host"];
    return url;
}

bool XunfeiSpeechEnginePrivate::onceAsrOnMessageCheck(const std::string &message) {
    if (int errorCode = xunfei_speech_server_error::parseErrorCode(message)) {
        LOGE("Once asr: xunfei recognize failed", message);
        int theErrorCode = xunfei_speech_server_error::asrErrorCode2speechResult(errorCode);
        EngineError error(theErrorCode, message);
        ai_engine::core_ai::speech::RecognitionResult result{
            false, error, currentDisplayName_,
            {},    0,     ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred};
        if (recognitionCallback_) {
            recognitionCallback_(result);
        }
        return false;
    }
    return true;
}
void XunfeiSpeechEnginePrivate::onceAsrOnMessage(const std::string &message, std::atomic_bool &isFinish) {
    if (!onceAsrOnMessageCheck(message)) {
        return;
    }

    Json::Value recognitionJson = xunfei_speech_util::formatJsonFromString(message);

    EngineError error{};
    ai_engine::core_ai::speech::RecognitionResult result;
    result.success = true;
    result.error = error;
    result.modelName = currentDisplayName_;
    result.speakerId = 0;
    for (int i = 0; i < recognitionJson["data"]["result"]["ws"].size(); i++) {
        for (int j = 0; j < recognitionJson["data"]["result"]["ws"][i]["cw"].size(); j++) {
            result.text += recognitionJson["data"]["result"]["ws"][i]["cw"][j]["w"].asString();
        }
    }
    result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognized;
    if (recognitionCallback_) {
        recognitionCallback_(result);
    }
    result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognitionCompleted;
    if (recognitionCallback_) {
        recognitionCallback_(result);
    }
    isFinish = true;
}

// 结束 - 一次语音识别私有函数

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::initRecognizeOnceModule() {
    ai_engine::result::AiEngineBaseResult result{false, EngineError{}};
    if (appIdContinuous_.empty() || apiKeyContinuous_.empty() || secretKeyContinuous_.empty()) {
        result.success = false;
        result.error = EngineError{AI_COMMON_MODEL_NOT_FOUND, "Init once asr: appId, apiKey or secretKey is empty."};
        return result;
    }
    std::atomic_bool isOpen{false};
    std::atomic_bool isClose{false};
    XunfeiWebSocketClient client{};
    client.onopen = [&isOpen]() {
        LOGI("Init once asr: on open.");
        isOpen = true;
    };
    client.onmessage = [](const std::string &message) { LOGI("Init once asr: on message:", message, "."); };
    client.onclose = [&isClose]() {
        LOGI("Init once asr: on close.");
        isClose = true;
    };
    client.setPingInterval(1000);
    std::string url = onceAsrBuildUrl();
    int ret = client.open(url.c_str());
    if (ret != 0) {
        LOGI("Init once asr: open failed.");
        result.success = false;
        result.error = EngineError(AI_COMMON_NET_ERROR, "Init once asr: open failed.");
        return result;
    }
    client.setSendTimeoutSec(3);
    client.setRecvTimeoutSec(3);
    // 最多等待100次100毫秒（10秒），之后，如果还没有打开，则认为失败。
    for (int i = 0; i < 30; ++i) {
        // 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (isOpen || isClose) {
            auto response = client.getHttpResponse();
            if (response == nullptr) {
                LOGE("Init once asr: http response is null.");
                result.success = false;
                result.error = EngineError(AI_COMMON_NET_ERROR, "Init once asr: http response is null.");
                break;
            }
            LOGI(response->status_code, response->status_message());
            // 401 缺少authorization参数
            //     {"message": "Unauthorized"}
            // 401 签名参数解析失败
            //     {"message": "HMAC signature cannot be verified"}
            // 401 签名校验失败
            //     {"message": "HMAC signature does not match"}
            // 403 时钟偏移校验失败
            //     {"message": "HMAC signature cannot be verified, a valid
            //     date or x-date header is required for HMAC Authentication"}
            // 403 IP白名单校验失败
            //     {"message" : "Your IP address is not allowed"}
            if (response->status_code == 101) {
                // 说明服务端接收到请求，并把协议切换成为websocket协议
                result.success = true;
            } else if (response->status_code == 302) {
                // TODO 后续可以完善跳转
                result.success = false;
                result.error = EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, "Init once tts: SslCertificateError.");
            } else if (response->status_code == 400) {
                result.success = false;
                result.error = EngineError(AI_COMMON_NET_ERROR, "Init once asr: NetworkDisconnected.");
            } else if (response->status_code == 403) {
                result.success = false;
                result.error = EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, "Init once asr: SslCertificateError.");
            } else {
                // TODO 先报鉴权错误，后续再进行区分
                result.success = false;
                result.error = EngineError(AI_COMMON_AUTHENTICATION_FAILURE, "Init once asr: unauthorized.");
            }
            break;
        }
    }
    if (!result.success && result.error.getCode() == -1) {
        // 不成功且错误码为默认值时
        result.error = EngineError(AI_COMMON_NET_ERROR, "Init once asr: time out.");
    }
    return result;
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::recognizeOnce(const std::string &params,
                                                                               const std::vector<uint8_t> &audioData) {
    if (appIdContinuous_.empty() || apiKeyContinuous_.empty() || secretKeyContinuous_.empty()) {
        ai_engine::result::AiEngineBaseResult result{};
        result.success = false;
        result.error = EngineError{AI_COMMON_MODEL_NOT_FOUND, "Init once asr: appId, apiKey or secretKey is empty."};
        return result;
    }
    EngineError engineError{};
    if (!onceAsrParamsCheck(audioData, engineError)) {
        return {false, engineError};
    }
    std::string data = onceAsrBuildData(params, audioData);
    std::string url = onceAsrBuildUrl();

    // 开始 - libhv websocket client准备
    std::atomic_bool isOpen{false};
    onceAsrFinished_ = false;
    XunfeiWebSocketClient client{};
    client.onopen = [&isOpen]() {
        LOGI("Once asr: on open.");
        isOpen = true;
    };
    client.onmessage = [this](const std::string &message) {
        LOGI("Once asr: on message:", message);
        onceAsrOnMessage(message, onceAsrFinished_);
    };
    client.onclose = [&isOpen, this]() {
        LOGI("Once asr: on close.");
        isOpen = true;
        onceAsrFinished_ = true;
    };
    client.setPingInterval(1000);
    // 结束 - libhv websocket client准备

    // 开始 - 发送代码
    int ret = client.open(url.c_str());
    if (ret != 0) {
        engineError = EngineError(AI_COMMON_NET_ERROR, "Once asr: open failed.");
        return {false, engineError};
    }
    client.setSendTimeoutSec(3);
    client.setRecvTimeoutSec(3);

    // 最多等待100次100毫秒（10秒），之后，如果还没有打开，则认为失败。
    for (int i = 0; i < 30; ++i) {
        // 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (isOpen) {
            break;
        }
    }

    ai_engine::core_ai::speech::RecognitionResult result;
    result.success = true;
    result.error = EngineError{};
    result.modelName = currentDisplayName_;
    result.text = std::string{};
    result.speakerId = 0;
    result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechRecognizing;
    if (recognitionCallback_) {
        recognitionCallback_(result);
    }

    ret = client.send(data.data(), data.size());
    if (ret == -1) {
        engineError = EngineError(AI_COMMON_NET_ERROR, "Once asr: send failed.");
        return {false, engineError};
    }
    while (!onceAsrFinished_) {
        usleep(100 * 1000);
    }
    client.close();
    return {true, EngineError{}};
    // 结束 - 发送代码
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::destroyRecognizeOnceModule() {
    onceAsrFinished_ = true;
    return {true, EngineError{}};
}

// 开始 - 一次语音合成私有函数
bool XunfeiSpeechEnginePrivate::onceTtsParamsCheck(const std::string &text, EngineError &engineError) {
    if (!synthesisCallback_) {
        engineError = EngineError(AI_COMMON_SPEECH_SYNTHESIS_TEXT_LENGTH_INVALID, "Once tts: callback is nullptr.");
        return false;
    }
    if (text.empty()) {
        // 此处不用调用回调函数通知
        engineError = EngineError(AI_COMMON_SPEECH_SYNTHESIS_TEXT_LENGTH_INVALID, "Once tts: text is empty.");
        return false;
    }
    return true;
}

std::string XunfeiSpeechEnginePrivate::onceTtsBuildData(const std::string &params, const std::string &text) {
    setSynthesisParams(params);
    Json::Value bodyJson;
    bodyJson["common"]["app_id"] = appIdContinuous_;
    bodyJson["business"]["aue"] = "raw";
    bodyJson["business"]["sfl"] = 1;
    bodyJson["business"]["vcn"] = "xiaoyan";
    bodyJson["business"]["speed"] = speed_;
    bodyJson["business"]["volume"] = volume_;
    bodyJson["business"]["pitch"] = pitch_;
    bodyJson["business"]["tte"] = "UTF8";
    bodyJson["data"]["status"] = 2;
    bodyJson["data"]["text"] = xunfei_speech_util::base64Encode(text);
    return bodyJson.toStyledString();
}

std::string XunfeiSpeechEnginePrivate::onceTtsBuildUrl() {
    auto urlData = xunfei_speech_util::parseURL(ttsHostUrl_);

    std::string date = xunfei_speech_util::getGmtTime();

    std::string originSignature;
    originSignature += "host: " + urlData["Host"] + "\n";
    originSignature += "date: " + date + "\n";
    originSignature += "GET " + urlData["Path"] + " HTTP/1.1";

    std::string shaSignature = xunfei_speech_util::hmacSha256Encode(originSignature, secretKeyContinuous_);

    std::string authorization;
    authorization += R"(hmac username=")" + apiKeyContinuous_ + R"(", )";
    authorization += R"(algorithm="hmac-sha256", )";
    authorization += R"(headers="host date request-line", )";
    authorization += R"(signature=")" + shaSignature + R"(")";
    authorization = xunfei_speech_util::base64Encode(authorization);

    std::string url;
    url += std::string(ttsHostUrl_);
    url += "?";
    url += "authorization=" + xunfei_speech_util::urlEncode(authorization);
    url += "&";
    url += "date=" + xunfei_speech_util::urlEncode(date);
    url += "&";
    url += "host=" + urlData["Host"];

    return url;
}

bool XunfeiSpeechEnginePrivate::onceTtsOnMessageCheck(const std::string &message) {
    if (int errorCode = xunfei_speech_server_error::parseErrorCode(message)) {
        LOGE("Xunfei synthesize failed", message, ".");
        int theErrorCode = xunfei_speech_server_error::ttsErrorCode2speechResult(errorCode);
        EngineError error(theErrorCode, message);
        ai_engine::core_ai::speech::SynthesisResult result{
            false,
            error,
            currentDisplayName_,
            {},
            {},
            {},
            {},
            ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred};
        if (theErrorCode == AI_COMMON_AUTHENTICATION_FAILURE) {
            std::lock_guard locker(onceTtsMutex_);
            onceTtsError_ = error;
        } else {
            if (synthesisCallback_) {
                synthesisCallback_(result);
            }
        }
        return false;
    }
    return true;
}

void XunfeiSpeechEnginePrivate::onceTtsOnMessage(const std::string &message, std::atomic_bool &isFinish) {
    if (!onceTtsOnMessageCheck(message)) {
        return;
    }

    Json::Value synthesisJson = xunfei_speech_util::formatJsonFromString(message);

    ai_engine::core_ai::speech::SynthesisResult result;
    result.success = true;
    result.error = EngineError{};
    result.modelName = currentDisplayName_;
    result.audioFormat = "pcm";
    result.audioRate = 16000;
    result.audioChannel = 1;
    result.base64AudioData = synthesisJson["data"]["audio"].asCString();
    result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesizing;

    if (synthesisJson["data"]["status"].asInt() == 2) {
        result.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesisCompleted;
        isFinish = true;
    }
    if (synthesisCallback_) {
        synthesisCallback_(result);
    }
}
// 结束 - 一次语音合成私有函数

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::initSynthesizeModule() {
    ai_engine::result::AiEngineBaseResult result{false, EngineError{}};
    if (appIdContinuous_.empty() || apiKeyContinuous_.empty() || secretKeyContinuous_.empty()) {
        result.success = false;
        result.error = EngineError{AI_COMMON_MODEL_NOT_FOUND, "Init once asr: appId, apiKey or secretKey is empty."};
        return result;
    }
    std::atomic_bool isOpen{false};
    std::atomic_bool isClose{false};
    XunfeiWebSocketClient client{};
    client.onopen = [&isOpen]() {
        LOGI("Init once tts: on open.");
        isOpen = true;
    };
    client.onmessage = [](const std::string &message) { LOGI("Init once tts: on message:", message, "."); };
    client.onclose = [&isClose]() {
        LOGI("Init once tts: on close.");
        isClose = true;
    };
    client.setPingInterval(1000);
    std::string url = onceTtsBuildUrl();
    int ret = client.open(url.c_str());
    if (ret != 0) {
        LOGI("Init once tts: open failed.");
        result.success = false;
        result.error = EngineError(AI_COMMON_NET_ERROR, "Init once tts: open failed.");
        return result;
    }
    client.setSendTimeoutSec(3);
    client.setRecvTimeoutSec(3);
    // 最多等待100次100毫秒（10秒），之后，如果还没有打开，则认为失败。
    for (int i = 0; i < 30; ++i) {
        // 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (isOpen || isClose) {
            auto response = client.getHttpResponse();
            if (response == nullptr) {
                LOGE("Init once tts: http response is null.");
                result.success = false;
                result.error = EngineError(AI_COMMON_NET_ERROR, "Init once tts: http response is null.");
                break;
            }
            LOGI(response->status_code, response->status_message());
            // 401 缺少authorization参数
            //     {"message": "Unauthorized"}
            // 401 签名参数解析失败
            //     {"message": "HMAC signature cannot be verified"}
            // 401 签名校验失败
            //     {"message": "HMAC signature does not match"}
            // 403 时钟偏移校验失败
            //     {"message": "HMAC signature cannot be verified, a valid
            //     date or x-date header is required for HMAC Authentication"}
            // 403 IP白名单校验失败
            //     {"message" : "Your IP address is not allowed"}
            if (response->status_code == 101) {
                // 说明服务端接收到请求，并把协议切换成为websocket协议
                result.success = true;
            } else if (response->status_code == 302) {
                // TODO 后续可以完善跳转
                result.success = false;
                result.error = EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, "Init once tts: SslCertificateError.");
            } else if (response->status_code == 400) {
                // TODO 临时映射到SslCertificateError错误，后续完善
                result.success = false;
                result.error = EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, "Init once tts: SslCertificateError.");
            } else if (response->status_code == 403) {
                result.success = false;
                result.error = EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, "Init once tts: SslCertificateError.");
            } else {
                // TODO 先报鉴权错误，后续再进行区分
                result.success = false;
                result.error = EngineError(AI_COMMON_AUTHENTICATION_FAILURE, "Init once tts: unauthorized");
            }
            break;
        }
    }
    if (!result.success && result.error.getCode() == -1) {
        // 不成功且错误码为默认值时
        result.error = EngineError(AI_COMMON_NET_ERROR, "Init once tts: time out.");
    }
    return result;
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::synthesizeText(const std::string &params,
                                                                                const std::string &text) {
    if (appIdContinuous_.empty() || apiKeyContinuous_.empty() || secretKeyContinuous_.empty()) {
        ai_engine::result::AiEngineBaseResult result{};
        result.success = false;
        result.error = EngineError{AI_COMMON_MODEL_NOT_FOUND, "Init once asr: appId, apiKey or secretKey is empty."};
        return result;
    }
    EngineError engineError{};
    if (!onceTtsParamsCheck(text, engineError)) {
        return {false, engineError};
    }

    std::string data = onceTtsBuildData(params, text);
    std::string url = onceTtsBuildUrl();

    // 开始 - libhv websocket client准备
    std::atomic_bool isOpen{false};
    std::atomic_bool isClose{false};
    onceTtsFinished_ = false;
    XunfeiWebSocketClient client{};
    client.onopen = [&isOpen]() {
        LOGI("Once tts: on open.");
        isOpen = true;
    };
    client.onmessage = [this](const std::string &message) {
        // LOGI("Once tts: on message:", message, ".");
        onceTtsOnMessage(message, onceTtsFinished_);
    };
    client.onclose = [&isClose, this]() {
        LOGI("Once tts: on close.");
        isClose = true;
        onceTtsClosed_ = true;
    };
    client.setPingInterval(5000);
    // 结束 - libhv websocket client准备
    ai_engine::core_ai::speech::SynthesisResult result{
        true,
        EngineError{},
        currentDisplayName_,
        {},
        {},
        {},
        {},
        ai_engine::core_ai::speech::SpeechResultReason::SpeechSynthesisStarted};
    if (synthesisCallback_) {
        synthesisCallback_(result);
    }
    // 开始 - 发送代码
    onceTtsClosed_ = false;
    int ret = client.open(url.c_str());
    LOGD("Once tts: open status:", ret);
    if (ret != 0) {
        engineError = EngineError(AI_COMMON_NET_ERROR, "Once tts: open failed.");
        return {false, engineError};
    }
    client.setSendTimeoutSec(10);
    client.setRecvTimeoutSec(10);
    // 等待打开后再发送数据
    for (int i = 0; i < 100; ++i) {
        // 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (isOpen || isClose) {
            break;
        }
    }
    // 如果关闭了，说明打开失败
    if (isClose) {
        HttpResponsePtr response = client.getHttpResponse();
        if (response == nullptr) {
            LOGE("Once tts: http response is null.");
        } else {
            LOGD(response->status_code, response->body);
        }
        if (response != nullptr && response->status_code == 400) {
            engineError = EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, "Once tts: open failed.");
            return {false, engineError};
        }
    }

    ret = client.send(data);
    if (ret == -1) {
        engineError = EngineError(AI_COMMON_NET_ERROR, "Once tts: send failed.");
        return {false, engineError};
    }
    while (!onceTtsFinished_) {
        // 此时 close 说明报错了
        if (onceTtsClosed_) {
            {
                std::lock_guard locker(onceTtsMutex_);
                engineError = onceTtsError_;
                onceTtsError_ = EngineError{AI_COMMON_SUCCESS, ""};  // 清空错误
            }
            if (engineError.getCode() == AI_COMMON_AUTHENTICATION_FAILURE) {
                return {false, engineError};
            }
            if (synthesisCallback_) {
                ai_engine::core_ai::speech::SynthesisResult synthesisResult{};
                synthesisResult.success = false;
                synthesisResult.error = EngineError{AI_COMMON_NET_ERROR, "Once tts: failed."};  // 错误码后续优化
                synthesisResult.modelName = currentDisplayName_;
                synthesisResult.audioFormat = std::string{};
                synthesisResult.audioRate = {};
                synthesisResult.audioChannel = {};
                synthesisResult.base64AudioData = std::string{};
                synthesisResult.modelName = std::string{};
                synthesisResult.reason = ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred;
                synthesisCallback_(synthesisResult);
            }
            break;
        }
        usleep(100 * 1000);
    }
    client.close();

    return {true, EngineError{}};
    // 结束 - 发送代码
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEnginePrivate::destroySynthesizeModule() {
    onceTtsFinished_ = true;
    return {true, EngineError{}};
}

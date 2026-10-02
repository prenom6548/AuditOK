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

#include "coreaispeechserviceproxy.h"

#include <string>
#include <thread>
#include <vector>

#include "_speechmodelconfig.h"
#include "_speechrecognitionresult.h"
#include "_speechsynthesisresult.h"
#include "coreaispeechserver.h"
#include "logger.h"
#include "protocolversion.h"
#include "util.h"

namespace kyai {
namespace core {
namespace speech {

const char *objectPath = "/com/kylin/AiRuntime/CoreSpeech";
const char *interface = "com.kylin.AiRuntime.CoreSpeech";

const char *recognitionSpeechResultSignal = "RecognitionSpeechResult";

const char *synthesisResultSignal = "SynthesisResult";

bool runtimeSupportsRequestId() {
    ProtocolVersion version = getRuntimeProtocolVersion();
    return version.major > 1 || (version.major == 1 && version.minor >= 3);
}

_SpeechRecognitionResult dissectRecognitionResult(GVariant *parameters) {
    _SpeechRecognitionResult result{};

    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);
    GVariant *reason = g_variant_iter_next_value(&iter);
    if (reason != nullptr) {
        int *pReason = (int *)g_variant_get_data(reason);
        if (pReason != nullptr) {
            result.reason = SpeechResultReason(*pReason);
        } else {
            LOGE("recognition result error: result is nullptr!");
        }
        g_variant_unref(reason);
    } else {
        LOGE("recognition result error: result is nullptr!");
    }

    GVariant *text = g_variant_iter_next_value(&iter);
    if (text != nullptr) {
        char *pText = (char *)g_variant_get_data(text);
        if (pText != nullptr) {
            result.text = pText;
        } else {
            LOGE("recognition result error: result is nullptr!");
        }
        g_variant_unref(text);
    } else {
        LOGE("recognition result error: result is nullptr!");
    }

    GVariant *speakerId = g_variant_iter_next_value(&iter);
    if (speakerId != nullptr) {
        int *pSpeakerId = (int *)g_variant_get_data(speakerId);
        if (pSpeakerId != nullptr) {
            result.speakerId = *pSpeakerId;
        } else {
            LOGE("recognition result error: result is nullptr!");
        }
        g_variant_unref(speakerId);
    } else {
        LOGE("recognition result error: result is nullptr!");
    }

    GVariant *errorCode = g_variant_iter_next_value(&iter);
    if (errorCode != nullptr) {
        int *pErrorCode = (int *)g_variant_get_data(errorCode);
        if (pErrorCode != nullptr) {
            result.errorCode = *pErrorCode;
        } else {
            LOGE("recognition result error: result is nullptr!");
        }
        g_variant_unref(errorCode);
    } else {
        LOGE("recognition result error: result is nullptr!");
    }

    GVariant *errorMessage = g_variant_iter_next_value(&iter);
    if (errorMessage != nullptr) {
        char *pErrorMessage = (char *)g_variant_get_data(errorMessage);
        if (pErrorMessage != nullptr) {
            result.errorMessage = pErrorMessage;
        } else {
            LOGE("recognition result error: result is nullptr!");
        }
        g_variant_unref(errorMessage);
    } else {
        LOGE("recognition result error: result is nullptr!");
    }

    bool hasRequestId = runtimeSupportsRequestId();
    if (hasRequestId) {
        GVariant *requestIdVariant = g_variant_iter_next_value(&iter);
        if (requestIdVariant != nullptr) {
            char *requestId = (char *)g_variant_get_data(requestIdVariant);
            if (requestId != nullptr) {
                result.requestId = requestId;
            } else {
                LOGE("recognition result error: result is nullptr!");
            }
            g_variant_unref(requestIdVariant);
        } else {
            LOGE("recognition result error: result is nullptr!");
        }
    }
    return result;
}

_SpeechSynthesisResult dissectSynthesisResult(GVariant *parameters) {
    _SpeechSynthesisResult result{};

    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    // 获取字节数组
    GVariant *audioData = g_variant_iter_next_value(&iter);
    if (audioData != nullptr) {
        gchar *pAudioData = (gchar *)g_variant_get_data(audioData);
        if (pAudioData != nullptr) {
            std::string encode = pAudioData;
            result.audioData = util::base64Decode(encode);
        } else {
            LOGE("synthesis result error: audioData is nullptr!");
        }
        g_variant_unref(audioData);
    } else {
        LOGE("synthesis result error: audioData is nullptr!");
    }

    GVariant *reason = g_variant_iter_next_value(&iter);
    if (reason != nullptr) {
        int *pReason = (int *)g_variant_get_data(reason);
        if (pReason != nullptr) {
            result.reason = SpeechResultReason(*pReason);
        } else {
            LOGE("synthesis result error: reason is nullptr!");
        }
        g_variant_unref(reason);
    } else {
        LOGE("synthesis result error: reason is nullptr!");
    }

    GVariant *audioFormat = g_variant_iter_next_value(&iter);
    if (audioFormat != nullptr) {
        char *pAudioFormat = (char *)g_variant_get_data(audioFormat);
        if (pAudioFormat != nullptr) {
            result.audioFormat = pAudioFormat;
        } else {
            LOGE("synthesis result error: audioFormat is nullptr!");
        }
        g_variant_unref(audioFormat);
    } else {
        LOGE("synthesis result error: audioFormat is nullptr!");
    }

    GVariant *audioRate = g_variant_iter_next_value(&iter);
    if (audioRate != nullptr) {
        int *pAudioRate = (int *)g_variant_get_data(audioRate);
        if (pAudioRate != nullptr) {
            result.audioRate = *pAudioRate;
        } else {
            LOGE("synthesis result error: audioRate is nullptr!");
        }
        g_variant_unref(audioRate);
    } else {
        LOGE("synthesis result error: audioRate is nullptr!");
    }

    GVariant *audioChannel = g_variant_iter_next_value(&iter);
    if (audioChannel != nullptr) {
        int *pAudioChannel = (int *)g_variant_get_data(audioChannel);
        if (pAudioChannel != nullptr) {
            result.audioChannel = *pAudioChannel;
        } else {
            LOGE("synthesis result error: audioChannel is nullptr!");
        }
        g_variant_unref(audioChannel);
    } else {
        LOGE("synthesis result error: audioChannel is nullptr!");
    }

    GVariant *errorCode = g_variant_iter_next_value(&iter);
    if (errorCode != nullptr) {
        int *pErrorCode = (int *)g_variant_get_data(errorCode);
        if (pErrorCode != nullptr) {
            result.errorCode = *pErrorCode;
        } else {
            LOGE("synthesis result error: errorCode is nullptr!");
        }
        g_variant_unref(errorCode);
    } else {
        LOGE("synthesis result error: errorCode is nullptr!");
    }

    GVariant *errorMessage = g_variant_iter_next_value(&iter);
    if (errorMessage != nullptr) {
        char *pErrorMessage = (char *)g_variant_get_data(errorMessage);
        if (pErrorMessage != nullptr) {
            result.errorMessage = pErrorMessage;
        } else {
            LOGE("synthesis result error: errorMessage is nullptr!");
        }
        g_variant_unref(errorMessage);
    } else {
        LOGE("synthesis result error: errorMessage is nullptr!");
    }

    bool hasRequestId = runtimeSupportsRequestId();
    if (hasRequestId) {
        GVariant *requestIdVariant = g_variant_iter_next_value(&iter);
        if (requestIdVariant != nullptr) {
            char *requestId = (char *)g_variant_get_data(requestIdVariant);
            if (requestId != nullptr) {
                result.requestId = requestId;
            } else {
                LOGE("recognition result error: result is nullptr!");
            }
            g_variant_unref(requestIdVariant);
        } else {
            LOGE("recognition result error: result is nullptr!");
        }
    }

    return result;
}

void CoreAiSpeechServiceProxy::setRecognitionResultCallback(SpeechRecognitionResultCallback callback, void *userData) {
    recognitionCallback_ = callback;
    recognitionUserData_ = userData;
}

void CoreAiSpeechServiceProxy::setSynthesizerCallback(SpeechSynthesisResultCallback callback, void *userData) {
    synthesizerCallback_ = callback;
    synthesizerUserData_ = userData;
}

void CoreAiSpeechServiceProxy::setReconnectedCallback(const ReconnectedCallback &callback) {
    reconnectedCallback_ = callback;
}

CoreAiSpeechServiceProxy::~CoreAiSpeechServiceProxy() {
    if (not CoreAiSpeechServer::getInstance().available()) {
        g_printerr(
            "Error creating core speech server proxy: Server proxy "
            "connection is unavailable.\n");
        return;
    }

    auto *connection = CoreAiSpeechServer::getInstance().getConnection();
    if (recognitionSubscriptionId_ != 0) {
        // 取消订阅
        g_dbus_connection_signal_unsubscribe(connection, recognitionSubscriptionId_);
    }
    if (synthesizerSubscriptionId_ != 0) {
        // 取消订阅
        g_dbus_connection_signal_unsubscribe(connection, synthesizerSubscriptionId_);
    }
    CoreAiSpeechServer::getInstance().setOnConnectionClosed(nullptr);
}

bool CoreAiSpeechServiceProxy::connectToServer() {
    CoreAiSpeechServer::getInstance().init();
    if (!CoreAiSpeechServer::getInstance().available()) {
        g_printerr(
            "Error creating core speech server proxy: Server proxy "
            "connection is unavailable.\n");
        return false;
    }
    if (proxy_ != nullptr) {
        g_object_unref(proxy_);
        proxy_ = nullptr;
    }

    GError *error = nullptr;
    auto *connection = CoreAiSpeechServer::getInstance().getConnection();
    proxy_ = ai_runtime_core_speech_service_proxy_new_sync(connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath,
                                                           nullptr, &error);
    if (proxy_ == nullptr) {
        if (error) {
            g_printerr("Error creating text processor proxy %s: %s\n", objectPath, error->message);
            g_error_free(error);
        }
        return false;
    }
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(proxy_), 60 * 60 * 1000);
    refreshRuntimeProtocolVersion();

    CoreAiSpeechServer::getInstance().setOnConnectionClosed([this] {
        // 流式语音识别报错
        if (continuousRecognitionStarted_ && recognitionCallback_) {
            SpeechRecognitionResult result{};
            result.reason = SPEECH_ERROR_OCCURRED;
            result.text = "";
            result.speakerId = -1;
            result.errorCode = AISDK_RUNTIME_ERROR;
            result.errorMessage = "Server closed unexpectedly.";
            recognitionCallback_(&result, recognitionUserData_);
        }
        continuousRecognitionStarted_ = false;

        if (reconnectServer()) {
            if (reconnectedCallback_) {
                reconnectedCallback_();
            }
        }
    });

    return true;
}

bool CoreAiSpeechServiceProxy::reconnectServer() {
    if (reconnectAttempts_ >= maxReconnectAttempts) {
        LOGE("Max reconnect attempts reached");
        return false;
    }

    reconnectAttempts_++;
    LOGD("Reconnect attempt {}.", reconnectAttempts_);

    if (!connectToServer()) {
        LOGE("Failed to reconnect to server");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return reconnectServer();
    }

    LOGI("Successfully reconnected to server.");

    return true;
}

int CoreAiSpeechServiceProxy::initRecognizer(const char *config, int &sessionId) {
    int errorCode = AISDK_NO_ERROR;
    GError *error = nullptr;

    bool success = ai_runtime_core_speech_service_call_init_recognizer_sync(proxy_, config, &sessionId, &errorCode,
                                                                            nullptr, &error);
    if (!success) {
        if (error != nullptr) {
            LOGE("Error calling init:", error->code, error->message);
            g_error_free(error);
            return AISDK_RUNTIME_ERROR;
        }
    }
    if (sessionId != -1) {
        if (!recognitionResultSignalConnected_) {
            // 只能调用一次，否则回调函数会被调用两次，关闭连接不行
            connectSpeechRecognitionResultSignals(sessionId);
        }
        recognitionResultSignalConnected_ = true;
        reconnectAttempts_ = 0;
    }
    return errorCode;
}

int CoreAiSpeechServiceProxy::initSynthesizer(const char *config, int &sessionId) {
    int errorCode = AISDK_NO_ERROR;
    GError *error = nullptr;
    bool success = ai_runtime_core_speech_service_call_init_synthesizer_sync(proxy_, config, &sessionId, &errorCode,
                                                                             nullptr, &error);
    LOGD(sessionId);
    if (!success) {
        if (error != nullptr) {
            LOGE("Error calling init:", error->code, error->message);
            g_error_free(error);
            return AISDK_RUNTIME_ERROR;
        }
    }
    if (sessionId != -1) {
        if (!synthesisResultSignalConnected_) {
            // 只能调用一次，否则回调函数会被调用两次，关闭连接不行
            connectSpeechSynthesisResultSignals(sessionId);
        }
        synthesisResultSignalConnected_ = true;
        reconnectAttempts_ = 0;
    }
    return errorCode;
}

void CoreAiSpeechServiceProxy::destroyRecognizer(int sessionId) {
    ai_runtime_core_speech_service_call_destroy_recognizer_sync(proxy_, sessionId, nullptr, nullptr);
    reconnectAttempts_ = 0;
}

void CoreAiSpeechServiceProxy::destroySynthesizer(int sessionId) {
    ai_runtime_core_speech_service_call_destroy_synthesizer_sync(proxy_, sessionId, nullptr, nullptr);
    reconnectAttempts_ = 0;
}

void CoreAiSpeechServiceProxy::setRecognizerAudioConfig(int sessionId, const std::string &audioConfig) {
    ai_runtime_core_speech_service_call_set_recognizer_audio_config_sync(proxy_, sessionId, audioConfig.c_str(),
                                                                         nullptr, nullptr);
    LOGD("setRecognizerAudioConfig:");
}

void CoreAiSpeechServiceProxy::startContinuousRecognition(int sessionId) {
    ai_runtime_core_speech_service_call_start_continuous_recognition(proxy_, sessionId, nullptr, nullptr, nullptr);
    LOGD("startContinuousRecognition.");
    continuousRecognitionStarted_ = true;
}

void CoreAiSpeechServiceProxy::setSynthesizerAudioConfig(int sessionId, const std::string &audioConfig) {
    bool ret = ai_runtime_core_speech_service_call_set_synthesizer_audio_config_sync(
        proxy_, sessionId, audioConfig.c_str(), nullptr, nullptr);
    if (!ret) {
        std::fprintf(stderr, "CoreSpeechServiceProxy: error to set synthesizer audio config\n");
    }
}

void CoreAiSpeechServiceProxy::synthesizeTextOnce(int sessionId, const char *text, uint32_t textLength) {
    ai_runtime_core_speech_service_call_synthesize_text_once(proxy_, sessionId, text, textLength, nullptr,
                                                             onOnceSynthesisFinishedCallback, this);
}

int CoreAiSpeechServiceProxy::synthesizeTextOnceWithRequestId(int sessionId, const char *requestId, const char *text,
                                                              uint32_t textLength) {
    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("synthesize_text_once_with_request_id", &errorCode)) {
        return errorCode;
    }
    ai_runtime_core_speech_service_call_synthesize_text_once_with_request_id(
        proxy_, sessionId, requestId, text, textLength, nullptr, onOnceSynthesisFinishedCallback, this);
    return AI_COMMON_SUCCESS;
}

void CoreAiSpeechServiceProxy::synthesizeTextContinuous(int sessionId, const char *text, uint32_t textLength) {
    ai_runtime_core_speech_service_call_synthesize_text_continuous(proxy_, sessionId, text, textLength, nullptr,
                                                                   nullptr, nullptr);
}

int CoreAiSpeechServiceProxy::stopSpeaking(int sessionId) {
    int errorCode;
    GError *error = nullptr;
    bool ret = ai_runtime_core_speech_service_call_stop_speaking_sync(proxy_, sessionId, &errorCode, nullptr, &error);
    if (!ret) {
        LOGE("Error to stop speaking.", error->message);
        return AiSdkCommonErrorCode::AISDK_RUNTIME_ERROR;
    }
    return (int)errorCode;
}

void CoreAiSpeechServiceProxy::startInternalEventLoop() {
    if (eventLoopThread_.joinable()) {
        return;
    }

    eventLoopContext_ = g_main_context_default();
    if (eventLoopContext_ == nullptr) {
        return;
    }

    g_main_context_ref(eventLoopContext_);
    try {
        eventLoopThread_ = std::thread([this]() {
            while (internalLoopRequested_.load(std::memory_order_acquire)) {
                g_main_context_iteration(eventLoopContext_, TRUE);
            }
        });
    } catch (const std::system_error &error) {
        LOGE("Failed to start internal event loop thread: {}", error.what());
        g_main_context_unref(eventLoopContext_);
        eventLoopContext_ = nullptr;
        internalLoopRequested_.store(false, std::memory_order_release);
    }
}

void CoreAiSpeechServiceProxy::stopInternalEventLoop() {
    if (eventLoopContext_ != nullptr) {
        g_main_context_wakeup(eventLoopContext_);
    }

    if (eventLoopThread_.joinable()) {
        eventLoopThread_.join();
    }

    if (eventLoopContext_ != nullptr) {
        g_main_context_unref(eventLoopContext_);
        eventLoopContext_ = nullptr;
    }
}

void CoreAiSpeechServiceProxy::enableInternalEventLoop(bool enable) {
    if (enable) {
        bool expected = false;
        if (!internalLoopRequested_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
            return;
        }
        startInternalEventLoop();
        if (eventLoopContext_ == nullptr) {
            internalLoopRequested_.store(false, std::memory_order_release);
        }
    } else {
        bool expected = true;
        if (!internalLoopRequested_.compare_exchange_strong(expected, false, std::memory_order_acq_rel)) {
            return;
        }
        stopInternalEventLoop();
    }
}

void CoreAiSpeechServiceProxy::stopContinuousRecognition(int sessionId) {
    ai_runtime_core_speech_service_call_stop_continuous_recognition(proxy_, sessionId, nullptr, nullptr, nullptr);
    LOGI("stopContinuousRecognition.");
    continuousRecognitionStarted_ = false;
}

void CoreAiSpeechServiceProxy::continuousRecognitionWriteAudioData(int sessionId, const uint8_t *data,
                                                                   uint32_t length) {
    const char *ptr = reinterpret_cast<const char *>(data);
    std::string audioData(ptr, length);
    std::string encode = util::base64Encode(audioData);
    ai_runtime_core_speech_service_call_continuous_recognition_write_audio_data(proxy_, sessionId, encode.c_str(),
                                                                                nullptr, nullptr, nullptr);
}

void CoreAiSpeechServiceProxy::recognizeOnce(int sessionId) {
    ai_runtime_core_speech_service_call_recognize_once(proxy_, sessionId, nullptr, onOnceRecognitionFinishedCallback,
                                                       this);
    LOGD("recognizeOnce.");
}

int CoreAiSpeechServiceProxy::recognizeOnceWithRequestId(int sessionId, const char *requestId) {
    int errorCode = AI_COMMON_SUCCESS;
    if (!ensureProtocolSupportedForApi("recognize_once_with_request_id", &errorCode)) {
        return errorCode;
    }
    ai_runtime_core_speech_service_call_recognize_once_with_request_id(proxy_, sessionId, requestId, nullptr,
                                                                       onOnceRecognitionFinishedCallback, this);
    return AI_COMMON_SUCCESS;
}

void CoreAiSpeechServiceProxy::connectSpeechRecognitionResultSignals(int sessionId) {
    auto *connection = CoreAiSpeechServer::getInstance().getConnection();

    std::string interfaceName = interface + std::to_string(sessionId);

    recognitionSubscriptionId_ = g_dbus_connection_signal_subscribe(
        connection, nullptr, interfaceName.c_str(), recognitionSpeechResultSignal, objectPath, nullptr,
        G_DBUS_SIGNAL_FLAGS_NONE, onRecognitionResultCallback, this, nullptr);
}

void CoreAiSpeechServiceProxy::connectSpeechSynthesisResultSignals(int sessionId) {
    auto *connection = CoreAiSpeechServer::getInstance().getConnection();
    std::string interfaceName = interface + std::to_string(sessionId);

    synthesizerSubscriptionId_ = g_dbus_connection_signal_subscribe(
        connection, nullptr, interfaceName.c_str(), synthesisResultSignal, objectPath, nullptr,
        G_DBUS_SIGNAL_FLAGS_NONE, onSynthesisResult, this, nullptr);
}

void CoreAiSpeechServiceProxy::onRecognitionResultCallback(GDBusConnection *connection, const gchar *senderName,
                                                           const gchar *objectPath, const gchar *interfaceName,
                                                           const gchar *signalName, GVariant *parameters,
                                                           gpointer userData) {
    auto *proxy = static_cast<CoreAiSpeechServiceProxy *>(userData);
    if (proxy->recognitionCallback_ != nullptr) {
        _SpeechRecognitionResult result = dissectRecognitionResult(parameters);
        proxy->recognitionCallback_(&result, proxy->recognitionUserData_);
    } else {
        LOGE("Recognizing callback is not set!");
    }
}

void CoreAiSpeechServiceProxy::onSynthesisResult(GDBusConnection *connection, const gchar *senderName,
                                                 const gchar *objectPath, const gchar *interfaceName,
                                                 const gchar *signalName, GVariant *parameters, gpointer userData) {
    auto *proxy = static_cast<CoreAiSpeechServiceProxy *>(userData);
    if (!proxy->synthesizerCallback_) {
        std::fprintf(stderr, "CoreSpeechServiceProxy: Synthesizer callback is not set!\n");
        return;
    }

    auto result = dissectSynthesisResult(parameters);
    proxy->synthesizerCallback_(&result, proxy->synthesizerUserData_);
}

void CoreAiSpeechServiceProxy::onOnceRecognitionFinishedCallback(GObject *sourceObject, GAsyncResult *res,
                                                                 gpointer userData) {
    auto *proxy = static_cast<CoreAiSpeechServiceProxy *>(userData);
    GError *error{nullptr};
    bool result = ai_runtime_core_speech_service_call_recognize_once_finish((AiRuntimeCoreSpeechService *)sourceObject,
                                                                            res, &error);
    if (!result) {
        if (error == nullptr) {
            g_printerr("Error calling once asr: Unknown error\n");
        } else {
            g_printerr("Error calling once asr: %s\n", error->message);
            proxy->handleOnceRecognitionError(error->code);
            g_error_free(error);
        }
    }
}

void CoreAiSpeechServiceProxy::handleOnceRecognitionError(int errorCode) {
    LOGE("Error occurred with code:", errorCode);
    switch (errorCode) {
        case G_IO_ERROR_CLOSED:
            handleOnceRecognitionServerClosed();
            break;
        case G_IO_ERROR_TIMED_OUT:
            handleOnceRecognitionResultTimeout();
            break;
        default:
            handleOnceRecognitionGeneralError();
            break;
    }
}

void CoreAiSpeechServiceProxy::handleOnceRecognitionServerClosed() {
    LOGE("Server closed unexpectedly.");
    if (recognitionCallback_) {
        SpeechRecognitionResult result{};
        result.reason = SPEECH_ERROR_OCCURRED;
        result.text = "";
        result.speakerId = -1;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "Server closed unexpectedly.";
        recognitionCallback_(&result, recognitionUserData_);
    }
}

void CoreAiSpeechServiceProxy::handleOnceRecognitionResultTimeout() {
    if (recognitionCallback_) {
        SpeechRecognitionResult result{};
        result.reason = SPEECH_ERROR_OCCURRED;
        result.text = "";
        result.speakerId = -1;
        result.errorCode = AISDK_MODEL_RUN_TIME_OUT;
        result.errorMessage = "Run model time out.";
        recognitionCallback_(&result, recognitionUserData_);
    }
}

void CoreAiSpeechServiceProxy ::handleOnceRecognitionGeneralError() {
    if (recognitionCallback_) {
        SpeechRecognitionResult result{};
        result.reason = SPEECH_ERROR_OCCURRED;
        result.text = "";
        result.speakerId = -1;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "General error occurred.";
        recognitionCallback_(&result, recognitionUserData_);
    }
}

void CoreAiSpeechServiceProxy::onOnceSynthesisFinishedCallback(GObject *sourceObject, GAsyncResult *res,
                                                               gpointer userData) {
    auto *proxy = static_cast<CoreAiSpeechServiceProxy *>(userData);
    GError *error{nullptr};
    bool result = ai_runtime_core_speech_service_call_recognize_once_finish((AiRuntimeCoreSpeechService *)sourceObject,
                                                                            res, &error);
    if (!result) {
        if (error == nullptr) {
            g_printerr("Error calling once tts: Unknown error\n");
        } else {
            g_printerr("Error calling once tts: %s\n", error->message);
            proxy->handleOnceSynthesisErrorOccurred(error->code);
            g_error_free(error);
        }
    }
}

void CoreAiSpeechServiceProxy::handleOnceSynthesisErrorOccurred(int errorCode) {
    LOGE("Error occurred with code:", errorCode);
    switch (errorCode) {
        case G_IO_ERROR_CLOSED:
            handleOnceSynthesisServerClosed();
            break;
        case G_IO_ERROR_TIMED_OUT:
            handleOnceSynthesisResultTimeout();
            break;
        default:
            handleOnceSynthesisGeneralError();
            break;
    }
}

void CoreAiSpeechServiceProxy::handleOnceSynthesisServerClosed() {
    if (synthesizerCallback_) {
        SpeechSynthesisResult result{};
        result.reason = SPEECH_ERROR_OCCURRED;
        result.audioChannel = 0;
        result.audioFormat = "";
        result.audioRate = 16000;
        result.audioChannel = 1;
        result.audioData = {};
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "Server closed unexpectedly.";
        synthesizerCallback_(&result, synthesizerUserData_);
    }
}

void CoreAiSpeechServiceProxy::handleOnceSynthesisResultTimeout() {
    if (synthesizerCallback_) {
        SpeechSynthesisResult result{};
        result.reason = SPEECH_ERROR_OCCURRED;
        result.audioChannel = 0;
        result.audioFormat = "";
        result.audioRate = 16000;
        result.audioChannel = 1;
        result.audioData = {};
        result.errorCode = AISDK_MODEL_RUN_TIME_OUT;
        result.errorMessage = "Run model time out.";
        synthesizerCallback_(&result, synthesizerUserData_);
    }
}

void CoreAiSpeechServiceProxy::handleOnceSynthesisGeneralError() {
    if (synthesizerCallback_) {
        SpeechSynthesisResult result{};
        result.reason = SPEECH_ERROR_OCCURRED;
        result.audioChannel = 0;
        result.audioFormat = "";
        result.audioRate = 16000;
        result.audioChannel = 1;
        result.audioData = {};
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "General error occurred.";
        synthesizerCallback_(&result, synthesizerUserData_);
    }
}

void CoreAiSpeechServiceProxy::refreshRuntimeProtocolVersion() {
    if (proxy_ == nullptr) {
        return;
    }

    const gchar *versionRaw = ai_runtime_core_speech_service_get_runtime_version(proxy_);
    const ProtocolVersion parsedVersion = parseProtocolVersion(versionRaw);
    if (parsedVersion.major == 0 && parsedVersion.minor == 0 && parsedVersion.patch == 0) {
        const ProtocolVersion fallbackVersion{1, 0, 0};
        LOGW("Runtime protocol version is invalid:", versionRaw ? versionRaw : "null", ", fallback to ",
             versionToString(fallbackVersion));
        setRuntimeProtocolVersion(fallbackVersion);
        return;
    }

    setRuntimeProtocolVersion(parsedVersion);
    if (isSdkAheadOfRuntime()) {
        LOGW("SDK protocol", versionToString(getSdkProtocolVersion()), "is newer than ", versionToString(parsedVersion),
             "some APIs may be disabled based on requirements.");
    } else {
        LOGD("Runtime protocol", versionToString(parsedVersion), "is compatible with SDK",
             versionToString(getSdkProtocolVersion()));
    }
}

bool CoreAiSpeechServiceProxy::ensureProtocolSupportedForApi(const char *apiName, int *errorCode) {
    if (ensureApiSupported(apiName, errorCode)) {
        return true;
    }

    const ProtocolRequirement *requirement = findApiRequirement(apiName);
    ProtocolRequirement defaultRequirement{ProtocolVersion{1, 0, 0}, kProtocolVersionMax};
    const ProtocolRequirement &activeRequirement = requirement ? *requirement : defaultRequirement;

    if (apiName) {
        LOGW("Protocol mismatch for API:", apiName, "runtime:", versionToString(getRuntimeProtocolVersion()),
             "sdk:", versionToString(getSdkProtocolVersion()), "min:", versionToString(activeRequirement.minVersion),
             "max:", versionToString(activeRequirement.maxVersion));
    }

    return false;
}

}  // namespace speech
}  // namespace core
}  // namespace kyai

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

#include "coreaispeechservice.h"

#include <kylin-ai/aiengine.h>
#include <kylin-ai/common/error.h>
#include <sodium.h>

#include <cstdio>

#include "coreaispeechmodelconfig.h"
#include "threadpool/async.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace {
constexpr const char *kSdkVersionAnnotation = "SdkVersion";
constexpr const char *kDefaultSdkVersion = "1.0.0";

constexpr const char *objectPath = "/com/kylin/AiRuntime/CoreSpeech";
constexpr const char *interface = "com.kylin.AiRuntime.CoreSpeech";
constexpr const char *recognitionResultSignal = "RecognitionSpeechResult";
constexpr const char *synthesisResultSignal = "SynthesisResult";
constexpr const char *serviceName = "CoreAiSpeechService";

GVariant *buildGVariantFromRecognitionResult(const SpeechRecognitionResult &result) {
    int errorCode{};
    if (result.errorCode == -1) {
        errorCode = AI_COMMON_SUCCESS;
    } else {
        errorCode = result.errorCode;
    }
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_add(&builder, "i", result.reason);
    g_variant_builder_add(&builder, "s", result.text.c_str());
    g_variant_builder_add(&builder, "i", result.speakerId);
    g_variant_builder_add(&builder, "i", errorCode);
    g_variant_builder_add(&builder, "s", result.errorMessage.c_str());
    g_variant_builder_add(&builder, "s", result.requestId.c_str());
    GVariant *resultVariant = g_variant_builder_end(&builder);
    return resultVariant;
}

GVariant *buildGVariantFromSynthesisResult(const SpeechSynthesisResult &result) {
    int errorCode{};
    if (result.errorCode == -1) {
        errorCode = AI_COMMON_SUCCESS;
    } else {
        errorCode = result.errorCode;
    }
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_add(&builder, "s", result.base64AudioData.c_str());
    g_variant_builder_add(&builder, "i", result.reason);
    g_variant_builder_add(&builder, "s", result.audioFormat.c_str());
    g_variant_builder_add(&builder, "i", result.audioRate);
    g_variant_builder_add(&builder, "i", result.audioChannel);
    g_variant_builder_add(&builder, "i", errorCode);
    g_variant_builder_add(&builder, "s", result.errorMessage.c_str());
    g_variant_builder_add(&builder, "s", result.requestId.c_str());
    GVariant *resultVariant = g_variant_builder_end(&builder);
    return resultVariant;
}

}  // namespace

CoreAiSpeechService::CoreAiSpeechService(GDBusConnection &connection, EngineSelector &engineSelector)
    : connection_(connection), engineSelector_(engineSelector) {
    exportSkeleton();
    const std::string protocolVersion = getProtocolVersion();
    if (protocolVersion.empty()) {
        LOGW("Protocol version is empty, use default protocol version: {}", kDefaultSdkVersion);
        setRuntimeProtocolVersion(kDefaultSdkVersion);
        LOGI("CoreAiSpeechService protocol version: {}", kDefaultSdkVersion);
    } else {
        setRuntimeProtocolVersion(protocolVersion);
        LOGI("CoreAiSpeechService protocol version: {}", protocolVersion);
    }
}

CoreAiSpeechService::~CoreAiSpeechService() {
    unexportSkeleton();
    LOGD("Core ai speech service destroy.");
}

void CoreAiSpeechService::stop() {
    for (const auto &handlerId : handlerIds_) {
        g_signal_handler_disconnect(delegate_, handlerId);
    }
    handlerIds_.clear();
    for (const auto &[sessionId, speechRecognizer] : speechRecognizers_) {
        if (speechRecognizer != nullptr) {
            speechRecognizer->destroy();
        }
        std::string uuid = serviceName + std::to_string(sessionId);
        ModelConfigManager::getInstance().updateAllDeployTypeCallback(AiCapability::Speech, uuid, nullptr);
    }
    for (const auto &[sessionId, speechSynthesizer] : speechSynthesizers_) {
        if (speechSynthesizer != nullptr) {
            speechSynthesizer->destroy();
        }
        std::string uuid = serviceName + std::to_string(sessionId);
        ModelConfigManager::getInstance().updateAllDeployTypeCallback(AiCapability::Speech, uuid, nullptr);
    }
}

void CoreAiSpeechService::exportSkeleton() {
    delegate_ = ai_runtime_core_speech_service_skeleton_new();
    connectSignal();

    GError *error = nullptr;

    isExported_ =
        g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath, &error);
    if (!isExported_) {
        g_printerr("Error creating server at address %s: %s\n", objectPath, error->message);
        g_error_free(error);
    }
}

bool CoreAiSpeechService::onHandleInitRecognizer(AiRuntimeCoreSpeechService *service, GDBusMethodInvocation *invocation,
                                                 const gchar *modelconfig, gpointer userData) {
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (!speechService) {
        LOGE("Core ai speech service is null.");
        ai_runtime_core_speech_service_complete_init_recognizer(service, invocation, -1, AISDK_RUNTIME_ERROR);
        return true;
    }

    std::shared_ptr sharedSpeechService = speechService->getSharedObject();
    const CoreSpeechModelConfig config = sharedSpeechService->parseModelConfig(modelconfig);

    int sessionId{-1};
    int errorCode{};
    bool success = sharedSpeechService->initSpeechRecognizer(sessionId, errorCode, config);
    if (!success) {
        ai_runtime_core_speech_service_complete_init_recognizer(service, invocation, sessionId, errorCode);
        return true;
    }
    std::string uuid = serviceName + std::to_string(sessionId);
    ModelConfigManager::getInstance().updateAllDeployTypeCallback(
        AiCapability::Speech, uuid,
        [sharedSpeechService, sessionId](ModelChangeType) { sharedSpeechService->modelChangedMap_[sessionId] = true; });

    ai_runtime_core_speech_service_complete_init_recognizer(service, invocation, sessionId, 0);
    return true;
}

bool CoreAiSpeechService::onHandleInitSynthesizer(AiRuntimeCoreSpeechService *service,
                                                  GDBusMethodInvocation *invocation, const gchar *modelconfig,
                                                  gpointer userData) {
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);

    if (!speechService) {
        LOGE("Core ai speech service is null.");
        ai_runtime_core_speech_service_complete_init_synthesizer(service, invocation, -1, AISDK_RUNTIME_ERROR);
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();
    const CoreSpeechModelConfig config = sharedSpeechService->parseModelConfig(modelconfig);

    int sessionId{-1};
    int errorCode{};
    bool success = sharedSpeechService->initSpeechSynthesizer(sessionId, errorCode, config);

    if (!success) {
        LOGE("Init synthesizer failed {} {}. ", sessionId, errorCode);
        ai_runtime_core_speech_service_complete_init_synthesizer(service, invocation, sessionId, errorCode);
        return true;
    }

    std::string uuid = serviceName + std::to_string(sessionId);
    ModelConfigManager::getInstance().updateAllDeployTypeCallback(
        AiCapability::Speech, uuid,
        [sharedSpeechService, sessionId](ModelChangeType) { sharedSpeechService->modelChangedMap_[sessionId] = true; });

    ai_runtime_core_speech_service_complete_init_synthesizer(service, invocation, sessionId, 0);
    return true;
}

bool CoreAiSpeechService::onHandleDestroyRecognizer(AiRuntimeCoreSpeechService *service,
                                                    GDBusMethodInvocation *invocation, int sessionId,
                                                    gpointer userData) {
    LOGD("On handle destroy recognizer.");
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (speechService == nullptr) {
        LOGE("Core ai speech service is null.");
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();
    if (not sharedSpeechService->isRecognizerSessionIdValid(sessionId)) {
        LOGE("No speech recognizer found for session ID: {}", sessionId);
        return true;
    }

    sharedSpeechService->speechRecognizers_[sessionId]->destroy();
    sharedSpeechService->speechRecognizers_.erase(sessionId);

    std::string uuid = serviceName + std::to_string(sessionId);
    ModelConfigManager::getInstance().updateAllDeployTypeCallback(AiCapability::Speech, uuid, nullptr);

    ai_runtime_core_speech_service_complete_destroy_recognizer(service, invocation);
    LOGD("On handle destroy recognizer complete.");
    return true;
}

bool CoreAiSpeechService::onHandleDestroySynthesizer(AiRuntimeCoreSpeechService *service,
                                                     GDBusMethodInvocation *invocation, int sessionId,
                                                     gpointer userData) {
    LOGD("On handle destroy synthesizer");
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (speechService == nullptr) {
        LOGE("Core ai speech service is null.");
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();

    if (not sharedSpeechService->isSynthesizerSessionIdValid(sessionId)) {
        LOGE("No speech recognizer found for session ID: {}", sessionId);
        return true;
    }

    sharedSpeechService->speechSynthesizers_[sessionId]->destroy();
    sharedSpeechService->speechSynthesizers_.erase(sessionId);

    std::string uuid = serviceName + std::to_string(sessionId);
    ModelConfigManager::getInstance().updateAllDeployTypeCallback(AiCapability::Speech, uuid, nullptr);

    ai_runtime_core_speech_service_complete_destroy_synthesizer(service, invocation);
    return true;
}

bool CoreAiSpeechService::onHandleSetRecognizerAudioConfig(AiRuntimeCoreSpeechService *service,
                                                           GDBusMethodInvocation *invocation, int sessionId,
                                                           gchar *audioConfig, gpointer userData) {
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (!speechService) {
        LOGE("Core ai speech service is null.");
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();

    if (!sharedSpeechService->isRecognizerSessionIdValid(sessionId)) {
        LOGE("No speech recognizer found for session ID: {}", sessionId);
        return true;
    }
    AudioConfigInfo audioConfigInfo(audioConfig);
    sharedSpeechService->speechRecognizers_[sessionId]->setAudioConfig(audioConfigInfo);

    ai_runtime_core_speech_service_complete_set_recognizer_audio_config(service, invocation);
    return true;
}

bool CoreAiSpeechService::onHandleStartContinuousRecognition(AiRuntimeCoreSpeechService *service,
                                                             GDBusMethodInvocation *invocation, gint sessionId,
                                                             gpointer userData) {
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (!speechService) {
        LOGE("Core ai speech service is null.");
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();

    if (!sharedSpeechService->isRecognizerSessionIdValid(sessionId)) {
        LOGE("No speech recognizer found for session ID: {}", sessionId);
        return true;
    }

    // 判断模型是否切换，如果切换了，重新初始化
    int errorCode{};
    bool success = sharedSpeechService->tryToChangeRecognizerModel(sessionId, errorCode);
    if (!success) {
        // 错误信息是AISDK_MODEL_NOT_FOUND时，才在这里发送
        // 其他情况在engine回调中发送
        if (errorCode == AISDK_MODEL_NOT_FOUND) {
            SpeechRecognitionResult result;
            result.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
            result.errorCode = AISDK_MODEL_NOT_FOUND;
            result.errorMessage = "Try to change recognizer model failed.";
            sharedSpeechService->sendRecognizerResult(sessionId, result);
        }
        ai_runtime_core_speech_service_complete_start_continuous_recognition(service, invocation);
        return true;
    }
    sharedSpeechService->speechRecognizers_[sessionId]->startContinuousRecognition();
    ai_runtime_core_speech_service_complete_start_continuous_recognition(service, invocation);

    LOGI("Start continuous recognition.");
    return true;
}

bool CoreAiSpeechService::onHandleStopContinuousRecognition(AiRuntimeCoreSpeechService *service,
                                                            GDBusMethodInvocation *invocation, gint sessionId,
                                                            gpointer userData) {
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (!speechService) {
        LOGE("Core ai speech service is null.");
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();

    if (!sharedSpeechService->isRecognizerSessionIdValid(sessionId)) {
        LOGE("No speech recognizer found for session ID: {}", sessionId);
        return true;
    }
    sharedSpeechService->speechRecognizers_[sessionId]->stopContinuousRecognition();
    LOGI("Stop continuous recognition.");
    ai_runtime_core_speech_service_complete_stop_continuous_recognition(service, invocation);

    return true;
}

bool CoreAiSpeechService::onHandleSetSynthesizerAudioConfig(AiRuntimeCoreSpeechService *service,
                                                            GDBusMethodInvocation *invocation, int sessionId,
                                                            gchar *audioConfig, gpointer userData) {
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (!speechService) {
        LOGE("Core ai speech service is null.");
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();

    AudioConfigInfo audioConfigInfo(audioConfig);
    if (!sharedSpeechService->isSynthesizerSessionIdValid(sessionId)) {
        LOGE("No speech synthesizer found for session ID: {}", sessionId);
        return true;
    }
    sharedSpeechService->speechSynthesizers_[sessionId]->setAudioConfig(audioConfigInfo);

    ai_runtime_core_speech_service_complete_set_synthesizer_audio_config(service, invocation);
    return true;
}

bool CoreAiSpeechService::onHandleStopSpeaking(AiRuntimeCoreSpeechService *service, GDBusMethodInvocation *invocation,
                                               gint sessionId, gpointer userData) {
    LOGD("onHandleStopSpeaking");
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (speechService == nullptr) {
        LOGE("Core ai speech service is null.");
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();
    if (!sharedSpeechService->isSynthesizerSessionIdValid(sessionId)) {
        LOGE("No speech synthesizer found for session ID: {}", sessionId);
        return true;
    }
    sharedSpeechService->speechSynthesizers_[sessionId]->stopSpeaking();
    ai_runtime_core_speech_service_complete_stop_speaking(service, invocation, AiSdkCommonErrorCode::AISDK_NO_ERROR);
    return true;
}

bool CoreAiSpeechService::onHandleSynthesizeTextOnce(AiRuntimeCoreSpeechService *service,
                                                     GDBusMethodInvocation *invocation, gint sessionId, gchar *text,
                                                     gint textLength, gpointer userData) {
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (!speechService) {
        LOGE("Core ai speech service is null.");
        ai_runtime_core_speech_service_complete_synthesize_text_once(service, invocation);
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();
    auto completeFunc = [service, invocation]() {
        ai_runtime_core_speech_service_complete_synthesize_text_once(service, invocation);
    };
    sharedSpeechService->synthesizeTextOnceImpl(sessionId, nullptr, text, completeFunc);
    return true;
}

bool CoreAiSpeechService::onHandleSynthesizeTextOnceWithRequestId(AiRuntimeCoreSpeechService *service,
                                                                  GDBusMethodInvocation *invocation, gint sessionId,
                                                                  const gchar *requestId, gchar *text, gint textLength,
                                                                  gpointer userData) {
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (!speechService) {
        LOGE("Core ai speech service is null.");
        ai_runtime_core_speech_service_complete_synthesize_text_once_with_request_id(service, invocation);
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();
    auto completeFunc = [service, invocation]() {
        ai_runtime_core_speech_service_complete_synthesize_text_once_with_request_id(service, invocation);
    };
    sharedSpeechService->synthesizeTextOnceImpl(sessionId, requestId, text, completeFunc);
    return true;
}

bool CoreAiSpeechService::synthesizeTextOnceImpl(gint sessionId, const gchar *requestId, gchar *text,
                                                 const CompleteFunc &completeFunc) {
    if (!isSynthesizerSessionIdValid(sessionId)) {
        LOGE("No speech synthesizer found for session ID: {}", sessionId);
        completeFunc();
        return true;
    }

    int errorCode{};
    bool success = tryToChangeSynthesizerModel(sessionId, errorCode);
    if (!success) {
        if (errorCode == AISDK_MODEL_NOT_FOUND) {
            SpeechSynthesisResult result{};
            result.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
            result.errorCode = AISDK_MODEL_NOT_FOUND;
            result.errorMessage = "Try to change synthesizer model failed.";
            result.requestId = requestId ? requestId : "";
            emitSynthesisResultSignal(interfaceName(sessionId), result);
        }
        completeFunc();
        return true;
    }

    std::string textString{text};
    std::string requestIdString{requestId ? requestId : ""};
    speechSynthesizers_[sessionId]->synthesizeTextOnce(textString, requestIdString);
    completeFunc();
    return true;
}

bool CoreAiSpeechService::onHandleContinuousRecognitionWriteAudioData(AiRuntimeCoreSpeechService *service,
                                                                      GDBusMethodInvocation *invocation, int sessionId,
                                                                      gchar *audioData, gpointer userData) {
    std::string encode(audioData);
    std::vector<uint8_t> decode = util::base64Decode(encode);
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (!speechService) {
        LOGE("Core ai speech service is null.");
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();

    if (!sharedSpeechService->isRecognizerSessionIdValid(sessionId)) {
        LOGE("No speech recognizer found for session ID: {}", sessionId);
        return true;
    }
    sharedSpeechService->speechRecognizers_[sessionId]->writeContinuousRecognitionFromAudioStream(decode);
    ai_runtime_core_speech_service_complete_continuous_recognition_write_audio_data(service, invocation);
    return true;
}

bool CoreAiSpeechService::onHandleRecognizeOnce(AiRuntimeCoreSpeechService *service, GDBusMethodInvocation *invocation,
                                                gint sessionId, gpointer userData) {
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (!speechService) {
        LOGE("Core ai speech service is null.");
        ai_runtime_core_speech_service_complete_recognize_once(service, invocation);
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();
    auto completeFunc = [service, invocation]() {
        ai_runtime_core_speech_service_complete_recognize_once(service, invocation);
    };
    sharedSpeechService->recognizeOnceImpl(sessionId, nullptr, completeFunc);
    return true;
}

bool CoreAiSpeechService::onHandleRecognizeOnceWithRequestId(AiRuntimeCoreSpeechService *service,
                                                             GDBusMethodInvocation *invocation, gint sessionId,
                                                             const gchar *requestId, gpointer userData) {
    auto *speechService = static_cast<CoreAiSpeechService *>(userData);
    if (!speechService) {
        LOGE("Core ai speech service is null.");
        ai_runtime_core_speech_service_complete_recognize_once_with_request_id(service, invocation);
        return true;
    }
    std::shared_ptr sharedSpeechService = speechService->getSharedObject();
    auto completeFunc = [service, invocation]() {
        ai_runtime_core_speech_service_complete_recognize_once_with_request_id(service, invocation);
    };
    sharedSpeechService->recognizeOnceImpl(sessionId, requestId, completeFunc);
    return true;
}

bool CoreAiSpeechService::recognizeOnceImpl(gint sessionId, const gchar *requestId, const CompleteFunc &completeFunc) {
    if (!isRecognizerSessionIdValid(sessionId)) {
        LOGE("No speech recognizer found for session ID: {}", sessionId);
        completeFunc();
        return true;
    }

    int errorCode{};
    bool success = tryToChangeRecognizerModel(sessionId, errorCode);
    if (!success) {
        if (errorCode == AISDK_MODEL_NOT_FOUND) {
            SpeechRecognitionResult result;
            result.reason = int(ai_engine::core_ai::speech::SpeechResultReason::SpeechErrorOccurred);
            result.errorCode = AISDK_MODEL_NOT_FOUND;
            result.errorMessage = "Try to change recognizer model failed.";
            result.requestId = requestId ? requestId : "";
            sendRecognizerResult(sessionId, result);
        }
        completeFunc();
        return true;
    }

    std::string requestIdString{requestId ? requestId : ""};
    speechRecognizers_[sessionId]->recognizeOnce(requestIdString);
    completeFunc();
    LOGI("Recognize once.");
    return true;
}

void CoreAiSpeechService::connectSignal() {
    gulong handlerId{};
    handlerId = g_signal_connect(delegate_, "handle-init-recognizer", G_CALLBACK(onHandleInitRecognizer), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-init-synthesizer", G_CALLBACK(onHandleInitSynthesizer), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-destroy-recognizer", G_CALLBACK(onHandleDestroyRecognizer), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-destroy-synthesizer", G_CALLBACK(onHandleDestroySynthesizer), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-set-recognizer-audio-config",
                                 G_CALLBACK(onHandleSetRecognizerAudioConfig), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-start-continuous-recognition",
                                 G_CALLBACK(onHandleStartContinuousRecognition), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-stop-continuous-recognition",
                                 G_CALLBACK(onHandleStopContinuousRecognition), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle_set_synthesizer_audio_config",
                                 G_CALLBACK(onHandleSetSynthesizerAudioConfig), this);
    handlerIds_.push_back(handlerId);
    handlerId =
        g_signal_connect(delegate_, "handle_synthesize_text_once", G_CALLBACK(onHandleSynthesizeTextOnce), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle_synthesize_text_once_with_request_id",
                                 G_CALLBACK(onHandleSynthesizeTextOnceWithRequestId), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle_stop_speaking", G_CALLBACK(onHandleStopSpeaking), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-continuous-recognition-write-audio-data",
                                 G_CALLBACK(onHandleContinuousRecognitionWriteAudioData), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-recognize-once", G_CALLBACK(onHandleRecognizeOnce), this);
    handlerIds_.push_back(handlerId);
    handlerId = g_signal_connect(delegate_, "handle-recognize-once-with-request-id",
                                 G_CALLBACK(onHandleRecognizeOnceWithRequestId), this);
    handlerIds_.push_back(handlerId);
}

void CoreAiSpeechService::unexportSkeleton() {
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }
    g_object_unref(delegate_);
}

std::string CoreAiSpeechService::getProtocolVersion() {
    if (delegate_ == nullptr) {
        LOGE("Delegate is null, get protocol version failed.");
        return kDefaultSdkVersion;
    }

    GDBusInterfaceInfo *interfaceInfo = ai_runtime_core_speech_service_interface_info();
    if (interfaceInfo == nullptr) {
        LOGW("Interface info is null, use default protocol version.");
        return kDefaultSdkVersion;
    }
    if (interfaceInfo->annotations == nullptr) {
        LOGW("Protocol version annotation not found, use default protocol version.");
        return kDefaultSdkVersion;
    }

    for (GDBusAnnotationInfo **annotation = interfaceInfo->annotations; annotation && *annotation; ++annotation) {
        if ((*annotation)->key == nullptr || (*annotation)->value == nullptr) {
            continue;
        }
        if (g_strcmp0((*annotation)->key, kSdkVersionAnnotation) == 0) {
            return (*annotation)->value;
        }
    }
    return kDefaultSdkVersion;
}

void CoreAiSpeechService::setRuntimeProtocolVersion(const std::string &version) {
    if (delegate_ == nullptr) {
        LOGE("Delegate is null, set runtime version failed.");
        return;
    }
    ai_runtime_core_speech_service_set_runtime_version(delegate_, version.c_str());
}

int CoreAiSpeechService::generateRandomSessionId() {
    uint32_t sessionId;
    randombytes_buf(&sessionId, sizeof(sessionId));
    sessionId = 1 + (sessionId & INT_MAX);
    while (sessionIdSet_.find(sessionId) != sessionIdSet_.end()) {
        randombytes_buf(&sessionId, sizeof(sessionId));
        sessionId = 1 + (sessionId & INT_MAX);
    }
    sessionIdSet_.insert(sessionId);
    return sessionId;
}

void CoreAiSpeechService::releaseRandomSessionId(int sessionId) { sessionIdSet_.erase(sessionId); }

bool CoreAiSpeechService::isRecognizerSessionIdValid(int sessionId) {
    return speechRecognizers_.find(sessionId) != speechRecognizers_.end();
}

bool CoreAiSpeechService::isSynthesizerSessionIdValid(int sessionId) {
    return speechSynthesizers_.find(sessionId) != speechSynthesizers_.end();
}

void CoreAiSpeechService::emitSynthesisResultSignal(const std::string &interfaceName,
                                                    const SpeechSynthesisResult &result) {
    GVariant *resultVariant = buildGVariantFromSynthesisResult(result);
    LOGI("TTS model name: {}", result.modelName);
    g_dbus_connection_emit_signal(&connection_, interface, objectPath, interfaceName.c_str(), synthesisResultSignal,
                                  resultVariant, nullptr);
}

bool CoreAiSpeechService::initSpeechRecognizer(int &sessionId, int &errorCode, const CoreSpeechModelConfig &config) {
    std::unique_ptr uniqueSpeechEngine = engineSelector_.selectCoreAiSpeechEngine(config.type, config.name);
    std::shared_ptr speechEngine{std::move(uniqueSpeechEngine)};
    if (speechEngine == nullptr) {
        LOGE("Failed to select speech engine.");
        sessionId = -1;
        errorCode = AISDK_MODEL_NOT_FOUND;
        return false;
    }
    if (!isRecognizerSessionIdValid(sessionId)) {
        sessionId = generateRandomSessionId();
    }
    std::shared_ptr<SpeechRecognizer> speechRecognizer = std::make_shared<SpeechRecognizer>(config.isDefaultMode());
    if (!speechRecognizer->init(errorCode, speechEngine)) {
        sessionId = -1;
        return false;
    }
    speechRecognizers_[sessionId] = speechRecognizer;
    setRecognizerCallback(sessionId);
    return true;
}

bool CoreAiSpeechService::initSpeechSynthesizer(int &sessionId, int &errorCode, const CoreSpeechModelConfig &config) {
    std::unique_ptr uniqueSpeechEngine = engineSelector_.selectCoreAiSpeechEngine(config.type, config.name);
    std::shared_ptr speechEngine{std::move(uniqueSpeechEngine)};
    if (speechEngine == nullptr) {
        LOGE("Failed to select speech engine.");
        sessionId = -1;
        errorCode = AISDK_MODEL_NOT_FOUND;
        return false;
    }
    if (!isSynthesizerSessionIdValid(sessionId)) {
        sessionId = generateRandomSessionId();
    }
    std::shared_ptr<SpeechSynthesizer> speechSynthesizer = std::make_shared<SpeechSynthesizer>(config.isDefaultMode());
    if (!speechSynthesizer->init(errorCode, speechEngine)) {
        LOGE("Failed to init speech synthesizer. Error code: {}", errorCode);
        sessionId = -1;
        return false;
    }
    speechSynthesizers_[sessionId] = speechSynthesizer;
    setSynthesizerCallback(sessionId);
    return true;
}

CoreSpeechModelConfig CoreAiSpeechService::parseModelConfig(const std::string &jsonString) {
    CoreSpeechModelConfig config{};
    if (jsonString.empty()) {
        return config;
    }

    Json::Value root = util::stringToJson(jsonString);

    if (root.isNull()) {
        return config;
    }
    if (root.isMember("name")) {
        config.name = root["name"].asString();
    }
    if (root.isMember("type")) {
        std::string typeStr = root["type"].asString();
        config.type = parseDeployType(typeStr);
    }
    return config;
}

DeployType CoreAiSpeechService::parseDeployType(const std::string &typeStr) {
    if (typeStr == "Local") {
        return DeployType::Local;
    }
    if (typeStr == "Custom") {
        return DeployType::Custom;
    }
    if (typeStr == "PublicCloud") {
        return DeployType::PublicCloud;
    }
    return DeployType::Unknown;  // default
}

bool CoreAiSpeechService::tryToChangeRecognizerModel(int sessionId, int &errorCode) {
    if (modelChangedMap_.find(sessionId) == modelChangedMap_.end()) {
        return true;
    }
    if (!modelChangedMap_[sessionId]) {
        return true;
    }
    if (speechRecognizers_[sessionId]->engineInUse()) {
        return true;
    }
    // 重新初始化时，使用当前模型
    CoreSpeechModelConfig config{};
    bool success = initSpeechRecognizer(sessionId, errorCode, config);
    if (!success) {
        LOGE("Try to change recognizer model failed.");
        return false;
    }
    modelChangedMap_[sessionId] = false;
    return true;
}

bool CoreAiSpeechService::tryToChangeSynthesizerModel(int sessionId, int &errorCode) {
    if (modelChangedMap_.find(sessionId) == modelChangedMap_.end()) {
        return true;
    }
    if (!modelChangedMap_[sessionId]) {
        return true;
    }
    if (speechSynthesizers_[sessionId]->engineInUse()) {
        return true;
    }
    CoreSpeechModelConfig config{};
    bool success = initSpeechSynthesizer(sessionId, errorCode, config);
    if (!success) {
        LOGE("Try to change synthesizer model failed.");
        return false;
    }
    modelChangedMap_[sessionId] = false;
    return true;
}

std::string CoreAiSpeechService::interfaceName(int sessionId) {
    return std::string(interface) + std::to_string(sessionId);
}

void CoreAiSpeechService::sendRecognizerResult(int sessionId, const SpeechRecognitionResult &result) {
    std::string temp;
    temp += fmt::format("Start        :==============================\n");
    temp += fmt::format("Model        :{}\n", result.modelName);
    temp += fmt::format("Request ID   :{}\n", result.requestId);
    temp += fmt::format("Type         :{}\n", result.reason);
    temp += fmt::format("Text         :{}\n", result.text);
    temp += fmt::format("Speaker ID   :{}\n", result.speakerId);
    temp += fmt::format("Error code   :{}\n", result.errorCode);
    temp += fmt::format("Error message:{}\n", result.errorMessage);
    temp += fmt::format("End          :==============================\n");
    LOGI("{}", temp);

    GError *error{nullptr};
    GVariant *resultVariant = buildGVariantFromRecognitionResult(result);
    g_dbus_connection_emit_signal(&connection_, interface, objectPath, interfaceName(sessionId).c_str(),
                                  recognitionResultSignal, resultVariant, &error);
    if (error != nullptr) {
        LOGE("Send recognizer result: {}", error->message);
        g_error_free(error);
    }
}

void CoreAiSpeechService::setRecognizerCallback(int sessionId) {
    std::shared_ptr sharedObject = getSharedObject();
    speechRecognizers_[sessionId]->setRecognitionResultCallback(
        [sharedObject, sessionId](const SpeechRecognitionResult &result) {
            sharedObject->sendRecognizerResult(sessionId, result);
        });
}

void CoreAiSpeechService::setSynthesizerCallback(int sessionId) {
    std::string resultSignalInterfaceName = interfaceName(sessionId);

    std::shared_ptr sharedObject = getSharedObject();
    speechSynthesizers_[sessionId]->setSynthesisResultCallback(
        [sharedObject, resultSignalInterfaceName](const SpeechSynthesisResult &result) {
            sharedObject->emitSynthesisResultSignal(resultSignalInterfaceName, result);
        });
}

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

#include "_speechrecognitionsession.h"

#include <atomic>
#include <chrono>
#include <sstream>

#include "logger.h"

namespace kyai {
namespace core {
namespace speech {

namespace {

std::string ensureRequestId(const char *requestId) {
    if (requestId != nullptr && requestId[0] != '\0') {
        return requestId;
    }
    static std::atomic<uint64_t> counter{0};
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    const uint64_t seq = ++counter;
    std::ostringstream oss;
    oss << "req-" << millis << "-" << seq;
    return oss.str();
}

}  // namespace

_SpeechRecognitionSession::_SpeechRecognitionSession() {
    serviceProxy_.setReconnectedCallback([this] { initSession(); });
}

_SpeechRecognitionSession::~_SpeechRecognitionSession() { serviceProxy_.setReconnectedCallback(nullptr); }

int _SpeechRecognitionSession::init() {
    if (!initServer()) {
        return AiSdkCommonErrorCode::AISDK_RUNTIME_ERROR;
    }
    return initSession();
}

void _SpeechRecognitionSession::destroyRecognizer() { serviceProxy_.destroyRecognizer(sessionId_); }

void _SpeechRecognitionSession::setRecognizerAudioConfig(_AudioConfig *config) {
    if (config == nullptr) {
        return;
    }
    audioConfig_ = config;
    serviceProxy_.setRecognizerAudioConfig(sessionId_, config->toJsonString());
}

_AudioConfig *_SpeechRecognitionSession::getRecognizerAudioConfig() const { return audioConfig_; }

void _SpeechRecognitionSession::setRecognitionResultCallback(SpeechRecognitionResultCallback callback, void *userData) {
    serviceProxy_.setRecognitionResultCallback(callback, userData);
}

void _SpeechRecognitionSession::startContinuousRecognition() { serviceProxy_.startContinuousRecognition(sessionId_); }

void _SpeechRecognitionSession::stopContinuousRecognition() { serviceProxy_.stopContinuousRecognition(sessionId_); }

void _SpeechRecognitionSession::continuousRecognitionWriteAudioData(const uint8_t *data, uint32_t length) {
    serviceProxy_.continuousRecognitionWriteAudioData(sessionId_, data, length);
}

void _SpeechRecognitionSession::recognizeOnce() { serviceProxy_.recognizeOnce(sessionId_); }

int _SpeechRecognitionSession::recognizeOnceWithRequestId(const char *requestId) {
    const std::string resolvedRequestId = ensureRequestId(requestId);
    return serviceProxy_.recognizeOnceWithRequestId(sessionId_, resolvedRequestId.c_str());
}

void _SpeechRecognitionSession::setRecognitionModelConfig(const SpeechModelConfig &config) { modelConfig_ = config; }

bool _SpeechRecognitionSession::initServer() {
    if (!serviceProxy_.connectToServer()) {
        return serviceProxy_.reconnectServer();
    }
    return true;
}

int _SpeechRecognitionSession::initSession() {
    const std::string jsonStringConfig = modelConfigToJson(modelConfig_);
    int errorCode = serviceProxy_.initRecognizer(jsonStringConfig.c_str(), sessionId_);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("Init session error: ", errorCode);
    }
    return errorCode;
}

void _SpeechRecognitionSession::enableInternalEventLoop(bool enable) { serviceProxy_.enableInternalEventLoop(enable); }

}  // namespace speech
}  // namespace core
}  // namespace kyai

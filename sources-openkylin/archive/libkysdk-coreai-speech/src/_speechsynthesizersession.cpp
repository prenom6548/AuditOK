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

#include "_speechsynthesizersession.h"

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

_SpeechSynthesizerSession::_SpeechSynthesizerSession() {
    serviceProxy_.setReconnectedCallback([this] { initSession(); });
}

_SpeechSynthesizerSession::~_SpeechSynthesizerSession() { serviceProxy_.setReconnectedCallback(nullptr); }

int _SpeechSynthesizerSession::init() {
    if (!initServer()) {
        return AiSdkCommonErrorCode::AISDK_RUNTIME_ERROR;
    }
    return initSession();
}

void _SpeechSynthesizerSession::destroySynthesizer() { serviceProxy_.destroySynthesizer(sessionId_); }

void _SpeechSynthesizerSession::setSynthesizerAudioConfig(_AudioConfig *config) {
    if (config == nullptr) {
        return;
    }
    audioConfig_ = config;

    serviceProxy_.setSynthesizerAudioConfig(sessionId_, config->toJsonString());
}

void _SpeechSynthesizerSession::setSynthesizerResultCallback(SpeechSynthesisResultCallback callback, void *userData) {
    serviceProxy_.setSynthesizerCallback(callback, userData);
}

void _SpeechSynthesizerSession::synthesizeTextOnce(const char *text, uint32_t textLength) {
    LOGD(sessionId_, std::string(text, textLength));
    serviceProxy_.synthesizeTextOnce(sessionId_, text, textLength);
}

int _SpeechSynthesizerSession::synthesizeTextOnceWithRequestId(const char *requestId, const char *text,
                                                               uint32_t textLength) {
    const std::string resolvedRequestId = ensureRequestId(requestId);
    LOGD(sessionId_, resolvedRequestId, std::string(text, textLength));
    return serviceProxy_.synthesizeTextOnceWithRequestId(sessionId_, resolvedRequestId.c_str(), text, textLength);
}

void _SpeechSynthesizerSession::synthesizeTextContinuous(const char *text, uint32_t textLength) {
    serviceProxy_.synthesizeTextContinuous(sessionId_, text, textLength);
}

int _SpeechSynthesizerSession::stopSpeaking() { return serviceProxy_.stopSpeaking(sessionId_); }

void _SpeechSynthesizerSession::setSynthesizerModelConfig(const SpeechModelConfig &config) { modelConfig_ = config; }

bool _SpeechSynthesizerSession::initServer() {
    if (!serviceProxy_.connectToServer()) {
        return serviceProxy_.reconnectServer();
    }
    return true;
}

int _SpeechSynthesizerSession::initSession() {
    const std::string jsonStringConfig = modelConfigToJson(modelConfig_);
    int errorCode = serviceProxy_.initSynthesizer(jsonStringConfig.c_str(), sessionId_);
    LOGD(sessionId_);
    if (errorCode != AISDK_NO_ERROR) {
        LOGE("Init session error: ", errorCode);
    }
    return errorCode;
}

void _SpeechSynthesizerSession::enableInternalEventLoop(bool enable) { serviceProxy_.enableInternalEventLoop(enable); }

}  // namespace speech
}  // namespace core
}  // namespace kyai

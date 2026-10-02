/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include "_textrecognitionsession.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <sstream>

namespace kyai {
namespace core {
namespace vision {

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

_TextRecognitionSession::_TextRecognitionSession() {
    serviceProxy_.setReconnectedCallback([this]() { initSession(); });
}
int _TextRecognitionSession::init() {
    if (!initServer()) {
        return AiSdkCommonErrorCode::AISDK_RUNTIME_ERROR;
    }

    return initSession();
}

void _TextRecognitionSession::destroyTextRecognitionSession() { serviceProxy_.destroyTextRecognition(sessionId_); }

void _TextRecognitionSession::setRecognitionResultCallback(TextRecognitionResultCallback callback, void *userData) {
    serviceProxy_.setRecognitionResultCallback(callback, userData);
}

void _TextRecognitionSession::setRecognizeTextModelConfig(const TextRecognitionModelConfig &config) {
    modelConfig_ = config;
}

void _TextRecognitionSession::recognizeTextFromImageFileAsync(const char *imageFile) {
    serviceProxy_.recognizeTextFromImageFileAsync(sessionId_, imageFile);
}

void _TextRecognitionSession::recognizeTextFromImageFileAsync(const char *requestId, const char *imageFile) {
    const std::string resolvedRequestId = ensureRequestId(requestId);
    serviceProxy_.recognizeTextFromImageFileAsync(sessionId_, resolvedRequestId.c_str(), imageFile);
}

void _TextRecognitionSession::recognizeTextFromImageDataAsync(const char *imageData, unsigned int imageDataLength) {
    serviceProxy_.recognizeTextFromImageDataAsync(sessionId_, imageData, imageDataLength);
}

void _TextRecognitionSession::recognizeTextFromImageDataAsync(const char *requestId, const char *imageData,
                                                              unsigned int imageDataLength) {
    const std::string resolvedRequestId = ensureRequestId(requestId);
    serviceProxy_.recognizeTextFromImageDataAsync(sessionId_, resolvedRequestId.c_str(), imageData, imageDataLength);
}

bool _TextRecognitionSession::initServer() {
    if (!serviceProxy_.connectToServer()) {
        return serviceProxy_.reconnectServer();
    }
    return true;
}

int _TextRecognitionSession::initSession() {
    const std::string jsonStringConfig = modelConfigToJson(modelConfig_);
    int errorCode = serviceProxy_.initEngine(jsonStringConfig.c_str(), &sessionId_);
    return errorCode;
}

void _TextRecognitionSession::enableInternalEventLoop(bool enable) { serviceProxy_.enableInternalEventLoop(enable); }

}  // namespace vision
}  // namespace core
}  // namespace kyai
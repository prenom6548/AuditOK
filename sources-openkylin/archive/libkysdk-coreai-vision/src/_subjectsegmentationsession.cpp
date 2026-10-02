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
#include "_subjectsegmentationsession.h"

#include <atomic>
#include <chrono>
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

_SubjectSegmentationSession::_SubjectSegmentationSession() {
    serviceProxy_.setReconnectedCallback([this]() {
        initCallback_ = nullptr;
        initUserData_ = nullptr;
        initSession();
    });
}

void _SubjectSegmentationSession::init(SubjectSegmentationInitCallback callback, void *userData) {
    if (!initServer()) {
        if (callback) {
            callback(AI_COMMON_RUNTIME_ERROR, userData);
        }
    }
    initCallback_ = callback;
    initUserData_ = userData;
    initSession();
}

void _SubjectSegmentationSession::destroySubjectSegmentationSession() {
    serviceProxy_.destroySubjectSegmentation(sessionId_);
}

void _SubjectSegmentationSession::setSubjectSegmentationResultCallback(SubjectSegmentationResultCallback callback,
                                                                       void *userData) {
    serviceProxy_.setSubjectSegmentationResultCallback(callback, userData);
}

void _SubjectSegmentationSession::setSubjectSegmentationModelConfig(const SubjectSegmentationConfig &config) {
    modelConfig_ = config;
}

void _SubjectSegmentationSession::enableInternalEventLoop(bool enable) {
    serviceProxy_.enableInternalEventLoop(enable);
}

void _SubjectSegmentationSession::subjectSegmentationFromImageFileAsync(const char *requestId,
                                                                        const char *imageFile) {
    const std::string jsonStringConfig = subjectSegmentationConfigToJson(modelConfig_);
    const std::string resolvedRequestId = ensureRequestId(requestId);
    serviceProxy_.subjectSegmentationFromImageFileAsync(sessionId_, resolvedRequestId.c_str(), imageFile,
                                                        jsonStringConfig.c_str());
}

void _SubjectSegmentationSession::subjectSegmentationFromImageDataAsync(const char *requestId, const char *data,
                                                                        unsigned int dataLength) {
    const std::string jsonStringConfig = subjectSegmentationConfigToJson(modelConfig_);
    const std::string resolvedRequestId = ensureRequestId(requestId);
    serviceProxy_.subjectSegmentationFromImageDataAsync(sessionId_, resolvedRequestId.c_str(), data, dataLength,
                                                        jsonStringConfig.c_str());
}

bool _SubjectSegmentationSession::initServer() {
    if (!serviceProxy_.connectToServer()) {
        return serviceProxy_.reconnectServer();
    }
    return true;
}

void _SubjectSegmentationSession::initSession() {
    serviceProxy_.initSubjectSegmentationEngine(onSessionInitialized, this);
}

void _SubjectSegmentationSession::onSessionInitialized(int sessionId, int errorCode, void *userData) {
    auto *session = static_cast<_SubjectSegmentationSession *>(userData);
    session->sessionId_ = sessionId;
    if (session->initCallback_) {
        session->initCallback_(errorCode, session->initUserData_);
    }
}

}  // namespace vision
}  // namespace core
}  // namespace kyai

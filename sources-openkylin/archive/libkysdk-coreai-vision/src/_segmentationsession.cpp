/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "_segmentationsession.h"

namespace kyai::core::vision {

_SegmentationSession::_SegmentationSession() {
    proxy_.setReconnectedCallback([this]() {
        initCallback_ = nullptr;
        initUserData_ = nullptr;
        initSessionAsync();
    });
}

_SegmentationSession::~_SegmentationSession() {}

void _SegmentationSession::initAsync(const SegmentationInitCallback &callback, void *userData) {
    if (!initServer()) {
        if (callback) {
            callback(AI_COMMON_RUNTIME_ERROR, userData);
        }
    }
    initCallback_ = callback;
    initUserData_ = userData;
    initSessionAsync();
}

void _SegmentationSession::destroy() {
    proxy_.destroySegmentation(sessionId_);
    sessionId_ = 0;
}

void _SegmentationSession::setSegmentationResultCallback(const SegmentationResultCallback &callback, void *userData) {
    proxy_.setSegmentationResultCallback(callback, userData);
}

void _SegmentationSession::encodeImageFileAsync(const std::string &imageFile,
                                                const SegmentationEncodeCallback &callback, void *userData) {
    proxy_.encodeImageFileAsync(sessionId_, imageFile, callback, userData);
}

void _SegmentationSession::encodeImageDataAsync(const std::vector<uint8_t> &imageData,
                                                const SegmentationEncodeCallback &callback, void *userData) {
    proxy_.encodeImageDataAsync(sessionId_, imageData, callback, userData);
}

int _SegmentationSession::clearCache() { return proxy_.clearCache(sessionId_); }

void _SegmentationSession::runAsync(const std::vector<PixelPoint> &points, uint32_t minArea) {
    proxy_.runAsync(sessionId_, points, minArea);
}

void _SegmentationSession::enableInternalEventLoop(bool enable) { proxy_.enableInternalEventLoop(enable); }

bool _SegmentationSession::initServer() {
    if (!proxy_.connectToServer()) {
        return proxy_.reconnectServer();
    }
    return true;
}

void _SegmentationSession::initSessionAsync() { proxy_.initSegmentationEngineAsync(onSessionInited, this); }

void _SegmentationSession::onSessionInited(int sessionId, int errorCode, void *userData) {
    _SegmentationSession *session = static_cast<_SegmentationSession *>(userData);
    session->sessionId_ = sessionId;
    if (session->initCallback_) {
        session->initCallback_(errorCode, session->initUserData_);
    }
}

}  // namespace kyai::core::vision
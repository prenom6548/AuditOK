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

#include "_genaiimagesession.h"

#include "logger.h"

namespace kylin {
namespace genai {
namespace vision {

_GenAiImageSession::_GenAiImageSession() {
    serviceProxy_ = std::make_unique<GenAiVisionServiceProxy>();
    bindReconnectedCallback();
}

void _GenAiImageSession::bindReconnectedCallback() {
    serviceProxy_->setReconnectedCallback([this] {
        initSession();
        serviceProxy_->setImageConfig(sessionId_, imageConfigToJson(imageConfig_));
    });
}

int _GenAiImageSession::init() {
    if (!initServer()) {
        return AiSdkCommonErrorCode::AISDK_RUNTIME_ERROR;
    }

    return initSession();
}

void _GenAiImageSession::setImageConfig(const ImageConfig &config) {
    imageConfig_ = config;
    serviceProxy_->setImageConfig(sessionId_, imageConfigToJson(config));
}

void _GenAiImageSession::setImageResultCallback(VisionImageResultCallback callback, void *userData) {
    serviceProxy_->setImageResultCallback(callback, userData);
}

const char **_GenAiImageSession::getSupportedImageStyle(int *number) {
    return serviceProxy_->getSupportedImageStyle(sessionId_, number);
}

const ImageSize *_GenAiImageSession::getSupportedImageSize(int *number) {
    return serviceProxy_->getSupportedImageSize(sessionId_, number);
}

const int _GenAiImageSession::getSupportedImageNumber() { return serviceProxy_->getSupportedImageNumber(sessionId_); }

void _GenAiImageSession::generateImageAsync(const std::string &prompt) {
    ImageGenerationRequest request;
    request.width = imageConfig_.width;
    request.height = imageConfig_.height;
    request.number = imageConfig_.number;
    request.style = imageConfig_.style;
    request.modelName = imageConfig_.modelName;
    request.prompt = prompt;
    return serviceProxy_->generateImageAsync(sessionId_, prompt, imageGenerationRequestToString(request));
}

int _GenAiImageSession::generateImageAsyncWithRequestId(const std::string &prompt, const std::string &requestId) {
    ImageGenerationRequest request;
    request.width = imageConfig_.width;
    request.height = imageConfig_.height;
    request.number = imageConfig_.number;
    request.style = imageConfig_.style;
    request.modelName = imageConfig_.modelName;
    request.prompt = prompt;
    return serviceProxy_->generateImageAsyncWithRequestId(sessionId_, imageGenerationRequestToString(request),
                                                          requestId);
}

int _GenAiImageSession::stopGeneration(const char *requestId) {
    return serviceProxy_->stopGeneration(sessionId_, requestId);
}

bool _GenAiImageSession::initServer() {
    if (!serviceProxy_->connectToServer()) {
        return serviceProxy_->reconnectServer();
    }
    return true;
}

int _GenAiImageSession::initSession() { return serviceProxy_->initEngine(&sessionId_); }

#ifdef UNIT_TEST
void _GenAiImageSession::setServiceProxyForTest(std::unique_ptr<IGenAiVisionServiceProxy> proxy) {
    if (!proxy) {
        return;
    }
    serviceProxy_ = std::move(proxy);
    bindReconnectedCallback();
}
#endif

void _GenAiImageSession::enableInternalEventLoop(bool enable) { serviceProxy_->enableInternalEventLoop(enable); }

}  // namespace vision
}  // namespace genai
}  // namespace kylin

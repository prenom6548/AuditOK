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

#include "xunfeivisionengine.h"
#include "xunfeivisionengine_p.h"

XunfeiVisionEngine::XunfeiVisionEngine() : d(new XunfeiVisionEnginePrivate()) {}

XunfeiVisionEngine::~XunfeiVisionEngine() { delete d; }

std::string XunfeiVisionEngine::engineName() const { return d->engineName(); }

std::string XunfeiVisionEngine::currentModel() const { return d->currentModel(); }

std::string XunfeiVisionEngine::currentModelName() const { return d->currentModelName(); }

bool XunfeiVisionEngine::isCloud() const { return d->isCloud(); }

bool XunfeiVisionEngine::isBuiltInEngine() const { return d->isBuiltInEngine(); }

bool XunfeiVisionEngine::isCustomModelEngine() const { return d->isCustomModelEngine(); }

void XunfeiVisionEngine::setConfig(const std::string &config) { d->setConfig(config); }

bool XunfeiVisionEngine::setCurrentModel(const std::string &model) { return d->setCurrentModel(model); }

std::string XunfeiVisionEngine::modelInfo() const { return d->modelInfo(); }

void XunfeiVisionEngine::setGenerationResultCallback(ai_engine::gen_ai::vision::ImageGenerationCallback callback) {
    d->setGenerationResultCallback(callback);
}

ai_engine::result::AiEngineBaseResult XunfeiVisionEngine::init() { return d->init(); }

ai_engine::result::AiEngineBaseResult XunfeiVisionEngine::destroy() { return d->destroy(); }

void XunfeiVisionEngine::setImageSize(int width, int height) { d->setImageSize(width, height); }

void XunfeiVisionEngine::setImageNumber(int number) { d->setImageNumber(number); }

void XunfeiVisionEngine::setImageStyle(const std::string &style) { d->setImageStyle(style); }

std::vector<std::string> XunfeiVisionEngine::supportedImageStyles() const { return d->supportedImageStyles(); }

int XunfeiVisionEngine::supportedImageNumber() const { return d->supportedImageNumber(); }

std::vector<std::pair<int, int>> XunfeiVisionEngine::supportedImageSizes() const { return d->supportedImageSizes(); }

ai_engine::result::AiEngineBaseResult XunfeiVisionEngine::generateImage(const std::string &prompt) {
    return d->generateImage(prompt);
}

void XunfeiVisionEngine::stopImageGeneration() { return d->stopImageGeneration(); }

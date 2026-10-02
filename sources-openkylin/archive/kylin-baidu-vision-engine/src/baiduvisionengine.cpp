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

#include "baiduvisionengine.h"
#include <string>
#include "baiduvisionengine_p.h"

BaiduVisionEngine::BaiduVisionEngine() : d(new BaiduVisionEnginePrivate()) {}

BaiduVisionEngine::~BaiduVisionEngine() { delete d; }

std::string BaiduVisionEngine::engineName() const { return d->engineName(); }

std::string BaiduVisionEngine::currentModel() const { return d->currentModel(); }

std::string BaiduVisionEngine::currentModelName() const {
    return d->currentModelName();
}

bool BaiduVisionEngine::isCloud() const { return d->isCloud(); }

bool BaiduVisionEngine::isBuiltInEngine() const { return d->isBuiltInEngine(); }

bool BaiduVisionEngine::isCustomModelEngine() const { return d->isCustomModelEngine(); }

void BaiduVisionEngine::setConfig(const std::string &config) { d->setConfig(config); }

bool BaiduVisionEngine::setCurrentModel(const std::string &model) { return d->setCurrentModel(model); }

std::string BaiduVisionEngine::modelInfo() const { return d->modelInfo(); }

void BaiduVisionEngine::setGenerationResultCallback(ai_engine::gen_ai::vision::ImageGenerationCallback callback) {
    d->setGenerationResultCallback(callback);
}

ai_engine::result::AiEngineBaseResult BaiduVisionEngine::init() { return d->init(); }

ai_engine::result::AiEngineBaseResult BaiduVisionEngine::destroy() { return d->destroy(); }

void BaiduVisionEngine::setImageSize(int width, int height) { d->setImageSize(width, height); }

void BaiduVisionEngine::setImageNumber(int number) { d->setImageNumber(number); }

void BaiduVisionEngine::setImageStyle(const std::string &style) { d->setImageStyle(style); }

std::vector<std::string> BaiduVisionEngine::supportedImageStyles() const { return d->supportedImageStyles(); }

int BaiduVisionEngine::supportedImageNumber() const { return d->supportedImageNumber(); }

std::vector<std::pair<int, int>> BaiduVisionEngine::supportedImageSizes() const { return d->supportedImageSizes(); }

ai_engine::result::AiEngineBaseResult BaiduVisionEngine::generateImage(const std::string &prompt) {
    return d->generateImage(prompt);
}

void BaiduVisionEngine::stopImageGeneration() { return d->stopImageGeneration(); }

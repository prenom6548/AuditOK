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

#include "baidunlpengine.h"
#include "baidunlpengine_p.h"

BaiduNlpEngine::BaiduNlpEngine() : d(new BaiduNlpEnginePrivate()) {}

BaiduNlpEngine::~BaiduNlpEngine() { delete d; }

std::string BaiduNlpEngine::engineName() const { return d->engineName(); }

std::string BaiduNlpEngine::currentModel() const { return d->currentModel(); }

std::string BaiduNlpEngine::currentModelName() const { return d->currentModelName(); }

bool BaiduNlpEngine::isCloud() const { return d->isCloud(); }

bool BaiduNlpEngine::isBuiltInEngine() const { return d->isBuiltInEngine(); }

bool BaiduNlpEngine::isCustomModelEngine() const { return d->isCustomModelEngine(); }

void BaiduNlpEngine::setConfig(const std::string &config) { d->setConfig(config); }

bool BaiduNlpEngine::setCurrentModel(const std::string &model) { return d->setCurrentModel(model); }

std::string BaiduNlpEngine::modelInfo() const { return d->modelInfo(); }

void BaiduNlpEngine::setTextGenerationResultCallback(ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    d->setTextGenerationResultCallback(callback);
}

ai_engine::result::AiEngineBaseResult BaiduNlpEngine::generateContent(const std::string &input) {
    return d->generateContent(input);
};

ai_engine::result::AiEngineBaseResult BaiduNlpEngine::init() { return d->init(); }

ai_engine::result::AiEngineBaseResult BaiduNlpEngine::chat(const std::string &message) { return d->chat(message); }

void BaiduNlpEngine::stopChat() { d->stopChat(); }

void BaiduNlpEngine::stopGenerateContent() { d->stopGenerateContent(); }

ai_engine::result::AiEngineBaseResult BaiduNlpEngine::destroy() { return d->destroy(); }

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

#include "xunfeinlpengine.h"
#include "xunfeinlpengine_p.h"

XunfeiNlpEngine::XunfeiNlpEngine() : d(new XunfeiNlpEnginePrivate()) {}

XunfeiNlpEngine::~XunfeiNlpEngine() { delete d; }

std::string XunfeiNlpEngine::engineName() const { return d->engineName(); }

std::string XunfeiNlpEngine::currentModel() const { return d->currentModel(); }

std::string XunfeiNlpEngine::currentModelName() const { return d->currentModelName(); }

bool XunfeiNlpEngine::isCloud() const { return d->isCloud(); }

bool XunfeiNlpEngine::isBuiltInEngine() const { return d->isBuiltInEngine(); }

bool XunfeiNlpEngine::isCustomModelEngine() const { return d->isCustomModelEngine(); }

void XunfeiNlpEngine::setConfig(const std::string &config) { d->setConfig(config); }

bool XunfeiNlpEngine::setCurrentModel(const std::string &model) { return d->setCurrentModel(model); }

std::string XunfeiNlpEngine::modelInfo() const { return d->modelInfo(); }

void XunfeiNlpEngine::setTextGenerationResultCallback(ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    d->setTextGenerationResultCallback(callback);
}

ai_engine::result::AiEngineBaseResult XunfeiNlpEngine::init() { return d->init(); }

ai_engine::result::AiEngineBaseResult XunfeiNlpEngine::chat(const std::string &message) { return d->chat(message); }

ai_engine::result::AiEngineBaseResult XunfeiNlpEngine::generateContent(const std::string &input) {
    return d->generateContent(input);
};

void XunfeiNlpEngine::stopChat() { d->stopChat(); }

void XunfeiNlpEngine::stopGenerateContent() { d->stopGenerateContent(); }

ai_engine::result::AiEngineBaseResult XunfeiNlpEngine::destroy() { return d->destroy(); }

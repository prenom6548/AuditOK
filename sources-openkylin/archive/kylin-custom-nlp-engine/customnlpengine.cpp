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

#include "customnlpengine.h"

#include <customnlpengine_p.h>

CustomNlpEngine::CustomNlpEngine() : d(new CustomNlpEnginePrivate()) {}

CustomNlpEngine::~CustomNlpEngine() { delete d; }

std::string CustomNlpEngine::engineName() const { return d->engineName(); }

std::string CustomNlpEngine::currentModel() const { return d->currentModel(); }

std::string CustomNlpEngine::currentModelName() const { return d->currentModelName(); }

bool CustomNlpEngine::isCloud() const { return d->isCloud(); }

bool CustomNlpEngine::isBuiltInEngine() const { return d->isBuiltInEngine(); }

bool CustomNlpEngine::isCustomModelEngine() const { return d->isCustomModelEngine(); }

std::string CustomNlpEngine::modelInfo() const { return d->modelInfo(); }

void CustomNlpEngine::setConfig(const std::string &config) { d->setConfig(config); }

bool CustomNlpEngine::setCurrentModel(const std::string &model) { return d->setCurrentModel(model); }

void CustomNlpEngine::setTextGenerationResultCallback(ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    d->setTextGenerationResultCallback(callback);
}

ai_engine::result::AiEngineBaseResult CustomNlpEngine::init() { return d->init(); }

ai_engine::result::AiEngineBaseResult CustomNlpEngine::generateContent(const std::string &input) {
    return d->generateContent(input);
}

ai_engine::result::AiEngineBaseResult CustomNlpEngine::chat(const std::string &message) { return d->chat(message); }

void CustomNlpEngine::stopChat() { d->stopChat(); }

void CustomNlpEngine::stopGenerateContent() { d->stopGenerateContent(); }

ai_engine::result::AiEngineBaseResult CustomNlpEngine::destroy() { return d->destroy(); }

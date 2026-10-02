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

#include "qwennlpengine.h"

#include <qwennlpengine_p.h>

QwenNlpEngine::QwenNlpEngine() : d(new QwenNlpEnginePrivate()) {}

QwenNlpEngine::~QwenNlpEngine() = default;

std::string QwenNlpEngine::engineName() const { return d->engineName(); }

std::string QwenNlpEngine::currentModel() const { return d->currentModel(); }

std::string QwenNlpEngine::currentModelName() const { return d->currentModelName(); }

bool QwenNlpEngine::isCloud() const { return d->isCloud(); }

bool QwenNlpEngine::isBuiltInEngine() const { return d->isBuiltInEngine(); }
bool QwenNlpEngine::isCustomModelEngine() const { return d->isCustomModelEngine(); }

std::string QwenNlpEngine::modelInfo() const { return d->modelInfo(); }

void QwenNlpEngine::setConfig(const std::string &config) { d->setConfig(config); }

bool QwenNlpEngine::setCurrentModel(const std::string &model) { return d->setCurrentModel(model); }

void QwenNlpEngine::setTextGenerationResultCallback(ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    d->setTextGenerationResultCallback(callback);
}
ai_engine::result::AiEngineBaseResult QwenNlpEngine::generateContent(const std::string &input) {
    return d->generateContent(input);
}

ai_engine::result::AiEngineBaseResult QwenNlpEngine::init() { return d->init(); }

ai_engine::result::AiEngineBaseResult QwenNlpEngine::chat(const std::string &message) { return d->chat(message); }

void QwenNlpEngine::stopChat() { d->stopChat(); }
void QwenNlpEngine::stopGenerateContent() { d->stopGenerateContent(); }

ai_engine::result::AiEngineBaseResult QwenNlpEngine::destroy() { return d->destroy(); }

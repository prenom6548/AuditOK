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

#include "deepseeknlpengine.h"

#include <deepseeknlpengine_p.h>

DeepseekNlpEngine::DeepseekNlpEngine() : d(new DeepseekNlpEnginePrivate()) {}

DeepseekNlpEngine::~DeepseekNlpEngine() { delete d; };

std::string DeepseekNlpEngine::engineName() const { return d->engineName(); }

std::string DeepseekNlpEngine::currentModel() const { return d->currentModel(); }

std::string DeepseekNlpEngine::currentModelName() const { return d->currentModelName(); }

std::string DeepseekNlpEngine::modelInfo() const { return d->modelInfo(); }

bool DeepseekNlpEngine::setCurrentModel(const std::string &model) { return d->setCurrentModel(model); }

void DeepseekNlpEngine::setConfig(const std::string &config) { d->setConfig(config); }

void DeepseekNlpEngine::setTextGenerationResultCallback(ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    d->setTextGenerationResultCallback(callback);
}

ai_engine::result::AiEngineBaseResult DeepseekNlpEngine::init() { return d->init(); }

ai_engine::result::AiEngineBaseResult DeepseekNlpEngine::destroy() { return d->destroy(); }

ai_engine::result::AiEngineBaseResult DeepseekNlpEngine::generateContent(const std::string &input) {
    return d->generateContent(input);
}
ai_engine::result::AiEngineBaseResult DeepseekNlpEngine::chat(const std::string &message) { return d->chat(message); }

void DeepseekNlpEngine::stopGenerateContent() { d->stopGenerateContent(); }

void DeepseekNlpEngine::stopChat() { d->stopChat(); }

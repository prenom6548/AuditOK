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

#include "sensetimenlpengine.h"
#include <sensetimenlpengine_p.h>

SensetimeNlpEngine::SensetimeNlpEngine() : d(new SensetimeNlpEnginePrivate()) {}

SensetimeNlpEngine::~SensetimeNlpEngine() { delete d; }

std::string SensetimeNlpEngine::engineName() const { return d->engineName(); }

std::string SensetimeNlpEngine::currentModel() const {
    return d->currentModel();
}

bool SensetimeNlpEngine::isCloud() const { return d->isCloud(); }

bool SensetimeNlpEngine::isBuiltInEngine() const {
    return d->isBuiltInEngine();
}
bool SensetimeNlpEngine::isCustomModelEngine() const {
    return d->isCustomModelEngine();
}

std::string SensetimeNlpEngine::modelInfo() const { return d->modelInfo(); }

void SensetimeNlpEngine::setConfig(const std::string &config) {
    d->setConfig(config);
}

bool SensetimeNlpEngine::setCurrentModel(const std::string &model) {
    return d->setCurrentModel(model);
}

void SensetimeNlpEngine::setTextGenerationResultCallback(
    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    d->setTextGenerationResultCallback(callback);
}
ai_engine::result::AiEngineBaseResult
SensetimeNlpEngine::generateContent(const std::string &input) {
    return d->generateContent(input);
};

ai_engine::result::AiEngineBaseResult SensetimeNlpEngine::init() {
    return d->init();
}

ai_engine::result::AiEngineBaseResult
SensetimeNlpEngine::chat(const std::string &message) {
    return d->chat(message);
}

void SensetimeNlpEngine::stopChat() { d->stopChat(); }
void SensetimeNlpEngine::stopGenerateContent() { d->stopGenerateContent(); }

ai_engine::result::AiEngineBaseResult SensetimeNlpEngine::destroy() {
    return d->destroy();
}
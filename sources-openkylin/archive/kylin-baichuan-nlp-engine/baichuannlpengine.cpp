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

#include "baichuannlpengine.h"

#include "baichuannlpengine_p.h"

BaichuanNlpEngine::BaichuanNlpEngine() : d(new BaichuanNlpEnginePrivate()) {}

BaichuanNlpEngine::~BaichuanNlpEngine() { delete d; }

std::string BaichuanNlpEngine::engineName() const { return d->engineName(); }

std::string BaichuanNlpEngine::currentModel() const {
    return d->currentModel();
}

bool BaichuanNlpEngine::isCloud() const { return d->isCloud(); }

bool BaichuanNlpEngine::isBuiltInEngine() const { return d->isBuiltInEngine(); }
bool BaichuanNlpEngine::isCustomModelEngine() const {
    return d->isCustomModelEngine();
}

std::string BaichuanNlpEngine::modelInfo() const { return d->modelInfo(); }
void BaichuanNlpEngine::setConfig(const std::string &config) {
    d->setConfig(config);
}

bool BaichuanNlpEngine::setCurrentModel(const std::string &model) {
    return d->setCurrentModel(model);
}

void BaichuanNlpEngine::setTextGenerationResultCallback(
    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    d->setTextGenerationResultCallback(callback);
}
ai_engine::result::AiEngineBaseResult BaichuanNlpEngine::generateContent(
    const std::string &input) {
    return d->generateContent(input);
};

ai_engine::result::AiEngineBaseResult BaichuanNlpEngine::init() {
    return d->init();
}

ai_engine::result::AiEngineBaseResult BaichuanNlpEngine::chat(
    const std::string &message) {
    return d->chat(message);
}

void BaichuanNlpEngine::stopChat() { d->stopChat(); }
void BaichuanNlpEngine::stopGenerateContent() { d->stopGenerateContent(); }

ai_engine::result::AiEngineBaseResult BaichuanNlpEngine::destroy() {
    return d->destroy();
}

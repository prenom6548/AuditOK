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

#include "ondevicenlpengine.h"
#include "client/llmtritonclient.h"
#include "utils/logger.h"

#include <nlohmann/json.hpp>

namespace ai_engine::gen_ai::nlp {
OndeviceNlpEngine::OndeviceNlpEngine() {
    loggerNlp::init();
    llm_triton_client_ = new LlmTritonClient();
}

OndeviceNlpEngine::~OndeviceNlpEngine() {
    destroy();
    delete llm_triton_client_;
    llm_triton_client_ = nullptr;
};

void OndeviceNlpEngine::setTextGenerationResultCallback(
    nlp::TextGenerationResultCallback callback) {
    llm_triton_client_->setTextGenerationResultCallback(callback);
}

result::AiEngineBaseResult OndeviceNlpEngine::chat(const std::string &message) {
    return llm_triton_client_->chat(message);
}

void OndeviceNlpEngine::stopChat() {
    llm_triton_client_->stopChat();
    return;
}

result::AiEngineBaseResult OndeviceNlpEngine::init() {

    return llm_triton_client_->init();
}

result::AiEngineBaseResult OndeviceNlpEngine::destroy() {
    result::AiEngineBaseResult error;
    error.success = true;

    return error;
}

bool OndeviceNlpEngine::setCurrentModel(const std::string &model) {
    return llm_triton_client_->setCurrentModel(model);
}

std::string OndeviceNlpEngine::currentModel() const {
    return llm_triton_client_->getCurrentModel();
}

std::string OndeviceNlpEngine::currentModelName() const {
    return llm_triton_client_->getModelName();
}

std::string OndeviceNlpEngine::modelInfo() const {
    return llm_triton_client_->getModelInfo();
}

void OndeviceNlpEngine::setConfig(const std::string &config) {
    llm_triton_client_->setConfig(config);
    return;
}

} // namespace ai_engine::gen_ai::nlp

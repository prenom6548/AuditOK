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

#include "xunfeiengine.h"
#include <kylin-ai/plugins/ai-engines/xunfeispeechengine.h>
#include "kylin-ai/plugins/ai-engines/xunfeinlpengine.h"
#include "kylin-ai/plugins/ai-engines/xunfeivisionengine.h"

std::string XunfeiAiEngine::name() const { return "xunfei"; }

std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine>
XunfeiAiEngine::createSpeechEngine() {
    return std::make_unique<XunfeiSpeechEngine>();
}

std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine>
XunfeiAiEngine::createTextGenerationEngine() {
    return std::make_unique<XunfeiNlpEngine>();
}

std::unique_ptr<ai_engine::gen_ai::vision::AbstractImageGenerationEngine>
XunfeiAiEngine::createImageGenerationEngine() {
    return std::make_unique<XunfeiVisionEngine>();
}

AI_ENGINE_PLUGIN_FACTORY(XunfeiAiEngine);

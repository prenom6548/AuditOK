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

#include "ondeviceengine.h"

#include <kylin-ai/plugins/ai-engines/ondevicenlpengine.h>
#include <kylin-ai/plugins/ai-engines/ondevicespeechengine.h>
#include <kylin-ai/plugins/ai-engines/ondevicevisioncapabilityengine.h>

namespace ai_engine {

// std::vector<AiCapability> OndeviceAiEngine::aiCapabilities() const {
//     return {AiCapability::Nlp, AiCapability::Vision};
// }

// std::unique_ptr<nlp::AbstractNlpEngine> OndeviceAiEngine::createNlpEngine()
std::unique_ptr<gen_ai::nlp::AbstractTextGenerationEngine> OndeviceAiEngine::createTextGenerationEngine() {

    return std::unique_ptr<gen_ai::nlp::AbstractTextGenerationEngine>(new gen_ai::nlp::OndeviceNlpEngine());
}

std::unique_ptr<core_ai::speech::AbstractSpeechEngine> OndeviceAiEngine::createSpeechEngine() {
    return std::unique_ptr<core_ai::speech::AbstractSpeechEngine>(new core_ai::speech::OndeviceSpeechEngine());
}

// std::unique_ptr<vision::AbstractVisionEngine>
// OndeviceAiEngine::createVisionEngine() {
//     return std::unique_ptr<vision::AbstractVisionEngine>(new
//     vision::OndeviceVisionEngine());
// }

std::unique_ptr<core_ai::vision::AbstractTextRecognitionEngine> OndeviceAiEngine::createTextRecognitionEngine() {
    return std::unique_ptr<core_ai::vision::AbstractTextRecognitionEngine>(new core_ai::vision::OndeviceVisionTextRecognitionEngine());
}

std::unique_ptr<core_ai::vision::AbstractImageProcessEngine> OndeviceAiEngine::createImageProcessEngine() {
    return std::unique_ptr<core_ai::vision::AbstractImageProcessEngine>(new core_ai::vision::OndeviceVisionImageProcessEngine());
}

AI_ENGINE_PLUGIN_FACTORY(OndeviceAiEngine)

} // namespace ai_engine

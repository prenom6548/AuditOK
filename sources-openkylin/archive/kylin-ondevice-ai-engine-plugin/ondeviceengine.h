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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>
#include <memory>
#include <kylin-ai/ai-engine/aiengine.h>
#include <kylin-ai/ai-engine/aienginepluginfactory.h>

namespace ai_engine {

class OndeviceAiEngine : public ai_engine::AbstractAiEnginePluginFactory {
public:
    ~OndeviceAiEngine() = default;
    std::string name() const override { return "ondevice"; }
    // std::vector<AiCapability> aiCapabilities() const override;
    std::unique_ptr<gen_ai::nlp::AbstractTextGenerationEngine> createTextGenerationEngine() override;
    std::unique_ptr<core_ai::speech::AbstractSpeechEngine> createSpeechEngine() override;
    std::unique_ptr<core_ai::vision::AbstractTextRecognitionEngine> createTextRecognitionEngine() override;
    std::unique_ptr<core_ai::vision::AbstractImageProcessEngine> createImageProcessEngine() override;
    // std::unique_ptr<vision::AbstractVisionEngine> createVisionEngine() override;
};


} // namespace ai_engine

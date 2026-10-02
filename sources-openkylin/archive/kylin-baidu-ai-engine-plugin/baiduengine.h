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

#ifndef KYLIN_BAIDU_AI_ENGINE_PLUGIN_BAIDU_ENGINE
#define KYLIN_BAIDU_AI_ENGINE_PLUGIN_BAIDU_ENGINE

#include <kylin-ai/ai-engine/aienginepluginfactory.h>

class BaiduAiEngine : public ai_engine::AbstractAiEnginePluginFactory {
public:
    ~BaiduAiEngine() override = default;
    std::string name() const override;
    std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine>
    createTextGenerationEngine() override;
    std::unique_ptr<ai_engine::core_ai::speech::AbstractSpeechEngine>
    createSpeechEngine() override;
    std::unique_ptr<ai_engine::gen_ai::vision::AbstractImageGenerationEngine>
    createImageGenerationEngine() override;
};

#endif

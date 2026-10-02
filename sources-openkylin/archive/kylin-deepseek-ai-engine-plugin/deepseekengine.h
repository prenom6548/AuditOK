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

#include <kylin-ai/ai-engine/aienginepluginfactory.h>

class DeepseekAiEngine : public ai_engine::AbstractAiEnginePluginFactory {
public:
    ~DeepseekAiEngine() = default;
    std::string name() const override;
    std::unique_ptr<ai_engine::gen_ai::nlp::AbstractTextGenerationEngine>
    createTextGenerationEngine() override;
};

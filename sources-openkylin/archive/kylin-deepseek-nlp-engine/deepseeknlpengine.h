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

#ifndef DEEPSEEKNLPENGINE_H
#define DEEPSEEKNLPENGINE_H

#include <kylin-ai/ai-engine/textgenerationengine.h>

class DeepseekNlpEnginePrivate;

class DeepseekNlpEngine : public ai_engine::gen_ai::nlp::AbstractTextGenerationEngine {
public:
    DeepseekNlpEngine();
    ~DeepseekNlpEngine() override;

    std::string engineName() const override;
    bool isCloud() const override { return true; }
    bool isBuiltInEngine() const override { return false; }
    bool isCustomModelEngine() const override { return false; }

    std::string modelInfo() const override;

    bool setCurrentModel(const std::string &model) override;
    std::string currentModel() const override;
    std::string currentModelName() const override;

    void setConfig(const std::string &config) override;

    void setTextGenerationResultCallback(ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) override;

    ai_engine::result::AiEngineBaseResult init() override;
    ai_engine::result::AiEngineBaseResult destroy() override;

    ai_engine::result::AiEngineBaseResult generateContent(const std::string &input) override;

    ai_engine::result::AiEngineBaseResult chat(const std::string &message) override;

    void stopGenerateContent() override;
    void stopChat() override;

private:
    DeepseekNlpEnginePrivate *d;
};

#endif  // DEEPSEEKNLPENGINE_H

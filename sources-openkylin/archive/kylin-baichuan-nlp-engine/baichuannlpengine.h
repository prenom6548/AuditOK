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

#ifndef BAICHUANNLPENGINE_H
#define BAICHUANNLPENGINE_H

#include <kylin-ai/ai-engine/textgenerationengine.h>

class BaichuanNlpEnginePrivate;

class BaichuanNlpEngine
    : public ai_engine::gen_ai::nlp::AbstractTextGenerationEngine {
public:
    BaichuanNlpEngine();
    ~BaichuanNlpEngine();
    std::string engineName() const override;
    bool isCloud() const override;
    bool isBuiltInEngine() const override;
    bool isCustomModelEngine() const override;

    void setConfig(const std::string &config) override;
    bool setCurrentModel(const std::string &model) override;
    std::string currentModel() const override;
    std::string modelInfo() const override;

    void setTextGenerationResultCallback(
        ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) override;

    ai_engine::result::AiEngineBaseResult generateContent(
        const std::string &input) override;
    ai_engine::result::AiEngineBaseResult chat(
        const std::string &message) override;
    void stopGenerateContent() override;
    void stopChat() override;

    ai_engine::result::AiEngineBaseResult init() override;
    ai_engine::result::AiEngineBaseResult destroy() override;

private:
    BaichuanNlpEnginePrivate *d;
};

#endif  // BAICHUANNLPENGINE_H

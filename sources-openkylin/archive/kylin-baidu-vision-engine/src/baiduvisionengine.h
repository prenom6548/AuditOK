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

#ifndef BAIDUVISIONENGINE_H
#define BAIDUVISIONENGINE_H

#include <kylin-ai/ai-engine/imagegenerationengine.h>
class BaiduVisionEnginePrivate;

class BaiduVisionEngine : public ai_engine::gen_ai::vision::AbstractImageGenerationEngine {
public:
    BaiduVisionEngine();
    ~BaiduVisionEngine() override;
    std::string engineName() const override;
    std::string currentModel() const override;
    std::string currentModelName() const override;
    bool isCloud() const override;
    bool isBuiltInEngine() const override;
    bool isCustomModelEngine() const override;

    void setConfig(const std::string &config) override;
    bool setCurrentModel(const std::string &model) override;
    std::string modelInfo() const override;

    void setGenerationResultCallback(ai_engine::gen_ai::vision::ImageGenerationCallback) override;

    ai_engine::result::AiEngineBaseResult init() override;
    ai_engine::result::AiEngineBaseResult destroy() override;

    void setImageSize(int width, int height) override;
    void setImageNumber(int number) override;
    void setImageStyle(const std::string &style) override;

    std::vector<std::string> supportedImageStyles() const override;
    int supportedImageNumber() const override;
    std::vector<std::pair<int, int>> supportedImageSizes() const override;

    ai_engine::result::AiEngineBaseResult generateImage(const std::string &prompt) override;
    void stopImageGeneration() override;

private:
    BaiduVisionEnginePrivate *d;
};

#endif // BAIDUVISIONENGINE_H

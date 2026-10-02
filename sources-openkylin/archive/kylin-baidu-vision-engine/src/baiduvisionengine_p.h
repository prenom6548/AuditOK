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

#ifndef BAIDUVISIONENGINE_P_H
#define BAIDUVISIONENGINE_P_H

#include <atomic>
#include <functional>
#include <string>
#include <kylin-ai/ai-engine/aiengine.h>
#include <kylin-ai/ai-engine/aiengineerror.h>
#include <kylin-ai/ai-engine/imagegenerationengine.h>

class BaiduVisionEnginePrivate {
public:
    BaiduVisionEnginePrivate();
    ~BaiduVisionEnginePrivate();
    std::string engineName() const;
    bool isCloud() const { return true; }
    bool isBuiltInEngine() const { return false; }
    bool isCustomModelEngine() const { return true; }
    std::string currentModel() const;
    std::string currentModelName() const;

    void setConfig(const std::string &config);
    bool setCurrentModel(const std::string &model);
    std::string modelInfo() const;

    void setGenerationResultCallback(ai_engine::gen_ai::vision::ImageGenerationCallback callback);

    ai_engine::result::AiEngineBaseResult init() { return {true}; };
    ai_engine::result::AiEngineBaseResult destroy() { return {true}; };

    void setImageSize(int width, int height);
    void setImageNumber(int number);
    void setImageStyle(const std::string &style);

    std::vector<std::string> supportedImageStyles() const;
    int supportedImageNumber() const;
    std::vector<std::pair<int, int>> supportedImageSizes() const;

    ai_engine::result::AiEngineBaseResult generateImage(const std::string &prompt);

    void stopImageGeneration();

private:
    std::string apiKey_;
    std::string secretKey_;
    std::string accessToken_;
    std::string currentModel_{"百度-文心AI作画高级版"};
    std::string currentModelName_{"baidu-ernie-drawing-advanced"};
    std::string currentDisplayName_{"百度-文心AI作画高级版"};
    std::string &getAccessToken_();
    bool generateAccessToken_();
    void decreaseGenerationCounter();
    void increaseGenerationCounter();
    bool isGenerating() const;
    bool waitForStopped();

    ai_engine::gen_ai::vision::ImageGenerationCallback imageGenerationResultCallback_;

    int imageWidth_{1024};
    int imageHeight_{1024};
    int imageNumber_{1};
    std::string imageStyle_;
    std::atomic<bool> isStopped_{false};
    std::atomic<int> generationCounter_{0};

    std::string requestPrompt2ImageTask(const std::string &prompt);
    ai_engine::result::AiEngineBaseResult processPrompt2ImageTask(const std::string &taskId);
    std::string doRequestTaskResult(const std::string &taskId);

    void runCallbackWithError(const ai_engine::EngineError &error, const int &index, const int &total);
    ai_engine::EngineError currentError_;
};

#endif // BAIDUVISIONENGINE_P_H

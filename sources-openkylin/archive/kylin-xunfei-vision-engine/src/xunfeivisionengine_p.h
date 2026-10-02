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

#ifndef XUNFEIVISIONENGINE_P_H
#define XUNFEIVISIONENGINE_P_H

#include <atomic>
#include <functional>
#include <string>
#include <cpr/cpr.h>
#include <kylin-ai/ai-engine/aiengineerror.h>
#include <kylin-ai/ai-engine/imagegenerationengine.h>

class XunfeiVisionEnginePrivate {
public:
    XunfeiVisionEnginePrivate();
    ~XunfeiVisionEnginePrivate();
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
    std::string appId_;
    std::string apiKey_;
    std::string secretKey_;
    std::string currentModel_{"讯飞-图片生成"};
    std::string currentModelName_{"xunfei-spark-image-generation"};
    std::string currentDisplayName_{"讯飞-图片生成"};
    std::string hostUrl_{"https://spark-api.cn-huabei-1.xf-yun.com/v2.1/tti"};

    ai_engine::gen_ai::vision::ImageGenerationCallback imageGenerationResultCallback_;

    friend bool writePrompt2ImageResult(std::string data, intptr_t userData);
    ai_engine::result::AiEngineBaseResult processPrompt2ImageResult(cpr::Response &response);

    int imageWidth_{1024};
    int imageHeight_{1024};
    int imageNumber_{1};
    std::string imageStyle_;
    std::atomic<bool> isStopped_{false};
    bool isGenerating_{false};
    bool isCallbackCalled_{false};

    std::string incompleteResult_;

    ai_engine::EngineError currentError_;
    bool currentStatus();
    void runCallbackWithError(const ai_engine::EngineError &error, const int &index, const int &total);
};

#endif // XUNFEIVISIONENGINE_P_H

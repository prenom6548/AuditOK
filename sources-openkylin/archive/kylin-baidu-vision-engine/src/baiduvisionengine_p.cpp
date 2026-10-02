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

#include "baiduvisionengine_p.h"
#include <unistd.h>
#include <map>
#include <cpr/cpr.h>
#include <kylin-ai/common/error.h>
#include "baiduresponseprocessor.h"
#include "imageloader.h"
#include "logger.h"
#include "servererror.h"
#include "token.h"
#include "util.h"

namespace {
std::map<int, std::string> imageStyleMap = {
    {0, "探索无限"},    {1, "古风"},     {2, "二次元"},      {3, "写实风格"}, {4, "浮世绘"},      {5, "low poly"},
    {6, "未来主义"},    {7, "像素风格"}, {8, "概念艺术"},    {9, "赛博朋克"}, {10, "洛丽塔风格"}, {11, "巴洛克风格"},
    {12, "超现实主义"}, {13, "水彩画"},  {14, "蒸汽波艺术"}, {15, "油画"},    {16, "卡通画"},
};
}

BaiduVisionEnginePrivate::BaiduVisionEnginePrivate() {}

BaiduVisionEnginePrivate::~BaiduVisionEnginePrivate() {}

std::string BaiduVisionEnginePrivate::engineName() const { return std::string("baidu"); }

std::string BaiduVisionEnginePrivate::currentModel() const { return currentModel_; }

std::string BaiduVisionEnginePrivate::currentModelName() const {
    return currentModelName_;
}

bool BaiduVisionEnginePrivate::setCurrentModel(const std::string &model) {
    Json::Value modelInfoJson = baidu_vision_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            currentModel_ = model;
            return true;
        }
    }
    LOGE("Invalid current model name:", model);
    return false;
}

void BaiduVisionEnginePrivate::setConfig(const std::string &config) {
    Json::Value configJson = baidu_vision_util::formatJsonFromString(config);
    if (configJson.isNull() || !configJson.isMember("apiKey") || !configJson.isMember("secretKey") ||
        !configJson["apiKey"].isString() || !configJson["secretKey"].isString()) {
        LOGE("Invalid config for baidu vision engine:", config);
        return;
    }
    apiKey_ = configJson["apiKey"].asString();
    secretKey_ = configJson["secretKey"].asString();
}

std::string BaiduVisionEnginePrivate::modelInfo() const {
    const char *info = R"(
    {
        "vendor": "百度",
        "models": [
            {
                "modelName": "baidu-ernie-drawing-advanced",
                "displayName": "百度-文心AI作画高级版",
                "model": "百度-文心AI作画高级版",
                "brand": "ernie",
                "subConfig": [
                    {
                        "name": "文心AI作画高级版",
                        "config": [
                            {
                                "friendlyName": "APIKey",
                                "configName": "apiKey"
                            },
                            {
                                "friendlyName": "APISecret",
                                "configName": "secretKey"
                            }
                        ]
                    }
                ],
                "is3rdModel": true,
                "feedbackUrl": "https://cloud.baidu.com/doc/support/index.html"
            }
        ]
    }
    )";

    return info;
}

void BaiduVisionEnginePrivate::setGenerationResultCallback(
    ai_engine::gen_ai::vision::ImageGenerationCallback callback) {
    imageGenerationResultCallback_ = std::move(callback);
}

void BaiduVisionEnginePrivate::setImageSize(int width, int height) {
    auto isSupporedSize = [this](const int inputWidth, const int inputHeight) -> bool {
        for (const auto &item : supportedImageSizes()) {
            if (item.first == inputWidth && item.second == inputHeight) {
                return true;
            }
        }
        return false;
    };
    if (width <= 0 || height <= 0 || !isSupporedSize(width, height)) {
        currentError_ = ai_engine::EngineError(AI_COMMON_VISION_IMAGE_SIZE_INVALID, "Unsupported size of images");
        imageWidth_ = -1;
        imageHeight_ = -1;
        return;
    }
    imageWidth_ = width;
    imageHeight_ = height;
}

void BaiduVisionEnginePrivate::setImageNumber(int number) {
    if (number > supportedImageNumber() || number <= 0) {
        currentError_ = ai_engine::EngineError(AI_COMMON_VISION_IMAGE_NUMBER_INVALID, "Unsupported number of images");
        imageNumber_ = -1;
        return;
    }
    imageNumber_ = number;
}

void BaiduVisionEnginePrivate::setImageStyle(const std::string &style) { imageStyle_ = style; }

std::vector<std::string> BaiduVisionEnginePrivate::supportedImageStyles() const {
    return {"探索无限",   "古风",     "二次元",     "写实风格", "浮世绘",     "low poly",
            "未来主义",   "像素风格", "概念艺术",   "赛博朋克", "洛丽塔风格", "巴洛克风格",
            "超现实主义", "水彩画",   "蒸汽波艺术", "油画",     "卡通画"};
}

int BaiduVisionEnginePrivate::supportedImageNumber() const { return 8; }

std::vector<std::pair<int, int>> BaiduVisionEnginePrivate::supportedImageSizes() const {
    std::vector<std::pair<int, int>> result{{512, 512},  {640, 360},   {360, 640},   {1024, 1024}, {1280, 720},
                                            {720, 1280}, {2048, 2048}, {2560, 1440}, {1440, 2560}};
    return result;
}

void BaiduVisionEnginePrivate::decreaseGenerationCounter() {
    if (generationCounter_ > 0) {
        --generationCounter_;
    }
}

void BaiduVisionEnginePrivate::increaseGenerationCounter() { ++generationCounter_; }

bool BaiduVisionEnginePrivate::isGenerating() const { return generationCounter_ > 0; }

void BaiduVisionEnginePrivate::stopImageGeneration() {
    LOGD("BaiduVisionEnginePrivate::stopImageGeneration");
    if (!isGenerating()) {
        LOGI("BaiduVisionEnginePrivate::stopImageGeneration: imageGeneration is not running");
        return;
    }
    decreaseGenerationCounter();
    isStopped_ = true;

    ai_engine::EngineError error(AI_COMMON_TASK_CANCELLED, "The image generation task is cancelled.");
    ai_engine::gen_ai::vision::ImageGenerationResult data{true, error, currentDisplayName_, 0, 0, 1, 1, "", ""};
    imageGenerationResultCallback_(data);
}

ai_engine::result::AiEngineBaseResult BaiduVisionEnginePrivate::generateImage(const std::string &prompt) {
    LOGI("BaiduVisionEnginePrivate::generateImage");
    if (!imageGenerationResultCallback_) {
        return {false, currentError_};
    }
    if (imageNumber_ == -1 || imageHeight_ == -1 || imageWidth_ == -1) {
        return {false, currentError_};
    }
    if (prompt.empty()) {
        currentError_ = ai_engine::EngineError(AI_COMMON_VISION_INPUT_TEXT_LENGTH_INVALID, "The prompt is empty");
        return {false, currentError_};
    }
    increaseGenerationCounter();

    if (!waitForStopped()) {
        isStopped_ = false;
    }

    if (!isGenerating()) {
        LOGI("BaiduVisionEnginePrivate::generateImage: no generation is running");
        return {true, ai_engine::EngineError{}};
    }

    if (!generateAccessToken_()) {
        if (isStopped_) {
            isStopped_ = false;
            return {true, ai_engine::EngineError{}};
        }
        decreaseGenerationCounter();
        return {false, currentError_};
    }
    std::string taskId = requestPrompt2ImageTask(prompt);
    if (taskId.empty()) {
        if (isStopped_) {
            isStopped_ = false;
            return {true, ai_engine::EngineError{}};
        }
        decreaseGenerationCounter();
        return {false, currentError_};
    }
    return processPrompt2ImageTask(taskId);
}

bool BaiduVisionEnginePrivate::waitForStopped() {
    auto startTime = std::chrono::steady_clock::now();
    while (isStopped_) {
        if (std::chrono::steady_clock::now() - startTime > std::chrono::seconds(10)) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

std::string &BaiduVisionEnginePrivate::getAccessToken_() {
    if (accessToken_.empty()) {
        generateAccessToken_();
    }
    return accessToken_;
}

bool BaiduVisionEnginePrivate::generateAccessToken_() {
    int errorCode{int(cpr::ErrorCode::OK)};
    std::string errorMessage;
    accessToken_ = baidu_vision_token::getBaiduToken(apiKey_, secretKey_, errorCode, errorMessage);
    if (accessToken_.empty()) {
        if (errorCode != int(cpr::ErrorCode::OK)) {
            if (errorCode == int(cpr::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR)) {
                currentError_ = ai_engine::EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, errorMessage);
            } else {
                currentError_ = ai_engine::EngineError(AI_COMMON_NET_ERROR, errorMessage);
            }
        } else {
            currentError_ = ai_engine::EngineError(AI_COMMON_AUTHENTICATION_FAILURE, errorMessage);
        }
        LOGE("BaiduVisionEnginePrivate: Get baidu token failed:", errorMessage);
        return false;
    }
    return true;
}

std::string BaiduVisionEnginePrivate::requestPrompt2ImageTask(const std::string &prompt) {
    const std::string postUrl{"https://aip.baidubce.com/rpc/2.0/ernievilg/v1/txt2imgv2?access_token=" +
                              getAccessToken_()};

    Json::Value postJson;
    if (imageStyle_.empty()) {
        postJson["prompt"] = std::string(prompt);
    } else if (imageStyle_ == "low poly") {
        postJson["prompt"] = std::string(prompt) + "，" + "低多边形";
    } else {
        postJson["prompt"] = std::string(prompt) + "，" + imageStyle_;
    }
    postJson["width"] = imageWidth_;
    postJson["height"] = imageHeight_;
    postJson["image_num"] = imageNumber_;
    std::string postString = postJson.toStyledString();

    cpr::Url url{postUrl};
    cpr::Header header{{"Content-Type", "application/json"}, {"Accept", "application/json"}};
    cpr::Body body{postString};

    auto future = cpr::PostAsync(url, header, body);

    while (future.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready) {
        if (isStopped_) {
            return std::string();
        }
    }

    cpr::Response r = future.get();

    if (r.error) {
        LOGE("net error:", r.error.message);
        currentError_ = ai_engine::EngineError(AI_COMMON_NET_ERROR, r.error.message);
        return std::string();
    }

    if (int errorCode = baidu_vision_server_error::parseErrorCode(r.text)) {
        LOGE("baidu request text2image task failed:", r.text);
        int convertedErrorCode = baidu_vision_server_error::convertErrorCode(errorCode);
        currentError_ = ai_engine::EngineError(convertedErrorCode, r.text);
        return std::string{};
    }

    Json::Value result = baidu_vision_util::formatJsonFromString(r.text);

    if (baidu_vision_token::isBaiduTokenExpiredByResult(result)) {
        generateAccessToken_();
        return requestPrompt2ImageTask(prompt);
    }

    return result["data"]["task_id"].asString();
}

ai_engine::result::AiEngineBaseResult BaiduVisionEnginePrivate::processPrompt2ImageTask(const std::string &taskId) {
    while (1) {
        const std::string taskResult = doRequestTaskResult(taskId);
        if (taskResult.empty()) {
            if (isStopped_ == true) {
                isStopped_ = false;
                return {true, ai_engine::EngineError{}};
            }
            decreaseGenerationCounter();
            runCallbackWithError(currentError_, imageNumber_, imageNumber_);
            return {false, currentError_};
        }

        ImageDataList list;
        bool isTaskBlocked = false;
        std::string errorMessage;
        ProcessResult processCode = BaiduResponseProcessor::process(taskResult, list, isTaskBlocked, errorMessage);

        if (processCode == ProcessResult::SUCCEEDED) {
            for (auto &imageData : list) {
                if (isStopped_ == true) {
                    isStopped_ = false;
                    return {true, ai_engine::EngineError{}};
                }
                auto imageInfo =
                    vision::image_loader::getImageInfoFromData(imageData.data_.data(), imageData.data_.size());
                std::vector<char> charDataVector;
                for (unsigned char value : imageData.data_) {
                    charDataVector.push_back(static_cast<char>(value));
                }
                std::string imageDataString{imageData.data_.begin(), imageData.data_.end()};
                ai_engine::gen_ai::vision::ImageGenerationResult data{true,
                                                                      ai_engine::EngineError{},
                                                                      currentDisplayName_,
                                                                      imageInfo.width,
                                                                      imageInfo.height,
                                                                      (int)imageData.index_,
                                                                      (int)list.size(),
                                                                      imageInfo.format,
                                                                      baidu_vision_util::base64Encode(imageDataString)};
                if (imageData.conclusion_ == ApproveConclusion::BLOCK) {
                    data.success = false;
                    data.error = ai_engine::EngineError(AI_COMMON_VISION_IMAGE_GENERATION_BLOCKED, errorMessage);
                    fprintf(stderr, "baidu image censorship failed: "
                                    "imageData.conclusion_ is block\n");
                } else {
                    ai_engine::EngineError error;
                    data.error = error;
                }
                imageGenerationResultCallback_(data);
            }
            decreaseGenerationCounter();
            return {true, ai_engine::EngineError{}};
        }

        if (processCode == ProcessResult::FAILED) {
            if (isStopped_ == true) {
                isStopped_ = false;
                return {true, ai_engine::EngineError{}};
            }
            decreaseGenerationCounter();
            if (isTaskBlocked) {
                currentError_ = ai_engine::EngineError(AI_COMMON_VISION_IMAGE_GENERATION_BLOCKED, errorMessage);
            } else {
                currentError_ = ai_engine::EngineError(AI_COMMON_VISION_IMAGE_GENERATION_FAILED, errorMessage);
            }
            runCallbackWithError(currentError_, imageNumber_, imageNumber_);
            return {false, currentError_};
        }
        sleep(1);
    }
}

std::string BaiduVisionEnginePrivate::doRequestTaskResult(const std::string &taskId) {
    const std::string postUrl{"https://aip.baidubce.com/rpc/2.0/ernievilg/v1/getImgv2?access_token=" +
                              getAccessToken_()};

    char postData[1024];
    snprintf(postData, sizeof(postData), "{\"task_id\":\"%s\"}", taskId.c_str());

    cpr::Url url{postUrl};
    cpr::Header header{{"Content-Type", "application/json"}, {"Accept", "application/json"}};
    cpr::Body body{postData};
    auto future = cpr::PostAsync(url, header, body, cpr::ConnectTimeout{10000});

    while (future.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready) {
        if (isStopped_) {
            return std::string();
        }
    }

    cpr::Response r = future.get();

    if (r.error) {
        LOGE("net error:", r.error.message);
        currentError_ = ai_engine::EngineError(AI_COMMON_NET_ERROR, r.error.message);
        return std::string();
    }

    if (baidu_vision_token::isBaiduTokenExpiredByResult(r.text)) {
        generateAccessToken_();
        return doRequestTaskResult(taskId);
    }

    if (int errorCode = baidu_vision_server_error::parseErrorCode(r.text)) {
        LOGE("baidu request text2image result failed:", r.text);
        int convertedErrorCode = baidu_vision_server_error::convertErrorCode(errorCode);
        currentError_ = ai_engine::EngineError(convertedErrorCode, r.text);
        return std::string{};
    }

    return r.text;
}

void BaiduVisionEnginePrivate::runCallbackWithError(const ai_engine::EngineError &error, const int &index,
                                                    const int &total) {
    int errorValue = 0;
    if (index == total) {
        for (int i = 0; i < total; i++) {
            ai_engine::gen_ai::vision::ImageGenerationResult data{
                false, error, currentDisplayName_, errorValue, errorValue, i, total, "", ""};
            imageGenerationResultCallback_(data);
        }
    } else {
        ai_engine::gen_ai::vision::ImageGenerationResult data{
            false, error, currentDisplayName_, errorValue, errorValue, index, total, "", ""};
        imageGenerationResultCallback_(data);
    }
}

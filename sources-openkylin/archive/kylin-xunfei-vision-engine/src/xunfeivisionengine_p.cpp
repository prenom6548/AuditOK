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

#include "xunfeivisionengine_p.h"
#include <unistd.h>
#include <map>

#include <kylin-ai/common/error.h>

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

XunfeiVisionEnginePrivate::XunfeiVisionEnginePrivate() {}

XunfeiVisionEnginePrivate::~XunfeiVisionEnginePrivate() {}

std::string XunfeiVisionEnginePrivate::engineName() const { return std::string("xunfei"); }

std::string XunfeiVisionEnginePrivate::currentModel() const { return currentModel_; }

std::string XunfeiVisionEnginePrivate::currentModelName() const { return currentModelName_; }

bool XunfeiVisionEnginePrivate::setCurrentModel(const std::string &model) {
    Json::Value modelInfoJson = xunfei_vision_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            currentModel_ = model;
            return true;
        }
    }
    LOGE("Invalid current model name:", model);
    return false;
}

void XunfeiVisionEnginePrivate::setConfig(const std::string &config) {
    Json::Value configJson = xunfei_vision_util::formatJsonFromString(config);
    if (configJson.isNull() || !configJson.isMember("apiKey") || !configJson.isMember("secretKey") ||
        !configJson.isMember("appId") || !configJson["apiKey"].isString() || !configJson["secretKey"].isString() ||
        !configJson["appId"].isString()) {
        LOGE("Invalid config for xunfei vision engine:", config);
        return;
    }
    appId_ = configJson["appId"].asString();
    apiKey_ = configJson["apiKey"].asString();
    secretKey_ = configJson["secretKey"].asString();
}

std::string XunfeiVisionEnginePrivate::modelInfo() const {
    const char *info = R"(
    {
        "vendor": "讯飞",
        "models": [
            {
                "modelName": "xunfei-spark-image-generation",
                "displayName": "讯飞-图片生成",
                "model": "讯飞-图片生成",
                "brand": "spark",
                "subConfig": [
                    {
                        "name": "图片生成",
                        "config": [
                            {
                                "friendlyName": "APPID",
                                "configName": "appId"
                            },
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
                "feedbackUrl": "https://console.xfyun.cn/workorder/commit"
            }
        ]
    }
    )";

    return info;
}

void XunfeiVisionEnginePrivate::setGenerationResultCallback(
    ai_engine::gen_ai::vision::ImageGenerationCallback callback) {
    imageGenerationResultCallback_ = std::move(callback);
}

void XunfeiVisionEnginePrivate::setImageSize(int width, int height) {
    auto isSupportedSize = [this](const int inputWidth, const int inputHeight) -> bool {
        for (const auto &item : supportedImageSizes()) {
            if (item.first == inputWidth && item.second == inputHeight) {
                return true;
            }
        }
        return false;
    };
    if (width <= 0 || height <= 0 || !isSupportedSize(width, height)) {
        currentError_ = ai_engine::EngineError(AI_COMMON_VISION_IMAGE_SIZE_INVALID, "Unsupported size of images");
        imageWidth_ = -1;
        imageHeight_ = -1;
        return;
    }
    imageWidth_ = width;
    imageHeight_ = height;
}

void XunfeiVisionEnginePrivate::setImageNumber(int number) {
    if (number > supportedImageNumber() || number <= 0) {
        currentError_ = ai_engine::EngineError(AI_COMMON_VISION_IMAGE_NUMBER_INVALID, "Unsupported number of images");
        imageNumber_ = -1;
        return;
    }
    imageNumber_ = number;
}

void XunfeiVisionEnginePrivate::setImageStyle(const std::string &style) { imageStyle_ = style; }

std::vector<std::string> XunfeiVisionEnginePrivate::supportedImageStyles() const {
    return {"探索无限",   "古风",     "二次元",     "写实风格", "浮世绘",     "low poly",
            "未来主义",   "像素风格", "概念艺术",   "赛博朋克", "洛丽塔风格", "巴洛克风格",
            "超现实主义", "水彩画",   "蒸汽波艺术", "油画",     "卡通画"};
}

int XunfeiVisionEnginePrivate::supportedImageNumber() const { return 1; }

std::vector<std::pair<int, int>> XunfeiVisionEnginePrivate::supportedImageSizes() const {
    std::vector<std::pair<int, int>> result{{512, 512}, {640, 360}, {640, 480},  {640, 640},  {680, 512},
                                            {512, 680}, {768, 768}, {720, 1280}, {1280, 720}, {1024, 1024}};
    return result;
}

void XunfeiVisionEnginePrivate::stopImageGeneration() {
    if (isStopped_.exchange(true)) {
        return;
    }
    if (isGenerating_) {
        ai_engine::gen_ai::vision::ImageGenerationResult data{true, currentError_, currentDisplayName_, 0, 0, 1, 1, "",
                                                              ""};
        imageGenerationResultCallback_(data);
        isGenerating_ = false;
    }
}

bool writePrompt2ImageResult(std::string data, intptr_t userData) {
    XunfeiVisionEnginePrivate *self = (XunfeiVisionEnginePrivate *)userData;
    if (self->isStopped_) {
        return false;
    }
    std::string resultString = self->incompleteResult_.data() + data;
    Json::Value resultJson = xunfei_vision_util::formatJsonFromString(resultString);
    if (resultJson.isNull() || !resultJson.isObject()) {
        self->incompleteResult_ += data;
        return true;
    }

    if (int errorCode = xunfei_vision_server_error::parseErrorCode(data)) {
        LOGE("xunfei text2image failed:", data);
        int convertedErrorCode = xunfei_vision_server_error::convertErrorCode(errorCode);
        self->currentError_ = ai_engine::EngineError(convertedErrorCode, data);
        self->runCallbackWithError(self->currentError_, self->imageNumber_, self->imageNumber_);
        self->isCallbackCalled_ = true;
        return false;
    }
    if (resultJson.isMember("message")) {
        int errorCode{};
        // 没有错误码，只能通过错误信息判断
        if (resultString.find("a valid date or x-date header is required") != std::string::npos) {
            errorCode = AI_COMMON_SSL_CERTIFICATE_ERROR;
        } else {
            errorCode = AI_COMMON_AUTHENTICATION_FAILURE;
        }
        self->currentError_ = ai_engine::EngineError(errorCode, resultJson["message"].asString());

        return false;
    }

    if (resultString.substr(resultString.length() - 3) != "}}}") {
        self->incompleteResult_ += data;
        self->currentError_ = ai_engine::EngineError(AI_COMMON_VISION_IMAGE_GENERATION_FAILED, "Incomplete image data");
        return true;
    }
    self->incompleteResult_.clear();

    ai_engine::gen_ai::vision::ImageGenerationResult imageData;
    for (int i = 0; i < resultJson["payload"]["choices"]["text"].size(); i++) {
        auto imageDataString =
            xunfei_vision_util::base64Decode(resultJson["payload"]["choices"]["text"][i]["content"].asString());
        auto imageInfo = vision::image_loader::getImageInfoFromData(
            reinterpret_cast<unsigned char *>(imageDataString.data()), imageDataString.size());
        ai_engine::EngineError error;
        imageData.success = true;
        imageData.error = error;
        imageData.modelName = self->currentDisplayName_;
        imageData.width = imageInfo.width;
        imageData.height = imageInfo.height;
        imageData.index = i;
        imageData.total = resultJson["payload"]["choices"]["text"].size();
        imageData.format = imageInfo.format;
        imageData.imageBase64Data = xunfei_vision_util::base64Encode(imageDataString);
    }
    self->imageGenerationResultCallback_(imageData);
    self->isGenerating_ = false;
    self->currentError_ = ai_engine::EngineError();
    return true;
}

ai_engine::result::AiEngineBaseResult XunfeiVisionEnginePrivate::generateImage(const std::string &prompt) {
    incompleteResult_.clear();
    isStopped_ = false;
    isCallbackCalled_ = false;

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
    isGenerating_ = true;
    Json::Value bodyJson;
    bodyJson["header"]["app_id"] = appId_;
    bodyJson["parameter"]["chat"]["domain"] = "general";
    bodyJson["parameter"]["chat"]["width"] = imageWidth_;
    bodyJson["parameter"]["chat"]["height"] = imageHeight_;
    bodyJson["payload"]["message"]["text"][0]["role"] = "user";
    if (imageStyle_.empty()) {
        bodyJson["payload"]["message"]["text"][0]["content"] = std::string(prompt);
    } else if (imageStyle_ == "low poly") {
        bodyJson["payload"]["message"]["text"][0]["content"] = std::string(prompt) + "，" + "低多边形";
    } else {
        bodyJson["payload"]["message"]["text"][0]["content"] = std::string(prompt) + "，" + imageStyle_;
    }
    std::string bodyString = bodyJson.toStyledString();

    cpr::Url url{xunfei_vision_token::getAuthenticationUrl(apiKey_, secretKey_, hostUrl_)};
    cpr::Header header{cpr::Header{{"Content-Type", "application/json"}, {"charset", "UTF-8"}}};
    cpr::Body body{cpr::Body{{bodyString}}};
    cpr::LowSpeed lowSpeed{1, 10};
    cpr::Response response =
        cpr::Post(url, header, body, lowSpeed, cpr::WriteCallback{writePrompt2ImageResult, (intptr_t)this});

    return processPrompt2ImageResult(response);
}

ai_engine::result::AiEngineBaseResult XunfeiVisionEnginePrivate::processPrompt2ImageResult(cpr::Response &response) {
    if (isStopped_ || isCallbackCalled_) {
        isCallbackCalled_ = false;
        return {true, currentError_};
    }
    if (response.error.code == cpr::ErrorCode::REQUEST_CANCELLED || !currentStatus()) {
        return {false, currentError_};
    }

    if (response.error) {
        if (response.error.code == cpr::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR) {
            currentError_ = ai_engine::EngineError(AI_COMMON_SSL_CERTIFICATE_ERROR, response.error.message);
        } else {
            currentError_ = ai_engine::EngineError(AI_COMMON_NET_ERROR, response.error.message);
        }
        return {false, currentError_};
    }

    return {true, currentError_};
}

void XunfeiVisionEnginePrivate::runCallbackWithError(const ai_engine::EngineError &error, const int &index,
                                                     const int &total) {
    int errorValue = 0;
    if (index == total) {
        for (int i = 0; i < total; i++) {
            ai_engine::gen_ai::vision::ImageGenerationResult data{
                false, error, currentDisplayName_, errorValue, errorValue, i, total, "", ""};
            imageGenerationResultCallback_(data);
        }
        isGenerating_ = false;
    } else {
        ai_engine::gen_ai::vision::ImageGenerationResult data{
            false, error, currentDisplayName_, errorValue, errorValue, index, total, "", ""};
        imageGenerationResultCallback_(data);
        isGenerating_ = false;
    }
}

bool XunfeiVisionEnginePrivate::currentStatus() { return (currentError_.getCode() == -1); }

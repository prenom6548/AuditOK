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

#include "sensetimenlpengine_p.h"
#include <unistd.h>
#include <cstdio>
#include <string>
#include <curl/websockets.h>
#include <jsoncpp/json/value.h>
#include "servererror.h"
#include "token.h"
#include "util.h"

SensetimeNlpEnginePrivate::SensetimeNlpEnginePrivate() {}

SensetimeNlpEnginePrivate::~SensetimeNlpEnginePrivate() {}

std::string SensetimeNlpEnginePrivate::engineName() const {
    return "sensetime";
}

std::string SensetimeNlpEnginePrivate::modelInfo() const {
    const char *info = R"(
    {
        "vendor": "商汤",
        "models": [
            {
                "name": "商汤-SenseChat",
                "model": "SenseChat-5",
                "subConfig": [
                    {
                        "name": "SenseChat",
                        "config": [
                            {
                                "friendlyName": "APIKey",
                                "configName": "apiKey"
                            },
                            {
                                "friendlyName": "SecretKey",
                                "configName": "secretKey"
                            }
                        ]
                    }
                ],
                "maxTokens": 2048,
                "feedbackUrl": "https://www.sensecore.cn/"
            }
        ]
    }
    )";

    return info;
}

void SensetimeNlpEnginePrivate::setConfig(const std::string &config) {
    Json::Value configJson = sensetime_nlp_util::formatJsonFromString(config);
    if (configJson.isNull() || !configJson.isMember("apiKey") ||
        !configJson.isMember("secretKey") || !configJson["apiKey"].isString() ||
        !configJson["secretKey"].isString()) {
        std::fprintf(stderr, "Invalid config for sensetime chat engine: %s\n",
                     config.c_str());
        return;
    }
    apiKey_ = configJson["apiKey"].asString();
    secretKey_ = configJson["secretKey"].asString();
}

bool SensetimeNlpEnginePrivate::setCurrentModel(const std::string &model) {
    Json::Value modelInfoJson =
        sensetime_nlp_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            currentModel_ = model;
            return true;
        }
    }
    fprintf(stderr, "Invalid current model name: %s\n", model.c_str());
    return false;
}

void SensetimeNlpEnginePrivate::setTextGenerationResultCallback(
    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    textGenerationResultCallback_ = callback;
}

ai_engine::result::AiEngineBaseResult SensetimeNlpEnginePrivate::init() {
    currentError_ = ai_engine::EngineError();
    if (getAccessToken_().empty()) {
        return {false, currentError_};
    }

    context_["model"] = currentModel_;
    context_["stream"] = true;

    if (getMaxTokens(currentModel_) != -1) {
        context_["max_new_tokens"] = maxTokens_;
    }
    refreshAccessTokenCompleted_ = false;
    refreshAccessTokenRequired_ = false;
    return {true, ai_engine::EngineError()};
}

bool writeChatData(std::string data, intptr_t userData) {
    SensetimeNlpEnginePrivate *engine = (SensetimeNlpEnginePrivate *)userData;
    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback =
        engine->textGenerationResultCallback_;

    if (engine->isStopped_) {
        std::fprintf(stderr, "sensetime chat stopped\n");
        return false;
    }

    if (sensetime_nlp_token::isSensetimeTokenForbidden(data)) {
        if (!engine->refreshAccessTokenCompleted_) {
            engine->refreshAccessTokenRequired_ = true;
            return false;
        }
    }

    if (int errorCode = sensetime_nlp_server_error::parseErrorCode(data)) {
        fprintf(stderr, "sensetime chat failed: %s\n", data.c_str());
        engine->currentError_ = ai_engine::EngineError(
            (int)sensetime_nlp_server_error::errorCode2nlpResult(errorCode),
            data);
        return false;
    }

    for (const std::string &jsonString :
         sensetime_nlp_util::extractJsonStrings(data)) {
        // size是1
        Json::Value obj = sensetime_nlp_util::formatJsonFromString(jsonString);

        if (!(obj.isObject() && obj.isMember("data") &&
              obj["data"].isMember("choices") &&
              obj["data"]["choices"].isArray() &&
              obj["data"]["choices"].size() > 0 &&
              obj["data"]["choices"][0].isMember("delta") &&
              obj["data"]["choices"][0].isMember("finish_reason"))) {

            engine->currentError_ = ai_engine::EngineError(
                (int)ai_engine::NlpEngineErrorCode::ServerNoResponse, data);
        }

        obj = obj["data"]["choices"][0];
        bool isEnd = !obj["finish_reason"].asString().empty();

        Json::Value callbackObj;
        callbackObj["sentence_id"] = engine->getSentenceId_();
        callbackObj["is_end"] = isEnd;
        callbackObj["result"] = obj["delta"];

        if (isEnd) {
            engine->resetSentenceId_();
        }
        ai_engine::gen_ai::nlp::TextGenerationResult result{
            true, ai_engine::EngineError(), engine->currentModelName_,
            callbackObj.toStyledString()};
        engine->refreshAccessTokenCompleted_ = false;
        engine->refreshAccessTokenRequired_ = false;
        callback(result);
    }
    return true;
}

int SensetimeNlpEnginePrivate::getMaxTokens(const std::string &model) {
    Json::Value modelInfoJson =
        sensetime_nlp_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            maxTokens_ = item["maxTokens"].asInt();
            return maxTokens_;
        }
    }
    return -1;
}

void SensetimeNlpEnginePrivate::clearContext() {
    if (context_.isMember("messages")) {
        context_["messages"].clear();
    }
}

void SensetimeNlpEnginePrivate::addContext_(const std::string &role,
                                            const std::string &message) {
    if (role == "system") {
        if (!message.empty()) {
            context_["messages"][0]["role"] = role;
            context_["messages"][0]["content"] = message;
            return;
        }
        return;
    }
    int size = context_["messages"].size();
    context_["messages"][size]["role"] = role;
    context_["messages"][size]["content"] = message;
}

ai_engine::result::AiEngineBaseResult
SensetimeNlpEnginePrivate::processChatResponse_(cpr::Response &response) {
    if (isStopped_) {
        isStopped_ = false;
        return {true, ai_engine::EngineError()};
    }

    if (response.error.code == cpr::ErrorCode::REQUEST_CANCELLED) {
        ai_engine::EngineError error(
            (int)sensetime_nlp_server_error::errorCode2nlpResult(
                response.status_code),
            "Request cancelled");
        return {false, currentError_};
    }

    if (response.error) {
        fprintf(stderr, "net error: %s\n", response.error.message.c_str());
        currentError_ = ai_engine::EngineError(
            (int)ai_engine::NlpEngineErrorCode::NetworkDisconnected,
            "HTTP response error");
        return {false, currentError_};
    }
    return {true, currentError_};
}

ai_engine::result::AiEngineBaseResult
SensetimeNlpEnginePrivate::chat(const std::string &message) {
    isChatCall_ = true;
    currentError_ = ai_engine::EngineError();
    if (!textGenerationResultCallback_) {
        return {false, currentError_};
    }
    if (message.empty()) {
        currentError_ = ai_engine::EngineError(
            (int)ai_engine::NlpEngineErrorCode::TooLittleData,
            "The message is empty");
        return {false, currentError_};
    }

    clearContext();

    if (!waitForStopped()) {
        isStopped_ = false;
    }

    context_["model"] = currentModel_;
    Json::Value context = sensetime_nlp_util::formatJsonFromString(message);
    for (int i = 0; i < context["messages"].size(); i++) {
        addContext_(context["messages"][i]["role"].asString(),
                    context["messages"][i]["content"].asString());
    }
    std::string postString = context_.toStyledString();

    cpr::Response response =
        cpr::Post(cpr::Url{"https://api.sensenova.cn/v1/llm/chat-completions"},
                  cpr::Header{{"Content-Type", "application/json"},
                              {"Authorization", "Bearer " + getAccessToken_()}},
                  cpr::Body{cpr::Body{{postString}}},
                  cpr::WriteCallback{writeChatData, (intptr_t)this},
                  cpr::LowSpeed(15, 20));

    if (refreshAccessTokenRequired_) {
        generateAccessToken_();
        refreshAccessTokenRequired_ = false;
        refreshAccessTokenCompleted_ = true;
        return chat(message);
    }

    return processChatResponse_(response);
}

ai_engine::result::AiEngineBaseResult
SensetimeNlpEnginePrivate::generateContent(const std::string &input) {
    isStopped_ = false;
    currentError_ = ai_engine::EngineError();
    if (!textGenerationResultCallback_) {
        return {false, currentError_};
    }
    if (input.empty()) {
        currentError_ = ai_engine::EngineError(
            (int)ai_engine::NlpEngineErrorCode::TooLittleData,
            "The message is empty");
        return {false, currentError_};
    }

    context_["model"] = "SenseChat-5";

    Json::Value context = sensetime_nlp_util::formatJsonFromString(input);
    context_["prompt"] = context["prompt"].asString();
    std::string postString = context_.toStyledString();

    cpr::Response response =
        cpr::Post(cpr::Url{"https://api.sensenova.cn/v1/llm/completions"},
                  cpr::Header{{"Content-Type", "application/json"},
                              {"Authorization", "Bearer " + getAccessToken_()}},
                  cpr::Body{cpr::Body{{postString}}},
                  cpr::WriteCallback{writeChatData, (intptr_t)this});

    if (refreshAccessTokenRequired_) {
        generateAccessToken_();
        refreshAccessTokenRequired_ = false;
        refreshAccessTokenCompleted_ = true;
        return generateContent(input);
    }

    return processChatResponse_(response);
}

bool SensetimeNlpEnginePrivate::waitForStopped() {
    auto startTime = std::chrono::steady_clock::now();
    while (isStopped_) {
        if (std::chrono::steady_clock::now() - startTime >
            std::chrono::seconds(10)) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

void SensetimeNlpEnginePrivate::stopChat() {
    if (isStopped_.exchange(true)) {
        return;
    }
    if (isChatCall_) {
        isChatCall_ = false;
        ai_engine::gen_ai::nlp::TextGenerationResult result{
            true, currentError_, currentModelName_,
            std::string{R"({"result":"","is_end":true})"}};

        if (textGenerationResultCallback_ != nullptr) {
            textGenerationResultCallback_(result);
        }
    }
}

void SensetimeNlpEnginePrivate::stopGenerateContent() { isStopped_ = true; }

ai_engine::result::AiEngineBaseResult SensetimeNlpEnginePrivate::destroy() {
    stopChat();
    return {true, ai_engine::EngineError()};
}

std::string &SensetimeNlpEnginePrivate::getAccessToken_() {
    if (accessToken_.empty()) {
        generateAccessToken_();
    }
    return accessToken_;
}

bool SensetimeNlpEnginePrivate::generateAccessToken_() {
    std::string errorMessage;
    accessToken_ = sensetime_nlp_token::getSensetimeToken(apiKey_, secretKey_,
                                                          errorMessage);
    if (accessToken_.empty()) {
        currentError_ = ai_engine::EngineError(
            (int)ai_engine::NlpEngineErrorCode::Unauthorized, errorMessage);

        std::fprintf(stderr, "get sensetime token failed\n");
        return false;
    }
    return true;
}
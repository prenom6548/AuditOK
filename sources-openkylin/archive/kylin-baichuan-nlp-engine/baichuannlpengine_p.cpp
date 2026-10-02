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

#include "baichuannlpengine_p.h"

#include <cstdio>

#include "servererror.h"
#include "util.h"

BaichuanNlpEnginePrivate::BaichuanNlpEnginePrivate() {}

BaichuanNlpEnginePrivate::~BaichuanNlpEnginePrivate() {
    streamChatData_.clear();
}

std::string BaichuanNlpEnginePrivate::engineName() const { return "baichuan"; }

std::string BaichuanNlpEnginePrivate::modelInfo() const {
    const char *info = R"(
    {
        "vendor": "百川",
        "models": [
            {
                "name": "百川-Turbo-3",
                "model": "baichuan3-turbo",
                "subConfig": [
                    {
                        "name": "Turbo-3",
                        "config": [
                            {
                                "friendlyName": "APIKey",
                                "configName": "apiKey"
                            }
                        ]
                    }
                ],
                "maxTokens": 2048,
                "feedbackUrl": "https://platform.baichuan-ai.com/homePage"
            }
        ]
    }
    )";

    return info;
}

void BaichuanNlpEnginePrivate::setConfig(const std::string &config) {
    Json::Value configJson = baichuan_nlp_util::formatJsonFromString(config);
    if (configJson.isNull() || !configJson.isMember("apiKey") ||
        !configJson["apiKey"].isString()) {
        std::fprintf(stderr, "Invalid config for baichuan nlp engine: %s\n",
                     config.c_str());
        return;
    }
    apiKey_ = configJson["apiKey"].asString();
}

bool BaichuanNlpEnginePrivate::setCurrentModel(const std::string &model) {
    Json::Value modelInfoJson =
        baichuan_nlp_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            currentModel_ = model;
            return true;
        }
    }
    fprintf(stderr, "Invalid current model name: %s\n", model.c_str());
    return false;
}

void BaichuanNlpEnginePrivate::setTextGenerationResultCallback(
    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    textGenerationResultCallback_ = callback;
}

namespace {
std::vector<std::string> splitDataString(const std::string &dataStr) {
    std::vector<std::string> datas;

    int oldIndex = 0;
    int newIndex = 0;
    while ((newIndex = dataStr.find_first_of("\n\n", newIndex)) !=
           std::string::npos) {
        datas.push_back(dataStr.substr(oldIndex, newIndex - oldIndex));
        newIndex += 2;
        oldIndex = newIndex;
    }
    return datas;
}
}  // namespace

bool writeChatData(std::string data, intptr_t userData) {
    BaichuanNlpEnginePrivate *engine = (BaichuanNlpEnginePrivate *)userData;
    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback =
        engine->textGenerationResultCallback_;

    if (engine->isStopped_) {
        return false;
    }

    Json::Value resultJson = baichuan_nlp_util::formatJsonFromString(data);

    if (resultJson.isObject() && !resultJson.isNull() &&
        resultJson.isMember("error")) {
        fprintf(stderr, "baichuan chat failed: %s\n", data.c_str());
        engine->currentErrorInfo_ = data;
        return false;
    }

    std::string resultString = engine->tempStreamChatData_.data() + data;
    if (resultString.substr(resultString.length() - 2) != "\n\n") {
        engine->tempStreamChatData_ += data;
        return true;
    }
    engine->tempStreamChatData_.clear();

    std::vector<std::string> dataStrings = splitDataString(resultString);
    for (std::string data : dataStrings) {
        data = data.substr(6, data.length() - 1);
        if (data == "[DONE]") {
            break;
        }
        Json::Value obj = baichuan_nlp_util::formatJsonFromString(data);
        Json::Value callbackObj;
        callbackObj["is_end"] = obj.isMember("usage");
        callbackObj["result"] = obj["choices"][0]["delta"]["content"];
        callbackObj["sentence_id"] = engine->sentenceId_;
        engine->sentenceId_ += 1;

        engine->streamChatData_.push_back(callbackObj["result"]);
        ai_engine::gen_ai::nlp::TextGenerationResult result{
            true, ai_engine::EngineError(), engine->currentModelName_,
            callbackObj.toStyledString()};
        callback(result);
    }
    return true;
}

int BaichuanNlpEnginePrivate::getMaxTokens(const std::string &model) {
    Json::Value modelInfoJson =
        baichuan_nlp_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            maxTokens_ = item["maxTokens"].asInt();
            return maxTokens_;
        }
    }
    return -1;
}

void BaichuanNlpEnginePrivate::clearContext() {
    if (context_.isMember("messages")) {
        context_["messages"].clear();
    }
}

void BaichuanNlpEnginePrivate::addContext_(const std::string &role,
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

void BaichuanNlpEnginePrivate::removeLastContext_() {
    int size = context_["messages"].size();
    if (size - 1 >= 0 &&
        context_["messages"][size - 1]["role"].asString() == "user") {
        Json::Value removeValue;
        context_["messages"].removeIndex(size - 1, &removeValue);
        return;
    }
    if ((size - 1) >= 0 && (size - 2) >= 0 &&
        context_["messages"][size - 1]["role"].asString() == "assistant" &&
        context_["messages"][size - 2]["role"].asString() == "user") {
        Json::Value removeValue;
        context_["messages"].removeIndex(size - 1, &removeValue);
        context_["messages"].removeIndex(size - 2, &removeValue);
        return;
    }
}

ai_engine::result::AiEngineBaseResult BaichuanNlpEnginePrivate::init() {
    currentError_ = ai_engine::EngineError();

    context_["model"] = currentModel_;
    if (getMaxTokens(currentModel_) != -1) {
        context_["max_tokens"] = maxTokens_;
    }
    context_["stream"] = true;
    context_["tools"] = Json::Value(Json::arrayValue);
    Json::Value tool;
    tool["type"] = "web_search";
    Json::Value webSearch;
    webSearch["enable"] = true;
    webSearch["search_mode"] = "quality_first";
    tool["web_search"] = webSearch;
    context_["tools"].append(tool);

    cpr::Url url{"https://api.baichuan-ai.com/v1/chat/completions"};
    cpr::Header header{cpr::Header{{"Content-Type", "application/json"},
                                   {"Authorization", "Bearer " + apiKey_}}};
    std::string postString = context_.toStyledString();
    cpr::Body body{postString};
    cpr::Response response = cpr::Post(url, header, body);
    if (response.error) {
        currentError_ = ai_engine::EngineError(
            (int)ai_engine::NlpEngineErrorCode::NetworkDisconnected,
            "HTTP response error");
        return {false, currentError_};
    }
    return {true, ai_engine::EngineError()};
}
ai_engine::result::AiEngineBaseResult BaichuanNlpEnginePrivate::generateContent(
    const std::string &input) {
    Json::Value inputJson = baichuan_nlp_util::formatJsonFromString(input);
    if (input.empty() || inputJson.empty() || !inputJson.isMember("prompt")) {
        currentError_ = ai_engine::EngineError(
            (int)ai_engine::NlpEngineErrorCode::TooLittleData,
            "The message is empty");
        return {false, currentError_};
    }
    std::string promptContent = inputJson["prompt"].asString();
    Json::Value messageJson;
    messageJson["messages"] = Json::Value(Json::arrayValue);
    Json::Value userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = promptContent;
    messageJson["messages"].append(userMessage);

    std::string message = messageJson.toStyledString();
    return chat(message);
}

ai_engine::result::AiEngineBaseResult BaichuanNlpEnginePrivate::chat(
    const std::string &message) {
    tempStreamChatData_.clear();
    isChatCall_ = true;
    currentError_ = ai_engine::EngineError();
    sentenceId_ = 0;

    if (!waitForStopped()) {
        isStopped_ = false;
    }

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

    Json::Value context = baichuan_nlp_util::formatJsonFromString(message);
    for (int i = 0; i < context["messages"].size(); i++) {
        addContext_(context["messages"][i]["role"].asString(),
                    context["messages"][i]["content"].asString());
    }
    std::string postString = context_.toStyledString();

    cpr::Url url{"https://api.baichuan-ai.com/v1/chat/completions"};
    cpr::Header header{cpr::Header{{"Content-Type", "application/json"},
                                   {"Authorization", "Bearer " + apiKey_}}};
    cpr::Body body{cpr::Body{{postString}}};
    cpr::Response response = cpr::Post(
        url, header, body, cpr::WriteCallback{writeChatData, (intptr_t)this},
        cpr::LowSpeed(15, 20));

    return processChatResponse_(response);
}

bool BaichuanNlpEnginePrivate::waitForStopped() {
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

void BaichuanNlpEnginePrivate::stopChat() {
    if (isStopped_.exchange(true)) {
        return;
    }
    streamChatData_.clear();
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

void BaichuanNlpEnginePrivate::stopGenerateContent() { isStopped_ = true; }

ai_engine::result::AiEngineBaseResult BaichuanNlpEnginePrivate::destroy() {
    stopChat();
    return {true, ai_engine::EngineError()};
}

ai_engine::result::AiEngineBaseResult
BaichuanNlpEnginePrivate::processChatResponse_(cpr::Response &response) {
    if (isStopped_) {
        isStopped_ = false;
        return {true, ai_engine::EngineError()};
    }

    if (response.error.code == cpr::ErrorCode::REQUEST_CANCELLED) {
        streamChatData_.clear();
        currentError_ = ai_engine::EngineError(
            (int)baichuan_nlp_server_error::errorCode2nlpResult(
                response.status_code),
            currentErrorInfo_);
        return {false, currentError_};
    }

    if (response.error) {
        streamChatData_.clear();
        fprintf(stderr, "net error: %s\n", response.error.message.c_str());
        currentError_ = ai_engine::EngineError(
            (int)ai_engine::NlpEngineErrorCode::NetworkDisconnected,
            "HTTP response error");
        return {false, currentError_};
    }

    streamChatData_.clear();
    return {true, ai_engine::EngineError()};
}
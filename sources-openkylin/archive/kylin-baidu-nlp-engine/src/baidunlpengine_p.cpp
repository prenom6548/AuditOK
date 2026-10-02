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

#include "baidunlpengine_p.h"
#include <cstdio>

#include "logger.h"
#include "servererror.h"
#include "token.h"
#include "util.h"
BaiduNlpEnginePrivate::BaiduNlpEnginePrivate() {}

BaiduNlpEnginePrivate::~BaiduNlpEnginePrivate() { streamChatData_.clear(); }

std::string BaiduNlpEnginePrivate::engineName() const { return "baidu"; }

void BaiduNlpEnginePrivate::setConfig(const std::string &config) {
    Json::Value configJson = baidu_nlp_util::formatJsonFromString(config);
    if (configJson.isNull() || !configJson.isMember("apiKey") || !configJson.isMember("secretKey") ||
        !configJson["apiKey"].isString() || !configJson["secretKey"].isString()) {
        LOGE("Invalid config for baidu nlp engine:", config);
        return;
    }
    apiKey_ = configJson["apiKey"].asString();
    secretKey_ = configJson["secretKey"].asString();
}

bool BaiduNlpEnginePrivate::setCurrentModel(const std::string &model) {
    Json::Value modelInfoJson = baidu_nlp_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            currentModel_ = model;
            return true;
        }
    }
    LOGE("Invalid current model name:", model);
    return false;
}

std::string BaiduNlpEnginePrivate::modelInfo() const {
    const char *info = R"(
    {
        "vendor": "百度",
        "models": [
            {
                "modelName": "baidu-ernie-4.0-8k",
                "displayName": "百度-ERNIE-Bot-4",
                "model": "ernie-4.0-8k",
                "subConfig": [
                    {
                        "name": "ERNIE-Bot-4",
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
                "feedbackUrl": "https://cloud.baidu.com/doc/support/index.html",
                "supportsWebSearch": true,
                "supportsThinking": false,
                "supportsThinkingToggle": false
            }
        ]
    }
    )";

    return info;
}

std::string &BaiduNlpEnginePrivate::getAccessToken() {
    if (accessToken_.empty()) {
        generateAccessToken();
    }
    return accessToken_;
}

bool BaiduNlpEnginePrivate::generateAccessToken() {
    int errorCode{int(cpr::ErrorCode::OK)};
    std::string errorMessage;
    accessToken_ = baidu_nlp_token::getBaiduToken(apiKey_, secretKey_, errorCode, errorMessage);
    if (accessToken_.empty()) {
        if (errorCode != int(cpr::ErrorCode::OK)) {
            if (errorCode == int(cpr::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR)) {
                currentError_ =
                    ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::SslCertificateError, errorMessage);
            } else {
                currentError_ =
                    ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::NetworkDisconnected, errorMessage);
            }
        } else {
            currentError_ = ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::Unauthorized, errorMessage);
        }
        LOGE("Get baidu token failed:", errorMessage);
        return false;
    }
    return true;
}

void BaiduNlpEnginePrivate::setTextGenerationResultCallback(
    const ai_engine::gen_ai::nlp::TextGenerationResultCallback &callback) {
    std::lock_guard locker(mutex_);
    textGenerationResultCallback_ = callback;
}

namespace {
const std::vector<std::string> modelContextMember{"stream", "max_tokens", "temperature", "top_p", "presence_penalty"};

std::vector<std::string> splitDataString(const std::string &dataStr) {
    std::vector<std::string> datas;
    datas.clear();

    int oldIndex = 0;
    int newIndex = 0;
    while ((newIndex = dataStr.find_first_of("\n\n", newIndex)) != std::string::npos) {
        datas.push_back(dataStr.substr(oldIndex, newIndex - oldIndex));
        newIndex += 2;
        oldIndex = newIndex;
    }
    return datas;
}

Json::Value parseSearchInfo(const Json::Value &searchInfo) {
    if (searchInfo == Json::nullValue) {
        return {};
    }
    if (!searchInfo.isMember("search_results")) {
        return {};
    }
    if (!searchInfo["search_results"].isArray()) {
        return {};
    }
    Json::Value results = Json::arrayValue;
    for (const auto &item : searchInfo["search_results"]) {
        if (!item.isMember("index")) {
            continue;
        }
        if (!item.isMember("title")) {
            continue;
        }
        if (!item.isMember("url")) {
            continue;
        }
        Json::Value result;
        result["index"] = item["index"];
        result["title"] = item["title"];
        result["url"] = item["url"];
        results.append(result);
    }
    return results;
}
} // namespace

void BaiduNlpEnginePrivate::decreaseChatCounter() {
    if (chatCounter_ > 0) {
        --chatCounter_;
    }
}

void BaiduNlpEnginePrivate::increaseChatCounter() { ++chatCounter_; }

bool BaiduNlpEnginePrivate::isChatRunning() const { return chatCounter_ > 0; }

bool BaiduNlpEnginePrivate::handleStreamMessage(const std::string &data) {
    Json::Value resultJson = baidu_nlp_util::formatJsonFromString(data);
    if (baidu_nlp_token::isBaiduTokenExpiredByResult(resultJson)) {
        if (!refreshAccessTokenCompleted_) {
            refreshAccessTokenRequired_ = true;
            LOGE("Baidu token expired, need to refresh token");
            return false;
        }
    }

    if (int errorCode = baidu_nlp_server_error::parseErrorCode(data)) {
        LOGE("baidu chat failed:", data);
        currentError_ = ai_engine::EngineError(baidu_nlp_server_error::errorCode2nlpResult(errorCode), data);
        return false;
    }

    std::string resultString = tempStreamChatData_ + data;
    if (resultString.substr(resultString.length() - 2) != "\n\n") {
        tempStreamChatData_ += data;
        return true;
    }
    tempStreamChatData_.clear();

    std::vector<std::string> dataStrings = splitDataString(resultString);
    for (std::string dataString : dataStrings) {
        dataString = dataString.substr(6, dataString.length() - 1);
        Json::Value receivedJson = baidu_nlp_util::formatJsonFromString(dataString);
        Json::Value callbackJson;
        callbackJson["sentence_id"] = receivedJson["sentence_id"];
        callbackJson["is_end"] = receivedJson["is_end"];
        callbackJson["result"] = receivedJson["result"];
        callbackJson["finish_reason"] = receivedJson["finish_reason"];
        callbackJson["search_info"] = parseSearchInfo(receivedJson["search_info"]);
        streamChatData_.push_back(receivedJson["result"]);
        ai_engine::gen_ai::nlp::TextGenerationResult result{};
        result.success = true;
        result.error = ai_engine::EngineError{};
        result.modelName = currentDisplayName_;
        result.text = baidu_nlp_util::jsonToString(callbackJson);
        refreshAccessTokenRequired_ = false;
        refreshAccessTokenCompleted_ = false;
        {
            std::lock_guard locker(mutex_);
            if (textGenerationResultCallback_ != nullptr) {
                textGenerationResultCallback_(result);
            }
        }
    }
    return true;
}

bool BaiduNlpEnginePrivate::handleNonStreamMessage(const std::string &data) {
    Json::Value receivedJson = baidu_nlp_util::stringToJson(data);
    Json::Value textJson;
    textJson["is_end"] = true;
    textJson["result"] = receivedJson["result"];
    textJson["finish_reason"] = receivedJson["finish_reason"];
    textJson["sentence_id"] = 0;
    ai_engine::gen_ai::nlp::TextGenerationResult result{};
    result.success = true;
    result.error = ai_engine::EngineError{};
    result.modelName = currentDisplayName_;
    result.text = baidu_nlp_util::jsonToString(textJson);
    if (textGenerationResultCallback_ != nullptr) {
        textGenerationResultCallback_(result);
    }
    return true;
}

bool BaiduNlpEnginePrivate::onMessage(const std::string &data, intptr_t userData) {
    BaiduNlpEnginePrivate *engine = (BaiduNlpEnginePrivate *)userData;
    {
        std::lock_guard locker(engine->mutex_);
        if (engine->textGenerationResultCallback_ == nullptr) {
            return false;
        }
    }
    if (engine->isStopped_) {
        return false;
    }
    if (engine->stream_) {
        return engine->handleStreamMessage(data);
    } else {
        return engine->handleNonStreamMessage(data);
    }
}

int BaiduNlpEnginePrivate::getMaxTokens(const std::string &model) {
    Json::Value modelInfoJson = baidu_nlp_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            maxTokens_ = item["maxTokens"].asInt();
            return maxTokens_;
        }
    }
    return -1;
}

void BaiduNlpEnginePrivate::clearContext() {
    if (context_.isMember("messages")) {
        context_["messages"].clear();
    }
    if (context_.isMember("system")) {
        context_["system"] = "";
    }

    // 清空请求参数
    for (const auto &item : modelContextMember) {
        context_.removeMember(item);
    }

    stream_ = true;
    context_["stream"] = stream_.load();
}

void BaiduNlpEnginePrivate::addContextMessage(const std::string &role, const std::string &message) {
    if (role == "system") {
        if (!message.empty()) {
            systemRole_ = message;
        }
        return;
    }
    int size = context_["messages"].size();
    context_["messages"][size]["role"] = role;
    context_["messages"][size]["content"] = message;
}

void BaiduNlpEnginePrivate::convertContextMember(Json::Value &context) {
    // 输入的参数名称和千问模型的参数映射关系，相同名称无需映射
    const std::map<std::string, std::string> convertMap{{"max_completion_tokens", "max_tokens"}};

    for (const auto &item : convertMap) {
        if (context.isMember(item.first)) {
            context[item.second] = context[item.first];
            context.removeMember(item.first);
        }
    }
}

void BaiduNlpEnginePrivate::addContextMember(const Json::Value &context) {
    for (const auto &item : modelContextMember) {
        if (!context.isMember(item)) {
            continue;
        }
        context_[item] = context[item];

        if (item == "stream") {
            stream_ = context["stream"].asBool();
        } else if (item == "max_tokens") {
            maxTokens_ = context[item].asInt();
        }
    }
}

void BaiduNlpEnginePrivate::addSystemContext() {
    if (context_.empty() || systemRole_.empty()) {
        return;
    }
    int size = context_["messages"].size();
    if (size >= 1) {
        context_["messages"][size]["role"] = context_["messages"][size - 1]["role"];
        context_["messages"][size]["content"] = context_["messages"][size - 1]["content"];
        context_["messages"][size - 1]["role"] = "user";
        context_["messages"][size - 1]["content"] = systemRole_;
    }
}

void BaiduNlpEnginePrivate::removeLastContext() {
    int size = context_["messages"].size();
    if (size - 1 >= 0 && context_["messages"][size - 1]["role"].asString() == "user") {
        Json::Value removeValue;
        context_["messages"].removeIndex(size - 1, &removeValue);
        return;
    }
    if ((size - 1) >= 0 && (size - 2) >= 0 && context_["messages"][size - 1]["role"].asString() == "assistant" &&
        context_["messages"][size - 2]["role"].asString() == "user") {
        Json::Value removeValue;
        context_["messages"].removeIndex(size - 1, &removeValue);
        context_["messages"].removeIndex(size - 2, &removeValue);
        return;
    }
}

ai_engine::result::AiEngineBaseResult BaiduNlpEnginePrivate::init() {
    context_["stream"] = true;
    if (getMaxTokens(currentModel_) != -1) {
        context_["max_output_tokens"] = maxTokens_;
    }
    refreshAccessTokenRequired_ = false;
    refreshAccessTokenCompleted_ = false;
    return {true, ai_engine::EngineError{}};
}

ai_engine::result::AiEngineBaseResult BaiduNlpEnginePrivate::generateContent(const std::string &input) {
    Json::Value inputJson = baidu_nlp_util::formatJsonFromString(input);
    if (input.empty() || inputJson.empty() || !inputJson.isMember("prompt")) {
        currentError_ =
            ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::TooLittleData, "The message is empty");
        return {false, currentError_};
    }
    std::string promptContent = inputJson["prompt"].asString();
    Json::Value messageJson;
    messageJson["messages"] = Json::Value(Json::arrayValue);
    Json::Value userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = promptContent;
    messageJson["messages"].append(userMessage);

    if (inputJson.isMember("stream") && inputJson["stream"].isBool()) {
        messageJson["stream"] = inputJson["stream"].asBool();
    }
    if (inputJson.isMember("web_search") && inputJson["web_search"].isBool()) {
        messageJson["web_search"] = inputJson["web_search"].asBool();
    }

    std::string message = baidu_nlp_util::jsonToString(messageJson);
    return chat(message);
}

ai_engine::result::AiEngineBaseResult BaiduNlpEnginePrivate::chat(const std::string &message) {
    LOGI("BaiduNlpEnginePrivate::chat");
    currentError_ = ai_engine::EngineError();
    tempStreamChatData_.clear();
    increaseChatCounter();

    if (!waitForStopped()) {
        isStopped_ = false;
    }

    if (!isChatRunning()) {
        LOGI("BaiduNlpEnginePrivate::chat: no chat is running");
        return {true, ai_engine::EngineError()};
    }

    {
        std::lock_guard locker(mutex_);
        if (textGenerationResultCallback_ == nullptr) {
            currentError_ = ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::TooLittleParams,
                                                   "TextGenerationResultCallback is nullptr");
            decreaseChatCounter();
            return {false, currentError_};
        }
    }
    if (message.empty()) {
        currentError_ =
            ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::TooLittleData, "The message is empty");
        decreaseChatCounter();
        return {false, currentError_};
    }

    clearContext();

    Json::Value context = baidu_nlp_util::formatJsonFromString(message);
    for (int i = 0; i < context["messages"].size(); i++) {
        addContextMessage(context["messages"][i]["role"].asString(), context["messages"][i]["content"].asString());
    }
    addSystemContext();
    if (context.isMember("web_search") && context["web_search"].asBool()) {
        context_["disable_search"] = false;
        context_["enable_citation"] = true;
        context_["enable_trace"] = true;
    } else {
        context_["disable_search"] = true;
        context_["enable_citation"] = false;
        context_["enable_trace"] = false;
    }

    convertContextMember(context);
    addContextMember(context);

    LOGI("Baidu chat context:", baidu_nlp_util::jsonToString(context_));
    const std::string postString = baidu_nlp_util::jsonToString(context_);
    const std::string token = getAccessToken();
    if (token.empty()) {
        if (isStopped_) {
            isStopped_ = false;
            return {true, ai_engine::EngineError()};
        }
        decreaseChatCounter();
        return {false, currentError_};
    }

    cpr::Url url{"https://aip.baidubce.com/rpc/2.0/ai_custom/v1/wenxinworkshop/chat/completions_pro"};
    cpr::Parameters parameters{{"access_token", token}};
    cpr::Header header{{"Content-Type", "application/json"}};
    cpr::Body body{{postString}};
    cpr::Response response;

    if (stream_) {
        response = cpr::Post(url, parameters, header, body, cpr::WriteCallback{onMessage, (intptr_t)this},
                             cpr::ConnectTimeout{10000}, cpr::LowSpeed{20, 10});
    } else {
        response = cpr::Post(url, parameters, header, body, cpr::WriteCallback{onMessage, (intptr_t)this});
    }

    if (refreshAccessTokenRequired_) {
        removeLastContext();
        streamChatData_.clear();
        generateAccessToken();
        refreshAccessTokenRequired_ = false;
        refreshAccessTokenCompleted_ = true;
        return chat(message);
    }

    return processChatResponse(response);
}

bool BaiduNlpEnginePrivate::waitForStopped() {
    auto startTime = std::chrono::steady_clock::now();
    while (isStopped_) {
        if (std::chrono::steady_clock::now() - startTime > std::chrono::seconds(10)) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

void BaiduNlpEnginePrivate::stopChat() {
    LOGD("BaiduNlpEnginePrivate::stopChat");
    if (!isChatRunning()) {
        LOGI("BaiduNlpEnginePrivate::stopChat: chat is not running");
        return;
    }
    decreaseChatCounter();

    isStopped_ = true;
    streamChatData_.clear();

    ai_engine::gen_ai::nlp::TextGenerationResult result{true, ai_engine::EngineError(), currentDisplayName_,
                                                        std::string{R"({"result":"","is_end":true})"}};
    {
        std::lock_guard locker(mutex_);
        if (textGenerationResultCallback_ != nullptr) {
            textGenerationResultCallback_(result);
        }
    }
}

void BaiduNlpEnginePrivate::stopGenerateContent() { isStopped_ = true; }

ai_engine::result::AiEngineBaseResult BaiduNlpEnginePrivate::destroy() {
    stopChat();
    return {true, ai_engine::EngineError()};
}

ai_engine::result::AiEngineBaseResult BaiduNlpEnginePrivate::processChatResponse(const cpr::Response &response) {
    if (isStopped_) {
        isStopped_ = false;
        return {true, ai_engine::EngineError()};
    }
    decreaseChatCounter();

    if (response.error.code == cpr::ErrorCode::REQUEST_CANCELLED) {
        streamChatData_.clear();
        return {false, currentError_};
    }

    if (response.error) {
        streamChatData_.clear();
        LOGE("net error:", response.error.message);
        currentError_ =
            ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::NetworkDisconnected, "Http response error");
        return {false, currentError_};
    }

    streamChatData_.clear();
    return {true, ai_engine::EngineError{}};
}

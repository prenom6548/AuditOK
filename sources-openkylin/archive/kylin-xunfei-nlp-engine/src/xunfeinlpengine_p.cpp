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

#include "xunfeinlpengine_p.h"
#include <unistd.h>
#include <regex>
#include <thread>
#include <curl/websockets.h>

#include "logger.h"
#include "servererror.h"
#include "token.h"
#include "util.h"

namespace {

// 样例如下
// [
//     {
//         "index": 1,
//         "url": "https://xueqiu.com/4528807922/339261085",
//         "title": "中东冲突详情、前景及市场影响分析（截至2025年6月18日..."
//     },
//     {
//         "index": 2,
//         "url": "https://cj.sina.com.cn/articles/view/7096020384/1a6f4ada006801e80o",
//         "title": "2025年6月伊以军事冲突急剧升级，中东近年最严重对抗事件..."
//     },
//     {
//         "index": 3,
//         "url": "http://www.toutiao.com/a7517799573679620647/?channel=\\u0026source=search_tab",
//         "title": "角色反转了，以色列首都上千人流离失所，终于尝到了加沙的滋味"
//     },
//     {
//         "index": 4,
//         "url": "http://www.toutiao.com/a7517761875308937791/?channel=\\u0026source=search_tab",
//         "title": "以伊局势将如何发展"
//     },
//     {
//         "index": 5,
//         "url": "https://news.cctv.cn/2025/06/15/ARTIkEaDz4Jfn4Oi2Xr3Zswa250615.shtml",
//         "title": "中东危情48小时以色列伊朗持续互袭 俄美首脑通话 …"
//     }
// ]
//
// {
//     "header":
//     {
//         "code": 0,
//         "message": "Success",
//         "sid": "cht000bbe1b@dx1978c5715adb809800",
//         "status": 1
//     },
//     "payload":
//     {
//         "plugins":
//         {
//             "text":
//             [
//                 {
//                     "name": "ifly_search",
//                     "content": "", // 搜索结果，json字符串，格式如上
//                     "content_type": "text",
//                     "content_meta": null,
//                     "role": "tool",
//                     "status": "finished",
//                     "invoked":
//                     {
//                         "namespace": "ifly_search",
//                         "plugin_id": "ifly_search",
//                         "plugin_ver": "",
//                         "status_code": 200,
//                         "status_msg": "Success",
//                         "type": "local"
//                     }
//                 }
//             ]
//         }
//     }
// }

Json::Value parseSearchInfoContent(const Json::Value &content) {
    Json::Value results = Json::arrayValue;
    for (auto &item : content) {
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

Json::Value parseSearchInfo(const Json::Value &searchInfo) {
    if (!searchInfo.isMember("plugins")) {
        return {};
    }
    if (!searchInfo["plugins"].isObject()) {
        return {};
    }
    if (!searchInfo["plugins"].isMember("text")) {
        return {};
    }
    if (!searchInfo["plugins"]["text"].isArray()) {
        return {};
    }
    Json::Value results = Json::arrayValue;
    for (auto &item : searchInfo["plugins"]["text"]) {
        if (!item.isMember("content")) {
            continue;
        }
        if (!item["content"].isString()) {
            continue;
        }
        std::string content = item["content"].asString();
        Json::Value jsonContent = xunfei_nlp_util::stringToJson(content);
        Json::Value value = parseSearchInfoContent(jsonContent);
        for (const auto &result : value) {
            results.append(result);
        }
    }
    return results;
}

std::string replaceCaretInBrackets(const std::string &input) {
    std::string result;
    std::string temp; // 暂存连续的 [x][y] 片段
    size_t i = 0;

    while (i < input.size()) {
        if (i + 3 < input.size() && input[i] == '[' && input[i + 1] == '^' && input[i + 3] == '^' &&
            input[i + 4] == ']') {
            // 匹配到了 [^x^]
            char digit = input[i + 2];
            temp += '[' + std::string(1, digit) + ']';
            i += 5; // 跳过 [^x^]
        } else {
            // 如果当前不是 [^x^]，先处理之前的 temp（如果有）
            if (!temp.empty()) {
                result += "^" + temp + "^";
                temp.clear();
            }
            result += input[i];
            ++i;
        }
    }

    // 处理最后可能剩余的 temp
    if (!temp.empty()) {
        result += "^" + temp + "^";
    }

    return result;
}

} // namespace

bool XunfeiNlpEnginePrivate::connectWebsocket(const std::string &hostUrl) {
    std::string callurl = xunfei_nlp_token::getAuthenticationUrl(apiKey_, secretKey_, hostUrl);

    CURLcode res;
    if (currentCurl_) {
        curl_easy_setopt(currentCurl_, CURLOPT_URL, callurl.c_str());
        curl_easy_setopt(currentCurl_, CURLOPT_CONNECT_ONLY, 2L);
        res = curl_easy_perform(currentCurl_);
        if (res == CURLE_HTTP_RETURNED_ERROR) {
            LOGE("xunfei authenticate failed:", curl_easy_strerror(res));
            currentError_ =
                ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::Unauthorized, "Authentication failed");
            return false;
        }
        if (res != CURLE_OK) {
            LOGE("net error:", res, curl_easy_strerror(res));
            if (res == CURLE_PEER_FAILED_VERIFICATION) {
                currentError_ = ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::SslCertificateError,
                                                       curl_easy_strerror(res));
            } else {
                currentError_ = ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::NetworkDisconnected,
                                                       curl_easy_strerror(res));
            }
            return false;
        }
        return true;
    }
    currentError_ =
        ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::InvalidWords, "Failed to create resource");
    return false;
}

void XunfeiNlpEnginePrivate::closeWebsocket() {
    size_t sent;
    if (currentCurl_) {
        curl_ws_send(currentCurl_, "", 0, &sent, 0, CURLWS_CLOSE);
    }
}

XunfeiNlpEnginePrivate::XunfeiNlpEnginePrivate() {}

XunfeiNlpEnginePrivate::~XunfeiNlpEnginePrivate() { streamChatData_.clear(); }

std::string XunfeiNlpEnginePrivate::engineName() const { return "xunfei"; }

void XunfeiNlpEnginePrivate::setConfig(const std::string &config) {
    Json::Value configJson = xunfei_nlp_util::stringToJson(config);
    if (configJson.isNull() || !configJson.isMember("apiKey") || !configJson.isMember("secretKey") ||
        !configJson.isMember("appId") || !configJson["apiKey"].isString() || !configJson["secretKey"].isString() ||
        !configJson["appId"].isString()) {
        LOGE("Invalid config for xunfei chat engine:", config);
        return;
    }
    appId_ = configJson["appId"].asString();
    apiKey_ = configJson["apiKey"].asString();
    secretKey_ = configJson["secretKey"].asString();

    context_["header"]["app_id"] = appId_;
}

bool XunfeiNlpEnginePrivate::setCurrentModel(const std::string &model) {
    Json::Value modelInfoJson = xunfei_nlp_util::stringToJson(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            currentModel_ = model;
            return true;
        }
    }
    LOGE("Invalid current model name:", model);
    return false;
}

std::string XunfeiNlpEnginePrivate::modelInfo() const {
    LOGI("xunfei model info:", "xunfei");
    const char *info = R"(
    {
        "vendor": "讯飞",
        "models": [
            {
                "modelName": "xunfei-generalv3.5",
                "displayName": "讯飞-Spark Max",
                "model": "generalv3.5",
                "subConfig": [
                    {
                        "name": "Spark Max",
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
                "maxTokens": 8192,
                "feedbackUrl": "https://console.xfyun.cn/workorder/commit",
                "supportsWebSearch": true,
                "supportsThinking": false,
                "supportsThinkingToggle": false
            }
        ]
    }
    )";

    return info;
}

void XunfeiNlpEnginePrivate::setTextGenerationResultCallback(
    const ai_engine::gen_ai::nlp::TextGenerationResultCallback &callback) {
    std::lock_guard locker(mutex_);
    textGenerationResultCallback_ = callback;
}

bool XunfeiNlpEnginePrivate::doSendData(const char *data, int dataLength, int type) {
    if (!isChatRunning()) {
        LOGI("XunfeiNlpEnginePrivate::chat: no chat is running, stop to send "
             "data");
        return false;
    }
    size_t sent;
    CURLcode resultCode = curl_ws_send(currentCurl_, data, dataLength, &sent, 0, type);
    if (resultCode != CURLE_OK) {
        LOGE("send data to xunfei failed:", curl_easy_strerror(resultCode));
        currentError_ = ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::NetworkDisconnected,
                                               curl_easy_strerror(resultCode));
        return false;
    }
    return true;
}

CURLcode XunfeiNlpEnginePrivate::testConnectToServer() {
    std::string callurl = xunfei_nlp_token::getAuthenticationUrl(apiKey_, secretKey_, hostUrl_);
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, callurl.c_str());

    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 2L); /* websocket style */

    /* Perform the request, res gets the return code */
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return res;
}

int XunfeiNlpEnginePrivate::getMaxTokens(const std::string &model) {
    Json::Value modelInfoJson = xunfei_nlp_util::stringToJson(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            maxTokens_ = item["maxTokens"].asInt();
            return maxTokens_;
        }
    }
    return -1;
}

void XunfeiNlpEnginePrivate::clearContext() {
    if (context_["payload"].isMember("message")) {
        context_["payload"]["message"]["text"].clear();
    }
}

void XunfeiNlpEnginePrivate::addContext(const std::string &role, const std::string &content) {
    if (role.empty() || content.empty()) {
        return;
    }
    if (role == "system") {
        context_["payload"]["message"]["text"][0]["role"] = role;
        context_["payload"]["message"]["text"][0]["content"] = content;
    } else {
        uint32_t size = context_["payload"]["message"]["text"].size();
        context_["payload"]["message"]["text"][size]["role"] = role;
        context_["payload"]["message"]["text"][size]["content"] = content;
    }
}

ai_engine::result::AiEngineBaseResult XunfeiNlpEnginePrivate::buildChatMessage(const std::string &message) {
    Json::Value context = xunfei_nlp_util::stringToJson(message);
    ai_engine::result::AiEngineBaseResult result;
    if (!context.isMember("messages")) {
        result.success = false;
        result.error = {int(ai_engine::NlpEngineErrorCode::InvalidParamType),
                        "The input information lacks messages content."};
        return result;
    }
    if (!context["messages"].isArray()) {
        result.success = false;
        result.error = {int(ai_engine::NlpEngineErrorCode::InvalidParamType), "The messages is not array."};
        return result;
    }
    uint32_t size = context["messages"].size();
    for (uint32_t i = 0; i < size; ++i) {
        Json::Value item = context["messages"][i];
        if ((!item.isMember("role")) || (!item["role"].isString())) {
            continue;
        }
        if ((!item.isMember("content")) || (!item["content"].isString())) {
            continue;
        }
        std::string role = item["role"].asString();
        std::string content = item["content"].asString();
        addContext(role, content);
    }
    if (context.isMember("top_k") && context["top_k"].isInt()) {
        context_["parameter"]["chat"]["top_k"] = context["top_k"].asInt();
    }
    if (context.isMember("stream") && context["stream"].isBool()) {
        stream_ = context["stream"].asBool();
    }
    Json::Value webSearch;
    if (context.isMember("web_search") && context["web_search"].asBool()) {
        webSearch["enable"] = true;
        webSearch["show_ref_label"] = true;
    } else {
        webSearch["enable"] = false;
        webSearch["show_ref_label"] = false;
    }
    Json::Value tool;
    tool["type"] = "web_search";
    tool["web_search"] = webSearch;
    context_["parameter"]["chat"]["tools"] = Json::arrayValue;
    context_["parameter"]["chat"]["tools"].append(tool);
    result.success = true;
    result.error = ai_engine::EngineError{};
    return result;
}

void XunfeiNlpEnginePrivate::removeLastContext() {
    int size = context_["payload"]["message"]["text"].size();
    if (size - 1 >= 0 && context_["payload"]["message"]["text"][size - 1]["role"].asString() == "user") {
        Json::Value removeValue;
        context_["payload"]["message"]["text"].removeIndex(size - 1, &removeValue);
        return;
    }
    if ((size - 1) >= 0 && (size - 2) >= 0 &&
        context_["payload"]["message"]["text"][size - 1]["role"].asString() == "assistant" &&
        context_["payload"]["message"]["text"][size - 2]["role"].asString() == "user") {
        Json::Value removeValue;
        context_["payload"]["message"]["text"].removeIndex(size - 1, &removeValue);
        context_["payload"]["message"]["text"].removeIndex(size - 2, &removeValue);
        return;
    }
}

ai_engine::result::AiEngineBaseResult XunfeiNlpEnginePrivate::init() {
    currentError_ = ai_engine::EngineError();
    context_["parameter"]["chat"]["domain"] = "generalv3.5";
    if (getMaxTokens(currentModel_) != -1) {
        context_["parameter"]["chat"]["max_tokens"] = maxTokens_;
    }
    return {true, ai_engine::EngineError()};
}

ai_engine::result::AiEngineBaseResult XunfeiNlpEnginePrivate::generateContent(const std::string &input) {
    Json::Value inputJson = xunfei_nlp_util::stringToJson(input);
    if (input.empty() || inputJson.empty() || !inputJson.isMember("prompt")) {
        currentError_ =
            ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::TooLittleData, "The message is empty");
        return {false, currentError_};
    }
    std::string promptContent = inputJson["prompt"].asString();

    Json::Value userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = promptContent;

    Json::Value messageJson;
    messageJson["messages"] = Json::Value(Json::arrayValue);
    messageJson["messages"].append(userMessage);

    if (inputJson.isMember("stream") && inputJson["stream"].isBool()) {
        messageJson["stream"] = inputJson["stream"].asBool();
    }
    if (inputJson.isMember("top_k") && inputJson["top_k"].isInt()) {
        messageJson["top_k"] = inputJson["top_k"].asInt();
    }
    if (inputJson.isMember("web_search") && inputJson["web_search"].isBool()) {
        messageJson["web_search"] = inputJson["web_search"].asBool();
    }

    std::string message = xunfei_nlp_util::jsonToString(messageJson);
    fprintf(stdout, "message: %s\n", message.c_str());

    return chat(message);
}

ai_engine::result::AiEngineBaseResult XunfeiNlpEnginePrivate::chat(const std::string &message) {
    currentError_ = ai_engine::EngineError();

    {
        std::lock_guard locker(mutex_);
        if (textGenerationResultCallback_ == nullptr) {
            return {false, currentError_};
        }
    }
    if (message.empty()) {
        currentError_ =
            ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::TooLittleData, "The message is empty");
        return {false, currentError_};
    }

    clearContext();
    increaseChatCounter();

    if (!waitForStopped()) {
        isStopped_ = false;
    }

    if (!isChatRunning()) {
        LOGI("XunfeiNlpEnginePrivate::chat: no chat is running");
        return {true, currentError_};
    }

    if (!initResources() || !connectWebsocket(hostUrl_)) {
        if (isStopped_) {
            return {true, ai_engine::EngineError()};
        }
        decreaseChatCounter();
        return {false, currentError_};
    }

    ai_engine::result::AiEngineBaseResult result = buildChatMessage(message);
    if (!result.success) {
        return result;
    }
    std::string postString = xunfei_nlp_util::jsonToString(context_);
    LOGI(postString);
    if (!doSendData(postString.c_str(), postString.size(), CURLWS_TEXT)) {
        removeLastContext();
        streamChatData_.clear();
        closeWebsocket();
        if (isStopped_) {
            return {true, ai_engine::EngineError()};
        }
        decreaseChatCounter();
        return {false, currentError_};
    }

    handleReceiveData();

    streamChatData_.clear();

    closeWebsocket();
    return {true, ai_engine::EngineError()};
}

bool XunfeiNlpEnginePrivate::waitForStopped() {
    auto startTime = std::chrono::steady_clock::now();
    while (isStopped_) {
        if (std::chrono::steady_clock::now() - startTime > std::chrono::seconds(10)) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

void XunfeiNlpEnginePrivate::stopChat() {
    if (!isChatRunning()) {
        LOGI("XunfeiNlpEnginePrivate::stopChat: chat is not running");
        return;
    }
    streamChatData_.clear();

    decreaseChatCounter();

    isStopped_ = true;
    ai_engine::gen_ai::nlp::TextGenerationResult result{true, currentError_, currentDisplayName_,
                                                        std::string{R"({"result":"","is_end":true})"}};
    {
        std::lock_guard locker(mutex_);
        if (textGenerationResultCallback_ != nullptr) {
            textGenerationResultCallback_(result);
        }
    }
    closeWebsocket();
    releaseResources();
}

void XunfeiNlpEnginePrivate::stopGenerateContent() { isStopped_ = true; }

ai_engine::result::AiEngineBaseResult XunfeiNlpEnginePrivate::destroy() {
    stopChat();
    return {true, ai_engine::EngineError()};
}

void XunfeiNlpEnginePrivate::threadSafeCallChatResultCallback(
    const ai_engine::gen_ai::nlp::TextGenerationResult &result) const {
    std::lock_guard locker(mutex_);
    if (textGenerationResultCallback_ != nullptr) {
        textGenerationResultCallback_(result);
    }
}

void XunfeiNlpEnginePrivate::decreaseChatCounter() {
    if (chatCounter_ > 0) {
        --chatCounter_;
    }
}

void XunfeiNlpEnginePrivate::increaseChatCounter() { ++chatCounter_; }

bool XunfeiNlpEnginePrivate::isChatRunning() const { return chatCounter_ > 0; }

void XunfeiNlpEnginePrivate::handleReceiveData() {
    int retryCount = 0;
    while (!isStopped_) {
        int retCode;
        const std::string chatData = receiveChatData(&retCode);

        if (isNetworkError(retCode)) {
            handleNetworkError(retCode);
            break;
        }
        if (needReReceiveData(retCode)) {
            ++retryCount;
            if (retryCount > 1000 * 10) {
                auto ret = testConnectToServer();
                if (ret != 0) {
                    handleNetworkError(ret);
                    break;
                } else {
                    retryCount = 0;
                }
            }
            usleep(1000);
            continue;
        }
        if (int errorCode = xunfei_nlp_server_error::parseErrorCode(chatData); errorCode != 0) {
            handleServerError(errorCode, chatData);
            break;
        }

        if (isChatDataReady(retCode)) {
            bool isFinished = false;
            handleChatData(chatData, &isFinished);
            if (isFinished) {
                decreaseChatCounter();
                break;
            }
        }
        retryCount = 0;
        usleep(1000);
    }
    if (isStopped_) {
        isStopped_ = false;
    }
}
std::string XunfeiNlpEnginePrivate::receiveChatData(int *retCode) const {
    size_t rlen;
    const curl_ws_frame *meta;
    char buffer[4100]{0};
    CURLcode curlCode = curl_ws_recv(currentCurl_, buffer, sizeof(buffer), &rlen, &meta);
    *retCode = curlCode;
    return std::string(buffer, rlen);
}

bool XunfeiNlpEnginePrivate::isNetworkError(int retCode) const {
    return retCode != CURLE_OK && retCode != CURLE_AGAIN && retCode != CURLE_RECV_ERROR;
}

bool XunfeiNlpEnginePrivate::isChatDataReady(int retCode) const { return retCode == CURLE_OK; }

bool XunfeiNlpEnginePrivate::needReReceiveData(int retCode) const {
    return retCode == CURLE_AGAIN || retCode == CURLE_RECV_ERROR;
}

void XunfeiNlpEnginePrivate::handleNetworkError(int retCode) {
    if (isStopped_) {
        isStopped_ = false;
        return;
    }
    LOGE("Net error:", curl_easy_strerror(CURLcode(retCode)));
    currentError_ = ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::NetworkDisconnected,
                                           curl_easy_strerror(CURLcode(retCode)));
    ai_engine::gen_ai::nlp::TextGenerationResult result{false, currentError_, currentDisplayName_, std::string{}};
    threadSafeCallChatResultCallback(result);
}

void XunfeiNlpEnginePrivate::handleServerError(int errorCode, const std::string &errorMessage) {
    if (isStopped_) {
        isStopped_ = false;
        return;
    }
    LOGE("Xunfei chat failed", errorMessage);
    currentError_ = ai_engine::EngineError((int)xunfei_nlp_server_error::errorCode2nlpResult(errorCode), errorMessage);
    ai_engine::gen_ai::nlp::TextGenerationResult result{false, currentError_, currentDisplayName_, std::string{}};
    threadSafeCallChatResultCallback(result);
}

void XunfeiNlpEnginePrivate::handleChatData(const std::string &chatData, bool *isFinished) {
    if (stream_) {
        Json::Value obj = xunfei_nlp_util::stringToJson(chatData);
        // 搜索信息中没有回答信息
        // 先保存到searchInfo_
        // sentence_id为0时，返回搜索信息
        if (searchInfo_ == Json::nullValue) {
            searchInfo_ = parseSearchInfo(obj["payload"]);
            if (searchInfo_ != Json::nullValue) {
                return;
            }
        }
        std::string content = obj["payload"]["choices"]["text"][0]["content"].asString();

        Json::Value callbackObj;
        callbackObj["sentence_id"] = obj["payload"]["choices"]["seq"];
        callbackObj["is_end"] = (obj["header"]["status"].asInt() == 2);
        callbackObj["result"] = replaceCaretInBrackets(content);
        callbackObj["finish_reason"] = "";
        streamChatData_.push_back(obj["payload"]["choices"]["text"][0]["content"]);
        if (callbackObj["sentence_id"].asInt() == 0) {
            callbackObj["search_info"] = searchInfo_;
            searchInfo_.clear();
        }
        ai_engine::gen_ai::nlp::TextGenerationResult result{
            true,
            ai_engine::EngineError(),
            currentDisplayName_,
            callbackObj.toStyledString(),
        };
        threadSafeCallChatResultCallback(result);
        *isFinished = obj["header"]["status"].asInt() == 2;
    } else {
        // LOGI("Received data", chatData);
        Json::Value receivedJson = xunfei_nlp_util::stringToJson(chatData);
        *isFinished = receivedJson["header"]["status"].asInt() == 2;
        receivedContentTemp_ += receivedJson["payload"]["choices"]["text"][0]["content"].asString();
        if (searchInfo_ == Json::nullValue) {
            searchInfo_ = parseSearchInfo(receivedJson["payload"]);
            if (searchInfo_ != Json::nullValue) {
                return;
            }
        }
        if (isFinished) {
            Json::Value callbackJson;
            callbackJson["sentence_id"] = 0;
            callbackJson["is_end"] = true;
            callbackJson["result"] = replaceCaretInBrackets(receivedContentTemp_);
            callbackJson["finish_reason"] = "";
            callbackJson["search_info"] = searchInfo_;
            receivedContentTemp_.clear();
            searchInfo_.clear();
            ai_engine::gen_ai::nlp::TextGenerationResult result{};
            result.success = true;
            result.error = ai_engine::EngineError{};
            result.modelName = currentDisplayName_;
            result.text = xunfei_nlp_util::jsonToString(callbackJson);
            threadSafeCallChatResultCallback(result);
        }
    }
}

bool XunfeiNlpEnginePrivate::initResources() {
    if (currentCurl_) {
        curl_easy_cleanup(currentCurl_);
        currentCurl_ = nullptr;
    }
    currentCurl_ = curl_easy_init();

    return true;
}

void XunfeiNlpEnginePrivate::releaseResources() {
    if (currentCurl_) {
        curl_easy_cleanup(currentCurl_);
        currentCurl_ = nullptr;
    }
}

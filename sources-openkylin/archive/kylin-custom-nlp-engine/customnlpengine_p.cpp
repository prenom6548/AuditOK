/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include "customnlpengine_p.h"

#include <cstdio>
#include <algorithm>

#include "servererror.h"
#include "util.h"
namespace {
const std::vector<std::string> modelContextMember{"stream",           "max_tokens", "temperature", "top_p", "top_k",
                                                  "presence_penalty", "n",          "tool_choice", "tools"};

std::vector<std::string> splitDataString(const std::string &dataStr, const std::string &endStr) {
    std::vector<std::string> datas;

    int oldIndex = 0;
    int newIndex = 0;
    while ((newIndex = dataStr.find_first_of(endStr, newIndex)) != std::string::npos) {
        datas.push_back(dataStr.substr(oldIndex, newIndex - oldIndex));
        newIndex += endStr.length();
        oldIndex = newIndex;
    }
    return datas;
}
}  // namespace
bool writeChatData(std::string data, intptr_t userData) {
    CustomNlpEnginePrivate *engine = (CustomNlpEnginePrivate *)userData;
    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback = engine->textGenerationResultCallback_;

    if (engine->isStopped_ || !engine->isChatRunning()) {
        return false;
    }
    if (engine->stream_) {
        std::string jsonStr = data;
        size_t dataPrefixLen = 6;
        if (data.compare(0, dataPrefixLen, "data: ") == 0) {
            size_t newlinePos = data.find('\n');
            if (newlinePos != std::string::npos && newlinePos > dataPrefixLen) {
                jsonStr = data.substr(dataPrefixLen, newlinePos - dataPrefixLen);
            } else if (newlinePos == std::string::npos) {
                jsonStr = data.substr(dataPrefixLen);
            }
        }

        Json::Value resultJson = custom_nlp_util::formatJsonFromString(jsonStr);

        if (resultJson.isObject() && !resultJson.isNull() && resultJson.isMember("error")) {
            fprintf(stderr, "custom chat failed: %s\n", data.c_str());
            engine->currentErrorInfo_ = data;
            return false;
        }

        if (resultJson.isObject() && !resultJson.isNull() && resultJson.isMember("code") &&
            resultJson["code"].isInt()) {
            int code = resultJson["code"].asInt();
            if (code != 0) {
                if (resultJson.isMember("message") && resultJson["message"].isString()) {
                    engine->currentErrorInfo_ = resultJson["message"].asString();
                }

                engine->currentError_ =
                    ai_engine::EngineError((int)ai_engine::NlpEngineErrorCode::UnknownError, engine->currentErrorInfo_);
                return false;
            }
        }

        if (data == ": keep-alive\n\n") {
            return true;
        }

        std::string resultString = engine->tempStreamChatData_.data() + data;

        if (resultString.length() <= 5) {
            engine->tempStreamChatData_ += data;
            return true;
        }

        if (!engine->isValidResponse(resultString)) {
            engine->currentErrorInfo_ = "Unrecognized response format, parsing failed, response is \n" + resultString;
            return false;
        }

        if (data.compare(0, 7, "data: {") == 0) {
            engine->endStr_ = "\n\n";
        }
        if (resultString.substr(resultString.length() - engine->endStr_.length()) != engine->endStr_) {
            engine->tempStreamChatData_ += data;
            return true;
        }
        engine->tempStreamChatData_.clear();
        std::vector<std::string> dataStrings = splitDataString(resultString, engine->endStr_);

        for (std::string data : dataStrings) {
            if (engine->endStr_ == "\n\n") {
                data = data.substr(6, data.length() - 1);
            }
            if (data == "[DONE]") {
                break;
            }
            Json::Value callbackObj = engine->formatResultJsonFromInputString(data);
            if (callbackObj.isNull()) {
                continue;
            }
            Json::Value dataJson = custom_nlp_util::formatJsonFromString(data);
            if (dataJson.isMember("error")) {
                fprintf(stderr, "custom chat failed: %s\n", data.c_str());
                engine->currentErrorInfo_ = data;
                return false;
            }
            bool isSparkApi = engine->chatUrl_.find("spark-api-open") != std::string::npos;
            if (isSparkApi && dataJson.isMember("usage") && !dataJson["usage"].isNull()) {
                callbackObj["is_end"] = true;
            }
            if (engine->hasToolCalls(dataJson["choices"][0])) {
                continue;
            }
            if (dataJson["choices"][0]["finish_reason"] == "tool_calls" ||
                dataJson["choices"][0]["finish_reason"] == "stop" && !engine->toolCallsAccumulator_.empty()) {
                auto result = engine->handleToolCallsCompletion();
                if (callback) {
                    callback(result);
                }
                return true;
            }

            engine->streamChatData_.push_back(callbackObj["result"]);
            ai_engine::gen_ai::nlp::TextGenerationResult result{
                true, ai_engine::EngineError(), engine->currentDisplayName_, callbackObj.toStyledString()};
            callback(result);
        }
    } else {
        engine->nonStreamData_ += data;
        Json::Value receivedJson = custom_nlp_util::stringToJson(engine->nonStreamData_);
        if (receivedJson == Json::objectValue) {
            return true;
        }
        if (engine->isOpenAiApiResult(receivedJson)) {
            Json::Value textJson;
            textJson["is_end"] = true;
            if (!receivedJson["choices"][0]["message"]["reasoning_content"].asString().empty()) {
                textJson["reasoning_content"] = receivedJson["choices"][0]["message"]["reasoning_content"];
            }
            textJson["result"] = receivedJson["choices"][0]["message"]["content"];
            textJson["sentence_id"] = 0;
            if (receivedJson["choices"][0]["message"].isMember("tool_calls")) {
                textJson["tool_calls"] = receivedJson["choices"][0]["message"]["tool_calls"];
            }

            std::string text = custom_nlp_util::jsonToString(textJson);

            ai_engine::gen_ai::nlp::TextGenerationResult result{};
            result.success = true;
            result.error = ai_engine::EngineError{};
            result.modelName = engine->currentDisplayName_;
            result.text = text;
            if (callback) {
                callback(result);
            }
        } else if (engine->isOllamaApiResult(receivedJson)) {
            Json::Value textJson;
            textJson["is_end"] = true;
            if (!receivedJson["message"]["reasoning_content"].asString().empty()) {
                textJson["reasoning_content"] = receivedJson["message"]["reasoning_content"];
            }
            textJson["result"] = receivedJson["message"]["content"];
            textJson["sentence_id"] = 0;

            std::string text = custom_nlp_util::jsonToString(textJson);

            ai_engine::gen_ai::nlp::TextGenerationResult result{};
            result.success = true;
            result.error = ai_engine::EngineError{};
            result.modelName = engine->currentDisplayName_;
            result.text = text;
            if (callback) {
                callback(result);
            }
        } else {
            fprintf(stderr, "custom chat failed: %s\n", data.c_str());
        }

        engine->nonStreamData_.clear();
    }
    return true;
}

ai_engine::gen_ai::nlp::TextGenerationResult CustomNlpEnginePrivate::handleToolCallsCompletion() {
    Json::Value finalToolCalls(Json::arrayValue);
    for (const auto &pair : toolCallsAccumulator_) {
        Json::Value toolCall = pair.second;
        finalToolCalls.append(toolCall);
    }

    // 构建统一格式的返回结果
    Json::Value callbackObj;
    callbackObj["is_end"] = true;
    callbackObj["result"] = "";
    callbackObj["sentence_id"] = sentenceId_;
    callbackObj["tool_calls"] = finalToolCalls;

    sentenceId_ += 1;

    // 清空累积器
    toolCallsAccumulator_.clear();
    return ai_engine::gen_ai::nlp::TextGenerationResult{true, ai_engine::EngineError{}, currentDisplayName_,
                                                        custom_nlp_util::jsonToString(callbackObj)};
}

CustomNlpEnginePrivate::CustomNlpEnginePrivate() {}

CustomNlpEnginePrivate::~CustomNlpEnginePrivate() {
    streamChatData_.clear();
    toolCallsAccumulator_.clear();
}

std::string CustomNlpEnginePrivate::engineName() const { return "custom"; }

std::string CustomNlpEnginePrivate::modelInfo() const {
    const char *info = R"(
    {
        "vendor": "自定义模型",
        "models": [
            {
            }
        ]
    }
    )";

    return info;
}

void CustomNlpEnginePrivate::setConfig(const std::string &config) {
    Json::Value configJson = custom_nlp_util::formatJsonFromString(config);
    if (configJson.isNull() || !configJson.isMember("apiKey") || !configJson["apiKey"].isString() ||
        !configJson.isMember("apiUrl") || !configJson["apiUrl"].isString() || !configJson.isMember("modelName") ||
        !configJson["modelName"].isString() || !configJson.isMember("modelVersion") ||
        !configJson["modelVersion"].isString()) {
        std::fprintf(stderr, "Invalid config for custom nlp engine: %s\n", config.c_str());
        return;
    }
    apiKey_ = configJson["apiKey"].asString();
    buildChatAndContentGenerationUrl(configJson["apiUrl"].asString());
    currentModel_ = configJson["modelVersion"].asString();
    currentModelName_ = configJson["modelName"].asString();
    currentDisplayName_ = configJson["displayName"].asString();
}

void CustomNlpEnginePrivate::buildChatAndContentGenerationUrl(const std::string &apiUrl) {
    const std::string chatStr = "chat/completions";
    const std::string generateContentStr = "completions";
    generateContentUrl_ = apiUrl;
    chatUrl_ = apiUrl;
    if (apiUrl.length() >= generateContentStr.length() &&
        apiUrl.substr(apiUrl.length() - generateContentStr.length()) != generateContentStr) {
        if (apiUrl.back() == '/') {
            generateContentUrl_ = apiUrl + generateContentStr;
        } else {
            generateContentUrl_ = apiUrl + "/" + generateContentStr;
        }
    }
    if (apiUrl.length() >= chatStr.length() && apiUrl.substr(apiUrl.length() - chatStr.length()) != chatStr) {
        if (apiUrl.back() == '/') {
            chatUrl_ = apiUrl + chatStr;
        } else {
            chatUrl_ = apiUrl + "/" + chatStr;
        }
    }
}

bool CustomNlpEnginePrivate::setCurrentModel(const std::string &model) { return true; }

void CustomNlpEnginePrivate::setTextGenerationResultCallback(
    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback) {
    textGenerationResultCallback_ = std::move(callback);
}

void CustomNlpEnginePrivate::clearContext() {
    if (context_.isMember("messages")) {
        context_["messages"].clear();
    }

    // 清空请求参数
    for (const auto &item : modelContextMember) {
        context_.removeMember(item);
    }

    stream_ = true;
    context_["stream"] = true;
}

void CustomNlpEnginePrivate::addContextMessage(const std::string &role, const Json::Value &message) {
    if (role == "system") {
        if (message.isMember("content") && !message["content"].asString().empty()) {
            context_["messages"][0]["role"] = role;
            context_["messages"][0]["content"] = message["content"];
        }
        return;
    }
    int size = context_["messages"].size();
    if (role == "tool" && message.isMember("content") && message.isMember("id")) {
        context_["messages"][size]["role"] = role;
        context_["messages"][size]["content"] = message["content"];
        context_["messages"][size]["id"] = message["id"];
        return;
    }
    context_["messages"][size]["role"] = role;
    context_["messages"][size]["content"] = message["content"];
    if (message.isMember("tool_calls") && !message["tool_calls"].empty()) {
        context_["messages"][size]["tool_calls"] = message["tool_calls"];
    }
}

void CustomNlpEnginePrivate::addContextMember(const Json::Value &context) {
    for (const auto &item : modelContextMember) {
        if (!context.isMember(item)) {
            continue;
        }

        context_[item] = context[item];

        // 单独存储的参数成员变量需要同步修改
        if (item == "stream") {
            stream_ = context[item].asBool();
        } else if (item == "tools") {
            context_["parallel_tool_calls"] = true;
            context_[item] = dissectToolFunction(context[item]);
        }
    }
}

void CustomNlpEnginePrivate::decreaseChatCounter() {
    if (chatCounter_ > 0) {
        --chatCounter_;
    }
}

void CustomNlpEnginePrivate::increaseChatCounter() { ++chatCounter_; }

bool CustomNlpEnginePrivate::isChatRunning() const { return chatCounter_ > 0; }

ai_engine::result::AiEngineBaseResult CustomNlpEnginePrivate::init() {
    currentError_ = ai_engine::EngineError();

    context_["model"] = currentModel_;
    context_["stream"] = true;

    return {true, ai_engine::EngineError()};
}

ai_engine::result::AiEngineBaseResult CustomNlpEnginePrivate::generateContent(const std::string &input) {
    bool error{false};
    Json::Value inputJson = custom_nlp_util::stringToJson(input);
    do {
        if (input.empty()) {
            error = true;
            break;
        }
        if (inputJson == Json::objectValue) {
            error = true;
            break;
        }
        if (!inputJson.isMember("prompt")) {
            error = true;
            break;
        }
        if (!inputJson["prompt"].isString()) {
            error = true;
            break;
        }
    } while (false);

    if (error) {
        ai_engine::result::AiEngineBaseResult result{};
        result.success = false;
        result.error =
            ai_engine::EngineError(int(ai_engine::NlpEngineErrorCode::TooLittleData), "Invalid data format.");
        return result;
    }

    std::string promptContent = inputJson["prompt"].asString();

    Json::Value userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = promptContent;

    inputJson["messages"] = Json::Value(Json::arrayValue);
    inputJson["messages"].append(userMessage);
    inputJson.removeMember("prompt");

    std::string message = custom_nlp_util::jsonToString(inputJson);
    fprintf(stdout, "message: %s\n", message.c_str());

    return chat(message);
}

ai_engine::result::AiEngineBaseResult CustomNlpEnginePrivate::chat(const std::string &message) {
    tempStreamChatData_.clear();
    isThinking_ = false;
    hasReasoningContent_ = false;
    sentenceId_ = 0;
    currentError_ = ai_engine::EngineError();

    if (!textGenerationResultCallback_) {
        return {false, currentError_};
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
        printf("CustomNlpEnginePrivate::chat: no chat is running");
        return {true, currentError_};
    }

    ai_engine::result::AiEngineBaseResult result = buildChatMessage(message);
    if (!result.success) {
        return result;
    }

    std::string postString = context_.toStyledString();

    printf("postString: %s", postString.c_str());

    session_.SetTimeout(0);
    session_.SetUrl(cpr::Url{chatUrl_});
    session_.SetHeader(cpr::Header{{"Content-Type", "application/json"}, {"Authorization", "Bearer " + apiKey_}});
    session_.SetBody(cpr::Body{postString});
    session_.SetWriteCallback(cpr::WriteCallback{writeChatData, intptr_t(this)});
    session_.SetConnectTimeout(10000);  // 连接超时10秒
    if (stream_) {
        session_.SetLowSpeed(cpr::LowSpeed{20, 10});
    }
    cpr::Response response = session_.Post();

    return processChatResponse(response);
}

ai_engine::result::AiEngineBaseResult CustomNlpEnginePrivate::buildChatMessage(const std::string &message) {
    Json::Value context = custom_nlp_util::formatJsonFromString(message);
    ai_engine::result::AiEngineBaseResult result;
    for (int i = 0; i < context["messages"].size(); i++) {
        addContextMessage(context["messages"][i]["role"].asString(), context["messages"][i]);
    }

    addContextMember(context);

    result.success = true;
    result.error = ai_engine::EngineError{};
    return result;
}

bool CustomNlpEnginePrivate::hasToolCalls(Json::Value &result) {
    return result.isMember("delta") && result["delta"].isMember("tool_calls") && !result["delta"]["tool_calls"].empty();
}

Json::Value CustomNlpEnginePrivate::dissectToolFunction(const Json::Value &tool) {
    Json::Value toolArray;
    for (uint32_t i = 0; i < tool.size(); ++i) {
        Json::Value toolItem;
        if (tool[i].isMember("name") && tool[i].isMember("description") && tool[i].isMember("parameters")) {
            toolItem["type"] = "function";
            toolItem["function"]["name"] = tool[i]["name"].asString();
            toolItem["function"]["description"] = tool[i]["description"].asString();
            toolItem["function"]["parameters"] = tool[i]["parameters"];
            toolArray.append(toolItem);
        }
    }
    return toolArray;
}

void CustomNlpEnginePrivate::accumulateToolCall(int index, const Json::Value &toolCall) {
    // 查找是否已存在相同索引的工具调用
    auto it = std::find_if(toolCallsAccumulator_.begin(), toolCallsAccumulator_.end(),
                           [index](const std::pair<int, Json::Value> &elem) { return elem.first == index; });

    if (it == toolCallsAccumulator_.end()) {
        // 不存在则添加新条目
        toolCallsAccumulator_.push_back({index, toolCall});
    } else {
        // 存在则更新/累积字段
        Json::Value &existingToolCall = it->second;

        // 更新所有字段，特别处理 arguments 的累积
        if (toolCall.isMember("id") && !toolCall["id"].asString().empty()) {
            existingToolCall["id"] = toolCall["id"];
        }
        if (toolCall.isMember("type")) {
            existingToolCall["type"] = toolCall["type"];
        }
        if (toolCall.isMember("function")) {
            if (toolCall["function"].isMember("name") && !toolCall["function"]["name"].asString().empty()) {
                existingToolCall["function"]["name"] = toolCall["function"]["name"];
            }
            // 累积 arguments 字段
            if (toolCall["function"].isMember("arguments") && toolCall["function"]["arguments"].isString()) {
                if (existingToolCall["function"].isMember("arguments") &&
                    existingToolCall["function"]["arguments"].isString()) {
                    existingToolCall["function"]["arguments"] = existingToolCall["function"]["arguments"].asString() +
                                                                toolCall["function"]["arguments"].asString();
                } else {
                    existingToolCall["function"]["arguments"] = toolCall["function"]["arguments"];
                }
            }
        }
    }
}

bool CustomNlpEnginePrivate::waitForStopped() {
    auto startTime = std::chrono::steady_clock::now();
    while (isStopped_) {
        if (std::chrono::steady_clock::now() - startTime > std::chrono::seconds(10)) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

void CustomNlpEnginePrivate::stopChat() {
    if (!isChatRunning()) {
        printf("CustomNlpEnginePrivate::stopChat: chat is not running");
        return;
    }
    decreaseChatCounter();
    session_.SetTimeout(1);

    isStopped_ = true;
    streamChatData_.clear();
    toolCallsAccumulator_.clear();

    ai_engine::gen_ai::nlp::TextGenerationResult result{true, currentError_, currentDisplayName_,
                                                        std::string{R"({"result":"","is_end":true})"}};

    if (textGenerationResultCallback_ != nullptr) {
        textGenerationResultCallback_(result);
    }
}

void CustomNlpEnginePrivate::stopGenerateContent() {
    if (isStopped_.exchange(true)) {
        return;
    }
    if (isGenerateContentCalled_) {
        isGenerateContentCalled_ = false;
        ai_engine::gen_ai::nlp::TextGenerationResult result{true, currentError_, currentDisplayName_,
                                                            std::string{R"({"result":"","is_end":true})"}};

        if (textGenerationResultCallback_ != nullptr) {
            textGenerationResultCallback_(result);
        }
    }
}

ai_engine::result::AiEngineBaseResult CustomNlpEnginePrivate::destroy() {
    stopChat();
    return {true, ai_engine::EngineError()};
}

bool CustomNlpEnginePrivate::isValidResponse(const std::string &response) const {
    if (response.length() >= 9 && response.compare(0, 5, "data:") != 0 && response.compare(0, 6, "\ndata:") != 0 &&
        response.compare(0, 9, R"({"model":)") != 0) {
        fprintf(stderr, "Invalid response: %s\n", response.c_str());
        return false;
    }
    return true;
}

Json::Value CustomNlpEnginePrivate::formatResultJsonFromInputString(const std::string &inputString) {
    Json::Value inputResultJson = custom_nlp_util::formatJsonFromString(inputString);

    if (preProcessResultJson(inputResultJson)) {
        return Json::Value();
    }

    Json::Value resultJson;
    resultJson["is_end"] = isResultEnd(inputResultJson);

    if (isThinking()) {
        resultJson["reasoning_content"] = getThinkingResult(inputResultJson);
    } else {
        resultJson["result"] = getResult(inputResultJson);
    }
    resultJson["sentence_id"] = sentenceId_;
    sentenceId_ += 1;
    if (inputResultJson["choices"][0]["delta"].isMember("tool_calls")) {
        Json::Value toolCalls = inputResultJson["choices"][0]["delta"]["tool_calls"];
        for (const auto &toolCall : toolCalls) {
            int index = toolCall["index"].asInt();
            accumulateToolCall(index, toolCall);
        }
    }

    return resultJson;
}

bool CustomNlpEnginePrivate::preProcessResultJson(const Json::Value &inputResultJson) {
    const std::string startThinkingStr = "\u003cthink\u003e";
    const std::string endThinkingStr = "\u003c/think\u003e";
    if (isOpenAiApiResult(inputResultJson)) {
        if (inputResultJson["choices"][0]["delta"].isMember("reasoning_content") &&
            !inputResultJson["choices"][0]["delta"]["reasoning_content"].empty()) {
            isThinking_ = true;
            hasReasoningContent_ = true;
            return false;
        }
        if (inputResultJson["choices"][0]["delta"].isMember("reasoning") &&
            !inputResultJson["choices"][0]["delta"]["reasoning"].empty()) {
            isThinking_ = true;
            hasReasoningContent_ = true;
            return false;
        }
        if (hasReasoningContent_) {
            hasReasoningContent_ = false;
            isThinking_ = false;
        }

        if (inputResultJson["choices"][0]["delta"]["content"].asString() == startThinkingStr ||
            inputResultJson["choices"][0]["text"].asString() == startThinkingStr) {
            isThinking_ = true;
            return true;
        }

        if (inputResultJson["choices"][0]["delta"]["content"].asString() == endThinkingStr ||
            inputResultJson["choices"][0]["text"].asString() == endThinkingStr) {
            isThinking_ = false;
            return true;
        }
    }

    if (isOllamaApiResult(inputResultJson)) {
        if (inputResultJson["message"]["content"].asString() == startThinkingStr) {
            isThinking_ = true;
            return true;
        }

        if (inputResultJson["message"]["content"].asString() == endThinkingStr) {
            isThinking_ = false;
            return true;
        }
    }

    return false;
}

bool CustomNlpEnginePrivate::isOpenAiApiResult(const Json::Value &result) const {
    return result.isObject() && result.isMember("choices");
}

bool CustomNlpEnginePrivate::isOllamaApiResult(const Json::Value &result) const {
    return result.isObject() && result.isMember("message") && result["message"].isObject() &&
           result["message"].isMember("content");
}

bool CustomNlpEnginePrivate::isResultEnd(const Json::Value &result) const {
    if (isOpenAiApiResult(result)) {
        return result["choices"][0]["finish_reason"] == "stop" || result["choices"][0]["finish_reason"] == "length";
    }

    if (isOllamaApiResult(result)) {
        return result["done"].asBool();
    }

    return false;
}

bool CustomNlpEnginePrivate::isThinking() const { return isThinking_; }

std::string CustomNlpEnginePrivate::getThinkingResult(const Json::Value &result) const {
    if (isOpenAiApiResult(result)) {
        if (!result["choices"][0]["delta"]["reasoning_content"].empty()) {
            return result["choices"][0]["delta"]["reasoning_content"].asString();
        }
        if (!result["choices"][0]["delta"]["reasoning"].empty()) {
            return result["choices"][0]["delta"]["reasoning"].asString();
        }
        if (!result["choices"][0]["text"].empty()) {
            return result["choices"][0]["text"].asString();
        }
        return result["choices"][0]["delta"]["content"].asString();
    }

    if (isOllamaApiResult(result)) {
        return result["message"]["content"].asString();
    }

    return "";
}

std::string CustomNlpEnginePrivate::getResult(const Json::Value &result) const {
    if (isOpenAiApiResult(result)) {
        return result["choices"][0].isMember("delta")  ? result["choices"][0]["delta"]["content"].asString()
               : result["choices"][0].isMember("text") ? result["choices"][0]["text"].asString()
                                                       : "";
    }

    if (isOllamaApiResult(result)) {
        return result["message"]["content"].asString();
    }

    return "";
}
ai_engine::result::AiEngineBaseResult CustomNlpEnginePrivate::processChatResponse(cpr::Response &response) {
    if (isStopped_ || !isChatRunning()) {
        isStopped_ = false;
        return {true, ai_engine::EngineError()};
    }
    decreaseChatCounter();
    if (response.error.code == cpr::ErrorCode::REQUEST_CANCELLED) {
        streamChatData_.clear();
        toolCallsAccumulator_.clear();
        auto errorCode = custom_nlp_server_error::errorCode2nlpResult(response.status_code);
        currentError_ = ai_engine::EngineError(static_cast<int>(errorCode), currentErrorInfo_);
        return {false, currentError_};
    }

    if (response.error) {
        streamChatData_.clear();
        toolCallsAccumulator_.clear();
        fprintf(stderr, "net error: %s\n", response.error.message.c_str());
        if (response.error.code == cpr::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR) {
            currentError_ =
                ai_engine::EngineError(int(ai_engine::NlpEngineErrorCode::SslCertificateError), response.error.message);
        } else {
            currentError_ =
                ai_engine::EngineError(int(ai_engine::NlpEngineErrorCode::NetworkDisconnected), "HTTP response error");
        }
        return {false, currentError_};
    }
    if (response.status_code < 200 || response.status_code >= 300) {
        streamChatData_.clear();
        toolCallsAccumulator_.clear();
        auto errorCode = custom_nlp_server_error::errorCode2nlpResult(response.status_code);
        currentError_ = ai_engine::EngineError(static_cast<int>(errorCode), "HTTP status error");
        return {false, currentError_};
    }

    streamChatData_.clear();
    toolCallsAccumulator_.clear();
    return {true, currentError_};
}

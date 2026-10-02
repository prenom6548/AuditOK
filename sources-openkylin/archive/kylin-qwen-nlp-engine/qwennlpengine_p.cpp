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

#include "qwennlpengine_p.h"

#include <cstdio>
#include <algorithm>

#include "servererror.h"
#include "util.h"

QwenNlpEnginePrivate::QwenNlpEnginePrivate() {}

QwenNlpEnginePrivate::~QwenNlpEnginePrivate() {
    streamChatData_.clear();
    toolCallsAccumulator_.clear();
    nonStreamData_.clear();
}

std::string QwenNlpEnginePrivate::engineName() const { return "qwen"; }

std::string QwenNlpEnginePrivate::modelInfo() const {
    const char *info = R"(
    {
        "vendor": "通义千问",
        "models": [
            {
                "modelName": "ali-qwen-plus",
                "displayName": "通义千问-qwen-plus",
                "model": "qwen-plus",
                "subConfig": [
                    {
                        "name": "qwen-plus",
                        "config": [
                            {
                                "friendlyName": "APIKey",
                                "configName": "apiKey"
                            }
                        ]
                    }
                ],
                "maxTokens": 8192,
                "feedbackUrl": "https://www.aliyun.com/service",
                "supportsWebSearch": true,
                "supportsThinking": true,
                "supportsThinkingToggle": true
            }
        ]
    }
    )";

    return info;
}

void QwenNlpEnginePrivate::setConfig(const std::string &config) {
    Json::Value configJson = qwen_nlp_util::formatJsonFromString(config);
    if (configJson.isNull() || !configJson.isMember("apiKey") || !configJson["apiKey"].isString()) {
        std::fprintf(stderr, "Invalid config for qwen nlp engine: %s\n", config.c_str());
        return;
    }
    apiKey_ = configJson["apiKey"].asString();
}

bool QwenNlpEnginePrivate::setCurrentModel(const std::string &model) {
    const Json::Value modelInfoJson = qwen_nlp_util::formatJsonFromString(modelInfo());
    for (const auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            currentModel_ = model;
            return true;
        }
    }
    fprintf(stderr, "Invalid current model name: %s\n", model.c_str());
    return false;
}

void QwenNlpEnginePrivate::setTextGenerationResultCallback(
    const ai_engine::gen_ai::nlp::TextGenerationResultCallback &callback) {
    textGenerationResultCallback_ = callback;
}

namespace {
const std::vector<std::string> modelContextMember{"stream",      "max_tokens",  "enable_search", "enable_thinking",
                                                  "temperature", "top_p",       "top_k",         "presence_penalty",
                                                  "n",           "tool_choice", "tools"};

std::vector<std::string> splitDataString(const std::string &dataStr) {
    std::vector<std::string> datas;

    int oldIndex = 0;
    int newIndex = 0;
    while ((newIndex = dataStr.find_first_of("\n\n", newIndex)) != std::string::npos) {
        datas.push_back(dataStr.substr(oldIndex, newIndex - oldIndex));
        newIndex += 2;
        oldIndex = newIndex;
    }
    return datas;
}

bool stringStartsWith(const std::string &str, const std::string &prefix) {
    if (prefix.length() > str.length()) {
        return false;
    }
    return str.substr(0, prefix.length()) == prefix;
}
}  // namespace

bool writeChatData(std::string data, intptr_t userData) {
    QwenNlpEnginePrivate *engine = (QwenNlpEnginePrivate *)userData;
    ai_engine::gen_ai::nlp::TextGenerationResultCallback callback = engine->textGenerationResultCallback_;

    if (engine->isStopped_ || !engine->isChatRunning()) {
        return false;
    }
    std::string resultJsonString;
    if (stringStartsWith(data, "data: ")) {
        resultJsonString = data.substr(6);
    } else {
        resultJsonString = data;
    }
    Json::Value resultJson = qwen_nlp_util::formatJsonFromString(resultJsonString);
    if (resultJson.isObject() && !resultJson.isNull() && resultJson.isMember("error")) {
        fprintf(stderr, "qwen chat failed: %s\n", data.c_str());
        engine->currentErrorInfo_ = data;
        return false;
    }
    if (engine->stream_) {
        std::string resultString = engine->tempStreamChatData_ + data;
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
            Json::Value obj = qwen_nlp_util::formatJsonFromString(data);
            Json::Value callbackObj;
            callbackObj["is_end"] = static_cast<bool>(obj["choices"][0]["finish_reason"] == "stop");
            callbackObj["result"] = obj["choices"][0]["delta"]["content"];
            callbackObj["reasoning_content"] = obj["choices"][0]["delta"]["reasoning_content"];
            callbackObj["sentence_id"] = engine->sentenceId_;
            if (obj["choices"][0]["delta"].isMember("tool_calls")) {
                Json::Value toolCalls = obj["choices"][0]["delta"]["tool_calls"];
                for (const auto &toolCall : toolCalls) {
                    int index = toolCall["index"].asInt();
                    engine->accumulateToolCall(index, toolCall);
                }
            } else {
                if ((obj["choices"][0]["finish_reason"] == "tool_calls" ||
                     obj["choices"][0]["finish_reason"] == "stop") &&
                    !engine->toolCallsAccumulator_.empty()) {
                    auto result = engine->handleToolCallsCompletion();
                    if (callback) {
                        callback(result);
                    }
                    return true;
                }
                engine->sentenceId_ += 1;
                engine->streamChatData_.push_back(callbackObj["result"]);
                ai_engine::gen_ai::nlp::TextGenerationResult result{
                    true, ai_engine::EngineError(), engine->currentDisplayName_, callbackObj.toStyledString()};
                if (callback) {
                    callback(result);
                }
            }
        }
    } else {
        engine->nonStreamData_ += data;
        auto optional = qwen_nlp_util::stringToJson(engine->nonStreamData_);
        if (!optional.has_value()) {
            return true;
        }
        auto result = engine->handleNonStreamCompletion(optional.value());
        callback(result);
    }
    return true;
}

ai_engine::gen_ai::nlp::TextGenerationResult QwenNlpEnginePrivate::handleToolCallsCompletion() {
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
                                                        qwen_nlp_util::jsonToString(callbackObj)};
}

ai_engine::gen_ai::nlp::TextGenerationResult QwenNlpEnginePrivate::handleNonStreamCompletion(
    Json::Value &receivedJson) {
    Json::Value textJson;
    textJson["is_end"] = receivedJson["choices"][0]["finish_reason"] == "stop";
    textJson["result"] = receivedJson["choices"][0]["message"]["content"];
    textJson["reasoning_content"] = receivedJson["choices"][0]["message"]["reasoning_content"];
    textJson["sentence_id"] = 0;
    if (receivedJson["choices"][0]["message"].isMember("tool_calls")) {
        textJson["tool_calls"] = receivedJson["choices"][0]["message"]["tool_calls"];
    }
    ai_engine::gen_ai::nlp::TextGenerationResult result{};
    result.success = true;
    result.error = ai_engine::EngineError{};
    result.modelName = currentDisplayName_;
    result.text = qwen_nlp_util::jsonToString(textJson);
    return result;
}

int QwenNlpEnginePrivate::getMaxTokens(const std::string &model) {
    Json::Value modelInfoJson = qwen_nlp_util::formatJsonFromString(modelInfo());
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            maxTokens_ = item["maxTokens"].asInt();
            return maxTokens_;
        }
    }
    return -1;
}

void QwenNlpEnginePrivate::clearContext() {
    if (context_.isMember("messages")) {
        context_["messages"].clear();
    }

    // 清空请求参数
    for (const auto &item : modelContextMember) {
        context_.removeMember(item);
    }

    // stream、max_tokens、enable_search 恢复默认值
    stream_ = true;
    context_["stream"] = stream_.load();
    if (getMaxTokens(currentModel_) != -1) {
        context_["max_tokens"] = maxTokens_;
    }
    context_["enable_search"] = true;
}

void QwenNlpEnginePrivate::addContextMessage(const std::string &role, const Json::Value &content) {
    if (role == "system") {
        if (content.isMember("content") && content["content"].asString() != "") {
            context_["messages"][0]["role"] = role;
            context_["messages"][0]["content"] = content["content"];
        }
        return;
    }
    int size = context_["messages"].size();
    if (role == "tool" && content.isMember("content") && content.isMember("id")) {
        context_["messages"][size]["role"] = role;
        context_["messages"][size]["content"] = content["content"];
        context_["messages"][size]["id"] = content["id"];
        return;
    }
    context_["messages"][size]["role"] = role;
    context_["messages"][size]["content"] = content["content"];
    if (content.isMember("tool_calls") && !content["tool_calls"].empty()) {
        context_["messages"][size]["tool_calls"] = content["tool_calls"];
    }
}

void QwenNlpEnginePrivate::convertContextMember(Json::Value &context) {
    // 输入的参数名称和千问模型的参数映射关系，相同名称无需映射
    const std::map<std::string, std::string> convertMap{{"web_search", "enable_search"},
                                                        {"max_completion_tokens", "max_tokens"}};

    for (const auto &item : convertMap) {
        if (context.isMember(item.first)) {
            context[item.second] = context[item.first];
            context.removeMember(item.first);
        }
    }
}

void QwenNlpEnginePrivate::addContextMember(const Json::Value &context) {
    for (const auto &item : modelContextMember) {
        if (!context.isMember(item)) {
            continue;
        }

        context_[item] = context[item];

        // 单独存储的参数成员变量需要同步修改
        if (item == "stream") {
            stream_ = context[item].asBool();
        } else if (item == "max_tokens") {
            maxTokens_ = context[item].asInt();
        } else if (item == "tools") {
            context_["parameters"]["parallel_tool_calls"] = false;
            context_[item] = dissectToolFunction(context[item]);
        }
    }
}

ai_engine::result::AiEngineBaseResult QwenNlpEnginePrivate::buildChatMessage(const std::string &message) {
    ai_engine::result::AiEngineBaseResult result;
    auto optional = qwen_nlp_util::stringToJson(message);
    if (!optional.has_value()) {
        result.success = false;
        result.error = {int(ai_engine::NlpEngineErrorCode::InvalidParamType), "The input information is not JSON."};
        return result;
    }
    Json::Value context = optional.value();

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
        addContextMessage(role, item);
    }

    convertContextMember(context);
    addContextMember(context);

    result.success = true;
    result.error = ai_engine::EngineError{};
    return result;
}

Json::Value QwenNlpEnginePrivate::dissectToolFunction(const Json::Value &tool) {
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

void QwenNlpEnginePrivate::removeLastContext() {
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

void QwenNlpEnginePrivate::accumulateToolCall(int index, const Json::Value &toolCall) {
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
            if (toolCall["function"].isMember("name")) {
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

void QwenNlpEnginePrivate::decreaseChatCounter() {
    if (chatCounter_ > 0) {
        --chatCounter_;
    }
}

void QwenNlpEnginePrivate::increaseChatCounter() { ++chatCounter_; }

bool QwenNlpEnginePrivate::isChatRunning() const { return chatCounter_ > 0; }

ai_engine::result::AiEngineBaseResult QwenNlpEnginePrivate::init() {
    currentError_ = ai_engine::EngineError();

    context_["model"] = currentModel_;
    context_["stream"] = stream_.load();
    if (getMaxTokens(currentModel_) != -1) {
        context_["max_tokens"] = maxTokens_;
    }
    context_["enable_search"] = true;

    return {true, ai_engine::EngineError()};
}

ai_engine::result::AiEngineBaseResult QwenNlpEnginePrivate::generateContent(const std::string &input) {
    bool error{false};
    auto optional = qwen_nlp_util::stringToJson(input);
    Json::Value inputJson;
    do {
        if (input.empty()) {
            error = true;
            break;
        }
        if (!optional.has_value()) {
            error = true;
            break;
        }
        inputJson = optional.value();
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

    std::string message = qwen_nlp_util::jsonToString(inputJson);
    fprintf(stdout, "message: %s\n", message.c_str());

    return chat(message);
}

ai_engine::result::AiEngineBaseResult QwenNlpEnginePrivate::chat(const std::string &message) {
    tempStreamChatData_.clear();
    currentError_ = ai_engine::EngineError();
    sentenceId_ = 0;

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
        printf("QwenNlpEnginePrivate::chat: no chat is running");
        return {true, currentError_};
    }

    ai_engine::result::AiEngineBaseResult result = buildChatMessage(message);
    if (!result.success) {
        return result;
    }
    std::string postString = qwen_nlp_util::jsonToString(context_);
    printf("post string %s\n", postString.c_str());

    cpr::Url url{"https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions"};
    cpr::Header header{cpr::Header{{"Content-Type", "application/json"}, {"Authorization", "Bearer " + apiKey_}}};
    cpr::Body body{cpr::Body{{postString}}};
    cpr::Response response;
    if (stream_) {
        response =
            cpr::Post(url, header, body, cpr::WriteCallback{writeChatData, (intptr_t)this}, cpr::LowSpeed(20, 10));
    } else {
        response = cpr::Post(url, header, body, cpr::WriteCallback{writeChatData, (intptr_t)this});
    }

    return processChatResponse(response);
}

bool QwenNlpEnginePrivate::waitForStopped() {
    auto startTime = std::chrono::steady_clock::now();
    while (isStopped_) {
        if (std::chrono::steady_clock::now() - startTime > std::chrono::seconds(10)) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

void QwenNlpEnginePrivate::stopChat() {
    if (!isChatRunning()) {
        printf("QwenNlpEnginePrivate::stopChat: chat is not running");
        return;
    }
    decreaseChatCounter();

    isStopped_ = true;
    streamChatData_.clear();
    toolCallsAccumulator_.clear();
    nonStreamData_.clear();

    ai_engine::gen_ai::nlp::TextGenerationResult result{true, currentError_, currentDisplayName_,
                                                        std::string{R"({"result":"","is_end":true})"}};

    if (textGenerationResultCallback_ != nullptr) {
        textGenerationResultCallback_(result);
    }
}

void QwenNlpEnginePrivate::stopGenerateContent() { isStopped_ = true; }

ai_engine::result::AiEngineBaseResult QwenNlpEnginePrivate::destroy() {
    stopChat();
    return {true, ai_engine::EngineError()};
}

bool QwenNlpEnginePrivate::isTopKLimitError(const std::string &errorString,
                                            ai_engine::result::AiEngineBaseResult &result) {
    //  errorString以"data: "开头
    //  {
    //      "error":
    //      {
    //          "code":"invalid_parameter_error",
    //          "param":null,
    //          "message":"<400> InternalError.Algo.InvalidParameter: Parameter top_k be greater than or equal to 0",
    //          "type":"invalid_request_error"
    //      },
    //      "id":"chatcmpl-3f4419f6-9a01-405e-8f44-e6b654632bdc"
    //  }
    const std::string prefix = "data: ";
    std::string errorJsonString;
    if (stringStartsWith(errorString, prefix)) {
        errorJsonString = errorString.substr(prefix.size());
    }
    auto optional = qwen_nlp_util::stringToJson(errorJsonString);
    if (optional.has_value()) {
        const Json::Value &errorJson = optional.value();
        if (errorJson.isMember("error") && errorJson["error"].isObject()) {
            const Json::Value &subErrorJson = errorJson["error"];
            bool codeValid = subErrorJson.isMember("code") && subErrorJson["code"].isString() &&
                             subErrorJson["code"].asString() == "invalid_parameter_error";
            bool typeValid = subErrorJson.isMember("type") && subErrorJson["type"].isString() &&
                             subErrorJson["type"].asString() == "invalid_request_error";
            bool messageValid = subErrorJson.isMember("message") && subErrorJson["message"].isString();
            if (codeValid && typeValid && messageValid) {
                std::string message = subErrorJson["message"].asString();
                if (message.find("Parameter top_k be greater than or equal to 0") != std::string::npos) {
                    ai_engine::EngineError error(int(ai_engine::NlpEngineErrorCode::InvalidParamType),
                                                 currentErrorInfo_);
                    result = {false, error};
                    return true;
                }
            }
        }
    }
    return false;
}

ai_engine::result::AiEngineBaseResult QwenNlpEnginePrivate::processChatResponse(cpr::Response &response) {
    if (isStopped_ || !isChatRunning()) {
        isStopped_ = false;
        return {true, ai_engine::EngineError()};
    }
    decreaseChatCounter();
    if (response.error.code == cpr::ErrorCode::REQUEST_CANCELLED) {
        streamChatData_.clear();
        toolCallsAccumulator_.clear();
        nonStreamData_.clear();
        if (ai_engine::result::AiEngineBaseResult result{}; isTopKLimitError(currentErrorInfo_, result)) {
            return result;
        }
        currentError_ = ai_engine::EngineError(int(qwen_nlp_server_error::errorCode2nlpResult(response.status_code)),
                                               currentErrorInfo_);
        return {false, currentError_};
    }

    if (response.error) {
        streamChatData_.clear();
        toolCallsAccumulator_.clear();
        nonStreamData_.clear();
        fprintf(stderr, "net error: %s\n", response.error.message.c_str());
        if (response.error.code == cpr::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR) {
            currentError_ =
                ai_engine::EngineError(int(ai_engine::NlpEngineErrorCode::SslCertificateError), response.error.message);
        } else {
            currentError_ =
                ai_engine::EngineError(int(ai_engine::NlpEngineErrorCode::NetworkDisconnected), response.error.message);
        }
        return {false, currentError_};
    }

    streamChatData_.clear();
    toolCallsAccumulator_.clear();
    nonStreamData_.clear();
    return {true, currentError_};
}

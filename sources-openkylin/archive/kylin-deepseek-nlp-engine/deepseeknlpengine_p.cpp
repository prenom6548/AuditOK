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

#include "deepseeknlpengine_p.h"

#include <kylin-ai/common/error.h>

#include <algorithm>
#include <cstdio>

#include "servererror.h"
#include "util.h"

namespace {
std::map<std::string, std::string> modelNames = {{"deepseek-chat", "deepseek-v3"},
                                                 {"deepseek-reasoner", "deepseek-r1"}};

std::map<std::string, std::string> displayNames = {{"deepseek-chat", "DeepSeek-V3"},
                                                   {"deepseek-reasoner", "DeepSeek-R1"}};

const std::vector<std::string> modelContextMember{"stream",           "max_tokens",  "temperature", "top_p",
                                                  "presence_penalty", "tool_choice", "tools"};
}  // namespace

DeepseekNlpEnginePrivate::DeepseekNlpEnginePrivate() : currentModel_("deepseek-chat") {}

DeepseekNlpEnginePrivate::~DeepseekNlpEnginePrivate() {
    streamChatData_.clear();
    toolCallsAccumulator_.clear();
}

std::string DeepseekNlpEnginePrivate::engineName() const { return "deepseek"; }

std::string DeepseekNlpEnginePrivate::currentModel() const { return currentModel_; }

std::string DeepseekNlpEnginePrivate::currentModelName() const { return modelNames[currentModel_]; }

std::string DeepseekNlpEnginePrivate::modelInfo() const {
    const char *info = R"(
    {
        "vendor": "深度求索",
        "models": [
            {
                "modelName": "deepseek-v3",
                "displayName": "DeepSeek-V3",
                "model": "deepseek-chat",
                "subConfig": [
                    {
                        "name": "deepseek-chat",
                        "config": [
                            {
                                "friendlyName": "APIKey",
                                "configName": "apiKey"
                            }
                        ]
                    }
                ],
                "maxTokens": 8192,
                "feedbackUrl": "https://platform.deepseek.com/usage",
                "supportsWebSearch": false,
                "supportsThinking": false,
                "supportsThinkingToggle": false
            },
            {
                "modelName": "deepseek-r1",
                "displayName": "DeepSeek-R1",
                "model": "deepseek-reasoner",
                "subConfig": [
                    {
                        "name": "deepseek-reasoner",
                        "config": [
                            {
                                "friendlyName": "APIKey",
                                "configName": "apiKey"
                            }
                        ]
                    }
                ],
                "maxTokens": 8192,
                "feedbackUrl": "https://platform.deepseek.com/usage",
                "supportsWebSearch": false,
                "supportsThinking": true,
                "supportsThinkingToggle": false
            }
        ]
    }
    )";

    return info;
}

void DeepseekNlpEnginePrivate::setConfig(const std::string &config) {
    auto optional = deepseek_nlp_util::stringToJson(config);
    if (!optional.has_value()) {
        std::fprintf(stderr, "Invalid config for deepseek nlp engine: %s\n", config.c_str());
        return;
    }
    Json::Value configJson = optional.value();
    if (!configJson.isMember("apiKey") || !configJson["apiKey"].isString() || configJson["apiKey"].asString().empty()) {
        std::fprintf(stderr, "Invalid config for deepseek nlp engine: %s\n", config.c_str());
        return;
    }
    apiKey_ = configJson["apiKey"].asString();
}

void DeepseekNlpEnginePrivate::setTextGenerationResultCallback(
    const ai_engine::gen_ai::nlp::TextGenerationResultCallback &callback) {
    textGenerationResultCallback_ = callback;
}

bool DeepseekNlpEnginePrivate::setCurrentModel(const std::string &model) {
    if (modelNames.find(model) == modelNames.end()) {
        return false;
    }

    currentModel_ = model;
    return true;
}
namespace {
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
}  // namespace

bool DeepseekNlpEnginePrivate::handleStreamMessage(const std::string &data) {
    std::string resultString = tempStreamChatData_ + data;
    if (resultString.substr(resultString.length() - 2) != "\n\n") {
        tempStreamChatData_ += data;
        return true;
    }
    tempStreamChatData_.clear();

    std::vector<std::string> dataStrings = splitDataString(resultString);
    if (dataStrings[0] == ": keep-alive") {
        return true;
    }
    for (std::string data : dataStrings) {
        data = data.substr(6, data.length() - 1);
        if (data == "[DONE]") {
            break;
        }
        auto optional = deepseek_nlp_util::stringToJson(data);
        if (!optional.has_value()) {
            continue;
        }
        Json::Value obj = optional.value();

        Json::Value callbackObj;
        callbackObj["is_end"] = static_cast<bool>(obj["choices"][0]["finish_reason"] == "stop");
        if (!obj["choices"][0]["delta"]["reasoning_content"].empty()) {
            callbackObj["reasoning_content"] = obj["choices"][0]["delta"]["reasoning_content"];
        }

        callbackObj["result"] = obj["choices"][0]["delta"]["content"];
        callbackObj["sentence_id"] = sentenceId_;
        if (obj["choices"][0]["delta"].isMember("tool_calls")) {
            Json::Value toolCalls = obj["choices"][0]["delta"]["tool_calls"];
            for (const auto &toolCall : toolCalls) {
                int index = toolCall["index"].asInt();
                accumulateToolCall(index, toolCall);
            }
        } else {
            if ((obj["choices"][0]["finish_reason"] == "tool_calls" || obj["choices"][0]["finish_reason"] == "stop") &&
                !toolCallsAccumulator_.empty()) {
                auto result = handleToolCallsCompletion();
                if (textGenerationResultCallback_) {
                    textGenerationResultCallback_(result);
                }
                return true;
            }
            sentenceId_ += 1;
            streamChatData_.push_back(callbackObj["result"]);
            ai_engine::gen_ai::nlp::TextGenerationResult result{};
            result.success = true;
            result.error = ai_engine::EngineError{};
            result.modelName = displayNames[currentModel_];
            result.text = deepseek_nlp_util::jsonToString(callbackObj);
            if (textGenerationResultCallback_) {
                textGenerationResultCallback_(result);
            }
        }
    }
    return true;
}

bool DeepseekNlpEnginePrivate::handleNonStreamMessage(const std::string &data) {
    nonStreamData_ += data;
    auto optional = deepseek_nlp_util::stringToJson(nonStreamData_);
    if (!optional.has_value()) {
        // 不完整的json，继续接收，直到完整的json
        return true;
    }
    Json::Value receivedJson = optional.value();
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
    result.modelName = displayNames[currentModel_];
    result.text = deepseek_nlp_util::jsonToString(textJson);
    if (textGenerationResultCallback_) {
        textGenerationResultCallback_(result);
    }
    nonStreamData_.clear();
    return true;
}

ai_engine::gen_ai::nlp::TextGenerationResult DeepseekNlpEnginePrivate::handleToolCallsCompletion() {
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

    ai_engine::gen_ai::nlp::TextGenerationResult result{};
    result.success = true;
    result.error = ai_engine::EngineError{};
    result.modelName = displayNames[currentModel_];
    result.text = deepseek_nlp_util::jsonToString(callbackObj);
    return result;
}

bool DeepseekNlpEnginePrivate::onMessage(const std::string &data, intptr_t userData) {
    DeepseekNlpEnginePrivate *engine = (DeepseekNlpEnginePrivate *)userData;
    auto callback = engine->textGenerationResultCallback_;

    if (engine->isStopped_ || !engine->isChatRunning()) {
        return false;
    }

    auto optional = deepseek_nlp_util::stringToJson(data);
    if (optional.has_value()) {
        // 解析正确并且读出error字段
        Json::Value resultJson = optional.value();
        if (resultJson.isObject() && resultJson.isMember("error")) {
            fprintf(stderr, "deepseek chat failed: %s\n", data.c_str());
            engine->currentErrorInfo_ = data;
            return false;
        }
    }

    if (engine->stream_) {
        return engine->handleStreamMessage(data);
    } else {
        return engine->handleNonStreamMessage(data);
    }
}

int DeepseekNlpEnginePrivate::getMaxTokens(const std::string &model) {
    auto optional = deepseek_nlp_util::stringToJson(modelInfo());
    if (!optional.has_value()) {
        return -1;
    }
    Json::Value modelInfoJson = optional.value();
    for (auto &item : modelInfoJson["models"]) {
        if (item["model"].asString() == model) {
            maxTokens_ = item["maxTokens"].asInt();
            return maxTokens_;
        }
    }
    return -1;
}

void DeepseekNlpEnginePrivate::clearContext() {
    if (context_.isMember("messages")) {
        context_["messages"].clear();
    }

    for (const auto &item : modelContextMember) {
        context_.removeMember(item);
    }

    stream_ = true;
    context_["stream"] = stream_.load();
    if (getMaxTokens(currentModel_) != -1) {
        context_["max_tokens"] = maxTokens_;
    }
}

void DeepseekNlpEnginePrivate::addContextMessage(const std::string &role, const Json::Value &message) {
    if (role == "system") {
        if (message.isMember("content") && message["content"].asString() != "") {
            context_["messages"][0]["role"] = role;
            context_["messages"][0]["content"] = message["content"];
        }
        return;
    }
    int size = context_["messages"].size();
    if (role == "tool" && message.isMember("content") && message.isMember("id")) {
        context_["messages"][size]["role"] = role;
        context_["messages"][size]["content"] = message["content"];
        context_["messages"][size]["tool_call_id"] = message["id"];
        return;
    }
    context_["messages"][size]["role"] = role;
    context_["messages"][size]["content"] = message["content"];
    if (message.isMember("tool_calls") && !message["tool_calls"].empty()) {
        context_["messages"][size]["tool_calls"] = message["tool_calls"];
        // reason_content is necessary when tool_call is not empty
        context_["messages"][size]["reasoning_content"] = "";
    }
}

void DeepseekNlpEnginePrivate::addContextMember(const Json::Value &context) {
    context_["model"] = currentModel_;

    for (const auto &item : modelContextMember) {
        if (!context.isMember(item)) {
            continue;
        }
        context_[item] = context[item];

        if (item == "stream") {
            stream_ = context["stream"].asBool();
        } else if (item == "max_tokens") {
            maxTokens_ = context[item].asInt();
        } else if (item == "tools") {
            context_["parameters"]["parallel_tool_calls"] = false;
            context_[item] = dissectToolFunction(context[item]);
        }
    }
}

void DeepseekNlpEnginePrivate::removeLastContext() {
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

void DeepseekNlpEnginePrivate::decreaseChatCounter() {
    if (chatCounter_ > 0) {
        --chatCounter_;
    }
}

void DeepseekNlpEnginePrivate::increaseChatCounter() { ++chatCounter_; }

bool DeepseekNlpEnginePrivate::isChatRunning() const { return chatCounter_ > 0; }

ai_engine::result::AiEngineBaseResult DeepseekNlpEnginePrivate::init() {
    context_["model"] = currentModel_;
    if (getMaxTokens(currentModel_) != -1) {
        context_["max_tokens"] = maxTokens_;
    }
    context_["stream"] = true;
    return {true};
}

ai_engine::result::AiEngineBaseResult DeepseekNlpEnginePrivate::destroy() {
    stopChat();
    return {true};
}

ai_engine::result::AiEngineBaseResult DeepseekNlpEnginePrivate::generateContent(const std::string &input) {
    bool error{false};
    auto optional = deepseek_nlp_util::stringToJson(input);
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

    std::string message = deepseek_nlp_util::jsonToString(inputJson);
    fprintf(stdout, "message: %s\n", message.c_str());

    return chat(message);
}

ai_engine::result::AiEngineBaseResult DeepseekNlpEnginePrivate::chat(const std::string &message) {
    tempStreamChatData_.clear();
    sentenceId_ = 0;

    if (!textGenerationResultCallback_) {
        return {false};
    }
    if (apiKey_.empty()) {
        ai_engine::EngineError error((int)ai_engine::NlpEngineErrorCode::Unauthorized, "The apiKey is empty");
        return {false, error};
    }
    if (message.empty()) {
        ai_engine::EngineError error((int)ai_engine::NlpEngineErrorCode::TooLittleData, "The message is empty");
        return {false, error};
    }

    clearContext();
    increaseChatCounter();

    if (!waitForStopped()) {
        isStopped_ = false;
    }

    if (!isChatRunning()) {
        printf("DeepseekNlpEnginePrivate::chat: no chat is running");
        return {true, currentError_};
    }

    ai_engine::result::AiEngineBaseResult result = buildChatMessage(message);
    if (!result.success) {
        return result;
    }

    std::string postString = deepseek_nlp_util::jsonToString(context_);

    cpr::Url url{"https://api.deepseek.com/chat/completions"};
    cpr::Header header{cpr::Header{{"Content-Type", "application/json"}, {"Authorization", "Bearer " + apiKey_}}};
    cpr::Body body{cpr::Body{{postString}}};

    std::fprintf(stderr, "DeepSeek postString: %s\n", postString.c_str());

    cpr::Response response;
    if (stream_) {
        response = cpr::Post(url, header, body, cpr::WriteCallback{onMessage, intptr_t(this)}, cpr::LowSpeed(20, 10));

    } else {
        response = cpr::Post(url, header, body, cpr::WriteCallback{onMessage, intptr_t(this)});
    }

    return processChatResponse(response);
}

ai_engine::result::AiEngineBaseResult DeepseekNlpEnginePrivate::buildChatMessage(const std::string &message) {
    auto optional = deepseek_nlp_util::stringToJson(message);
    ai_engine::result::AiEngineBaseResult result;
    if (!optional.has_value()) {
        ai_engine::EngineError error(int(ai_engine::NlpEngineErrorCode::InvalidParamType),
                                     "The JSON message body format is incorrect.");
        return {false, error};
    }
    const Json::Value &context = optional.value();
    for (int i = 0; i < context["messages"].size(); i++) {
        addContextMessage(context["messages"][i]["role"].asString(), context["messages"][i]);
    }

    addContextMember(context);

    result.success = true;
    result.error = ai_engine::EngineError{};
    return result;
}

Json::Value DeepseekNlpEnginePrivate::dissectToolFunction(const Json::Value &tool) {
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

void DeepseekNlpEnginePrivate::accumulateToolCall(int index, const Json::Value &toolCall) {
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

void DeepseekNlpEnginePrivate::stopGenerateContent() {}

bool DeepseekNlpEnginePrivate::waitForStopped() {
    auto startTime = std::chrono::steady_clock::now();
    while (isStopped_) {
        if (std::chrono::steady_clock::now() - startTime > std::chrono::seconds(10)) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

void DeepseekNlpEnginePrivate::stopChat() {
    if (!isChatRunning()) {
        printf("DeepseekNlpEnginePrivate::stopChat: chat is not running");
        return;
    }
    decreaseChatCounter();

    isStopped_ = true;
    streamChatData_.clear();
    toolCallsAccumulator_.clear();

    ai_engine::gen_ai::nlp::TextGenerationResult result{
        true, {}, modelNames[currentModel_], std::string{R"({"result":"","is_end":true})"}};
    if (textGenerationResultCallback_ != nullptr) {
        textGenerationResultCallback_(result);
    }
}

bool DeepseekNlpEnginePrivate::isTokenLimitError(const std::string &errorString,
                                                 ai_engine::result::AiEngineBaseResult &result) {
    //  {
    //      "error":
    //      {
    //          "message":"This model's maximum context length is 131072 tokens. However, you requested 194097 tokens
    //          (185905 in the messages, 8192 in the completion). Please reduce the length of the messages or
    //          completion.",
    //          "type":"invalid_request_error",
    //          "param":null,
    //          "code":"invalid_request_error"
    //      }
    //  }

    auto optional = deepseek_nlp_util::stringToJson(errorString);
    if (optional.has_value()) {
        const Json::Value &errorJson = optional.value();
        if (errorJson.isMember("error") && errorJson["error"].isObject()) {
            const Json::Value &subErrorJson = errorJson["error"];
            bool codeValid = subErrorJson.isMember("code") && subErrorJson["code"].isString() &&
                             subErrorJson["code"].asString() == "invalid_request_error";
            bool typeValid = subErrorJson.isMember("type") && subErrorJson["type"].isString() &&
                             subErrorJson["type"].asString() == "invalid_request_error";
            bool messageValid = subErrorJson.isMember("message") && subErrorJson["message"].isString();
            if (codeValid && typeValid && messageValid) {
                std::string message = subErrorJson["message"].asString();
                if (message.find("Please reduce the length of the messages or completion") != std::string::npos) {
                    ai_engine::EngineError error(AiSdkCommonErrorCode::AI_COMMON_NLP_TOKEN_LIMITED, currentErrorInfo_);
                    result = {false, error};
                    return true;
                }
            }
        }
    }
    return false;
}

ai_engine::result::AiEngineBaseResult DeepseekNlpEnginePrivate::processChatResponse(cpr::Response &response) {
    if (isStopped_ || !isChatRunning()) {
        isStopped_ = false;
        return {true};
    }

    decreaseChatCounter();
    if (response.error.code == cpr::ErrorCode::REQUEST_CANCELLED) {
        streamChatData_.clear();
        toolCallsAccumulator_.clear();
        if (ai_engine::result::AiEngineBaseResult result{}; isTokenLimitError(currentErrorInfo_, result)) {
            return result;
        }
        ai_engine::EngineError error((int)deepseek_nlp_server_error::errorCode2nlpResult(response.status_code),
                                     currentErrorInfo_);
        return {false, error};
    }

    if (response.error) {
        streamChatData_.clear();
        toolCallsAccumulator_.clear();
        fprintf(stderr, "net error: %s\n", response.error.message.c_str());
        ai_engine::EngineError error;
        if (response.error.code == cpr::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR) {
            error =
                ai_engine::EngineError(int(ai_engine::NlpEngineErrorCode::SslCertificateError), response.error.message);
        } else {
            error =
                ai_engine::EngineError(int(ai_engine::NlpEngineErrorCode::NetworkDisconnected), response.error.message);
        }
        return {false, error};
    }

    streamChatData_.clear();
    toolCallsAccumulator_.clear();
    return {true};
}

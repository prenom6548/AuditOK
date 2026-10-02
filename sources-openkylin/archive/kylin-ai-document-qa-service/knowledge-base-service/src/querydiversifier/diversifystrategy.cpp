#include "diversifystrategy.h"

#include <kyai-business-framework/log.h>
#include <kylin-ai/genai/text/chat.h>
#include <kylin-ai/genai/text/chatmessage.h>
#include <kylin-ai/genai/text/config.h>
#include <kylin-ai/genai/text/error.h>
#include <kylin-ai/genai/text/result.h>

#include <nlohmann/json.hpp>

#include "../configmanager.h"
#include "querydiversifier.h"

namespace knowledgebase {

LLMDiversifierStrategy::LLMDiversifierStrategy() {
    session_ = genai_text_create_session();
    if (session_ == nullptr) {
        PRINT_ERROR("创建 llm diversify session 失败\n");
        return;
    }

    chatConfig_ = chat_model_config_create();
    if (chatConfig_ == nullptr) {
        PRINT_ERROR("创建 llm chatConfig 失败\n");
        return;
    }
}
LLMDiversifierStrategy::~LLMDiversifierStrategy() {
    if (session_ != nullptr) {
        genai_text_destroy_session(&session_);
        session_ = nullptr;
    }

    if (chatConfig_ != nullptr) {
        chat_model_config_destroy(&chatConfig_);
        chatConfig_ = nullptr;
    }
}

void LLMDiversifierStrategy::callback(ChatResult* result, void* user_data) {
    LLMDiversifierStrategy* diversifyStrategy = static_cast<LLMDiversifierStrategy*>(user_data);
    if (diversifyStrategy == nullptr) {
        return;
    }
    int genErrcode = chat_result_get_error_code(result);
    if (genErrcode != AISDK_NO_ERROR) {
        PRINT_ERROR("error code is :%d\n", genErrcode);
        diversifyStrategy->callbackErrCode_ = genaisdkErrorCodeToKBErrorCode(genErrcode);
        diversifyStrategy->callbackErrMsg_ = chat_result_get_error_message(result);
        PRINT_ERROR("error message is :%s\n", diversifyStrategy->callbackErrMsg_.c_str());
        std::lock_guard<std::mutex> lock(diversifyStrategy->mtx_);
        diversifyStrategy->isCompleted_ = true;
        diversifyStrategy->cv_.notify_all();
        return;
    }

    const char* msg = chat_result_get_assistant_message(result);
    if (msg != nullptr) {
        std::lock_guard<std::mutex> lock(diversifyStrategy->mtx_);
        diversifyStrategy->expandResult_ += msg;
    }

    if (chat_result_get_is_end(result)) {
        PRINT_DEBUG("finish reason message is:%s\n", chat_result_get_finish_reason_message(result));
        std::lock_guard<std::mutex> lock(diversifyStrategy->mtx_);

        // 去除字符串开始和结尾的所有空白字符（空格、换行等）
        auto trim = [](const std::string& str) -> std::string {
            std::string result = str;
            result.erase(0, result.find_first_not_of(" \t\n\r\f\v"));
            result.erase(result.find_last_not_of(" \t\n\r\f\v") + 1);
            return result;
        };
        diversifyStrategy->expandResult_ = trim(diversifyStrategy->expandResult_);
        diversifyStrategy->isCompleted_ = true;
        diversifyStrategy->cv_.notify_all();
    }
}

QueryDiversification LLMDiversifierStrategy::diversify(const std::string& query, const QueryDiversifyConfig& config,
                                                       KBErrorCode& errCode, std::string& errMsg) {
    QueryDiversification diversifyResult;
    if (config.modelName.empty() == false) {
        chat_model_config_set_name(chatConfig_, config.modelName.c_str());
    }

    chat_model_config_set_top_k(chatConfig_, config.topK);
    chat_model_config_set_top_p(chatConfig_, config.topP);
    chat_model_config_set_temperature(chatConfig_, config.temperature);
    chat_model_config_set_max_completion_tokens(chatConfig_, config.maxTokens);

    genai_text_set_model_config(session_, chatConfig_);
    int errorCode = genai_text_init_session(session_);
    if (errorCode != AISDK_NO_ERROR) {
        errCode = genaisdkErrorCodeToKBErrorCode(errorCode);
        PRINT_INFO("初始化错误：错误码:%d\n", errorCode);
        return diversifyResult;
    }

    isCompleted_ = false;
    callbackErrCode_ = KBErrorCode::SUCCESS;
    genai_text_result_set_callback(session_, callback, this);

    std::string prompt = getQueryDiversifyPrompt();
    genai_text_set_chat_system_prompt(session_, prompt.c_str());
    genai_text_chat_async(session_, query.c_str());
    PRINT_INFO("开始查询扩展...\n");

    // 等待完成
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] { return isCompleted_; });
    lock.unlock();
    if (callbackErrCode_ != KBErrorCode::SUCCESS) {
        errCode = callbackErrCode_;
        errMsg = callbackErrMsg_;
        PRINT_INFO("大模型错误：错误码:%d\n", callbackErrCode_);
        return diversifyResult;
    }
    PRINT_INFO("查询扩展完成\n");

    std::vector<std::string> queries, keywords;
    // 添加原始查询，防止问题过于发散，检索命中效果不好
    queries.push_back(query);
    try {
        PRINT_DEBUG("expandQueryResult: %s\n", expandResult_.c_str());
        expandResult_ = deserializeModelResponse(expandResult_);
        nlohmann::json json = nlohmann::json::parse(expandResult_);
        for (auto& [key, value] : json.items()) {
            PRINT_DEBUG("key: %s, value: %s\n", key.c_str(), value.dump().c_str());
            if (value.is_string()) {
                if (key == "kw") {
                    keywords = parseStringToVector(value.get<std::string>(), ' ');
                    continue;
                }
                queries.push_back(value.get<std::string>());
            }
        }
    } catch (const std::exception& e) {
        PRINT_ERROR("query expansion json output parse error: %s, using raw query: %s\n", e.what(), query.c_str());
    }

    if (queries.size() > 6) {
        queries.resize(6);
    }
    if (keywords.size() > 12) {
        keywords.resize(12);
    }
    if ((queries.size() < 4 || keywords.size() < 3) && retryCount < 3 && !callStoped_) {
        ++retryCount;
        expandResult_.clear();
        PRINT_INFO("Query expansion retrying... %d \n", retryCount);
        return diversify(query, config, errCode, errMsg);  // 重试
    }
    retryCount = 0;
    callStoped_ = false;
    diversifyResult.queryTexts = queries;
    diversifyResult.keywords = keywords;
    return diversifyResult;
}

std::string LLMDiversifierStrategy::getQueryDiversifyPrompt() {
    return ConfigManager::getInstance().getQueryDiversifyPrompt();
}

std::string LLMDiversifierStrategy::deserializeModelResponse(const std::string& modelOutput) {
    std::string formattedContent = modelOutput;
    if (formattedContent.substr(0, 7) == "```json") {
        formattedContent = formattedContent.substr(7);
    }
    if (formattedContent.substr(formattedContent.size() - 3, formattedContent.size()) == "```") {
        formattedContent = formattedContent.substr(0, formattedContent.size() - 3);
    }
    if (formattedContent.substr(0, 1) == "{" &&
        formattedContent.substr(formattedContent.size() - 1, formattedContent.size()) != "}") {
        formattedContent = formattedContent + "}";
    }
    if (formattedContent.substr(0, 1) != "{" &&
        formattedContent.substr(formattedContent.size() - 1, formattedContent.size()) == "}") {
        formattedContent = "{" + formattedContent;
    }
    return formattedContent;
}

void LLMDiversifierStrategy::stopDiversify() {
    assert(session_ != nullptr);
    callStoped_ = true;
    genai_text_stop_chat(session_);
}

}  // namespace knowledgebase

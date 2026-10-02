#include "chunkevaluator.h"

#include <kyai-business-framework/log.h>
#include <kylin-ai/genai/text/chat.h>
#include <kylin-ai/genai/text/chatmessage.h>
#include <kylin-ai/genai/text/config.h>
#include <kylin-ai/genai/text/error.h>
#include <kylin-ai/genai/text/result.h>

#include <algorithm>
#include <map>
#include <nlohmann/json.hpp>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace knowledgebase {

ChunkEvaluator::ChunkEvaluator() {
    session_ = genai_text_create_session();
    if (session_ == nullptr) {
        PRINT_ERROR("创建 llm RerankerClassifier session 失败\n");
        return;
    }

    chatConfig_ = chat_model_config_create();
    if (chatConfig_ == nullptr) {
        PRINT_ERROR("创建 llm RerankerClassifier chatConfig 失败\n");
        return;
    }
}
ChunkEvaluator::~ChunkEvaluator() {
    if (session_ != nullptr) {
        genai_text_destroy_session(&session_);
        session_ = nullptr;
    }

    if (chatConfig_ != nullptr) {
        chat_model_config_destroy(&chatConfig_);
        chatConfig_ = nullptr;
    }
}

void ChunkEvaluator::callback(ChatResult* result, void* user_data) {
    ChunkEvaluator* chunkEvaluator = static_cast<ChunkEvaluator*>(user_data);
    if (chunkEvaluator == nullptr) {
        return;
    }
    int genErrcode = chat_result_get_error_code(result);
    if (genErrcode != AISDK_NO_ERROR) {
        PRINT_ERROR("error code is :%d\n", genErrcode);
        PRINT_ERROR("error message is :%s\n", chat_result_get_error_message(result));
        chunkEvaluator->callbackErrCode_ = genaisdkErrorCodeToKBErrorCode(genErrcode);
        std::lock_guard<std::mutex> lock(chunkEvaluator->mtx_);
        chunkEvaluator->isCompleted_ = true;
        chunkEvaluator->cv_.notify_all();
        return;
    }

    const char* msg = chat_result_get_assistant_message(result);
    if (msg != nullptr) {
        std::lock_guard<std::mutex> lock(chunkEvaluator->mtx_);
        chunkEvaluator->rerankAndClassifyResult_ += msg;
    }

    if (chat_result_get_is_end(result)) {
        PRINT_DEBUG("finish reason message is:%s\n", chat_result_get_finish_reason_message(result));
        std::lock_guard<std::mutex> lock(chunkEvaluator->mtx_);

        // 去除字符串开始和结尾的所有空白字符（空格、换行等）
        auto trim = [](const std::string& str) -> std::string {
            std::string result = str;
            result.erase(0, result.find_first_not_of(" \t\n\r\f\v"));
            result.erase(result.find_last_not_of(" \t\n\r\f\v") + 1);
            return result;
        };
        chunkEvaluator->rerankAndClassifyResult_ = trim(chunkEvaluator->rerankAndClassifyResult_);
        chunkEvaluator->isCompleted_ = true;
        chunkEvaluator->cv_.notify_all();
    }
}

// 从字符串中提取知识领域
KnowledgeField parseKnowledgeField(const std::string& llmResult) {
    size_t naPos = llmResult.find("na:");
    if (naPos != std::string::npos) {
        size_t startPos = naPos + 3;  // 跳过 "na:"
        size_t endPos = llmResult.find_first_not_of("0123456789", startPos);
        std::string fieldNumStr = llmResult.substr(startPos, endPos - startPos);

        try {
            int fieldNum = std::stoi(fieldNumStr);
            // 确保数值在有效范围内
            if (fieldNum >= 0 && fieldNum <= 6) {
                return static_cast<KnowledgeField>(fieldNum);
            }
        } catch (...) {
            // 解析失败时返回默认值
        }
    }
    return KnowledgeField::GENERAL;
}

// 重新排序检索结果
void ChunkEvaluator::sortChunks(const std::string& llmResult, RetrievalResult& retrievalResult) {
    // 解析知识领域并更新
    retrievalResult.field = parseKnowledgeField(llmResult);

    // 解析LLM结果，提取排序信息
    std::vector<std::pair<int, float>> orderInfo;  // {chunkIndex, score}
    std::regex chunkPattern(R"(Chunk(\d+)【评分：([\d.]+)分】)");
    std::sregex_iterator iter(llmResult.begin(), llmResult.end(), chunkPattern);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        int index = std::stoi(match[1].str()) - 1;  // 转换为0基索引
        float score = std::stof(match[2].str());
        orderInfo.push_back({index, score});
    }

    // 创建一个映射，用于处理重复的chunk索引，保留最高分
    std::map<int, float> bestScores;
    for (const auto& [index, score] : orderInfo) {
        if (bestScores.find(index) == bestScores.end() || score > bestScores[index]) {
            bestScores[index] = score;
        }
    }

    // 创建一个临时向量来存储重新排序的项目
    std::vector<KnowledgeItem> reorderedItems;
    std::vector<bool> processed(retrievalResult.items.size(), false);  // 标记哪些项目已经被处理

    // 按照LLM结果中的顺序添加项目
    for (const auto& [index, score] : orderInfo) {
        if (index >= 0 && index < static_cast<int>(retrievalResult.items.size()) && !processed[index]) {
            // 更新相似度分数
            retrievalResult.items[index].item_->similarity_ = score;
            reorderedItems.push_back(retrievalResult.items[index]);
            processed[index] = true;
        }
    }

    // 添加未在LLM结果中列出的项目，保持原有相似度（从0-1映射到0-100）
    for (size_t i = 0; i < retrievalResult.items.size(); ++i) {
        if (!processed[i]) {
            retrievalResult.items[i].item_->similarity_ = retrievalResult.items[i].item_->similarity_ * 100;
            reorderedItems.push_back(retrievalResult.items[i]);
        }
    }

    // 更新retrievalResult的items
    retrievalResult.items = reorderedItems;
}

void ChunkEvaluator::rerankAndClassify(RetrievalResult& retrievalResult, const ChunkEvaluateConfig& config,
                                       KBErrorCode& errCode) {
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
        return;
    }

    isCompleted_ = false;
    callbackErrCode_ = KBErrorCode::SUCCESS;
    genai_text_result_set_callback(session_, callback, this);

    std::string rerankerClassifierSystemPrompt = config.systemPrompt;
    std::string rerankerClassifierUserPrompt =
        getRerankerClassifierPrompt(retrievalResult.queryText, config.userPrompt, retrievalResult.items);
    genai_text_set_chat_system_prompt(session_, rerankerClassifierSystemPrompt.c_str());
    genai_text_chat_async(session_, rerankerClassifierUserPrompt.c_str());

    PRINT_DEBUG("大模型重排序提示词: %s\n", rerankerClassifierUserPrompt.c_str());
    // 等待完成
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] { return isCompleted_; });
    lock.unlock();
    if (callbackErrCode_ != KBErrorCode::SUCCESS) {
        errCode = callbackErrCode_;
        PRINT_INFO("大模型错误：错误码:%d\n", callbackErrCode_);
        return;
    }
    PRINT_DEBUG("大模型完成重排序完成:\n\t原始chunk数量: %d, 大模型重排结果:%s\n", retrievalResult.items.size(),
                rerankAndClassifyResult_.c_str());
    try {
        RetrievalResult tmpRetrievalResult = retrievalResult;
        sortChunks(rerankAndClassifyResult_, tmpRetrievalResult);
        rerankAndClassifyResult_.clear();
        if (tmpRetrievalResult.items.size() == retrievalResult.items.size()) {
            retrievalResult = tmpRetrievalResult;
        } else {
            PRINT_INFO("大模型重排错误，模型排序输出数量与检索结果不一致, 重排结果数量: %s\n",
                       tmpRetrievalResult.items.size());
            errCode = KBErrorCode::RETRIEVAL_RERANK_FAILED;
            return;
        }
    } catch (const std::exception& e) {
        PRINT_ERROR("大模型重排错误失败: %s\n", e.what());
        errCode = KBErrorCode::RETRIEVAL_RERANK_FAILED;
        return;
    }
}

std::string ChunkEvaluator::getRerankerClassifierPrompt(const std::string& query, const std::string& userPrompt,
                                                        const std::vector<KnowledgeItem>& items) {
    std::string rerankerClassifierPrompt = userPrompt;
    std::string queryPlaceHolder = "{query}";
    size_t queryPos = rerankerClassifierPrompt.find(queryPlaceHolder);
    if (queryPos != std::string::npos) {
        rerankerClassifierPrompt.replace(queryPos, queryPlaceHolder.length(), " " + query + " ");
    }
    std::string chunksCountPlaceHolder = "{total_chunks}";
    size_t chunksCountPos = rerankerClassifierPrompt.find(chunksCountPlaceHolder);
    if (chunksCountPos != std::string::npos) {
        rerankerClassifierPrompt.replace(chunksCountPos, chunksCountPlaceHolder.length(),
                                         " " + std::to_string(items.size()) + " ");  // size统计方式待修改
    }
    std::string chunksPlaceHolder = "{formatted_chunks}";
    size_t chunksPos = rerankerClassifierPrompt.find(chunksPlaceHolder);
    if (chunksPos != std::string::npos) {
        std::string formatChunks = "";
        int chunkIndex = 1;
        for (auto& chunk : items) {
            formatChunks += "Chunk" + std::to_string(chunkIndex++) + ":" + chunk.item_->content_ + "\n";
        }
        rerankerClassifierPrompt.replace(chunksPos, chunksPlaceHolder.length(), formatChunks + " ");
    }

    return rerankerClassifierPrompt;
}

std::string ChunkEvaluator::deserializeModelResponse(const std::string& modelOutput) {
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

void ChunkEvaluator::stop() {
    assert(session_ != nullptr);
    genai_text_stop_chat(session_);
}

}  // namespace knowledgebase

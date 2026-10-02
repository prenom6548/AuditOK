#include "configmanager.h"

#include <nlohmann/json.hpp>

#include "common/common.h"
#include "common/utils.h"

namespace knowledgebase {

ConfigManager::ConfigManager() {
    nlohmann::json config = Configuration::getAllValues();
    if (config.contains("file_process")) {
        auto localConfig = config["file_process"];
        fileProcessorConfig_.chunkMaxSize_ =
            localConfig.contains("chunk_max_size") ? localConfig["chunk_max_size"].get<uint32_t>() : 800;
        fileProcessorConfig_.enhanceParse_ =
            localConfig.contains("enhance_parse") ? localConfig["enhance_parse"].get<bool>() : false;
    }

    if (config.contains("query_diversify")) {
        auto localConfig = config["query_diversify"];
        queryDiversifyConfig_.enable = localConfig.contains("enable") ? localConfig["enable"].get<bool>() : true;
        queryDiversifyConfig_.modelName =
            localConfig.contains("model_name") ? localConfig["model_name"].get<std::string>() : "text-davinci-003";
        queryDiversifyConfig_.topK = localConfig.contains("top_k") ? localConfig["top_k"].get<int>() : 40;
        queryDiversifyConfig_.topP = localConfig.contains("top_p") ? localConfig["top_p"].get<double>() : 0.9f;
        queryDiversifyConfig_.temperature =
            localConfig.contains("temperature") ? localConfig["temperature"].get<double>() : 0.7f;
        queryDiversifyConfig_.maxTokens =
            localConfig.contains("max_tokens") ? localConfig["max_tokens"].get<uint32_t>() : 2048;
    }

    if (config.contains("chunk_evaluater")) {
        auto localConfig = config["chunk_evaluater"];
        chunkEvaluateConfig_.enable = localConfig.contains("enable") ? localConfig["enable"].get<bool>() : true;
        chunkEvaluateConfig_.modelName =
            localConfig.contains("model_name") ? localConfig["model_name"].get<std::string>() : "text-davinci-003";
        chunkEvaluateConfig_.topK = localConfig.contains("top_k") ? localConfig["top_k"].get<int>() : 40;
        chunkEvaluateConfig_.topP = localConfig.contains("top_p") ? localConfig["top_p"].get<double>() : 0.9f;
        chunkEvaluateConfig_.temperature =
            localConfig.contains("temperature") ? localConfig["temperature"].get<double>() : 0.3f;
        chunkEvaluateConfig_.maxTokens =
            localConfig.contains("max_tokens") ? localConfig["max_tokens"].get<uint32_t>() : 512;

        std::string rerankerClassifierUserPromptPath = localConfig["user_prompt_path"].get<std::string>();
        std::string rerankerClassifierSystemPromptPath = localConfig["system_prompt_path"].get<std::string>();

        chunkEvaluateConfig_.systemPrompt = readFileToString(rerankerClassifierSystemPromptPath);
        chunkEvaluateConfig_.userPrompt = readFileToString(rerankerClassifierUserPromptPath);
        retrievalConfig_.chunkEvaluateConfig = chunkEvaluateConfig_;
    }

    if (config.contains("retrieval")) {
        auto localConfig = config["retrieval"];
        retrievalConfig_.expandContext =
            localConfig.contains("expand_context") ? localConfig["expand_context"].get<bool>() : false;
        retrievalConfig_.maxTokens =
            localConfig.contains("max_tokens") ? localConfig["max_tokens"].get<uint32_t>() : 1024;
        retrievalConfig_.rerankMethod = localConfig.contains("rerank_method")
                                            ? stringToRerankMethod(localConfig["rerank_method"].get<std::string>())
                                            : RerankMethod::RRF;
        retrievalConfig_.retrievalMethod =
            localConfig.contains("retrieval_method")
                ? stringToRetrievalMethod(localConfig["retrieval_method"].get<std::string>())
                : RetrievalMethod::HYBRID;
        retrievalConfig_.vectorScoreThreshold =
            localConfig.contains("vector_score_threshold") ? localConfig["vector_score_threshold"].get<float>() : 0.75f;
        retrievalConfig_.bm25ScoreThreshold =
            localConfig.contains("bm25_score_threshold") ? localConfig["bm25_score_threshold"].get<float>() : 0.5f;
        retrievalConfig_.topK = localConfig.contains("top_k") ? localConfig["top_k"].get<int>() : 4;
        retrievalConfig_.queryDiversifyConfig = queryDiversifyConfig_;
        retrievalConfig_.fuzzywuzzyRerankEnable = localConfig.contains("fuzzywuzzy_rerank_enable")
                                                      ? localConfig["fuzzywuzzy_rerank_enable"].get<bool>()
                                                      : false;
    }

    if (config.contains("chat")) {
        auto localConfig = config["chat"];
        chatConfig_.modelName = localConfig.contains("model_name") ? localConfig["model_name"].get<std::string>() : "";
        chatConfig_.topK = localConfig.contains("top_k") ? localConfig["top_k"].get<int>() : 40;
        chatConfig_.topP = localConfig.contains("top_p") ? localConfig["top_p"].get<double>() : 0.9f;
        chatConfig_.temperature = localConfig.contains("temperature") ? localConfig["temperature"].get<double>() : 0.7f;
        chatConfig_.maxTokens = localConfig.contains("max_tokens") ? localConfig["max_tokens"].get<uint32_t>() : 4096;
        chatConfig_.retrievalConfig = retrievalConfig_;
    }

    std::string queryDiversifyPromptPath = config["query_diversify"]["prompt_path"].get<std::string>();
    queryDiversifyPrompt_ = readFileToString(queryDiversifyPromptPath);

    std::string fieldPromptDir = config["chat"]["field_prompt_dir"];
    int count = static_cast<int>(KnowledgeField::KNOWLEDGE_FIELD_COUNT);

    for (int i = 0; i < count; ++i) {
        std::string name = knowledgeFieldMap.at(static_cast<KnowledgeField>(i));
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::string systemPromptPath = fieldPromptDir + "field_" + name + "_system";

        fieldSystemPromptMap_[static_cast<KnowledgeField>(i)] = readFileToString(systemPromptPath);
    }

    std::string fieldUserPromptPath = fieldPromptDir + "field_user_prompt";
    fieldUserPrompt_ = readFileToString(fieldUserPromptPath);
}

std::string ConfigManager::getFieldSystemPrompt(const KnowledgeField& field) const {
    // 1. 如果 field 是 KNOWN_FIELD_COUNT，尝试使用通用提示词
    if (field == KnowledgeField::KNOWLEDGE_FIELD_COUNT) {
        auto generalIt = fieldSystemPromptMap_.find(KnowledgeField::GENERAL);
        if (generalIt != fieldSystemPromptMap_.end()) {
            return generalIt->second;
        } else {
            return "";
        }
    }
    // 2. 先尝试查找指定的 field
    auto it = fieldSystemPromptMap_.find(field);
    if (it != fieldSystemPromptMap_.end()) {
        return it->second;
    }

    // 3. 如果没找到，尝试 fallback 到 GENERAL
    auto generalIt = fieldSystemPromptMap_.find(KnowledgeField::GENERAL);
    if (generalIt != fieldSystemPromptMap_.end()) {
        return generalIt->second;
    }

    // 4. 如果连 GENERAL 都没有，返回空字符串
    return "";
}

}  // namespace knowledgebase

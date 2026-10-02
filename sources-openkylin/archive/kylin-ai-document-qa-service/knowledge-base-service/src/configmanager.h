#pragma once

#include <kyai-business-framework/conf.h>

#include "common/common.h"
#include "common/errno.h"

namespace knowledgebase {

class ConfigManager {
public:
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    FileProcessorConfig getFileProcessorConfig() const { return fileProcessorConfig_; }
    QueryDiversifyConfig getQueryDiversifyConfig() const { return queryDiversifyConfig_; }
    RetrievalConfig getRetrievalConfig() const { return retrievalConfig_; }
    ChatConfig getChatConfig() const { return chatConfig_; }
    std::string getQueryDiversifyPrompt() const { return queryDiversifyPrompt_; }
    ChunkEvaluateConfig getChunkEvaluateConfig() const { return chunkEvaluateConfig_; }
    std::string getChatPrompt() const { return chatPrompt_; }
    std::string getFieldSystemPrompt(const KnowledgeField& field) const;
    std::string getFieldUserPrompt() const { return fieldUserPrompt_; }

private:
    ConfigManager();
    ~ConfigManager() = default;

private:
    FileProcessorConfig fileProcessorConfig_;
    QueryDiversifyConfig queryDiversifyConfig_;
    ChunkEvaluateConfig chunkEvaluateConfig_;
    RetrievalConfig retrievalConfig_;
    ChatConfig chatConfig_;
    std::string queryDiversifyPrompt_;
    std::string chatPrompt_;
    std::unordered_map<KnowledgeField, std::string> fieldSystemPromptMap_;
    std::string fieldUserPrompt_;
};

}  // namespace knowledgebase

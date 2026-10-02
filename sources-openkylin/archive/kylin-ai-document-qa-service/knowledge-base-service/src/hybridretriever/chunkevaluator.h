#pragma once

#include <kylin-ai/genai/text/chat.h>

#include <condition_variable>
#include <nlohmann/json.hpp>
#include <vector>

#include "../common/common.h"
#include "../common/utils.h"

namespace knowledgebase {
typedef struct _ChatResult ChatResult;

class ChunkEvaluator {
public:
    ChunkEvaluator();
    ~ChunkEvaluator();

    // 回调函数
    static void callback(ChatResult* result, void* user_data);
    void rerankAndClassify(RetrievalResult& retrievalResult, const ChunkEvaluateConfig& config, KBErrorCode& errCode);
    void stop();

private:
    std::string getRerankerClassifierPrompt(const std::string& query, const std::string& userPrompt,
                                            const std::vector<KnowledgeItem>& items);
    std::string deserializeModelResponse(const std::string& modelOutput);
    void sortChunks(const std::string& llmResult, RetrievalResult& retrievalResult);

private:
    bool isCompleted_ = false;
    std::string rerankAndClassifyResult_;
    std::mutex mtx_;
    std::condition_variable cv_;
    KBErrorCode callbackErrCode_;
    GenAiTextSession* session_ = nullptr;
    ChatModelConfig* chatConfig_ = nullptr;
};

}  // namespace knowledgebase

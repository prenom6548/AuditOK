#pragma once

#include <kylin-ai/genai/text/chat.h>

#include <condition_variable>
#include <nlohmann/json.hpp>
#include <vector>

#include "../common/common.h"
#include "../common/utils.h"

namespace knowledgebase {
typedef struct _ChatResult ChatResult;

// Query 扩展方法基类
class QueryDiversifierStrategy {
public:
    virtual ~QueryDiversifierStrategy() = default;
    virtual QueryDiversification diversify(const std::string& query, const QueryDiversifyConfig& config,
                                           KBErrorCode& errCode, std::string& errMsg) = 0;
    virtual void stopDiversify() = 0;
};

class LLMDiversifierStrategy : public QueryDiversifierStrategy {
public:
    LLMDiversifierStrategy();
    ~LLMDiversifierStrategy();

    // 回调函数
    static void callback(ChatResult* result, void* user_data);
    QueryDiversification diversify(const std::string& query, const QueryDiversifyConfig& config, KBErrorCode& errCode,
                                   std::string& errMsg) override;
    void stopDiversify() override;

private:
    std::string getQueryDiversifyPrompt();
    std::string deserializeModelResponse(const std::string& modelOutput);

private:
    int retryCount = 0;
    bool isCompleted_ = false;
    bool callStoped_ = false;
    std::string expandResult_;
    std::mutex mtx_;
    std::condition_variable cv_;
    KBErrorCode callbackErrCode_;
    std::string callbackErrMsg_ = "";
    GenAiTextSession* session_ = nullptr;
    ChatModelConfig* chatConfig_ = nullptr;
};

}  // namespace knowledgebase

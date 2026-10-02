#include "retrievalprocessor.h"

#include "../common/performancetimer.h"

namespace knowledgebase {

RetrievalResult RetrievalProcessor::retrieveAndRerank(const std::string& knowledgeBaseId, const std::string& query,
                                                      const RetrievalConfig& config, KBErrorCode& errCode,
                                                      std::string& errMsg) {
    // 进入检索前需要给stopFlag_置位，防止上一次对话总结阶段的stop操作对本次对话从产生影响
    stopFlag_.store(false, std::memory_order_release);
    auto& timer = PerformanceTimer::getInstance();  // 获取单例
    RetrievalResult retrievalResult = {.queryText = query, .field = KnowledgeField::GENERAL};

    // 1. 验证权限
    if (!verifyPermissions(knowledgeBaseId, errCode)) {
        return retrievalResult;
    }

    if (stopFlag_.load(std::memory_order_acquire)) {
        stopFlag_.store(false, std::memory_order_release);
        return retrievalResult;
    }
    PRINT_INFO("进入问题扩展");
    // 2. 查询多样化
    timer.start("问题扩展");
    QueryDiversification diversifiedQueries = diversifyQuery(query, config.queryDiversifyConfig, errCode, errMsg);
    timer.end("问题扩展");
    if (errCode != KBErrorCode::SUCCESS) {
        return retrievalResult;
    }

    if (stopFlag_.load(std::memory_order_acquire)) {
        stopFlag_.store(false, std::memory_order_release);
        return retrievalResult;
    }
    PRINT_INFO("进入知识库检索");
    // 3. 执行检索
    timer.start("知识库检索");
    KnowledgeItemsMap allKnowledgeItems = performRetrieval(knowledgeBaseId, diversifiedQueries, config, errCode);
    timer.end("知识库检索");
    if (errCode != KBErrorCode::SUCCESS) {
        return retrievalResult;
    }

    if (stopFlag_.load(std::memory_order_acquire)) {
        stopFlag_.store(false, std::memory_order_release);
        return retrievalResult;
    }
    PRINT_INFO("进入结果重排序");
    // 4. 重排序
    timer.start("重排序");
    retrievalResult = rerankRetrievedResults(query, allKnowledgeItems, config);
    timer.end("重排序");

    if (stopFlag_.load(std::memory_order_acquire)) {
        stopFlag_.store(false, std::memory_order_release);
        return retrievalResult;
    }
    PRINT_INFO("进入后处理");
    // 5. 后处理
    postProcessResults(retrievalResult, config);

    if (retrievalResult.items.size() == 0) {
        errCode = KBErrorCode::RETRIEVAL_NO_RESULTS;
    }
    return retrievalResult;
}

void RetrievalProcessor::stopRetrieve() {
    stopFlag_.store(true, std::memory_order_release);
    performStopRetrieve();
}

}  // namespace knowledgebase
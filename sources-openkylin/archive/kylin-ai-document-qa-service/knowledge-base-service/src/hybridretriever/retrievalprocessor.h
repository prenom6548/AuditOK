#pragma once

#include <atomic>
#include <string>
#include <vector>

#include "../common/common.h"

namespace knowledgebase {

class RetrievalProcessor {
public:
    virtual ~RetrievalProcessor() = default;

    // 模板方法模式：定义检索的标准流程
    RetrievalResult retrieveAndRerank(const std::string& knowledgeBaseId, const std::string& query,
                                      const RetrievalConfig& config, KBErrorCode& errCode, std::string& errMsg);
    void stopRetrieve();

protected:
    virtual bool verifyPermissions(const std::string& knowledgeBaseId, KBErrorCode& errCode) = 0;

    // 钩子方法，子类可以选择性重写
    virtual QueryDiversification diversifyQuery(const std::string& query, const QueryDiversifyConfig& config,
                                                KBErrorCode& errCode, std::string& errMsg) {
        // 默认实现不进行多样化
        QueryDiversification diversified;
        diversified.queryTexts.push_back(query);
        errCode = KBErrorCode::SUCCESS;
        return diversified;
    }

    virtual KnowledgeItemsMap performRetrieval(const std::string& knowledgeBaseId,
                                               const QueryDiversification& diversifiedQueries,
                                               const RetrievalConfig& config, KBErrorCode& errCode) = 0;

    virtual RetrievalResult rerankRetrievedResults(const std::string& query, const KnowledgeItemsMap& retrievalResults,
                                                   const RetrievalConfig& config) = 0;
    virtual void postProcessResults(RetrievalResult& retrievalResults, const RetrievalConfig& config) = 0;

    virtual void performStopRetrieve() = 0;
    std::atomic<bool> stopFlag_;
};

}  // namespace knowledgebase
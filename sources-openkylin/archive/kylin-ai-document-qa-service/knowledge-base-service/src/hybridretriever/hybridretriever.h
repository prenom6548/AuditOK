#pragma once

#include <string>
#include <vector>

#include "../common/common.h"
#include "../querydiversifier/querydiversifier.h"
#include "../storagemanager/traditionaldatabaseengine.h"
#include "chunkevaluator.h"
#include "fuzzycompositererankerplugin.h"
#include "rerankstrategy.h"
#include "retrievalprocessor.h"
#include "retrievalstrategy.h"

namespace knowledgebase {

class HybridRetriever : public RetrievalProcessor {
public:
    HybridRetriever(const std::string appId, const std::string password, const std::string dbFilePath);

protected:
    // 实现模板方法中的抽象方法
    virtual bool verifyPermissions(const std::string& knowledgeBaseId, KBErrorCode& errCode) override;
    virtual QueryDiversification diversifyQuery(const std::string& query, const QueryDiversifyConfig& config,
                                                KBErrorCode& errCode, std::string& errMsg) override;
    virtual KnowledgeItemsMap performRetrieval(const std::string& knowledgeBaseId,
                                               const QueryDiversification& diversifiedQueries,
                                               const RetrievalConfig& config, KBErrorCode& errCode) override;
    virtual RetrievalResult rerankRetrievedResults(const std::string& query, const KnowledgeItemsMap& retrievalResults,
                                                   const RetrievalConfig& config) override;
    virtual void postProcessResults(RetrievalResult& retrievalResults, const RetrievalConfig& config) override;
    virtual void performStopRetrieve() override;

private:
    void printRetrievalResults(const std::vector<KnowledgeItem>& finalItems);
    void filterKnowledgeItemsByScore(std::vector<KnowledgeItem>& knowledgeItems);
    void filterKnowledgeItemByFileIds(std::vector<KnowledgeItem>& knowledgeItems, const RetrievalConfig& config);
    void removeDuplicatesAndSortDescending(std::vector<KnowledgeItem>& items);

private:
    QueryDiversifier querydiversifier_;
    std::unique_ptr<RerankingStrategy> rerankingstrategy_;
    std::unique_ptr<RetrievalStrategy> retrievalStrategy_;
    std::unique_ptr<ChunkEvaluator> chunkEvaluator_;
    std::unique_ptr<FuzzywuzzyRerankingPlugin> fuzzywuzzyRerankingPlugin_;
    std::unique_ptr<TraditionalDatabaseEngine> traditionalDatabaseEngine_;
    std::string appId_;
    std::string password_;
    std::string dbFilePath_;
};

}  // namespace knowledgebase

#include "retrievalstrategy.h"

#include <chinese-segmentation/chinese-segmentation.h>
#include <kyai-business-framework/log.h>

#include "../storagemanager/textsearchengine.h"
#include "../storagemanager/vectorizationengine.h"

namespace knowledgebase {

KnowledgeItemsMap TextRetrievalStrategy::search(const QueryDiversification& queryDiverdification,
                                                const RetrievalConfig& config, const KBType kbType,
                                                KBErrorCode& errCode) {
    TextSearchEngine& textEngine = TextSearchEngine::getInstance();
    KnowledgeItemsMap knowledgeItems;
    std::string textSearchDbPath = dbFilePath_ + "/" + textSearchDbName_;
    std::vector<KnowledgeItem>& items = knowledgeItems[RetrievalMethod::TEXT];
    if (stopFlag_.load(std::memory_order_acquire)) {
        stopFlag_.store(false, std::memory_order_release);
        return knowledgeItems;
    }

    for (const std::string& queryText : queryDiverdification.queryTexts) {
        PRINT_INFO("Begin text similarity search, query: %s\n", queryText.c_str());
        std::vector<KnowledgeItem> results = textEngine.search(textSearchDbPath, queryText, config, kbType, errCode);
        items.insert(items.end(), results.begin(), results.end());
    }

    return knowledgeItems;
}

void TextRetrievalStrategy::stopRetrieve() { stopFlag_.store(true, std::memory_order_release); }

KnowledgeItemsMap KeywordRetrievalStrategy::search(const QueryDiversification& queryDiverdification,
                                                   const RetrievalConfig& config, const KBType kbType,
                                                   KBErrorCode& errCode) {
    TextSearchEngine& textEngine = TextSearchEngine::getInstance();
    KnowledgeItemsMap knowledgeItems;
    std::string textSearchDbPath = dbFilePath_ + "/" + textSearchDbName_;
    std::vector<KnowledgeItem>& items = knowledgeItems[RetrievalMethod::KEYWORD];

    PRINT_INFO("Begin keyword similarity search\n");
    if (stopFlag_.load(std::memory_order_acquire)) {
        stopFlag_.store(false, std::memory_order_release);
        return knowledgeItems;
    }
    std::vector<KnowledgeItem> results =
        textEngine.search(textSearchDbPath, queryDiverdification.keywords, config, kbType, errCode);
    items.insert(items.end(), results.begin(), results.end());

    return knowledgeItems;
}

void KeywordRetrievalStrategy::stopRetrieve() { stopFlag_.store(true, std::memory_order_release); }

KnowledgeItemsMap VectorRetrievalStrategy::search(const QueryDiversification& queryDiverdification,
                                                  const RetrievalConfig& config, const KBType kbType,
                                                  KBErrorCode& errCode) {
    stopFlag_.store(false, std::memory_order_release);
    VectorizationEngine& vectorEngine = VectorizationEngine::getInstance();
    KnowledgeItemsMap knowledgeItems;
    std::vector<KnowledgeItem>& items = knowledgeItems[RetrievalMethod::SEMANTIC];

    for (const std::string& queryText : queryDiverdification.queryTexts) {
        PRINT_INFO("Begin vector similarity search, query: %s\n", queryText.c_str());
        if (stopFlag_.load(std::memory_order_acquire)) {
            stopFlag_.store(false, std::memory_order_release);
            return knowledgeItems;
        }
        std::vector<KnowledgeItem> results =
            vectorEngine.search(appId_, password_, dbFilePath_, vectorDbName_, queryText, config, kbType, errCode);
        items.insert(items.end(), results.begin(), results.end());
    }

    return knowledgeItems;
}

void VectorRetrievalStrategy::stopRetrieve() { stopFlag_.store(true, std::memory_order_release); }

KnowledgeItemsMap HybridRetrievalStrategy::search(const QueryDiversification& queryDiverdification,
                                                  const RetrievalConfig& config, const KBType kbType,
                                                  KBErrorCode& errCode) {
    stopFlag_.store(false, std::memory_order_release);
    VectorizationEngine& vectorEngine = VectorizationEngine::getInstance();
    TextSearchEngine& textEngine = TextSearchEngine::getInstance();
    std::string textSearchDbPath = dbFilePath_ + "/" + textSearchDbName_;
    KnowledgeItemsMap knowledgeItems;
    std::vector<KnowledgeItem>& textItems = knowledgeItems[RetrievalMethod::TEXT];
    std::vector<KnowledgeItem>& vectorItems = knowledgeItems[RetrievalMethod::SEMANTIC];
    std::vector<KnowledgeItem>& keywordItems = knowledgeItems[RetrievalMethod::KEYWORD];

    for (const std::string& queryText : queryDiverdification.queryTexts) {
        PRINT_INFO("Begin hybrid similarity search, query: %s\n", queryText.c_str());

        if (stopFlag_.load(std::memory_order_acquire)) {
            stopFlag_.store(false, std::memory_order_release);
            return knowledgeItems;
        }
        std::vector<KnowledgeItem> results;
        if (textSearchDbName_ != "") {
            PRINT_INFO("Begin Text similarity search\n");
            results = textEngine.search(textSearchDbPath, queryText, config, kbType, errCode);
            textItems.insert(textItems.end(), results.begin(), results.end());
        }
        if (stopFlag_.load(std::memory_order_acquire)) {
            stopFlag_.store(false, std::memory_order_release);
            return knowledgeItems;
        }
        PRINT_INFO("Begin Vector similarity search\n");
        results =
            vectorEngine.search(appId_, password_, dbFilePath_, vectorDbName_, queryText, config, kbType, errCode);
        vectorItems.insert(vectorItems.end(), results.begin(), results.end());
    }
    if (stopFlag_.load(std::memory_order_acquire)) {
        stopFlag_.store(false, std::memory_order_release);
        return knowledgeItems;
    }
    if (textSearchDbName_ != "") {
        PRINT_INFO("Begin keyword similarity search\n");
        std::vector<KnowledgeItem> results =
            textEngine.search(textSearchDbPath, queryDiverdification.keywords, config, kbType, errCode);
        keywordItems.insert(keywordItems.end(), results.begin(), results.end());
    }

    return knowledgeItems;
}

void HybridRetrievalStrategy::stopRetrieve() { stopFlag_.store(true, std::memory_order_release); }

std::unique_ptr<RetrievalStrategy> RetrievalStrategyFactory::createStrategy(
    RetrievalMethod method, const std::string& textSearchDbName, const std::string& vectorDbName,
    const std::string& appId, const std::string& password, const std::string& dbFilePath) {
    switch (method) {
        case RetrievalMethod::TEXT:
            return std::make_unique<TextRetrievalStrategy>(textSearchDbName, dbFilePath);
        case RetrievalMethod::KEYWORD:
            return std::make_unique<KeywordRetrievalStrategy>(textSearchDbName, dbFilePath);
        case RetrievalMethod::SEMANTIC:
            return std::make_unique<VectorRetrievalStrategy>(vectorDbName, appId, password, dbFilePath);
        case RetrievalMethod::HYBRID:
            return std::make_unique<HybridRetrievalStrategy>(textSearchDbName, vectorDbName, appId, password,
                                                             dbFilePath);
        default:
            return std::make_unique<HybridRetrievalStrategy>(textSearchDbName, vectorDbName, appId, password,
                                                             dbFilePath);
    }
}

}  // namespace knowledgebase
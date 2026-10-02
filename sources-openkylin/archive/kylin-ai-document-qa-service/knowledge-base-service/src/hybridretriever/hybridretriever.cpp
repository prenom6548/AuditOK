#include "hybridretriever.h"

#include <kyai-business-framework/log.h>

#include "../common/performancetimer.h"
#include "../storagemanager/textsearchengine.h"
#include "../storagemanager/vectorizationengine.h"
#include "retrievalstrategy.h"

namespace knowledgebase {

HybridRetriever::HybridRetriever(const std::string appId, const std::string password, const std::string dbFilePath)
    : appId_(appId), password_(password), dbFilePath_(dbFilePath) {
    traditionalDatabaseEngine_ = std::make_unique<TraditionalDatabaseEngine>(dbFilePath);
}

bool HybridRetriever::verifyPermissions(const std::string& knowledgeBaseId, KBErrorCode& errCode) {
    if (knowledgeBaseId.empty()) {  // 对app下所有知识库进行访问，不需要对知识库是否属于app进行鉴权
        return true;
    }
    return traditionalDatabaseEngine_->hasAccessPermission(appId_, knowledgeBaseId, errCode);
}

QueryDiversification HybridRetriever::diversifyQuery(const std::string& query, const QueryDiversifyConfig& config,
                                                     KBErrorCode& errCode, std::string& errMsg) {
    QueryDiversification diversifyResult;

    if (config.enable == false) {
        diversifyResult.queryTexts.push_back(query);
        errCode = KBErrorCode::SUCCESS;
        return diversifyResult;
    }

    querydiversifier_.setStrategy(std::make_unique<LLMDiversifierStrategy>());
    diversifyResult = querydiversifier_.expandQuery(query, config, errCode, errMsg);
    if (errCode != KBErrorCode::SUCCESS) {
        diversifyResult.queryTexts.clear();
        diversifyResult.keywords.clear();
        diversifyResult.queryTexts.push_back(query);
    }

    return diversifyResult;
}

KnowledgeItemsMap HybridRetriever::performRetrieval(const std::string& knowledgeBaseId,
                                                    const QueryDiversification& diversifiedQueries,
                                                    const RetrievalConfig& config, KBErrorCode& errCode) {
    KnowledgeItemsMap allKnowledgeItems;

    // 获取知识库信息
    std::vector<KnowledgeBaseInfo> knowledgeBaseInfos;
    if (knowledgeBaseId.empty() && !appId_.empty()) {
        knowledgeBaseInfos = traditionalDatabaseEngine_->getAllKnowledgeBase(appId_, errCode);
        int RAGfileCount = 0;
        std::vector<std::string> appFileIdList;
        for (auto& kbInfoItem : knowledgeBaseInfos) {
            std::vector<std::string> fileIdList =
                traditionalDatabaseEngine_->getFileListInKnowledgeBase(kbInfoItem.knowledgeBaseId_, errCode);
            RAGfileCount += fileIdList.size();
            appFileIdList.insert(appFileIdList.end(), fileIdList.begin(), fileIdList.end());

            if (kbInfoItem.type_ == KBType::FAQ) {
                // FAQ知识库中不存在文件，且FAQ为预制知识库，必然存在数据，做+1处理
                RAGfileCount += 1;
            }
        }
        if (RAGfileCount == 0) {
            errCode = KBErrorCode::KB_DATA_EMPTY;
        }
        for (auto& fileId : config.fileIds) {
            if (std::find(appFileIdList.begin(), appFileIdList.end(), fileId) == appFileIdList.end()) {
                errCode = KBErrorCode::RETRIEVAL_FILEID_FILTER_INVALID;
                return allKnowledgeItems;
            }
        }
    } else {
        KnowledgeBaseInfo kbInfo = traditionalDatabaseEngine_->getKnowledgeBase(knowledgeBaseId, errCode);

        std::vector<std::string> fileIdList =
            traditionalDatabaseEngine_->getFileListInKnowledgeBase(kbInfo.knowledgeBaseId_, errCode);
        // FAQ知识库中不存在文件，且FAQ为预制知识库，必然存在数据，直接跳过检查
        if (fileIdList.empty() && kbInfo.type_ != KBType::FAQ) {
            errCode = KBErrorCode::KB_DATA_EMPTY;
        }
        for (auto& fileId : config.fileIds) {
            if (std::find(fileIdList.begin(), fileIdList.end(), fileId) == fileIdList.end()) {
                errCode = KBErrorCode::RETRIEVAL_FILEID_FILTER_INVALID;
                return allKnowledgeItems;
            }
        }
        knowledgeBaseInfos.emplace_back(kbInfo);
    }
    if (errCode != KBErrorCode::SUCCESS) {
        return allKnowledgeItems;
    } else if (knowledgeBaseInfos.empty()) {
        errCode = KBErrorCode::KB_NOT_FOUND;
        return allKnowledgeItems;
    }

    auto& timer = PerformanceTimer::getInstance();
    RetrievalMethod retrievalMethod = config.retrievalMethod;

    for (const auto& kbInfo : knowledgeBaseInfos) {
        std::string knowledgeBaseId = kbInfo.knowledgeBaseId_;
        PRINT_INFO("Searching in knowledge base: %s\n", knowledgeBaseId.c_str());

        std::string collectionName, textSearchDbName;
        traditionalDatabaseEngine_->getKnowledgeBaseDatabaseName(knowledgeBaseId, collectionName, textSearchDbName,
                                                                 errCode);
        KBType kbType = kbInfo.type_;

        retrievalStrategy_ = RetrievalStrategyFactory::createStrategy(retrievalMethod, textSearchDbName, collectionName,
                                                                      appId_, password_, dbFilePath_);
        timer.start("单个知识库检索: " + knowledgeBaseId);
        if (stopFlag_.load(std::memory_order_acquire)) {
            stopFlag_.store(false, std::memory_order_release);
            return allKnowledgeItems;
        }
        KnowledgeItemsMap partKnowledgeItems = retrievalStrategy_->search(diversifiedQueries, config, kbType, errCode);

        // RAG的检索信息中可能用到知识库ID，所以将知识库ID返回
        if (kbType == KBType::RAG) {
            for (auto& knowledgeItems : partKnowledgeItems) {
                for (auto& knowledgeItem : knowledgeItems.second) {
                    FileChunk* chunk = static_cast<FileChunk*>(knowledgeItem.item_.get());
                    chunk->knowledgeBaseId_ = knowledgeBaseId;
                }
            }
        }

        for (const auto& pair : partKnowledgeItems) {
            const RetrievalMethod& method = pair.first;
            const std::vector<KnowledgeItem>& items = pair.second;

            allKnowledgeItems[method].insert(allKnowledgeItems[method].end(), items.begin(), items.end());
        }
        timer.end("单个知识库检索: " + knowledgeBaseId);
    }

    for (auto& knowledgeItems : allKnowledgeItems) {
        removeDuplicatesAndSortDescending(knowledgeItems.second);
    }

    return allKnowledgeItems;
}

RetrievalResult HybridRetriever::rerankRetrievedResults(const std::string& query,
                                                        const KnowledgeItemsMap& retrievalResults,
                                                        const RetrievalConfig& config) {
    RerankMethod rerankMethod = config.rerankMethod;
    std::unique_ptr<RerankingStrategy> rerankingstrategy = nullptr;
    switch (rerankMethod) {
        case RerankMethod::RRF:
            rerankingstrategy = RerankingStrategyFactory::createStrategy(RerankMethod::RRF);
            break;
        case RerankMethod::WEIGHT:
            rerankingstrategy = RerankingStrategyFactory::createStrategy(RerankMethod::WEIGHT);
            break;
        default:
            rerankingstrategy = RerankingStrategyFactory::createStrategy(RerankMethod::RRF);
            break;
    }

    RetrievalResult retrievalResult = {.queryText = query, .field = KnowledgeField::GENERAL};
    retrievalResult.items = rerankingstrategy->rerank(query, retrievalResults);

    // 基于 fuzzywuzzy 的重排序（可选）
    if (config.fuzzywuzzyRerankEnable && retrievalResult.items.size() > 0) {
        if (fuzzywuzzyRerankingPlugin_ == nullptr) {
            fuzzywuzzyRerankingPlugin_ = std::make_unique<FuzzywuzzyRerankingPlugin>();
        }
        retrievalResult.items = fuzzywuzzyRerankingPlugin_->fuzzyWuzzyRerank(query, retrievalResult.items);
    }

    // LLM 重排序与分类（可选）
    if (config.chunkEvaluateConfig.enable && retrievalResult.items.size() > 0) {
        auto& timer = PerformanceTimer::getInstance();
        timer.start("大语言模型重排");
        if (chunkEvaluator_ == nullptr) {
            chunkEvaluator_ = std::make_unique<ChunkEvaluator>();
        }
        KBErrorCode errCode = KBErrorCode::SUCCESS;
        chunkEvaluator_->rerankAndClassify(retrievalResult, config.chunkEvaluateConfig, errCode);
        if (errCode != KBErrorCode::SUCCESS) {
            PRINT_INFO("大语言模型重排失败 Error: %s", message(errCode).c_str());
        } else {
            filterKnowledgeItemsByScore(retrievalResult.items);
        }
        timer.end("大语言模型重排");
    }

    return retrievalResult;
}

void HybridRetriever::postProcessResults(RetrievalResult& retrievalResults, const RetrievalConfig& config) {
    filterKnowledgeItemByFileIds(retrievalResults.items, config);

    if (retrievalResults.items.size() > config.topK) {
        retrievalResults.items.resize(config.topK);
    }

    printRetrievalResults(retrievalResults.items);
}

void HybridRetriever::performStopRetrieve() {
    querydiversifier_.stopExpandQuery();
    if (chunkEvaluator_ != nullptr) {
        chunkEvaluator_->stop();
    }
    if (retrievalStrategy_ != nullptr) {
        retrievalStrategy_->stopRetrieve();
    }
}

void HybridRetriever::printRetrievalResults(const std::vector<KnowledgeItem>& finalItems) {
    PRINT_DEBUG("*******************************************************\n");
    int ragIndex = 0, faqIndex = 0;
    for (auto& knowledgeItem : finalItems) {
        if (knowledgeItem.type_ == KBType::RAG) {
            FileChunk* chunk = static_cast<FileChunk*>(knowledgeItem.item_.get());
            PRINT_DEBUG("RAG: KnowledgeBaseId: %s, num: %d, fileId: %s, chunkIndex: %d, similarity_:%f\n",
                        chunk->knowledgeBaseId_.c_str(), ragIndex, chunk->fileId_.c_str(), chunk->chunkIndex_,
                        chunk->similarity_);
            PRINT_DEBUG("content_:%s\n", chunk->content_.c_str());
            ragIndex++;
        } else if (knowledgeItem.type_ == KBType::FAQ) {
            FaqItem* faq = static_cast<FaqItem*>(knowledgeItem.item_.get());
            PRINT_DEBUG("FAQ: KnowledgeId: %s, num: %d, question: %s, similarity_:%f\n", faq->knowledgeId_.c_str(),
                        faqIndex, faq->question_.c_str(), faq->similarity_);
            PRINT_DEBUG("answer_:%s\n", faq->content_.c_str());
            faqIndex++;
        }
    }
    PRINT_DEBUG("*******************************************************\n");
}

void HybridRetriever::filterKnowledgeItemsByScore(std::vector<KnowledgeItem>& knowledgeItems) {
    // Step 1: 提取 similarity > 40 的条目
    std::vector<KnowledgeItem> filtered;

    for (const auto& item : knowledgeItems) {
        if (item.item_ && item.item_->similarity_ > 40.0f) {
            filtered.push_back(item);
        }
    }

    // 如果找到了 similarity > 40 的条目，直接保留并返回
    if (!filtered.empty()) {
        knowledgeItems = std::move(filtered);
        return;
    }

    // Step 2: 没有 >40 的，找 similarity > 10 的，并按相似度降序排序，取前 3
    std::vector<KnowledgeItem> candidates;
    for (const auto& item : knowledgeItems) {
        if (item.item_ && item.item_->similarity_ > 10.0f) {
            candidates.push_back(item);
        }
    }

    // 按 similarity_ 降序排序
    std::sort(candidates.begin(), candidates.end(), [](const KnowledgeItem& a, const KnowledgeItem& b) {
        return a.item_->similarity_ > b.item_->similarity_;
    });

    // 保留最多前 3 条
    if (!candidates.empty()) {
        knowledgeItems.assign(candidates.begin(), candidates.begin() + std::min(candidates.size(), size_t(3)));
    } else {
        // Step 3: 连 >10 的都没有，清空
        knowledgeItems.clear();
    }
}

void HybridRetriever::filterKnowledgeItemByFileIds(std::vector<KnowledgeItem>& knowledgeItems,
                                                   const RetrievalConfig& config) {
    if (config.fileIds.empty()) {
        return;
    }

    // 创建fileId查找集合
    std::unordered_set<std::string> fileIdSet(config.fileIds.begin(), config.fileIds.end());

    // 过滤chunks
    std::vector<KnowledgeItem> filteredKnowledgeItem;
    for (const auto& knowledgeItem : knowledgeItems) {
        // FAQ不与特定文件关联，所以不过滤
        if (knowledgeItem.type_ == KBType::FAQ) {
            continue;
        }
        FileChunk* chunk = static_cast<FileChunk*>(knowledgeItem.item_.get());
        if (fileIdSet.find(chunk->fileId_) != fileIdSet.end()) {
            filteredKnowledgeItem.push_back(knowledgeItem);
        }
    }

    knowledgeItems = filteredKnowledgeItem;
}

void HybridRetriever::removeDuplicatesAndSortDescending(std::vector<KnowledgeItem>& items) {
    std::sort(items.begin(), items.end(), [](const KnowledgeItem& a, const KnowledgeItem& b) {
        return a.item_->similarity_ > b.item_->similarity_;  // 降序
    });

    std::unordered_set<std::string> chunkSeen;
    std::unordered_set<std::string> faqSeen;
    std::vector<KnowledgeItem> uniqueItems;
    uniqueItems.reserve(items.size());

    for (auto& knowledgeItem : items) {
        if (knowledgeItem.type_ == KBType::RAG) {
            FileChunk* chunk = static_cast<FileChunk*>(knowledgeItem.item_.get());
            std::string key = chunk->fileId_ + ":" + std::to_string(chunk->chunkIndex_) + chunk->md5sum_;
            if (chunkSeen.insert(key).second) {
                uniqueItems.push_back(std::move(knowledgeItem));
            }
        } else {
            FaqItem* faq = static_cast<FaqItem*>(knowledgeItem.item_.get());
            std::string& key = faq->question_;
            if (faqSeen.insert(key).second) {
                uniqueItems.push_back(std::move(knowledgeItem));
            }
        }
    }

    items = std::move(uniqueItems);
}

}  // namespace knowledgebase

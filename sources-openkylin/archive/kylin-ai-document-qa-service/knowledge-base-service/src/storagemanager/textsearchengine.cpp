
#include "textsearchengine.h"

#include <chinese-segmentation/chinese-segmentation.h>
#include <kyai-business-framework/log.h>

#include <filesystem>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "../common/performancetimer.h"

namespace knowledgebase {

bool TextSearchEngine::createTextSearchDatabase(const std::string& appId, const std::string& textSearchDbPath,
                                                KBErrorCode& errCode) {
    errCode = KBErrorCode::SUCCESS;
    try {
        Xapian::WritableDatabase db(textSearchDbPath, Xapian::DB_CREATE_OR_OPEN);
        return true;
    } catch (const Xapian::Error& e) {
        PRINT_ERROR("Create DB error: %s\n", e.get_msg().c_str());
        errCode = KBErrorCode::TEXT_DB_CREATE_FAILED;
        return false;
    }
}

bool TextSearchEngine::deleteTextDatabase(const std::string& textSearchDbPath, KBErrorCode& errCode) {
    errCode = KBErrorCode::SUCCESS;
    try {
        std::filesystem::remove_all(textSearchDbPath);
        return true;
    } catch (const std::exception& e) {
        PRINT_ERROR("Delete DB error: %s\n", e.what());
        errCode = KBErrorCode::TEXT_DB_DELETE_FAILED;
        return false;
    }
}

bool TextSearchEngine::addKnowledgeItems(const std::string& textSearchDbPath,
                                         const std::vector<KnowledgeItem>& knowledgeItems,
                                         std::function<void(float)> callback, KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();  // 获取单例
    errCode = KBErrorCode::SUCCESS;
    ChineseSegmentation* chineseSegmentCut = ChineseSegmentation::getInstance();
    try {
        Xapian::WritableDatabase db(textSearchDbPath, Xapian::DB_CREATE_OR_OPEN);
        Xapian::TermGenerator termGenerator;

        // 添加 FileChunks
        size_t index = 0;
        float totalNum = static_cast<float>(knowledgeItems.size());
        // 在循环开始前预计算所有回调点，避免在循环中重复计算
        std::vector<size_t> callbackPoints;
        callbackPoints.reserve(21);
        for (int i = 1; i <= 20; ++i) {
            size_t point = static_cast<size_t>(totalNum * i / 20.0f);
            if (point == 0) {
                point = 1;
            }
            callbackPoints.push_back(point);
        }
        // 确保最后一个是总数
        callbackPoints.push_back(static_cast<size_t>(totalNum));
        size_t callbackPointIndex = 0;
        FileChunk* firstChunk = static_cast<FileChunk*>(knowledgeItems.at(0).item_.get());
        timer.start("文本分词: " + firstChunk->fileId_);
        for (const auto& item : knowledgeItems) {
            FileChunk* chunk = static_cast<FileChunk*>(item.item_.get());
            Xapian::Document doc;
            termGenerator.set_document(doc);
            ++index;
            if (callbackPointIndex < callbackPoints.size() && index >= callbackPoints[callbackPointIndex]) {
                float progress = static_cast<float>(index) / totalNum;
                callback(progress);
                ++callbackPointIndex;
            }
            PRINT_INFO("Text Database chunk index: %d, total size: %f\n", index, totalNum);

            // 使用中文分词替代原始 index_text
            std::vector<std::string> tokens = chineseSegmentCut->callMixSegmentCutStr(chunk->content_);
            PRINT_DEBUG("Mixed Segment Cut Result: **************************\n");
            for (const auto& word : tokens) {
                termGenerator.index_text(word, 1);  // 第2个参数是权重
                if (LOG_LEVEL >= LOG_LEVEL_DEBUG) {
                    fprintf(stdout, "%s ", word.c_str());
                    fflush(stdout);
                }
            }
            PRINT_DEBUG("Mixed Segment Cut Result End: **************************\n");
            doc.add_value(0, chunk->fileId_);
            doc.add_value(1, std::to_string(chunk->chunkIndex_));
            doc.add_value(2, chunk->metaData_);
            doc.add_value(3, chunk->sectionTitles_);
            doc.set_data(chunk->content_);
            db.add_document(doc);
        }
        timer.end("文本分词: " + firstChunk->fileId_);
        timer.start("文本存储: " + firstChunk->fileId_);
        db.commit();
        timer.end("文本存储: " + firstChunk->fileId_);

        return true;
    } catch (const Xapian::Error& e) {
        PRINT_ERROR("Add error: %s\n", e.get_msg().c_str());
        errCode = KBErrorCode::TEXT_DB_DATAUPLOAD_FAILED;
        return false;
    }
}

bool TextSearchEngine::deleteKnowledgeItems(const std::string& textSearchDbPath,
                                            const std::vector<std::string>& fileIds, KBErrorCode& errCode) {
    errCode = KBErrorCode::SUCCESS;
    try {
        Xapian::WritableDatabase db(textSearchDbPath, Xapian::DB_CREATE_OR_OPEN);
        Xapian::Enquire enquire(db);
        enquire.set_query(Xapian::Query::MatchAll);

        for (const auto& id : fileIds) {
            Xapian::MSet matches = enquire.get_mset(0, db.get_doccount());
            for (Xapian::MSetIterator it = matches.begin(); it != matches.end(); ++it) {
                Xapian::Document doc = it.get_document();
                std::string stored_path = doc.get_value(0);
                if (stored_path == id) {
                    db.delete_document(doc.get_docid());
                }
            }
        }
        return true;
    } catch (const Xapian::Error& e) {
        PRINT_ERROR("Delete error: %s\n", e.get_msg().c_str());
        errCode = KBErrorCode::TEXT_DB_DATADELETE_FAILED;
        return false;
    }
}

std::vector<KnowledgeItem> TextSearchEngine::getKnowledgeItems(const std::string& textSearchDbPath,
                                                               const std::vector<std::string>& ids, const KBType kbType,
                                                               KBErrorCode& errCode) {
    if (kbType == KBType::FAQ) {
        PRINT_ERROR("Not supported type : KBType::FAQ\n");
        errCode = KBErrorCode::INVALID_PARAMETER;
        return {};
    }
    std::vector<KnowledgeItem> results;
    errCode = KBErrorCode::SUCCESS;
    try {
        Xapian::WritableDatabase db(textSearchDbPath);
        Xapian::Enquire enquire(db);
        enquire.set_query(Xapian::Query::MatchAll);

        for (const auto& id : ids) {
            Xapian::MSet matches = enquire.get_mset(0, db.get_doccount());
            for (Xapian::MSetIterator it = matches.begin(); it != matches.end(); ++it) {
                Xapian::Document doc = it.get_document();
                std::string stored_path = doc.get_value(0);
                if (stored_path == id) {
                    // db.delete_document(doc.get_docid());
                    std::shared_ptr<FileChunk> chunkPtr = std::make_shared<FileChunk>();
                    chunkPtr->fileId_ = doc.get_value(0);
                    chunkPtr->content_ = doc.get_data();
                    chunkPtr->sectionTitles_ = doc.get_value(3);
                    chunkPtr->chunkIndex_ = atoi(doc.get_value(1).c_str());
                    chunkPtr->metaData_ = doc.get_value(2);
                    KnowledgeItem item(chunkPtr, KBType::RAG);
                    results.push_back(item);
                }
            }
        }
        return results;
    } catch (const Xapian::Error& e) {
        PRINT_ERROR("Delete error: %s\n", e.get_msg().c_str());
        errCode = KBErrorCode::TEXT_RETRIEVAL_FAILED;
        return results;
    }
}

std::vector<KnowledgeItem> TextSearchEngine::search(const std::string& textSearchDbPath, const std::string& queryText,
                                                    const RetrievalConfig& config, const KBType kbType,
                                                    KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();  // 获取单例
    errCode = KBErrorCode::SUCCESS;
    std::vector<KnowledgeItem> results;
    ChineseSegmentation* chineseSegmentCut = ChineseSegmentation::getInstance();
    try {
        Xapian::Database db(textSearchDbPath);

        // 使用中文分词替代原始 index_text
        timer.start("问题分词: " + queryText + " " + textSearchDbPath);
        std::vector<std::string> tokens = chineseSegmentCut->callMixSegmentCutStr(queryText);
        PRINT_DEBUG("问题分词结果：");
        for (const auto& term : tokens) {
            PRINT_DEBUG("%s ", term.c_str());
        }
        PRINT_DEBUG("\n");
        timer.end("问题分词: " + queryText + " " + textSearchDbPath);
        std::vector<Xapian::Query> subqueries;
        for (const auto& term : tokens) {
            subqueries.emplace_back(term);
        }

        Xapian::Query query(Xapian::Query::OP_OR, subqueries.begin(), subqueries.end());
        Xapian::Enquire enquire(db);
        enquire.set_query(query);

        timer.start("文本检索: " + queryText + " " + textSearchDbPath);
        extractMatchResults(enquire, config, kbType, results);
        timer.end("文本检索: " + queryText + " " + textSearchDbPath);
    } catch (const Xapian::Error& e) {
        PRINT_ERROR("Search error: %s\n", e.get_msg().c_str());
        errCode = KBErrorCode::TEXT_RETRIEVAL_FAILED;
    }
    return results;
}

std::vector<KnowledgeItem> TextSearchEngine::search(const std::string& textSearchDbPath,
                                                    const std::vector<std::string>& keywords,
                                                    const RetrievalConfig& config, const KBType kbType,
                                                    KBErrorCode& errCode) {
    auto& timer = PerformanceTimer::getInstance();  // 获取单例
    errCode = KBErrorCode::SUCCESS;
    std::vector<KnowledgeItem> results;
    try {
        Xapian::Database db(textSearchDbPath);

        std::vector<Xapian::Query> subqueries;
        for (const auto& term : keywords) {
            subqueries.emplace_back(term);
        }

        Xapian::Query query(Xapian::Query::OP_OR, subqueries.begin(), subqueries.end());
        Xapian::Enquire enquire(db);
        enquire.set_query(query);

        std::string keywordList;
        for (const auto& keyword : keywords) {
            keywordList += keyword + " ";
        }
        timer.start("关键词检索: " + keywordList);
        extractMatchResults(enquire, config, kbType, results);
        timer.end("关键词检索: " + keywordList);
    } catch (const Xapian::Error& e) {
        PRINT_ERROR("Search error: %s\n", e.get_msg().c_str());
        errCode = KBErrorCode::TEXT_RETRIEVAL_FAILED;
    }
    return results;
}

void TextSearchEngine::extractMatchResults(Xapian::Enquire& enquire, const RetrievalConfig& config, KBType kbType,
                                           std::vector<KnowledgeItem>& results) {
    Xapian::MSet matches = enquire.get_mset(0, config.topK);
    // 在假设有一篇文档包含了所有查询词项，并且每个词项在该文档中出现的频率足够高（达到饱和）的情况下，该文档可能获得的得分
    double maxPossibleScore = matches.get_max_possible();
    PRINT_DEBUG("Max possible score: %f\n", maxPossibleScore);
    for (auto it = matches.begin(); it != matches.end(); ++it) {
        const Xapian::Document& doc = it.get_document();
        double score = it.get_weight() / maxPossibleScore;
        if (score < config.bm25ScoreThreshold) {
            PRINT_INFO("Search similarity %f less than threshold %f, chunkIndex: %d, fileId: %s\n", score,
                       config.bm25ScoreThreshold, std::stoi(doc.get_value(1)), doc.get_value(0).c_str());
            continue;
        }

        if (kbType == KBType::RAG) {  // 简单判断是否为FAQ或Chunk
            std::shared_ptr<FileChunk> chunkPtr = std::make_shared<FileChunk>();
            chunkPtr->fileId_ = doc.get_value(0);
            chunkPtr->content_ = doc.get_data();
            chunkPtr->sectionTitles_ = doc.get_value(3);
            chunkPtr->chunkIndex_ = atoi(doc.get_value(1).c_str());
            chunkPtr->metaData_ = doc.get_value(2);
            chunkPtr->similarity_ = score;
            KnowledgeItem item(chunkPtr, KBType::RAG);
            results.push_back(item);
            PRINT_INFO("Search similarity %f, chunkIndex: %d, fileId: %s\n", score, chunkPtr->chunkIndex_,
                       chunkPtr->fileId_.c_str());
        } else if (kbType == KBType::FAQ) {
            std::shared_ptr<FaqItem> faqItemPtr = std::make_shared<FaqItem>();
            faqItemPtr->knowledgeId_ = doc.get_value(0);
            faqItemPtr->question_ = doc.get_value(1);
            faqItemPtr->description_ = doc.get_value(2);
            faqItemPtr->content_ = doc.get_value(3);
            faqItemPtr->similarity_ = score;
            KnowledgeItem item(faqItemPtr, KBType::FAQ);
            results.push_back(item);
        }
    }
}

}  // namespace knowledgebase

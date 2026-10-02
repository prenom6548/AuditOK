#include "fuzzycompositererankerplugin.h"

#include <Python.h>
namespace knowledgebase {

// Python集成类
class PythonFuzzyMatcher {
public:
    PythonFuzzyMatcher() {
        PyGILState_STATE gstate = PyGILState_Ensure();

        // 导入fuzzywuzzy模块
        PyObject* fuzzModule = PyImport_ImportModule("fuzzywuzzy.fuzz");
        if (fuzzModule) {
            partialRatioFunc_ = PyObject_GetAttrString(fuzzModule, "partial_ratio");
            Py_DECREF(fuzzModule);
        } else {
            PyErr_Print();
        }
        PyGILState_Release(gstate);
    }

    ~PythonFuzzyMatcher() {
        if (partialRatioFunc_) {
            Py_DECREF(partialRatioFunc_);
        }
    }

    int partialRatio(const std::string& str1, const std::string& str2) {
        if (!partialRatioFunc_) {
            return 0;
        }
        PyGILState_STATE gstate = PyGILState_Ensure();

        PyObject* args = PyTuple_Pack(2, PyUnicode_FromString(str1.c_str()), PyUnicode_FromString(str2.c_str()));

        PyObject* result = PyObject_CallObject(partialRatioFunc_, args);
        Py_DECREF(args);

        int score = 0;
        if (result) {
            score = PyLong_AsLong(result);
            Py_DECREF(result);
        }
        PyGILState_Release(gstate);

        return score;
    }

private:
    PyObject* partialRatioFunc_ = nullptr;
};

// Reranker实现类
class FuzzyCompositeReranker {
public:
    FuzzyCompositeReranker() : fuzzyMatcher_(std::make_unique<PythonFuzzyMatcher>()) {}

    std::unordered_set<std::string> extractKeywords(const std::string& text) {
        static const std::unordered_set<std::string> stopWords = {
            "的", "了", "是", "在", "和", "与",  "或", "等", "中",  "上", "下",  "为", "有", "以", "及", "将", "对",
            "从", "到", "由", "被", "把", "the", "a",  "an", "and", "or", "but", "in", "on", "at", "to", "for"};

        std::unordered_set<std::string> keywords;

        // 提取中文字符
        for (size_t i = 0; i < text.length();) {
            unsigned char c = text[i];
            if (c >= 0xE4 && c <= 0xE9 && i + 2 < text.length()) {
                std::string zh_char = text.substr(i, 3);
                if (stopWords.find(zh_char) == stopWords.end()) {
                    keywords.insert(zh_char);
                }
                i += 3;
            } else {
                i++;
            }
        }

        // 提取英文单词
        std::regex enPattern(R"(\b[a-zA-Z]{2,}\b)");
        std::sregex_iterator iter(text.begin(), text.end(), enPattern);
        std::sregex_iterator end;

        for (; iter != end; ++iter) {
            std::string word = iter->str();
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            if (stopWords.find(word) == stopWords.end()) {
                keywords.insert(word);
            }
        }

        return keywords;
    }

    std::unordered_set<std::string> extractProperNouns(const std::string& text) {
        std::unordered_set<std::string> nouns;

        // 英文专有名词：大写字母开头的单词或连续大写字母
        std::regex enPattern(R"(\b[A-Z][a-z]+\b|[A-Z]{2,})");
        std::sregex_iterator iter(text.begin(), text.end(), enPattern);
        std::sregex_iterator end;

        for (; iter != end; ++iter) {
            nouns.insert(iter->str());
        }

        // 中文词组（简化处理：提取连续的中文字符，长度>=2）
        // 使用简单的字节处理而不是wstring_convert
        std::string currentWord;
        for (size_t i = 0; i < text.length();) {
            unsigned char c = text[i];
            // 检测UTF-8中文字符（3字节）
            if (c >= 0xE4 && c <= 0xE9 && i + 2 < text.length()) {
                currentWord += text.substr(i, 3);
                i += 3;
            } else {
                // 非中文字符，保存之前累积的词
                if (currentWord.length() >= 6) {  // 至少2个中文字符（6字节）
                    nouns.insert(currentWord);
                }
                currentWord.clear();
                i++;
            }
        }
        // 处理最后一个词
        if (currentWord.length() >= 6) {
            nouns.insert(currentWord);
        }

        return nouns;
    }

    double calculateCompositeScore(const std::string& query, const KnowledgeItem& item, int milvusRank, double& fuzzy,
                                   double& kw, double& vec) {
        const std::string& content = item.item_->content_;

        // 1. 模糊匹配分数
        double fuzzyScore = fuzzyMatcher_->partialRatio(query, content);

        // 2. 关键词覆盖率
        auto queryKeywords = extractKeywords(query);
        auto contentKeywords = extractKeywords(content);

        double keywordCoverage = 0.0;
        if (!queryKeywords.empty()) {
            int hits = 0;
            for (const auto& kw : queryKeywords) {
                if (contentKeywords.find(kw) != contentKeywords.end()) {
                    hits++;
                }
            }
            keywordCoverage = (static_cast<double>(hits) / queryKeywords.size()) * 100.0;
        }

        // 3. Milvus语义相似度
        double milvusSimilarity = item.item_->similarity_ * 100.0;

        // 4. 位置权重
        double positionBonus = 0.0;
        if (milvusRank > 0) {
            positionBonus = std::max(0.0, 20.0 - milvusRank);
        }

        // 5. 长度评分
        size_t contentLen = content.length();
        double lengthScore = 100.0;
        if (contentLen >= 200 && contentLen <= 600) {
            lengthScore = 100.0;
        } else if (contentLen < 200) {
            lengthScore = 50.0 + (static_cast<double>(contentLen) / 200.0) * 50.0;
        } else {
            lengthScore = 100.0 - std::min(50.0, static_cast<double>(contentLen - 600) / 20.0);
        }

        // 6. 专有名词加分
        auto queryNouns = extractProperNouns(query);
        auto contentNouns = extractProperNouns(content);

        double properNounBonus = 0.0;
        if (!queryNouns.empty()) {
            int hits = 0;
            for (const auto& noun : queryNouns) {
                if (contentNouns.find(noun) != contentNouns.end()) {
                    hits++;
                }
            }
            if (hits > 0) {
                properNounBonus = 30.0;
            }
        }

        // 7. 综合评分
        double baseScore = fuzzyScore * 0.25 + keywordCoverage * 0.25 + milvusSimilarity * 0.35 + lengthScore * 0.15;

        double finalScore = baseScore + positionBonus + properNounBonus;
        fuzzy = fuzzyScore;
        kw = keywordCoverage;
        vec = milvusSimilarity;

        return finalScore;
    }

private:
    std::unique_ptr<PythonFuzzyMatcher> fuzzyMatcher_;
};

// 主要的rerank函数
std::vector<KnowledgeItem> FuzzywuzzyRerankingPlugin::fuzzyWuzzyRerank(
    const std::string& query, const std::vector<KnowledgeItem>& retrievalResults) {
    if (retrievalResults.empty()) {
        return {};
    }

    if (retrievalResults.size() == 1) {
        return retrievalResults;
    }

    // 收集所有items并记录其排名
    struct ItemWithRank {
        KnowledgeItem item;
        int milvusRank;
    };

    std::vector<ItemWithRank> allItems;
    nlohmann::json jsonResult;
    jsonResult["query"] = query;
    nlohmann::json jsonArray;
    int index = 0;
    for (const auto& item : retrievalResults) {
        ItemWithRank itemWithRank;
        itemWithRank.item = item;
        itemWithRank.milvusRank = static_cast<int>(index) + 1;
        allItems.push_back(itemWithRank);
        FileChunk* chunk = static_cast<FileChunk*>(itemWithRank.item.item_.get());
        std::string contentId = chunk->fileId_ + ":" + std::to_string(chunk->chunkIndex_);
        jsonArray.push_back(nlohmann::json{
            {"content_id", contentId},
            {"content", item.item_->content_},
            {"semantic_score", item.item_->similarity_},
        });
        index++;
    }
    jsonResult["sq_context"] = jsonArray;
    PRINT_DEBUG("[RerankerAndFilterStrategy::rerank] jsonResult: %s\n", jsonResult.dump().c_str());

    // 去重（基于FileChunk的fileId+chunkIndex+md5sum或FaqItem的question）
    std::unordered_map<std::string, ItemWithRank> uniqueItems;
    for (const auto& itemWithRank : allItems) {
        std::string key;
        if (itemWithRank.item.type_ == KBType::RAG) {
            FileChunk* chunk = static_cast<FileChunk*>(itemWithRank.item.item_.get());
            key = chunk->fileId_ + ":" + std::to_string(chunk->chunkIndex_) + chunk->md5sum_;
        } else if (itemWithRank.item.type_ == KBType::FAQ) {
            FaqItem* faq = static_cast<FaqItem*>(itemWithRank.item.item_.get());
            key = faq->question_;
        }

        if (uniqueItems.find(key) == uniqueItems.end()) {
            uniqueItems[key] = itemWithRank;
        }
    }

    // 计算综合评分
    FuzzyCompositeReranker reranker;
    std::vector<std::pair<KnowledgeItem, double>> scoredItems;
    scoredItems.reserve(uniqueItems.size());

    nlohmann::json jsonOutput;
    jsonOutput["query"] = query;
    nlohmann::json jsonOutputArray;
    for (auto& [key, itemWithRank] : uniqueItems) {
        double fuzzy = .0, kw = .0, vec = .0;
        double score =
            reranker.calculateCompositeScore(query, itemWithRank.item, itemWithRank.milvusRank, fuzzy, kw, vec);
        scoredItems.emplace_back(itemWithRank.item, score);
        FileChunk* chunk = static_cast<FileChunk*>(itemWithRank.item.item_.get());
        std::string contentId = chunk->fileId_ + ":" + std::to_string(chunk->chunkIndex_);
        nlohmann::json jsonScoreDetails;
        jsonScoreDetails = {{"final", score}, {"fuzzy", fuzzy}, {"keyword", kw}, {"vector", vec}};
        jsonOutputArray.push_back(nlohmann::json{
            {"content_id", contentId},
            {"content", itemWithRank.item.item_->content_},
            {"composite_score", score},
            {"score_details", jsonScoreDetails},
        });
    }
    jsonOutput["sq_context"] = jsonOutputArray;
    PRINT_DEBUG("jsonOutput :%s\n", jsonOutput.dump().c_str());

    // 按综合评分降序排序
    std::sort(scoredItems.begin(), scoredItems.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

    std::vector<KnowledgeItem> finalResult;
    finalResult.reserve(scoredItems.size());
    for (auto& [item, score] : scoredItems) {
        finalResult.push_back(std::move(item));
    }

    return finalResult;
}
}  // namespace knowledgebase
#pragma once
#include <xapian.h>

#include "../common/common.h"
#include "../common/utils.h"

namespace knowledgebase {

class TextSearchEngine {
public:
    TextSearchEngine(const TextSearchEngine&) = delete;
    TextSearchEngine& operator=(const TextSearchEngine&) = delete;
    static TextSearchEngine& getInstance() {
        static TextSearchEngine instance;
        return instance;
    }

private:
    TextSearchEngine() = default;
    ~TextSearchEngine() = default;

public:
    // 初始化知识库的文本索引
    bool createTextSearchDatabase(const std::string& appId, const std::string& textSearchDbPath, KBErrorCode& errCode);
    // 删除知识库的文本索引
    bool deleteTextDatabase(const std::string& textSearchDbPath, KBErrorCode& errCode);

    // 向文本索引中添加一组Chunk
    bool addKnowledgeItems(const std::string& textSearchDbPath, const std::vector<KnowledgeItem>& knowledgeItems,
                           std::function<void(float)> callback, KBErrorCode& errCode);

    // 从文本索引中删除一组文件Chunk或一组faq的Knowledge条目
    bool deleteKnowledgeItems(const std::string& textSearchDbPath, const std::vector<std::string>& fileIds,
                              KBErrorCode& errCode);

    // 从文本索引中获取一组文件Chunk或一组faq的Knowledge条目
    std::vector<KnowledgeItem> getKnowledgeItems(const std::string& textSearchDbPath,
                                                 const std::vector<std::string>& ids, const KBType kbType,
                                                 KBErrorCode& errCode);

    // 搜索：给定查询向量，返回最相似的文档Chunks对象和相似度
    std::vector<KnowledgeItem> search(const std::string& textSearchDbPath, const std::string& queryText,
                                      const RetrievalConfig& config, const KBType kbType, KBErrorCode& errCode);

    std::vector<KnowledgeItem> search(const std::string& textSearchDbPath, const std::vector<std::string>& keywords,
                                      const RetrievalConfig& config, const KBType kbType, KBErrorCode& errCode);

private:
    void extractMatchResults(Xapian::Enquire& enquire, const RetrievalConfig& config, KBType kbType,
                             std::vector<KnowledgeItem>& results);

private:
    std::string dbPath_;
};

}  // namespace knowledgebase
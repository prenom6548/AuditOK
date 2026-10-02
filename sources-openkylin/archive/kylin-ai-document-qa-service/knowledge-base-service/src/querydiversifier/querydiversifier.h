// querydiversifier.h
#pragma once

#include "../common/common.h"
#include "diversifystrategy.h"

namespace knowledgebase {
class QueryDiversifier {
public:
    QueryDiversifier() = default;
    ~QueryDiversifier() = default;

    // 设置新的扩展策略
    void setStrategy(std::unique_ptr<QueryDiversifierStrategy> strategy) { strategy_ = std::move(strategy); };

    // 对查询进行扩展
    QueryDiversification expandQuery(const std::string& query, const QueryDiversifyConfig& config,
                                     KBErrorCode& errCode, std::string& errMsg) const;

    void stopExpandQuery();

private:
    std::unique_ptr<QueryDiversifierStrategy> strategy_;
};

}  // namespace knowledgebase

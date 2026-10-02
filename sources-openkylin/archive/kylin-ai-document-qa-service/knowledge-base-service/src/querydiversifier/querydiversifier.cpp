#include "querydiversifier.h"

#include <kyai-business-framework/log.h>

#include <nlohmann/json.hpp>

#include "../common/common.h"

namespace knowledgebase {
QueryDiversification QueryDiversifier::expandQuery(const std::string& query, const QueryDiversifyConfig& config,
                                                   KBErrorCode& errCode, std::string& errMsg) const {
    return strategy_->diversify(query, config, errCode, errMsg);
};

void QueryDiversifier::stopExpandQuery() {
    if (strategy_ != nullptr) {
        strategy_->stopDiversify();
    }
}

}  // namespace knowledgebase

#pragma once
#include <kyai-business-framework/log.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <nlohmann/json.hpp>
#include <regex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "rerankstrategy.h"

namespace knowledgebase {

class FuzzywuzzyRerankingPlugin {
public:
    FuzzywuzzyRerankingPlugin() = default;
    ~FuzzywuzzyRerankingPlugin() = default;

    std::vector<KnowledgeItem> fuzzyWuzzyRerank(const std::string& query,
                                                const std::vector<KnowledgeItem>& retrievalResults);

private:
};

}  // namespace knowledgebase

#ifndef RETRIEVAL_CONFIG_INTERNAL_H_
#define RETRIEVAL_CONFIG_INTERNAL_H_

#include <string>
#include <vector>

#include "core.h"
#include "retrievalconfig.h"

struct KBRetrievalConfig {
    std::vector<std::string> docIds;
    bool topKSet{false};
    int topK{0};
    kyai::advancedai::knowledgebase::core::RetrievalMethod retrievalMethod{
        kyai::advancedai::knowledgebase::core::RetrievalMethod::HYBRID};
    kyai::advancedai::knowledgebase::core::RerankMethod rerankMethod{
        kyai::advancedai::knowledgebase::core::RerankMethod::WEIGHT};
};

#endif

#ifndef KB_INFO_H_
#define KB_INFO_H_

#include <string>

struct KnowledgeBaseInfo {
    std::string name;
    std::string description;
    std::string knowledgeBaseId;
    std::string indexType = "FLAT";
    std::string kbType = "RAG";
};
#endif
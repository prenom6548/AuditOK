#ifndef ERROR_H_
#define ERROR_H_

#include <string>

#include "error.h"

namespace kyai::advancedai::knowledgebase {
extern int getErrorCodeFromAuth(int code);
extern int getErrorCodeFromKnowledgebase(int code);
extern KBDocumentStatus getDocumentStatus(const std::string& status);
extern std::string getErrorMessageFromKnowledgebase(int code);
}  // namespace kyai::advancedai::knowledgebase
#endif
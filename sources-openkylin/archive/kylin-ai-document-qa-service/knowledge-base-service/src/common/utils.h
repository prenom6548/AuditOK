#pragma once

#include <string>
#include <vector>

#include "errno.h"

namespace knowledgebase {

std::string stringToUpper(const std::string& str);

RetrievalMethod stringToRetrievalMethod(const std::string& str);

std::string retrievalMethodToString(RetrievalMethod method);

RerankMethod stringToRerankMethod(const std::string& str);

std::string fileUploadStatusToString(FileUploadStatus status);

std::string getHomeDirectory();

std::string removeWhitespace(const std::string& str);

KBErrorCode genaisdkErrorCodeToKBErrorCode(int errCode);

std::string computeMD5FromFile(const std::string& absolutePath);

bool pathReadable(const std::string& path);

bool pathWritable(const std::string& path);

std::vector<std::string> parseStringToVector(const std::string& input, const char delimiter);

std::vector<int> parseStringToIntVector(const std::string& input, const char delimiter);

std::string readFileToString(const std::string& filePath);

bool createDirectoryIfNotExists(const std::string& path);

}  // namespace knowledgebase
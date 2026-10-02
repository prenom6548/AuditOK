/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef RAGSERVERPROXY_H
#define RAGSERVERPROXY_H

#include <string>
#include <vector>

#include "business/knowledge-base/knowledgebase.h"

struct RagErrorInfo {
    int32_t errorCode;
    std::string errorMessage;
};

class RagServerProxy {
public:
    struct SimilaritySearchResult {
        std::string filePath;
        std::string fileContent;
        double similarity{0.0};
    };

    using SimilaritySearchResults = std::vector<SimilaritySearchResult>;

public:
    static RagServerProxy &instance();

    bool isValid() const;
    RagServerProxy(const RagServerProxy &) = delete;
    RagServerProxy &operator=(const RagServerProxy &) = delete;

    std::string createKnowledgeBase(RagErrorInfo &error);
    void deleteKnowledgeBase(const std::string &knowledgeBaseName);
    RagErrorInfo addFiles(const std::string &knowledgeBaseName, const std::vector<std::string> &files);

    SimilaritySearchResults similaritySearch(const std::string &knowledgeBaseName, const std::string &searchConditions,
                                             RagErrorInfo &error);

    std::string getFileContent(const std::string &knowledgeBaseName, const std::string &filePath);

private:
    RagServerProxy();

    SimilaritySearchResults handleSearchResult(const std::string &searchResults);

private:
    knowledgebase::KnowledgebaseSession session_;
};

#endif  // RAGSERVERPROXY_H

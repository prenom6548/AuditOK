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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DATAMANAGEMENTSERVICE_H
#define DATAMANAGEMENTSERVICE_H

#include "datamanagement/datamanagementdatabase.h"

#include <string>
#include <functional>
#include <mutex>

class FileInfo;
class Parser;
class DataManagementService
{
public:
    using SimilaritySearchCallback = std::function<void(const std::string&, int)>;
    using AddTextFilesCallback = std::function<void(int)>;
    using AddImageFilesCallback = std::function<void(int)>;
    using DeleteFilesCallback = std::function<void(int)>;
    using UpdateFilesNameCallback = std::function<void(int)>;
    using UpdateFilesContentCallback = std::function<void(int)>;
    using GetAllFileinfosCallback = std::function<void(const std::string&, int)>;
    using GetFeatureStatusCallback = std::function<void(int, int)>;

    static DataManagementService* getInstance() {
        static DataManagementService service;
        return &service;
    }

    void similaritySearch(const std::string& searchConditions, SimilaritySearchCallback callback);
    void addTextFiles(const std::string& fileinfos, AddTextFilesCallback callback);
    void addImageFiles(const std::string& fileinfos, AddImageFilesCallback callback);
    void deleteFiles(const std::string& fileinfos, DeleteFilesCallback callback);
    void updateFilesName(const std::string& fileinfos, UpdateFilesNameCallback callback);
    void updateFilesContent(const std::string& fileinfos, UpdateFilesContentCallback callback);
    void getAllFileInfos(GetAllFileinfosCallback callback);
    void getFeatureStatus(GetFeatureStatusCallback callback);

private:
    DataManagementService();

    std::string doSimilaritySearch(const std::string& searchConditions, int& errorCode);
    void doAddImageFiles(const std::string& fileinfosJson, int& errorCode);
    std::vector<DBFileInfo> makeDatasByImageFilePathAndFormat(const std::vector<std::pair<std::string, std::string>>& fileinfos);
    void doAddTextFiles(const std::string& fileinfosJson, int& errorCode);
    std::vector<DBFileInfo> makeDatasByTextFilePathAndFormat(const std::vector<std::pair<std::string, std::string>>& fileinfos);
    std::string doGetAllFileInfos(int& errorCode);
    void doDeleteFiles(const std::string& fileinfosJson, int& errorCode);
    void doUpdateFilesContent(const std::string& fileinfosJson, int& errorCode);
    int doGetFeatureStatus(int& errorCode);

    SimilaritySearchResult mergeSearchResults(const SimilaritySearchResult& visionSearchResult,
                                              const SimilaritySearchResult& textSearchResult) const;
    void filtSearchResults(SimilaritySearchResult& searchResult, double threshold) const;
    SimilaritySearchResult deduplicatedSearchResults(const SimilaritySearchResult &textSearchResult) const;
    FileInfo parserTextFileInfo(const std::string& filepath, const std::string& format) const;
    void truncateTexts(std::vector<std::string>& texts, int maxLength) const;
    bool isSupportedImageFormat(const std::string& format) const;
    bool isSupportedTextFormat(const std::string& format) const;

    std::vector<float> textSideEmbeddingText(const std::string& text);
    std::vector<std::vector<float>> textSideEmbeddingTexts(const std::vector<std::string>& texts);
    std::vector<float> imageSideEmbeddingText(const std::string& text);
    std::vector<float> imageSideEmbeddingImage(const std::string& filePath);

private:
    DataManagementDatabase database_;
};

#endif // DATAMANAGEMENTSERVICE_H

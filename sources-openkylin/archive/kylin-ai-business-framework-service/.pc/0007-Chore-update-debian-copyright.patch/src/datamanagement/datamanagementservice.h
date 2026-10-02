#ifndef DATAMANAGEMENTSERVICE_H
#define DATAMANAGEMENTSERVICE_H

#include "datamanagement/datamanagementdatabase.h"

#include <string>
#include <functional>
#include <mutex>

class FileInfo;
class Parser;
class EmbeddingTaskManager;
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

    static DataManagementService* getInstance() {
        static DataManagementService service;
        return &service;
    }

    void similaritySearch(const std::string& searchConditions, uid_t uid, SimilaritySearchCallback callback);
    void addTextFiles(const std::string& fileinfos, uid_t uid, AddTextFilesCallback callback);
    void addImageFiles(const std::string& fileinfos, uid_t uid, AddImageFilesCallback callback);
    void deleteFiles(const std::string& fileinfos, uid_t uid, DeleteFilesCallback callback);
    void updateFilesName(const std::string& fileinfos, uid_t uid, UpdateFilesNameCallback callback);
    void updateFilesContent(const std::string& fileinfos, uid_t uid, UpdateFilesContentCallback callback);
    void getAllFileInfos(uid_t uid, GetAllFileinfosCallback callback);


private:
    DataManagementService();

    std::string doSimilaritySearch(const std::string& searchConditions, uid_t uid, int& errorCode);
    void doAddImageFiles(const std::string& fileinfosJson, uid_t uid, int& errorCode);
    std::vector<DBFileInfo> makeDatasByImageFilePathAndFormat(const std::vector<std::pair<std::string, std::string>>& fileinfos);
    void doAddTextFiles(const std::string& fileinfosJson, uid_t uid, int& errorCode);
    std::vector<DBFileInfo> makeDatasByTextFilePathAndFormat(const std::vector<std::pair<std::string, std::string>>& fileinfos);
    std::string doGetAllFileInfos(uid_t uid, int& errorCode);
    void doDeleteFiles(const std::string& fileinfosJson, uid_t uid, int& errorCode);
    void doUpdateFilesContent(const std::string& fileinfosJson, uid_t uid, int& errorCode);

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
    EmbeddingTaskManager* embeddingTaskManager_ = nullptr;


    std::mutex taskMutex_;
};

#endif // DATAMANAGEMENTSERVICE_H

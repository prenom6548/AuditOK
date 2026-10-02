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

#include "datamanagementservice.h"
#include "thirdparty/threadpool/async.h"
#include "datamanagement/datamanagementjsonhelper.h"
#include "utils/parser/parser.h"
#include "utils/parser/fileparserfactory.h"
#include "datamanagement/segmenttokenizer.h"
#include "embeddingtaskmanager/embeddingtaskmanager.h"
#include "embeddingtaskmanager/embeddingtask.h"

#include <iostream>
#include <filesystem>

static const double TEXT_SEARCH_THRESHOLD = 0.8;
static const double VISION_SEARCH_THRESHOLD = 0.4;

typedef enum {
    DATA_MANAGEMENT_SUCCESS = 0,
    DATA_MANAGEMENT_UNKNOWN_ERROR, // 未知错误
    DATA_MANAGEMENT_SESSION_ERROR, // session 错误
    DATA_MANAGEMENT_INVAILD_PATH, // 无效的路径
    DATA_MANAGEMENT_CANT_OPEN_DB, // 无法打开数据库
    DATA_MANAGEMENT_INPUT_TEXT_LENGTH_INVAILD, // 输入文本长度超限
    DATA_MANAGEMENT_PARAM_ERROR, // 参数错误
    DATA_MANAGEMENT_FILE_FORMAT_ERROR, // 文件格式错误
} DataManagementResult;

DataManagementService::DataManagementService()
{
    //! \note chromadb 内部也有使用多线程，如果直接在其他线程里使用chromadb，
    //! 可能会导致对 Python GIL 的操作冲突，经过验证，首次使用 chromadb 需要
    //! 在初始化 Python 环境的线程，这里选择调用数据库接口初始化 collection
    //! 此外还存在一个问题，实际上每次destroyClient都没有能真正释放掉数据库资源，
    //! 如果强行在 destroyClient 中清理所有的引用计数，会导致使用数据库接口时概率崩溃
    database_.createClient();
    database_.initCollections();
    database_.destroyClient();
}

void DataManagementService::similaritySearch(const std::string& searchConditions, SimilaritySearchCallback callback)
{
    if (searchConditions.empty()) {
        callback(std::string(), DATA_MANAGEMENT_PARAM_ERROR);
        return;
    }
    cpr::async([searchConditions, callback, this]() {
        int errorCode = 0;

        std::string result = doSimilaritySearch(searchConditions, errorCode);
        callback(result, errorCode);
    });
}

void DataManagementService::addTextFiles(const std::string& fileinfos, AddTextFilesCallback callback)
{
    if (fileinfos.empty()) {
        callback(DATA_MANAGEMENT_PARAM_ERROR);
        return;
    }
    cpr::async([fileinfos, callback, this]() {
        int errorCode = 0;

        doAddTextFiles(fileinfos, errorCode);
        callback(errorCode);
    });
}

void DataManagementService::addImageFiles(const std::string& fileinfos, AddImageFilesCallback callback)
{
    if (fileinfos.empty()) {
        callback(DATA_MANAGEMENT_PARAM_ERROR);
        return;
    }

    cpr::async([fileinfos, callback, this]() {
        int errorCode = 0;

        doAddImageFiles(fileinfos, errorCode);
        callback(errorCode);
    });
}

void DataManagementService::deleteFiles(const std::string& fileinfos, DeleteFilesCallback callback)
{
    if (fileinfos.empty()) {
        callback(DATA_MANAGEMENT_PARAM_ERROR);
        return;
    }

    cpr::async([fileinfos, callback, this]() {
        int errorCode = 0;
        doDeleteFiles(fileinfos, errorCode);
        callback(errorCode);
    });
}

void DataManagementService::updateFilesName(const std::string& fileinfos, UpdateFilesNameCallback callback)
{

}

void DataManagementService::updateFilesContent(const std::string& fileinfos, UpdateFilesContentCallback callback)
{
    if (fileinfos.empty()) {
        callback(DATA_MANAGEMENT_PARAM_ERROR);
        return;
    }

    cpr::async([fileinfos, callback, this]() {
        int errorCode = 0;
        doUpdateFilesContent(fileinfos, errorCode);
        callback(errorCode);
    });
}

void DataManagementService::getAllFileInfos(GetAllFileinfosCallback callback)
{
    cpr::async([callback, this]() {
        int errorCode = 0;
        std::string result = doGetAllFileInfos(errorCode);
        callback(result, errorCode);
    });
}

SimilaritySearchResult DataManagementService::mergeSearchResults(
    const SimilaritySearchResult& visionSearchResult, const SimilaritySearchResult& textSearchResult) const {

    // 合并两个搜索结果列表, 数据库返回的搜索结果是排好序的不需要重排
    std::vector<std::pair<std::string, double>> mergedResult;
    mergedResult.insert(mergedResult.end(), visionSearchResult.begin(), visionSearchResult.end());
    mergedResult.insert(mergedResult.end(), textSearchResult.begin(), textSearchResult.end());

    return mergedResult;
}

void DataManagementService::filtSearchResults(SimilaritySearchResult &searchResult, double threshold) const
{
    // 丢弃文件相似度小于给定阈值的结果
    searchResult.erase(
        std::remove_if(searchResult.begin(), searchResult.end(),
                       [threshold](const std::pair<std::string, double>& result) {
                           return result.second < threshold;
                       }),
        searchResult.end());
}

SimilaritySearchResult DataManagementService::deduplicatedSearchResults(const SimilaritySearchResult &textSearchResult) const
{
    std::unordered_map<std::string, double> maxValues;

    for (const auto& pair : textSearchResult) {
        auto it = maxValues.find(pair.first);
        if (it != maxValues.end()) {
            it->second = std::max(it->second, pair.second);
        } else {
            maxValues.insert(pair);
        }
    }

    std::vector<std::pair<std::string, double>> result;
    for (const auto& pair : maxValues) {
        result.push_back(pair);
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    return result;
}

FileInfo DataManagementService::parserTextFileInfo(
    const std::string &filepath, const std::string &format) const
{
    std::unique_ptr<Parser> parser = FileParserFactory::createParser(format);
    if (!parser) {
        std::cerr << "Can't create parser for " << format << std::endl;
        return FileInfo();
    }

    return parser->parse(filepath);
}

void DataManagementService::truncateTexts(std::vector<std::string> &texts, int maxLength) const
{
    if (texts.size() > maxLength) {
        int start = (texts.size() - maxLength) / 2;
        texts.erase(texts.begin(), texts.begin() + start); // 删除前面多余的部分
        texts.erase(texts.begin() + maxLength, texts.end()); // 删除后面多余的部分
    }
}

bool DataManagementService::isSupportedImageFormat(const std::string& format) const
{
    const std::vector<std::string> supportFormat = {"png", "jpg", "jpeg", "jpe", "bmp", "dib"};
    bool isSupported = std::find(supportFormat.begin(), supportFormat.end(), format) != supportFormat.end();

    return isSupported;
}

bool DataManagementService::isSupportedTextFormat(const std::string& format) const
{
    const std::vector<std::string> supportFormat = {"txt", "pdf", "docx", "pptx"};
    bool isSupported = std::find(supportFormat.begin(), supportFormat.end(), format) != supportFormat.end();

    return isSupported;
}

std::vector<DBFileInfo> DataManagementService::makeDatasByImageFilePathAndFormat(const std::vector<std::pair<std::string, std::string>> &fileinfos)
{
    std::vector<DBFileInfo> infos;
    for (const std::pair<std::string, std::string>& fileinfo : fileinfos) {
        // 跳过不支持格式的文件
        std::string filepath = fileinfo.first;
        std::string fileformat = fileinfo.second;
        std::cout << "Start handle file: " << filepath << " format: " << fileformat << std::endl;

        if (!isSupportedImageFormat(fileformat)) {
            std::cerr << "Format: " << fileformat << " not support" << std::endl;
            continue;
        }
        if (!std::filesystem::exists(filepath)) {
            std::cerr << "File: " << filepath << " not exit" << std::endl;
            continue;
        }

        std::vector<float> vector = imageSideEmbeddingImage(filepath);
        if (vector.empty()) {
            std::cerr << "File: " << filepath << " embedding error" << std::endl;
            continue;
        }
        time_t modifyTime = Parser::modifyTime(filepath);
        DBFileInfo info(filepath, modifyTime, {vector});
        infos.emplace_back(std::move(info));
    }
    return infos;
}

std::vector<DBFileInfo> DataManagementService::makeDatasByTextFilePathAndFormat(const std::vector<std::pair<std::string, std::string> > &fileinfos)
{
    std::vector<DBFileInfo> infos;
    for (const auto& fileinfo : fileinfos) {
        std::string path = fileinfo.first;
        std::string format = fileinfo.second;
        std::cout << "Start handle file: " << path << " format: " << format << std::endl;

        if (!isSupportedTextFormat(format)) {
            std::cerr << "Format: " << format << " not support" << std::endl;
            continue;
        }
        if (!std::filesystem::exists(path)) {
            std::cerr << "File: " << path << " not exit" << std::endl;
            continue;
        }
        FileInfo file = parserTextFileInfo(path, format);
        if (file.contents.empty()) {
            std::cerr << "Parser file " << path << " to text failed." << std::endl;
            continue;
        }

        SegmentTokenizer::segmentTokenize(file.contents, 100);
        std::cout << "start encode text file " << path << std::endl;
        std::vector<std::vector<float>> vectors = textSideEmbeddingTexts(file.contents);
        std::cout << "finish encode text file " << path << std::endl;

        if (vectors.empty()) {
            std::cerr << "File: " << path << " embedding error" << std::endl;
            continue;
        }
        DBFileInfo info(path, file.modifyTime, vectors);
        infos.emplace_back(std::move(info));
    }

    return infos;
}

std::string DataManagementService::doSimilaritySearch(
    const std::string &searchConditions, int &errorCode)
{
    errorCode = 0;
    std::pair<std::string, double> condition = DataManagementJsonHelper::parserSearchConditions(searchConditions);
    if (condition.first.empty()) {
        errorCode = DATA_MANAGEMENT_PARAM_ERROR;
        return std::string();
    }

    std::vector<float> visionSideEmbedding = imageSideEmbeddingText(condition.first);
    std::vector<float> textSideEmbedding = textSideEmbeddingText(condition.first);
    if (visionSideEmbedding.empty() && textSideEmbedding.empty()) {
        errorCode = DATA_MANAGEMENT_INVAILD_PATH;
        return std::string();
    }

    if (!database_.createClient()) {
        errorCode = DATA_MANAGEMENT_CANT_OPEN_DB;
        return std::string();
    }
    auto visionSearchResult = database_.visionSearch(visionSideEmbedding);
    auto textSearchResult = database_.textSearch(textSideEmbedding);
    database_.destroyClient();

    if (visionSearchResult.empty() && textSearchResult.empty()) {
        errorCode = DATA_MANAGEMENT_CANT_OPEN_DB;
        return std::string();
    }

    textSearchResult = deduplicatedSearchResults(std::move(textSearchResult));

    filtSearchResults(textSearchResult, TEXT_SEARCH_THRESHOLD);
    filtSearchResults(visionSearchResult, VISION_SEARCH_THRESHOLD);

    std::vector<std::pair<std::string, double>> mergedResult =
        mergeSearchResults(std::move(visionSearchResult), std::move(textSearchResult));

    std::string result = DataManagementJsonHelper::convertSearchResultToJson(mergedResult);

    return result;
}

void DataManagementService::doAddImageFiles(
    const std::string &fileinfosJson, int &errorCode)
{
    auto fileinfos = DataManagementJsonHelper::parserFilePathAndFormat(fileinfosJson);
    if (fileinfos.empty()) {
        errorCode = DATA_MANAGEMENT_PARAM_ERROR;
        std::cerr << "Parser add image files input falied" << std::endl;
        return;
    }

    std::vector<DBFileInfo> infos = makeDatasByImageFilePathAndFormat(fileinfos);

    if (infos.empty()) {
        std::cerr << "Get file info failed" << std::endl;
        errorCode = DATA_MANAGEMENT_UNKNOWN_ERROR;
        return;
    }

    if (!database_.createClient()) {
        errorCode = DATA_MANAGEMENT_CANT_OPEN_DB;
        return;
    }
    database_.addImageDatas(infos);
    database_.destroyClient();
}

void DataManagementService::doAddTextFiles(
    const std::string &fileinfosJson, int &errorCode)
{
    auto fileinfos = DataManagementJsonHelper::parserFilePathAndFormat(fileinfosJson);
    if (fileinfos.empty()) {
        errorCode = DATA_MANAGEMENT_PARAM_ERROR;
        std::cerr << "Parser add text files input falied" << std::endl;
        return;
    }

    std::vector<DBFileInfo> infos = makeDatasByTextFilePathAndFormat(fileinfos);
    if (infos.empty()) {
        std::cerr << "Can't get any file info, ignore request" << std::endl;
        errorCode = DATA_MANAGEMENT_UNKNOWN_ERROR;
        return;
    }

    if (!database_.createClient()) {
        errorCode = DATA_MANAGEMENT_CANT_OPEN_DB;
        return;
    }
    database_.addTextDatas(infos);
    database_.destroyClient();
}

std::string DataManagementService::doGetAllFileInfos(int &errorCode)
{
    if (!database_.createClient()) {
        errorCode = DATA_MANAGEMENT_CANT_OPEN_DB;
        return std::string();
    }

    GetAllFileInfosResult result = database_.getAllFileInfos();
    return DataManagementJsonHelper::convertFileInfosResultToJson(result);
}

void DataManagementService::doDeleteFiles(const std::string &fileinfosJson, int &errorCode)
{
    std::vector<std::string> files = DataManagementJsonHelper::parserDeletedFileInfos(fileinfosJson);
    if (files.empty()) {
        errorCode = DATA_MANAGEMENT_PARAM_ERROR;
        return;
    }

    if (!database_.createClient()) {
        errorCode = DATA_MANAGEMENT_CANT_OPEN_DB;
        return;
    }
    database_.deleteFiles(files);

    database_.destroyClient();
}

void DataManagementService::doUpdateFilesContent(const std::string &fileinfosJson, int &errorCode)
{
    auto fileinfos = DataManagementJsonHelper::parserFilePathAndFormat(fileinfosJson);
    if (fileinfos.empty()) {
        errorCode = DATA_MANAGEMENT_PARAM_ERROR;
        std::cerr << "Parser updata files content input falied" << std::endl;
        return;
    }

    std::vector<std::string> files;
    std::vector<std::pair<std::string, std::string>> imageFiles;
    std::vector<std::pair<std::string, std::string>> textFiles;

    for (auto begin = fileinfos.begin(); begin != fileinfos.end();) {
        files.emplace_back(begin->first);
        if (isSupportedTextFormat(begin->second)) {
            textFiles.emplace_back(*begin);
        } else if (isSupportedImageFormat(begin->second)) {
            imageFiles.emplace_back(*begin);
        }
        begin = fileinfos.erase(begin);
    }

    auto imageDatas = makeDatasByImageFilePathAndFormat(std::move(imageFiles));
    auto textDatas = makeDatasByTextFilePathAndFormat(std::move(textFiles));

    if (!database_.createClient()) {
        errorCode = DATA_MANAGEMENT_CANT_OPEN_DB;
        return;
    }
    database_.deleteFiles(files);

    database_.addImageDatas(std::move(imageDatas));
    database_.addTextDatas(std::move(textDatas));

    database_.destroyClient();
}

std::vector<float> DataManagementService::textSideEmbeddingText(const std::string& text)
{
    std::vector<float> textEmbedding;
    EmbeddingTask task(text, EmbeddingTask::EmbeddingDataType::Text);
    EmbeddingTaskManager::getInstance()->addTextSideTask(task);
    textEmbedding = EmbeddingTaskManager::getInstance()->getTextSideResult(task.id());
    return textEmbedding;
}

std::vector<std::vector<float>> DataManagementService::textSideEmbeddingTexts(const std::vector<std::string>& texts)
{
    std::vector<std::vector<float>> textEmbeddings;
    for (int i = 0; i < texts.size(); i++) {
        if(texts.empty()) {
            continue;
        }
        EmbeddingTask task(texts.at(i), EmbeddingTask::EmbeddingDataType::Text);
        EmbeddingTaskManager::getInstance()->addTextSideTask(task);
        std::vector<float> textEmbedding = EmbeddingTaskManager::getInstance()->getTextSideResult(task.id());
        if(!textEmbedding.empty()) {
            textEmbeddings.push_back(textEmbedding);
        }
    }

    return textEmbeddings;
}

std::vector<float> DataManagementService::imageSideEmbeddingText(const std::string& text)
{
    std::vector<float> textEmbedding;
    EmbeddingTask task(text, EmbeddingTask::EmbeddingDataType::Text);
    EmbeddingTaskManager::getInstance()->addImageSideTask(task);
    textEmbedding = EmbeddingTaskManager::getInstance()->getImageSideResult(task.id());
    return textEmbedding;
}

std::vector<float> DataManagementService::imageSideEmbeddingImage(const std::string& filePath)
{
    std::vector<float> imageEmbedding;
    EmbeddingTask task(filePath, EmbeddingTask::EmbeddingDataType::FilePath);
    EmbeddingTaskManager::getInstance()->addImageSideTask(task);
    imageEmbedding = EmbeddingTaskManager::getInstance()->getImageSideResult(task.id());
    return imageEmbedding;
}

#include "datamanagementdatabase.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <random>

static const char* VECTOR_DATABASE_NAME = "search";
static const char* FILE_INFO_COLLECTION_NAME = "files-info";
static const char* VISION_FILE_CONTENT_COLLECTION_NAME = "vision-files-content-vector";
static const char* TEXT_FILE_CONTENT_COLLECTION_NAME = "text-files-content-vector";

namespace {
std::string getVectorDatabasePath() {
    std::string home_env = std::getenv("HOME") ? std::string(std::getenv("HOME")) : "";
    std::string vector_database_path = home_env + "/.local/kylin-ai-business-framework/datamanagement/database/";
    return vector_database_path;
}

std::string generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < 32; ++i) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            ss << "-";
        }
        ss << dis(gen);
    }

    return ss.str();
}

std::vector<double> convertFloatToDouble(const std::vector<float>& floatVec) {
    std::vector<double> doubleVec;
    doubleVec.reserve(floatVec.size()); // 预留空间以避免不必要的重新分配

    for (float floatValue : floatVec) {
        doubleVec.push_back(static_cast<double>(floatValue));
    }

    return doubleVec;
}

}

DataManagementDatabase::~DataManagementDatabase()
{
    if (connected_)
        database_.destroyClient();
}

bool DataManagementDatabase::createClient(uid_t uid)
{
    std::string databasePath = getVectorDatabasePath() + "/" + VECTOR_DATABASE_NAME;

    if (database_.createClient(databasePath) != VectorDBErrorCode::Success) {
        return false;
    }

    connected_ = true;
    return true;
}

void DataManagementDatabase::destroyClient()
{
    connected_ = false;
    database_.destroyClient();
}

SimilaritySearchResult DataManagementDatabase::visionSearch(const std::vector<float> &vector)
{
    return similaritySearch(VISION_FILE_CONTENT_COLLECTION_NAME, vector);
}

SimilaritySearchResult DataManagementDatabase::textSearch(const std::vector<float> &vector)
{
    return similaritySearch(TEXT_FILE_CONTENT_COLLECTION_NAME, vector);
}

SimilaritySearchResult DataManagementDatabase::similaritySearch(
    const std::string &collection, const std::vector<float>& vector)
{
    if (vector.empty() || collection.empty() || !connected_) {
        return std::vector<std::pair<std::string, double>>();
    }

    std::vector<std::string> collections = {FILE_INFO_COLLECTION_NAME};
    collections.push_back(collection);

    if (!hasCollections(collections)) {
        return std::vector<std::pair<std::string, double>>();
    }
    std::vector<std::vector<double>> vectors;
    vectors.emplace_back(convertFloatToDouble(vector));

    auto searchResults = database_.queryData(collection, vectors, 50);

    std::vector<std::pair<std::string, double>> result;
    // 通过相似度搜到得到的sourceId，在另一个collection中查找对应的文件
    for (const std::pair<VectorData, float>& searchResult : searchResults) {
        VectorData data = searchResult.first;
        double similarity = 1 - searchResult.second; // 目前向量库返回的是距离，我们要转换成相似度
        std::string sourceId = std::get<std::string>(data.metadata["sourceId"]);
        std::vector<std::string> ids = { sourceId };

        auto filedatas = database_.getData(FILE_INFO_COLLECTION_NAME, ids);
        if (filedatas.empty())
            continue;
        std::string filepath = std::get<std::string>(filedatas[0].metadata["source"]);

        result.emplace_back(std::move(filepath), std::move(similarity));
    }

    return result;
}

void DataManagementDatabase::addImageDatas(const std::vector<DBFileInfo> &fileinfos)
{
    addDatas(fileinfos, VISION_FILE_CONTENT_COLLECTION_NAME);
}

void DataManagementDatabase::addTextDatas(const std::vector<DBFileInfo> &fileinfos)
{
    addDatas(fileinfos, TEXT_FILE_CONTENT_COLLECTION_NAME);
}

void DataManagementDatabase::addDatas(const std::vector<DBFileInfo> &fileinfos, const std::string& collection)
{
    if (!connected_) {
        std::cerr << "Please create client first" << std::endl;
        return;
    }

    std::vector<VectorData> fileDatas;
    std::vector<VectorData> vectorDatas;

    for (const auto& fileinfo : fileinfos) {
        if (fileinfo.empty()) {
            std::cerr << "Add file " << fileinfo.filepath << " to database failed,"
                      << "file vector is empty" << std::endl;
            continue;
        }

        VectorData fileData = getFileDataFromFileInfo(fileinfo);
        std::vector<VectorData> vectorData = getVectorDatasFromFileInfo(fileinfo, fileData.id);
        std::cout << "Add file: " << fileinfo.filepath
                  << " file id: " << fileData.id << std::endl;

        fileDatas.emplace_back(std::move(fileData));
        vectorDatas.insert(vectorDatas.end(),
                           std::make_move_iterator(vectorData.begin()),
                           std::make_move_iterator(vectorData.end()));
    }

    if (fileDatas.empty() || vectorDatas.empty()) {
        return;
    }

    const std::vector<std::string> collections =
        {FILE_INFO_COLLECTION_NAME, collection};
    checkAndCreateCollections(collections);

    // 分别向两个collection添加数据，若有一方添加失败则撤回操作
    auto filesResult = database_.addData(FILE_INFO_COLLECTION_NAME, fileDatas);
    if (filesResult != VectorDBErrorCode::Success) {
        std::cerr << "Add file data failed." << std::endl;
        return;
    }
    auto vectorResult = database_.addData(collection, vectorDatas);
    if (vectorResult != VectorDBErrorCode::Success) {
        std::cerr << "Add vector data failed." << std::endl;
        std::vector<std::string> ids;
        for (const auto& data : fileDatas) {
            ids.push_back(data.id);
        };
        database_.deleteData(FILE_INFO_COLLECTION_NAME, {}, ids);
    }
}

GetAllFileInfosResult DataManagementDatabase::getAllFileInfos()
{
    GetAllFileInfosResult result;
    if (!connected_) {
        std::cerr << "Please create client first" << std::endl;
        return result;
    }
    if (!hasCollections({FILE_INFO_COLLECTION_NAME})) {
        std::cerr << "Don't have " << FILE_INFO_COLLECTION_NAME << " collection, "
                  << "can't get file infos" << std::endl;
        return result;
    }

    std::vector<VectorData> datas = database_.getData(FILE_INFO_COLLECTION_NAME);
    for (auto& data : datas) {
        result.emplace_back(std::move(std::get<std::string>(data.metadata["source"])),
                            std::move(std::get<int64_t>(data.metadata["modifytime"])));
    }

    return result;
}

void DataManagementDatabase::deleteFiles(const std::vector<std::string> &files)
{
    if (!connected_) {
        std::cerr << "Please create client first" << std::endl;
        return;
    }
    if (!hasCollections({FILE_INFO_COLLECTION_NAME})) {
        std::cerr << "Don't have " << FILE_INFO_COLLECTION_NAME << " collection, "
                  << "can't get file infos" << std::endl;
        return;
    }

    for (const auto& file : files) {
        std::cout << "Delete file: " << file << std::endl;
    }

    std::vector<std::string> deletedSourceIds;
    for (const auto& file : files) {
        std::vector<VectorData> res = database_.getData(FILE_INFO_COLLECTION_NAME, {}, file);
        if (res.empty()) {
            std::cerr << "Get file: " << file << " data, failed." << std::endl;
            continue;
        }
        for (const auto& filedata : res) {
            deletedSourceIds.emplace_back(filedata.id);
        }
    }
    if (deletedSourceIds.empty()) {
        std::cerr << "Can't get any data from files, cancel delete." << std::endl;
        return;
    }

    database_.deleteData(FILE_INFO_COLLECTION_NAME, {}, deletedSourceIds);
    database_.deleteData(VISION_FILE_CONTENT_COLLECTION_NAME, deletedSourceIds);
    database_.deleteData(TEXT_FILE_CONTENT_COLLECTION_NAME, deletedSourceIds);
}

bool DataManagementDatabase::hasCollections(const std::vector<std::string>& collections)
{
    if (collections.empty())
        return false;

    std::vector<std::string> collectionList = database_.collections();

    bool hasCollections = std::all_of(collections.begin(), collections.end(),
                                      [&collectionList](const std::string& str) {
        auto it = std::find(collectionList.begin(), collectionList.end(), str);
        bool hasCollection = it != collectionList.end();
        return hasCollection;
    });

    return hasCollections;
}

void DataManagementDatabase::checkAndCreateCollections(const std::vector<std::string> &collections)
{
    std::vector<std::string> collectionList = database_.collections();
    for (const auto& collection : collections) {
        auto it = std::find(collectionList.begin(), collectionList.end(), collection);
        if (it == collectionList.end()) {
            database_.createCollection(collection, "cosine");
        }
    }
}

VectorData DataManagementDatabase::getFileDataFromFileInfo(const DBFileInfo &fileinfo) const
{
    VectorData fileData;
    fileData.id = generateUUID();
    fileData.metadata.insert({"source", std::string(fileinfo.filepath)});
    fileData.metadata.insert({"modifytime", int(fileinfo.modifyTime)});

    return fileData;
}

std::vector<VectorData> DataManagementDatabase::getVectorDatasFromFileInfo(const DBFileInfo &fileinfo, const std::string& sourceId) const
{
    std::vector<VectorData> datas;
    for (auto const& vector : fileinfo.vectors) {
        VectorData vectorData;
        vectorData.id = generateUUID();
        vectorData.embedding = convertFloatToDouble(vector);
        vectorData.metadata.insert({"sourceId", sourceId});
        datas.emplace_back(std::move(vectorData));
    }

    return datas;
}

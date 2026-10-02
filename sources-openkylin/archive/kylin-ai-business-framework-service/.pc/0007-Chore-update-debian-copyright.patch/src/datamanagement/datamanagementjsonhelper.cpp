#include "datamanagementjsonhelper.h"

#include <jsoncpp/json/json.h>

#include <iostream>
#include <sstream>
#include <memory>

static double DEFAULT_SIMILARITY_THRESHOLD = 0.3;

std::pair<std::string, double> DataManagementJsonHelper::parserSearchConditions(const std::string &searchConditions)
{
    std::pair<std::string, double> result;

    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    Json::Value root;
    std::string errors;

    bool parsingSuccessful = reader->parse(
        searchConditions.c_str(), searchConditions.c_str() + searchConditions.size(),
        &root, &errors);
    delete reader;

    if (!parsingSuccessful) {
        std::cerr << "Failed to parse JSON: " << errors << std::endl;
        return result;
    }

    if (root.isMember("text")) {
        result.first = root["text"].asString();
    }

    if (root.isMember("similarity-threshold")) {
        result.second = root["similarity-threshold"].asDouble();
    } else {
        result.second = DEFAULT_SIMILARITY_THRESHOLD;
    }

    return result;
}

std::string DataManagementJsonHelper::convertSearchResultToJson(
    const std::vector<std::pair<std::string, double>> &searchResult)
{
    Json::Value root;

    // 遍历结果列表并转换为 JSON 格式
    for (const auto& result : searchResult) {
        Json::Value item;
        item["filepath"] = result.first;
        item["similarity"] = result.second;

        root.append(item);
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::ostringstream oss;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(root, &oss);

    std::string jsonString = oss.str();

    return jsonString;
}

std::vector<std::pair<std::string, std::string>> DataManagementJsonHelper::parserFilePathAndFormat(const std::string &fileinfos)
{
    std::vector<std::pair<std::string, std::string>> result;

    // 解析 JSON 字符串
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING err;
    std::istringstream jsonStream(fileinfos);

    if (!Json::parseFromStream(builder, jsonStream, &root, &err)) {
        std::cerr << "Error parsing JSON: " << err << std::endl;
        return result;
    }

    if (root.isArray()) {
        for (const auto& item : root) {
            if (item.isObject() && item.isMember("filepath") && item.isMember("fileformat")) {
                std::string filepath = item["filepath"].asString();
                std::string fileformat = item["fileformat"].asString();
                result.emplace_back(filepath, fileformat);
            }
        }
    }

    return result;
}

std::vector<std::string> DataManagementJsonHelper::parserDeletedFileInfos(const std::string &fileinfos)
{
    std::vector<std::string> result;
    // 解析 JSON 字符串
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING err;
    std::istringstream jsonStream(fileinfos);

    if (!Json::parseFromStream(builder, jsonStream, &root, &err)) {
        std::cerr << "Error parsing JSON: " << err << std::endl;
        return result;
    }

    if (!root.isArray()) {
        std::cerr << "Delete file input json " << fileinfos
                  << " is not array, can't parser" << std::endl;
        return result;
    }

    for (const auto& item : root) {
        if (item.isObject() && item.isMember("filepath")) {
            std::string filepath = item["filepath"].asString();
            result.emplace_back(std::move(filepath));
        }
    }

    return result;
}

std::string DataManagementJsonHelper::convertFileInfosResultToJson(const std::vector<std::pair<std::string, int>> &fileinfos)
{
    Json::Value root(Json::arrayValue); // 直接初始化一个数组类型的根节点

    for (const auto &info : fileinfos) {
        Json::Value file;
        file["filepath"] = info.first;
        file["timestamp"] = info.second;
        root.append(file); // 直接向根节点添加文件信息对象
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string jsonString = Json::writeString(builder, root);

    return jsonString;
}

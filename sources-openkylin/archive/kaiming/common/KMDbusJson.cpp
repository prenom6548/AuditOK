/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMDbusJson.h"

/**
 * @brief 将包信息序列化成Json字符串
 *
 * @param name 包名，如 "org.kde.kclock"
 * @param version 包版本，如 "23.04.3"
 * @param size 包大小，如 12363264 字节，开明命令对应 "11.8 MB"
 * @param branch 分支名，如 "stable"
 * @return std::string
 */
std::string KMDbusJson::packLocalInfoToJson(const std::string &id, const std::string &name, const std::string &version, const std::string &size, const std::string &branch, 
                                                                    const std::string &arch, const std::string &module, const std::string &description)
{
    nlohmann::json infoJson;
    infoJson["id"] = id;
    infoJson["name"] = name;
    infoJson["version"] = version;
    infoJson["size"] = size;
    infoJson["branch"] = branch;
    infoJson["arch"] = arch;
    infoJson["module"] = module;
    infoJson["description"] = description;

    return infoJson.dump();
}

std::string KMDbusJson::packProgressInfoToJson(const std::string &refId, int progress, const std::string &speed, int timeout)
{
    nlohmann::json progressInfo;
    progressInfo["refId"] = refId;
    progressInfo["progress"] = progress;
    progressInfo["speed"] = speed;
    progressInfo["timeout"] = timeout;

    return progressInfo.dump();
}

std::string KMDbusJson::packProgressOffInfoToJson(const std::string &refId, int progress)
{
    nlohmann::json progressInfo;
    progressInfo["refId"] = refId;
    progressInfo["progress"] = progress;

    return progressInfo.dump();
}

std::string KMDbusJson::errorLocalJson()
{
    nlohmann::json infoJson;
    infoJson["id"] = "";
    infoJson["name"] = "";
    infoJson["version"] = "";
    infoJson["size"] = "";
    infoJson["branch"] = "";
    infoJson["arch"] = "";
    infoJson["module"] = "";
    infoJson["description"] = "";

    return infoJson.dump();
}

std::string KMDbusJson::parseRemotePathToJson(std::string& stringPath) 
{
    // 分割路径
    std::vector<std::string> pathParts;
    pathParts.reserve(8);

    std::string::size_type pos = 0;
    while ((pos = stringPath.find('/')) != std::string::npos) {
        std::string token = stringPath.substr(0, pos);
        if (!token.empty()) {
            pathParts.push_back(std::move(token));
        }
        stringPath.erase(0, pos + 1);
    }

    if (!stringPath.empty()) {
        pathParts.push_back(std::move(stringPath));
    }

    if (pathParts.size() >= 10) 
    {
        return "{\"branch\":\"" + pathParts[0] + 
               "\",\"id\":\"" + pathParts[3] + 
               "\",\"version\":\"" + pathParts[5] + 
               "\",\"size\":\"" + pathParts[7] + 
               "\",\"debinfo\":{\"package\":\"" + pathParts[8] + 
               "\",\"version\":\"" + pathParts[9] + "\"}}";
    }
    else if (pathParts.size() >= 8) 
    {
        return "{\"branch\":\"" + pathParts[0] + 
               "\",\"id\":\"" + pathParts[3] + 
               "\",\"version\":\"" + pathParts[5] + 
               "\",\"size\":\"" + pathParts[7] + 
               "\",\"debinfo\":{\"package\":\"" + "" + 
               "\",\"version\":\"" + "" + "\"}}";
    }


    return "{}";
}

std::string KMDbusJson::errorRemoteJson()
{
    nlohmann::json infoJson;
    infoJson["branch"] = "";
    infoJson["id"] = "";
    infoJson["version"] = "";
    infoJson["size"] = "";
    
    // 添加 debinfo 对象
    nlohmann::json debinfoJson;
    debinfoJson["package"] = "";
    debinfoJson["version"] = "";
    infoJson["debinfo"] = debinfoJson;

    return infoJson.dump();
}

std::string KMDbusJson::packRemoteStatusToJson(const std::string &updateTime, bool signStatus, const std::string &signCN)
{
    nlohmann::json infoJson;
    infoJson["updateTime"] = updateTime;
    infoJson["signStatus"] = signStatus;
    infoJson["signCN"] = signCN;

    return infoJson.dump();
}
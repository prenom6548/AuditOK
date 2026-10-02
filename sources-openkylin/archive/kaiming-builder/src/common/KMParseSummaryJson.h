/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMPARSESUMMARYJSON_H__
#define __KMPARSESUMMARYJSON_H__

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <nlohmann/json.hpp>

#include "../KMBuilderUtils.h"
#include "KMRef.h"

using json = nlohmann::json;
using namespace std;

namespace fs = std::filesystem;

struct VersionInfo
{
    uint64_t installedSize = 0;         // 安装大小
    string base = "";                   // 依赖的 base
    string runtime = "";                // 依赖的 runtime
    vector<string> depend;
    vector<string> extensions = { "" }; // 依赖的 extensions 列表
};

struct ModuleInfo
{
    map<string, string> nameInfo;        // name, name[zh_CN]
    map<string, string> descriptionInfo; // description, description[zh_CN]
    map<string, VersionInfo> versionInfo;  // 0.0.0.1, 0.0.0.2
};

struct IdInfo
{
    map<string, ModuleInfo> moduleInfo;  // binary/debuginfo/locale_zh_cn
};

struct KindInfo
{
    map<string, IdInfo> idInfo; // top.openkylin.Music, top.openkylin.Video
};

struct ArchInfo
{
    map<string, KindInfo> kindInfo; // app/runtime/base/extension
};
struct ChannelInfo
{ 
    map<string, ArchInfo> archInfo; // x86_64/arm64/loongarch
};

struct SummaryInfo
{
    map<string, ChannelInfo> channelInfo; // main/stable
};

struct searchRefInfo
{
    uint64_t installedSize = 0;         // 安装大小
    map<string, string> nameInfo;        // name, name[zh_CN]
    map<string, string> descriptionInfo; // description, description[zh_CN]
};

/**
 * @brief 解析 summary.json 依赖文件
 * 
 */
class KMParseSummaryJson
{
public:
    KMParseSummaryJson();
    KMParseSummaryJson(const std::string& filePath);
    ~KMParseSummaryJson();

    /**
     * @brief 函数用于移除 JSON 文件中的注释
     * 
     * @param jsonString 
     * @return std::string 
     */
    std::string removeComments(const std::string& jsonString);

    /**
     * @brief 加载 json 文件
     * 
     * @param filePath json 文件路径
     * @return true 加载成功
     * @return false 加载失败
     */
    bool loadFile(const std::string &filePath);
    void parseSummaryJson();

    std::vector<KMRef> search(const std::string &specifyArch, const std::string &specifyKind, const std::string &specifyModule, const std::string &searchId , const std::string &specifyVersion, const std::string &base, const std::string &runtime);
    bool regexVersion(const std::string &version, bool strict = true);

private:
    void from_json(const json& j, VersionInfo& version);
    void from_json(const json& j, ModuleInfo& module);
    void from_json(const json& j, IdInfo& id);
    void from_json(const json& j, KindInfo& kind);
    void from_json(const json& j, ArchInfo& arch);
    void from_json(const json& j, ChannelInfo& channel);
    void from_json(const json& j, SummaryInfo& summary);

private:
    class Private;
    shared_ptr<Private> d;

};

#endif // KMPARSESUMMARYJSON_H
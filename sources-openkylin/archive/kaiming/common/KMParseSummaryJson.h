/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMPARSESUMMARYJSON_H
#define KMPARSESUMMARYJSON_H

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <nlohmann/json.hpp>
#include <dbus/dbus.h>

#include "common/KMError.h"
#include "common/KMUtil.h"
#include "common/KMStringUtils.h"

using json = nlohmann::json;
using namespace std;

namespace fs = std::filesystem;

struct OnlineDebInfo
{
    std::string package = "";         // 软件包唯一名称
    std::string version = "";  
    std::string section = ""; 
    std::string package_type = "";
    std::string recommends = "";
    std::string uploaders = "";
    std::string provides = "";
    std::string origin = "";
    std::string multi_arch = "";
    std::string build_depends = "";
    std::string source = "";
    std::string suggests = "";
    std::string conflicts = "";
    std::string breaks = "";
    std::string replaces = "";
    std::string vcs_git = "";
    std::string vcs_browser = "";
};

struct VersionInfo
{
    uint64_t installedSize = 0; // 安装大小
    string base = "";           // 依赖的 base
    string runtime = "";        // 依赖的 runtime
    vector<string> depend;
    vector<string> extensions = {""}; // 依赖的 extensions 列表
    map<string, OnlineDebInfo> debInfo; // kylin.debinfo
};

struct ModuleInfo
{
    map<string, string> nameInfo;         // name, name[zh_CN]
    map<string, string> descriptionInfo;  // description, description[zh_CN]
    map<string, VersionInfo> versionInfo; // 0.0.0.1, 0.0.0.2
};

struct IdInfo
{
    map<string, ModuleInfo> moduleInfo; // binary/debuginfo/locale_zh_cn
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
    uint64_t installedSize = 0;          // 安装大小
    map<string, string> nameInfo;        // name, name[zh_CN]
    map<string, string> descriptionInfo; // description, description[zh_CN]
};

struct CacheInfo
{
    std::string filePath;
    time_t lastModified;
    bool isValid;
    SummaryInfo summaryInfo;
};

/**
 * @brief 解析 summary.json 依赖文件
 * 
 */
class KMParseSummaryJson
{
public:
    KMParseSummaryJson();
    KMParseSummaryJson(const std::string &filePath);
    ~KMParseSummaryJson();

    /**
     * @brief 函数用于移除 JSON 文件中的注释
     * 
     * @param jsonString 
     * @return std::string 
     */
    std::string removeComments(const std::string &jsonString);

    /**
     * @brief 加载 json 文件
     * 
     * @param filePath json 文件路径
     * @return KM_ERROR_NO 加载成功
     * @return false 加载失败
     */
    KMErrorCodeEnum loadFile(const std::string &filePath, bool forceReload = false);

    /**
     * @description: 生成id和分支的映射关系
     * @return {*}
     */
    void buildIdArchCache();
    /**
     * @brief 解析 summary.json 依赖文件
     * 
     */
    bool parseSummaryJson();

    void gernerateKMPackage();

    /**
     * @brief 输出整个 Json 数据
     * 
     */
    void printJson();

    /**
     * @brief 将序列化的 SummaryInfo 结构体数据进行打印
     * 
     * @param data 序列化的数据
     */
    void printSummaryJson(const SummaryInfo &data);

    /**
     * @brief 将 UTF-8 字符串转换为小写
     * 
     * @param str 输入的 UTF-8 字符串
     * @return std::string 转换后的小写 UTF-8 字符串
     */
    string toLowerUtf8(const std::string &str);
    /**
     * @brief 将序列化的 SummaryInfo 结构体数据进行打印
     * 
     * @param id 程序id
     * 
     * @param specifyArch 指定架构
     */
    std::vector<std::string> printSearchJson(const string &searchId, const string &specifyArch, const string &specifyKind, const string &specifyModule, bool isPrint, bool isGetDevel);

    /**
     * @brief 获取summary信息并更新给定的 KMPMRef
     * 
     * @param pmRef 需要更新信息的 KMPMRef
     * @return true 匹配到 ref
     * @return false 未匹配到 ref
     */
    bool getSummaryInfo(KMPMRef &pmRef);

    const SummaryInfo& getSummaryInfo() const;

    /**
     * @brief 获取并更新 KMPMRef 中的 name 信息
     * 
     * @param pmRef 需要更新信息的 KMPMRef
     * @param moduleInfo 遍历的module模块
     * @return true 匹配到 ref
     * @return false 未匹配到 ref
     */
    bool getSummaryNameInfo(KMPMRef &pmRef, ModuleInfo moduleInfo);

    /**
     * @brief 根据命令行，模糊匹配全部的字符串
     * 
     * @param pmRef 通过命令行组装的 KMPMRef 结构体
     * @param fuzzyMatchRefs  每个字符串都是 channel/arch/kind/id//version/module 格式
     * @return true 匹配到 ref
     * @return false 未匹配到 ref
     */
    bool fuzzyMatchCompletedRef(KMPMRef pmRef, vector<string> &fuzzyMatchRefs);

    /**
     * @brief 在 id 层往下匹配全部的字符串并拼接到fuzzyMatchRefs中
     * 
     * @param pmRef ref 结构体
     * @param moduleKey module 的key，如 "binary", "devel", "debuginfo"
     * @param moduleData module 的值
     * @param fuzzyMatchRefs id 层往下匹配全部的字符串拼接
     * 
     * @return true 匹配成功
     * @return false 未匹配成功
     */
    bool matchVersionData(KMPMRef pmRef, string moduleKey, ModuleInfo moduleData, string searchRef, vector<string> &fuzzyMatchRefs, const std::string &remoateName);

    /**
     * @brief 当前 ref 是否安装
     * 
     * @param pmRef 
     * @return true 
     * @return false 
     */
    bool currentFuzzyRefIsInstalled(KMPMRef &pmRef);

    /**
     * @brief 获取匹配到的最大的版本号
     * 
     * 一、获取所有符合条件的版本号
     * 1. 指定版本号，版本号需要分两种情况处理：
     *      1.1 指定版本号全，匹配到则符合
     *      1.2 指定版本号不全，进行匹配前面部分，匹配到则符合
     * 2. 不指定版本号，
     * 
     * 二、在符合条件的版本号中找版本号的最大值
     * 
     * @param pmRef ref 结构体
     * @param moduleKey module 的key，如 "binary", "devel", "debuginfo"
     * @param moduleData module 的值
     * @param maxVersionStr 返回的最大版本号
     * @return true 获取匹配到的最大的版本号成功
     * @return false 获取匹配到的最大的版本号失败
     */
    bool findMatchMaxVesion(KMPMRef pmRef, string moduleKey, ModuleInfo moduleData, string &maxVersionStr);

    /**
     * @brief 将summary.json中所有id名生成packages文件，用于kaiming install 命令行补全脚本实现
     * 
     * @param data 序列化的数据
     */
    void getPackagesFromSummary();

    /**
     * @brief 安转或卸载管控
     * 
     * @param interfaceName 接口名
     * @param methordName 方法名
     * @param package 包名
     * 
     * @return true 获取匹配到的最大的版本号成功
     */
    int getDbus(const char *interfaceName, const char *methordName, const char *package);

    /**
     * @brief: 管控不可安装的包
     * @return 可安装的包
     */
    std::string getNotInstallable();

    /**
     * @brief: summary未找到的包
     * @return summary未找到的包
     */
    vector<std::string> getNotFoundAppIds();

    /**
     * @brief:清空不可安装队列
     */
    bool clearNotInstallable();
    void clearCache();

    std::map<std::string, std::vector<std::string>> getRepoToRefs() const;
    void clearRepoToRefs();
private:
    void from_json(const json &j, OnlineDebInfo &debInfo);
    void from_json(const json &j, VersionInfo &version);
    void from_json(const json &j, ModuleInfo &module);
    void from_json(const json &j, IdInfo &id);
    void from_json(const json &j, KindInfo &kind);
    void from_json(const json &j, ArchInfo &arch);
    void from_json(const json &j, ChannelInfo &channel);
    void from_json(const json &j, SummaryInfo &summary);

    bool isCacheValid(const std::string& filePath, time_t currentMtime);
    void updateFileCache(const std::string& filePath, time_t mtime);
    void restoreFromCache(const std::string& filePath);
    void clearFileCache(const std::string& filePath);

private:
    class Private;
    shared_ptr<Private> d;
};

#endif // KMPARSESUMMARYJSON_H
/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include <fnmatch.h>
#include <algorithm>
#include "KMParseSummaryJson.h"

#include "common/KMLogger.h"
#include "common/KMInstalledAppQuery.h"
#include "common/KMVersion.h"
#include "common/KMFormatter.h"
#include "common/KMUtil.h"

 static inline void addAppidNotFound(std::vector<std::string> &vec, const KMRef &ref)
 {
     std::string entry = ref.id;
     if (!ref.version.empty() && ref.version[0] != '-')
     {
         const std::string module = ref.module.empty() ? KMUtil::kmGetDefaultModule() : ref.module;
         const std::string channel = ref.channel.empty() ? KMUtil::kmGetDefaultChannel() : ref.channel;
         const std::string arch = ref.arch.empty() ? KMUtil::kmGetArch() : ref.arch;
         entry = ref.id + "/" + ref.version + "/" + module + "/" + channel + "/" + arch;
     }
     if (std::find(vec.begin(), vec.end(), entry) == vec.end())
     {
         vec.push_back(entry);
     }
 }

class KMParseSummaryJson::Private
{
public:
    Private();
    ~Private();

    json m_jsonData;
    bool m_cacheValid = false;
    SummaryInfo m_summaryInfo;
    string completeRef;
    std::vector<std::string> allCompleteRefs;
    std::shared_ptr<KMFormatter> m_format;
    bool m_isThereDbusService = true;
    std::string m_notInstallable;
    vector<std::string> m_vecNotFoundAppid;
    std::string m_currentLocale;
    std::unordered_map<std::string, std::string> m_idArch2CompleteRef;
    std::vector<std::pair<std::string, CacheInfo>> m_summaryCache;  // 按插入顺序存储
    std::map<std::string, std::vector<std::string>> m_repoToRefs;
    // DBus 连接缓存，避免重复创建连接
    GDBusConnection* m_dbusConnection = nullptr;
};

KMParseSummaryJson::Private::Private()
{
}

KMParseSummaryJson::Private::~Private()
{
    if (m_dbusConnection)
    {
        g_object_unref(m_dbusConnection);
        m_dbusConnection = nullptr;
    }
}

KMParseSummaryJson::KMParseSummaryJson()
    : d(make_shared<Private>())
{
    d->m_format = shared_ptr<KMFormatter>(new KMFormatter());
}

KMParseSummaryJson::KMParseSummaryJson(const std::string &filePath)
    : d(make_shared<Private>())
{
    std::ifstream inputFile(filePath);

    if (inputFile.is_open())
    {
        std::string jsonContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
        std::string cleanedJson = removeComments(jsonContent); // 移除注释
        std::cout << cleanedJson << std::endl;

        // 使用 nlohmann::json 解析
        d->m_jsonData = json::parse(cleanedJson);

        // d->m_jsonData = json::parse(inputFile); // 解析带注释的 json 文件会报错，需要直接赋值，否则会报错
        inputFile.close();
    }
    else
    {
        throw std::runtime_error("Unable to open json file");
    }
}

KMParseSummaryJson::~KMParseSummaryJson()
{
}

void KMParseSummaryJson::clearCache()
{
    d->m_summaryCache.clear();
    d->m_cacheValid = false;
    d->m_summaryInfo.channelInfo.clear();
    d->m_idArch2CompleteRef.clear();
    d->m_vecNotFoundAppid.clear();
    d->m_notInstallable.clear();
    kmlogger.info("Cleared summary.json cache");
}

std::string KMParseSummaryJson::removeComments(const std::string &jsonString)
{
    std::regex commentPattern(R"(\/\/[^\n]*|\/\*[\s\S]*?\*\/)");
    return std::regex_replace(jsonString, commentPattern, "");
}

KMErrorCodeEnum KMParseSummaryJson::loadFile(const std::string &filePath, bool forceReload)
{
    fs::path p(filePath);
    if (!fs::exists(p)) {
        clearFileCache(filePath);
        return KM_ERROR_LOAD_FILE_FAILED;
    }

    // 检查文件是否修改
    auto ftime = fs::last_write_time(p);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now());
    time_t currentMtime = std::chrono::system_clock::to_time_t(sctp);

    // 如果不强制重载，且缓存有效，则使用缓存
    if (!forceReload && isCacheValid(filePath, currentMtime))
    {
        kmlogger.info("using cached json data from %s", filePath.c_str());
        // 恢复缓存的数据到当前状态
        restoreFromCache(filePath);
        return KM_ERROR_NO;
    }

    std::ifstream inputFile(filePath);
    if (!inputFile.is_open())
    {
        clearFileCache(filePath);
        return KM_ERROR_LOAD_FILE_FAILED;
    }

    d->m_jsonData = json::parse(inputFile);
    inputFile.close();

    // 解析并缓存
    if (parseSummaryJson()) {
        updateFileCache(filePath, currentMtime);
        gernerateKMPackage();
    }

    kmlogger.debug("loadFile success!");
    return KM_ERROR_NO;
}

bool KMParseSummaryJson::findMatchMaxVesion(KMPMRef pmRef, string moduleKey, ModuleInfo moduleData, string &maxVersionStr)
{
    vector<string> versionVector; // 保存符合条件的版本号
    std::string version;

    if (pmRef.ref.isSpecialVersion != true && !pmRef.ref.version.empty() && pmRef.ref.version[0] != '-')
    {
        KMVersion ver = KMVersion::parse(pmRef.ref.version, false);
        version = std::to_string(ver.getMajor());
    }
    else
    {
        version = pmRef.ref.version;
    }

    for (const auto &[versionKey, versionData] : moduleData.versionInfo)
    {
        if (!pmRef.ref.version.empty() && pmRef.ref.version[0] != '-')
        {
            if (pmRef.ref.isForceInstall)
            {
                if (versionKey == pmRef.ref.version)
                {
                    versionVector.push_back(versionKey);
                    break;
                }
            }
            else
            {
                if (versionKey == version )
                {
                    maxVersionStr = version;
                    return true;
                }

                if(KMStringUtils::startsWith(versionKey, version))
                {
                    KMVersion idVer = KMVersion::parse(pmRef.ref.version, false);
                    KMVersion keyVer = KMVersion::parse(versionKey, false);
                    if (keyVer >= idVer)
                    {
                        versionVector.push_back(versionKey);
                    }
                }
            }
        }
        else
        {
            versionVector.push_back(versionKey);
        }
    }

    if (versionVector.size() > 0)
    {
        KMVersion maxVersion = KMVersion::parse(versionVector.at(0), false);
        maxVersionStr = versionVector.at(0);
        for (const auto &version : versionVector)
        {
            KMVersion ver = KMVersion::parse(version, false);

            if (maxVersion < ver)
            {
                maxVersion = ver;
                maxVersionStr = version;
            }
        }

        // maxVersionStr = maxVersion.str();
        return true;
    }
    else
    {
        return false;
    }
}

void KMParseSummaryJson::from_json(const json &j, OnlineDebInfo &debInfo)
{
    if (j.contains("Package"))
    {
        j.at("Package").get_to(debInfo.package);
    }

    if (j.contains("Version"))
    {
        j.at("Version").get_to(debInfo.version);
    }

    if (j.contains("Section"))
    {
        j.at("Section").get_to(debInfo.section);
    }

    if (j.contains("Package-Type"))
    {
        j.at("Package-Type").get_to(debInfo.package_type);
    }

    if (j.contains("Recommends"))
    {
        j.at("Recommends").get_to(debInfo.recommends);
    }

    if (j.contains("Uploaders"))
    {
        j.at("Uploaders").get_to(debInfo.uploaders);
    }

    if (j.contains("Provides"))
    {
        j.at("Provides").get_to(debInfo.provides);
    }

    if (j.contains("Origin"))
    {
        j.at("Origin").get_to(debInfo.origin);
    }

    if (j.contains("Multi-Arch"))
    {
        j.at("Multi-Arch").get_to(debInfo.multi_arch);
    }

    if (j.contains("Build-Depends"))
    {
        j.at("Build-Depends").get_to(debInfo.build_depends);
    }

    if (j.contains("Source"))
    {
        j.at("Source").get_to(debInfo.source);
    }

    if (j.contains("Suggests"))
    {
        j.at("Suggests").get_to(debInfo.suggests);
    }

    if (j.contains("Conflicts"))
    {
        j.at("Conflicts").get_to(debInfo.conflicts);
    }

    if (j.contains("Breaks"))
    {
        j.at("Breaks").get_to(debInfo.breaks);
    }

    if (j.contains("Replaces"))
    {
        j.at("Replaces").get_to(debInfo.replaces);
    }

    if (j.contains("Vcs-Git"))
    {
        j.at("Vcs-Git").get_to(debInfo.vcs_git);
    }

    if (j.contains("Vcs-Browser"))
    {
        j.at("Vcs-Browser").get_to(debInfo.vcs_browser);
    }
}

void KMParseSummaryJson::from_json(const json &j, VersionInfo &version)
{
    if (j.contains("installed-size"))
    {
        j.at("installed-size").get_to(version.installedSize);
    }

    if (j.contains("extensions"))
    {
        j.at("extensions").get_to(version.extensions);
    }

    if (j.contains("base"))
    {
        j.at("base").get_to(version.base);
    }

    if (j.contains("runtime"))
    {
        j.at("runtime").get_to(version.runtime);
    }
    if (j.contains("depends") && j.at("depends").is_array())
    {
        j.at("depends").get_to(version.depend);
    }
    if (j.contains("kylin.debinfo") && j.at("kylin.debinfo").is_object())
    {
        OnlineDebInfo debInfo;
        from_json(j.at("kylin.debinfo"), debInfo);
        version.debInfo["kylin.debinfo"] = debInfo;
    }
}

void KMParseSummaryJson::from_json(const json &j, ModuleInfo &module)
{
    for (auto &item : j.items())
    {
        if (KMStringUtils::startsWith(item.key(), "name"))
        {
            module.nameInfo[item.key()] = item.value();
        }
        else if (KMStringUtils::startsWith(item.key(), "description"))
        {
            module.descriptionInfo[item.key()] = item.value();
        }
        else
        {
            VersionInfo versionData;
            from_json(item.value(), versionData);
            module.versionInfo[item.key()] = versionData;
        }
    }
}

void KMParseSummaryJson::from_json(const json &j, IdInfo &id)
{
    for (auto &module : j.items())
    {
        ModuleInfo moduleData;
        from_json(module.value(), moduleData);
        id.moduleInfo[module.key()] = moduleData;
    }
}

void KMParseSummaryJson::from_json(const json &j, KindInfo &kind)
{
    for (auto &id : j.items())
    {
        IdInfo idData;
        from_json(id.value(), idData);
        kind.idInfo[id.key()] = idData;
    }
}

void KMParseSummaryJson::from_json(const json &j, ArchInfo &arch)
{
    for (auto &kind : j.items())
    {
        KindInfo kindData;
        from_json(kind.value(), kindData);
        arch.kindInfo[kind.key()] = kindData;
    }
}

void KMParseSummaryJson::from_json(const json &j, ChannelInfo &channel)
{
    for (auto &arch : j.items())
    {
        // a.archInfo[arch.key()] = arch.value().get<TypeInfo>(); // 此种方式不行
        ArchInfo archData;
        from_json(arch.value(), archData);
        channel.archInfo[arch.key()] = archData;
    }
}

void KMParseSummaryJson::from_json(const json &j, SummaryInfo &summary)
{
    for (const auto &[channelKey, channelValue] : j.items())
    {
        kmlogger.debug("arch: %s value: %s", channelKey.c_str(), channelValue.dump().c_str());

        ChannelInfo channelData;
        from_json(channelValue, channelData);
        summary.channelInfo[channelKey] = std::move(channelData);
    }
}

void KMParseSummaryJson::printJson()
{
    kmlogger.debug("summary.json: %s", d->m_jsonData.dump(4).c_str());

    // 第一层遍历 Json 数据
    for (auto &item : d->m_jsonData.items())
    {
        std::string valueStr = item.value().dump();
        kmlogger.debug("item: %s value: %s", item.key().c_str(), valueStr.c_str());
    }
}

void KMParseSummaryJson::buildIdArchCache() {
    // 清空旧的缓存
    d->m_idArch2CompleteRef.clear();

    // 遍历 summaryInfo
    for (const auto &[channelName, channelData] : d->m_summaryInfo.channelInfo) {
        for (const auto &[archName, archData] : channelData.archInfo) {
            for (const auto &[kindName, kindData] : archData.kindInfo) {
                // 只处理 kindName == "app"
                if (kindName != "app") continue;
                for (const auto &[idName, idData] : kindData.idInfo) {
                    for (const auto &[moduleName, moduleData] : idData.moduleInfo) {
                        // 收集所有版本号
                        std::vector<std::string> versionVector;
                        for (const auto &[versionStr, versionInfo] : moduleData.versionInfo) {
                            versionVector.push_back(versionStr);
                        }
                        if (versionVector.empty()) continue;

                        // 找到最大版本号的原始字符串
                        std::string maxVersionStr = versionVector.front();
                        KMVersion maxVersion = KMVersion::parse(maxVersionStr, true);
                        for (const auto &version : versionVector) {
                            KMVersion ver = KMVersion::parse(version, true);
                            if (maxVersion < ver) {
                                maxVersion = ver;
                                maxVersionStr = version;
                            }
                        }

                        // 检查最大版本号是否存在
                        auto vinfoIt = moduleData.versionInfo.find(maxVersionStr);
                        if (vinfoIt == moduleData.versionInfo.end()) {
                            kmlogger.warn("maxVersion %s not found in versionInfo for id %s", maxVersionStr.c_str(), idName.c_str());
                            continue;
                        }

                        // 构造 completeRef
                        std::string completeRef = channelName + "/" + archName + "/" +
                                                  kindName + "/" + idName + "/" +
                                                  moduleName + "/" + maxVersionStr + "/" +
                                                  KMUtil::formatInstalledSize(vinfoIt->second.installedSize) + "/" +
                                                  std::to_string(vinfoIt->second.installedSize) + "/";

                        // 拼接 key
                        std::string key = idName + "|" + archName;
                        d->m_idArch2CompleteRef[key] = completeRef;
                    }
                }
            }
        }
    }
}

// void KMParseSummaryJson::buildIdArchCache() {
//     d->m_idArch2CompleteRef.clear();
//     for (const auto &[channelName, channelData] : d->m_summaryInfo.channelInfo) {
//         for (const auto &[archName, archData] : channelData.archInfo) {
//             for (const auto &[kindName, kindData] : archData.kindInfo) {
//                 if (kindName != "app") continue;

//                 for (const auto &[idName, idData] : kindData.idInfo) {
//                     for (const auto &[moduleName, moduleData] : idData.moduleInfo) {
//                         // 计算 maxVersion
//                         std::vector<std::string> versionVector;
//                         for (const auto &[versionStr, versionInfo] : moduleData.versionInfo) {
//                             versionVector.push_back(versionStr);
//                         }
//                         if (versionVector.empty()) continue;
//                         KMVersion maxVersion = KMVersion::parse(versionVector.front(), true);
//                         for (const auto &version : versionVector) {
//                             KMVersion ver = KMVersion::parse(version, true);
//                             if (maxVersion < ver) {
//                                 maxVersion = ver;
//                             }
//                         }
//                         // 检查 maxVersion 是否真的存在
//                          auto vinfoIt = moduleData.versionInfo.find(maxVersion.str());
//                         // if (vinfoIt == moduleData.versionInfo.end()) {
//                         //     kmlogger.warn("maxVersion %s not found in versionInfo for id %s", maxVersion.str().c_str(), idName.c_str());
//                         //     continue;
//                         // }

//                         std::string completeRef = channelName + "/" + archName + "/" +
//                                                   kindName + "/" + idName + "/" +
//                                                   moduleName + "/" + maxVersion.str() + "/" +
//                                                   KMUtil::formatInstalledSize(vinfoIt->second.installedSize) + "/" +
//                                                   std::to_string(vinfoIt->second.installedSize) + "/";

//                         std::string key = idName + "|" + archName;
//                         d->m_idArch2CompleteRef[key] = completeRef;
//                     }
//                 }
//             }
//         }
//     }
// }

bool KMParseSummaryJson::parseSummaryJson() {

    KMUtil localeUtil;
    d->m_currentLocale = localeUtil.getCurrentLocale();

    try {
        from_json(d->m_jsonData, d->m_summaryInfo);
        //buildIdArchCache();
        return true;
    } catch (const std::exception &e) {
        kmlogger.error("Failed to parse summary.json: %s", e.what());
        return false;
    }
}

void KMParseSummaryJson::gernerateKMPackage()
{
    //printJson();
    getPackagesFromSummary();
}

void KMParseSummaryJson::getPackagesFromSummary()
{
    SummaryInfo data = d->m_summaryInfo;
    fs::path packagesPath = "/opt/kaiming/packages";
    std::set<std::string> packageIds; //去重

    std::string localArch = KMUtil::kmGetArch();

    // 打开文件 写入模式，自动创建文件
    std::ofstream outfile(packagesPath);
    for (const auto &[channelKey, channelData] : data.channelInfo)
    {
        kmlogger.debug("channelKey: %s", channelKey.c_str()); // main, stable
        for (const auto &[archKey, archData] : channelData.archInfo)
        {
            kmlogger.debug("archKey: %s", archKey.c_str()); // x86_64, arm64

            if (archKey != localArch) {
                kmlogger.debug("Skipping non-local architecture: %s", archKey.c_str());
                continue;
            }

            for (const auto &[kindKey, kindData] : archData.kindInfo)
            {
                kmlogger.debug("kindKey: %s", kindKey.c_str()); // app, runtime, base, extension
                for (const auto &[idKey, idData] : kindData.idInfo)
                {
                    kmlogger.debug("idKey: %s", idKey.c_str()); // top.openkylin.Music, top.openkylin.base, top.ukui.runtime
                    packageIds.insert(idKey.c_str());
                }
            }
        }
    }

    for (const auto &id : packageIds)
    {
        outfile << id << std::endl;
    }
    outfile.close();
}

void KMParseSummaryJson::printSummaryJson(const SummaryInfo &data)
{
    for (const auto &[channelKey, channelData] : data.channelInfo)
    {
        kmlogger.debug("channelKey: %s", channelKey.c_str()); // main, stable
        for (const auto &[archKey, archData] : channelData.archInfo)
        {
            kmlogger.debug("archKey: %s", archKey.c_str()); // x86_64, arm64
            for (const auto &[kindKey, kindData] : archData.kindInfo)
            {
                kmlogger.debug("kindKey: %s", kindKey.c_str()); // app, runtime, base, extension
                for (const auto &[idKey, idData] : kindData.idInfo)
                {
                    kmlogger.debug("idKey: %s", idKey.c_str()); // top.openkylin.Music, top.openkylin.base, top.ukui.runtime
                    for (const auto &[moduleKey, moduleData] : idData.moduleInfo)
                    {
                        kmlogger.debug("moduleKey: %s", moduleKey.c_str());
                        {
                            for (const auto &[nameKey, nameData] : moduleData.nameInfo)
                            {
                                if (KMStringUtils::startsWith(nameKey, "name"))
                                {
                                    kmlogger.debug("nameKey: %s", nameKey.c_str());
                                }
                            }

                            for (const auto &[descriptionKey, descriptionData] : moduleData.descriptionInfo)
                            {
                                if (KMStringUtils::startsWith(descriptionKey, "name"))
                                {
                                    kmlogger.debug("descriptionKey: %s", descriptionKey.c_str());
                                }
                            }
                        }

                        {
                            for (const auto &[versionKey, versionData] : moduleData.versionInfo)
                            {
                                kmlogger.debug("versionKey: %s", versionKey.c_str()); // 2.0.0.0

                                // 打印每个 version
                                kmlogger.debug("installedSize: %ld base: %s runtime: %s", versionData.installedSize, versionData.base.empty() ? "" : versionData.base.c_str(),
                                               versionData.runtime.empty() ? "" : versionData.runtime.c_str());

                                if (!versionData.extensions.empty())
                                {
                                    kmlogger.debug(" Extension: ");
                                    for (const auto &ext : versionData.extensions)
                                    {
                                        if (!ext.empty())
                                        {
                                            kmlogger.debug("ext: %s", ext.c_str());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

string KMParseSummaryJson::toLowerUtf8(const std::string &str)
{
    wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    wstring wstr = converter.from_bytes(str);
    transform(wstr.begin(), wstr.end(), wstr.begin(),
              [](wchar_t c)
              { return std::towlower(c); });
    return converter.to_bytes(wstr);
}

std::vector<std::string> KMParseSummaryJson::printSearchJson(const string &searchId, const string &specifyArch, const string &specifyKind, const string &specifyModule, bool isPrint, bool isGetDevel) {
    // std::string key = searchId + "|" + specifyArch;
    // auto it = d->m_idArch2CompleteRef.find(key);
    // if (it != d->m_idArch2CompleteRef.end())
    // {
    //     return it->second;
    // }
    d->allCompleteRefs.clear();

    // 初始化最大宽度
    size_t maxIdWidth = 0;
    size_t maxNameWidth = 0;
    size_t maxVersionWidth = 0;

    // 第一次遍历：计算最大宽度
    if (isPrint)
    {
        auto calculateMaxWidths = [&](const KMEntry &entry)
        {
            maxIdWidth = std::max(maxIdWidth, entry.id.length());
            maxNameWidth = std::max(maxNameWidth, entry.name.length());
            maxVersionWidth = std::max(maxVersionWidth, entry.version.length());
        };

        const string searchIdLower = toLowerUtf8(searchId);
        auto matchId = [](const string &pattern, const string &text) -> bool
        {
            if (pattern.find('*') != string::npos)
            {
                return fnmatch(pattern.c_str(), text.c_str(), FNM_NOESCAPE) == 0;
            }
            return text.find(pattern) != string::npos;
        };

        // 仅计算宽度，不打印
        for (const auto &[summaryKey, cacheInfo] : d->m_summaryCache)
        {
            for (const auto &[channelKey, channelData] : cacheInfo.summaryInfo.channelInfo)
            {
                for (const auto &[archName, archData] : channelData.archInfo)
                {
                    if (!specifyArch.empty() && archName != specifyArch)
                        continue;

                    for (const auto &[kindName, kindData] : archData.kindInfo)
                    {
                        if (!specifyKind.empty() && kindName != specifyKind)
                            continue;

                        for (const auto &[idName, idData] : kindData.idInfo)
                        {
                            const string idLower = toLowerUtf8(idName);
                            if (!matchId(searchIdLower, idLower))
                                continue;

                            for (const auto &[moduleName, moduleData] : idData.moduleInfo)
                            {
                                if (!specifyModule.empty() && moduleName != specifyModule)
                                    continue;

                                // 获取本地化名称和描述
                                const string localeNameKey = "name[" + d->m_currentLocale + "]";
                                const string localeDescKey = "description[" + d->m_currentLocale + "]";

                                KMEntry tmpEntry;
                                tmpEntry.id = idName;
                                tmpEntry.name = moduleData.nameInfo.count(localeNameKey) &&
                                                        !moduleData.nameInfo.at(localeNameKey).empty()
                                                    ? moduleData.nameInfo.at(localeNameKey)
                                                    : moduleData.nameInfo.at("name");
                                tmpEntry.version = moduleData.versionInfo.begin()->first; // 取第一个版本计算宽度

                                calculateMaxWidths(tmpEntry);
                            }
                        }
                    }
                }
            }
        }

        // 设置最小宽度（确保不小于列标题）
        const size_t minIdWidth = 10;      // "ID" 标题长度
        const size_t minNameWidth = 20;    // "Name" 标题长度
        const size_t minVersionWidth = 15; // "Version" 标题长度

        maxIdWidth = std::max(maxIdWidth, minIdWidth) + 5;
        maxNameWidth = std::max(maxNameWidth, minNameWidth) + 5;
        maxVersionWidth = std::max(maxVersionWidth, minVersionWidth) + 5;

        if (isPrint)
        {
            if (specifyModule == "devel")
            {
                d->m_format->setShowBase(true);
                d->m_format->setShowRuntime(true);
            }
            d->m_format->setColumnWidth(maxIdWidth, maxNameWidth, maxVersionWidth, 10, 15, 15, 30, 15, 25, 25);
            d->m_format->setShowArch(false);
            d->m_format->setShowChannel(true);
            d->m_format->printHeader();
        }
    }

    // 第二次遍历：实际处理数据并打印
    d->completeRef.clear();
    const string searchIdLower = toLowerUtf8(searchId);
    KMEntry entry;
    vector<string> versionVector;
    versionVector.reserve(10);

    auto matchId = [](const string &pattern, const string &text) -> bool {
        if (pattern.find('*') != string::npos) {
            return fnmatch(pattern.c_str(), text.c_str(), FNM_NOESCAPE) == 0;
        }
        return text.find(pattern) != string::npos;
    };

    // Iterate through cached summary info
    for (const auto &[summaryKey, cacheInfo] : d->m_summaryCache)
    {
        for (const auto &[channelName, channelData] : cacheInfo.summaryInfo.channelInfo)
        {
            entry.channel = channelName;

            for (const auto &[archName, archData] : channelData.archInfo)
            {
                if (!specifyArch.empty() && archName != specifyArch)
                {
                    continue;
                }
                entry.arch = archName;

                for (const auto &[kindName, kindData] : archData.kindInfo)
                {
                    if (!specifyKind.empty() && kindName != specifyKind)
                    {
                        continue;
                    }

                    for (const auto &[idName, idData] : kindData.idInfo)
                    {
                        const string idLower = toLowerUtf8(idName);
                        if (!matchId(searchIdLower, idLower))
                        {
                            continue;
                        }
                        entry.id = idName;

                        for (const auto &[moduleName, moduleData] : idData.moduleInfo)
                        {
                            if (!specifyModule.empty() && moduleName != specifyModule)
                            {
                                continue;
                            }
                            entry.module = moduleName;

                            // Get localized name and description
                            const string localeNameKey = "name[" + d->m_currentLocale + "]";
                            const string localeDescKey = "description[" + d->m_currentLocale + "]";

                            entry.name = moduleData.nameInfo.count(localeNameKey) &&
                                                 !moduleData.nameInfo.at(localeNameKey).empty()
                                             ? moduleData.nameInfo.at(localeNameKey)
                                             : moduleData.nameInfo.at("name");

                            entry.description = moduleData.descriptionInfo.count(localeDescKey) &&
                                                        !moduleData.descriptionInfo.at(localeDescKey).empty()
                                                    ? moduleData.descriptionInfo.at(localeDescKey)
                                                    : moduleData.descriptionInfo.at("description");

                            versionVector.clear();
                            for (const auto &[versionStr, versionData] : moduleData.versionInfo)
                            {
                                entry.version = versionStr;
                                if (searchId == idName && (moduleName == "binary" || isGetDevel))
                                {
                                    versionVector.push_back(entry.version);
                                }

                                entry.size = KMUtil::formatInstalledSize(versionData.installedSize);
                                entry.base = versionData.base;
                                entry.runtime = versionData.runtime;

                                if (searchId.empty() && versionData.debInfo.empty())
                                {
                                    d->completeRef = entry.channel + "/" + entry.arch + "/" +
                                                     kindName + "/" + entry.id + "/" +
                                                     entry.module + "/" + entry.version + "/" +
                                                     entry.size + "/" +
                                                     to_string(moduleData.versionInfo.at(entry.version).installedSize) + "/" + "" + "/" + "" + "/";
                                    d->allCompleteRefs.push_back(d->completeRef);
                                }

                                if (isPrint && versionData.debInfo.empty())
                                {
                                    d->m_format->printEntry(entry);
                                }

                                for (const auto &[debStr, debInfo] : versionData.debInfo)
                                {
                                    entry.deb_package = debInfo.package;
                                    entry.deb_version = debInfo.version;
                                    if (searchId.empty())
                                    {
                                        d->completeRef = entry.channel + "/" + entry.arch + "/" +
                                                        kindName + "/" + entry.id + "/" +
                                                        entry.module + "/" + entry.version + "/" +
                                                        entry.size + "/" +
                                                        to_string(moduleData.versionInfo.at(entry.version).installedSize) + "/" + entry.deb_package + "/" + entry.deb_version + "/";
                                        d->allCompleteRefs.push_back(d->completeRef);
                                    }

                                    if (isPrint)
                                    {
                                        d->m_format->printEntry(entry);
                                    }
                                }
                            }

                            if (!versionVector.empty())
                            {
                                KMVersion maxVersion = KMVersion::parse(versionVector.front(), false);
                                for (const auto &version : versionVector)
                                {
                                    KMVersion ver = KMVersion::parse(version, false);
                                    if (maxVersion < ver)
                                    {
                                        maxVersion = ver;
                                    }
                                }

                                d->completeRef = entry.channel + "/" + entry.arch + "/" +
                                                 kindName + "/" + entry.id + "/" +
                                                 entry.module + "/" + maxVersion.str() + "/" +
                                                 entry.size + "/" +
                                                 to_string(moduleData.versionInfo.at(maxVersion.str()).installedSize) + "/" + entry.deb_package + "/" + entry.deb_version + "/";
                                d->allCompleteRefs.push_back(d->completeRef);
                            }
                        }
                    }
                }
            }
        }
    }

    if (isPrint) {
        d->m_format->printFooter();
    }
    return d->allCompleteRefs;
}

bool KMParseSummaryJson::getSummaryInfo(KMPMRef &pmRef)
{
    string sortSearchRef;
    bool isSearched = false;

    for (const auto &[summaryKey, cacheInfo] : d->m_summaryCache)
    {
        for (const auto &[channelKey, channelData] : cacheInfo.summaryInfo.channelInfo)
        {
            kmlogger.debug("channelKey: %s", channelKey.c_str());
            if (!pmRef.ref.channel.empty() && channelKey != pmRef.ref.channel)
            {
                continue;
            }

            for (const auto &[archKey, archData] : channelData.archInfo)
            {
                kmlogger.debug("archKey: %s", archKey.c_str());
                if (!pmRef.ref.arch.empty() && pmRef.ref.arch != archKey)
                {
                    continue;
                }

                for (const auto &[kindKey, kindData] : archData.kindInfo)
                {
                    kmlogger.debug("kindKey: %s", kindKey.c_str());
                    if (!pmRef.ref.kind.empty() && pmRef.ref.kind != kindKey)
                    {
                        continue;
                    }

                    for (const auto &[idKey, idData] : kindData.idInfo)
                    {
                        kmlogger.debug("idKey: %s", idKey.c_str());
                        if (!pmRef.ref.id.empty() && pmRef.ref.id != idKey)
                        {
                            continue;
                        }

                        for (const auto &[moduleKey, moduleData] : idData.moduleInfo)
                        {
                            kmlogger.debug("moduleKey: %s", moduleKey.c_str());
                            if (!pmRef.ref.module.empty() && pmRef.ref.module != moduleKey)
                            {
                                continue;
                            }

                            // 获取 name 和 description 信息
                            getSummaryNameInfo(pmRef, moduleData);

                            {
                                string maxVersionStr;

                                bool getMaxVersionStrFlag = false;
                                getMaxVersionStrFlag = findMatchMaxVesion(pmRef, moduleKey, moduleData, maxVersionStr);

                                if (!getMaxVersionStrFlag)
                                {
                                    return isSearched;
                                }

                                for (const auto &[versionKey, versionData] : moduleData.versionInfo)
                                {
                                    kmlogger.debug("versionKey: %s", versionKey.c_str());
                                    if (maxVersionStr != versionKey)
                                    {
                                        continue;
                                    }

                                    isSearched = true;

                                    pmRef.installedSize = versionData.installedSize;

                                    // 打印每个module
                                    kmlogger.debug("installedSize: %ld base: %s runtime: %s", versionData.installedSize, versionData.base.empty() ? "" : versionData.base.c_str(),
                                                   versionData.runtime.empty() ? "" : versionData.runtime.c_str());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return isSearched;
}

const SummaryInfo& KMParseSummaryJson::getSummaryInfo() const {
    return d->m_summaryInfo;
}

bool KMParseSummaryJson::getSummaryNameInfo(KMPMRef &pmRef, ModuleInfo moduleInfo)
{
    string locale = KMUtil::getCurrentLocale();
    string nameStr;
    string descriptionStr;

    for (const auto &[nameKey, nameData] : moduleInfo.nameInfo)
    {
        if (KMStringUtils::startsWith(nameKey, "name"))
        {
            kmlogger.debug("nameKey: %s", nameKey.c_str());
            if (nameKey == "name" || locale == "C")
            {
                nameStr = nameData;
            }
            else if (KMStringUtils::contains(nameKey, locale))
            {
                pmRef.name = nameData;
            }
        }
    }

    if (pmRef.name.empty())
    {
        pmRef.name = nameStr;
    }

    for (const auto &[descriptionKey, descriptionData] : moduleInfo.descriptionInfo)
    {
        if (KMStringUtils::startsWith(descriptionKey, "description"))
        {
            kmlogger.debug("descriptionKey: %s", descriptionKey.c_str());
            if (descriptionKey == "description" || locale == "C")
            {
                descriptionStr = descriptionData;
                KMStringUtils::removeTrailingNewline(descriptionStr);
            }
            else if (KMStringUtils::contains(descriptionKey, locale))
            {
                pmRef.description = descriptionData;
                KMStringUtils::removeTrailingNewline(pmRef.description);
            }
        }
    }

    if (pmRef.description.empty())
    {
        pmRef.description = descriptionStr;
    }

    return true;
}

bool KMParseSummaryJson::fuzzyMatchCompletedRef(KMPMRef pmRef, vector<string> &fuzzyMatchRefs)
{
    if (d->m_isThereDbusService)
    {
        const char *dbusmethod = "GetPackageInstallStatus";
        const char *idChar = pmRef.ref.id.c_str();
        const char *dbusInterface = "com.kylin.kysdk.applicationsec.antiinstall";
        int ret = getDbus(dbusInterface, dbusmethod, idChar);
        if (ret == -1)
        {
            d->m_isThereDbusService = false;
        }
        if (ret == 1)
        {
            d->m_notInstallable = pmRef.ref.id;
            return false;
        }
    }

    bool isSearched = false;
    string searchPath;

    auto matchVersionsTemplate = [&](const auto &moduleKey, const auto &moduleData, const string &path, const std::string &remoteName)
    {
        if (pmRef.ref.module.empty())
        {
            if (moduleKey != "binary")
            {
                return false;
            }
        }
        else if (moduleKey != pmRef.ref.module)
        {
            return false;
        }
        return matchVersionData(pmRef, moduleKey, moduleData, path, fuzzyMatchRefs, remoteName);
    };

    for (const auto &[summaryKey, cacheInfo] : d->m_summaryCache)
    {
        std::string remoteName = summaryKey;
        fs::path sk(summaryKey);
        std::string filename = sk.filename().string();
        if (KMStringUtils::startsWith(filename, "summary_") && KMStringUtils::endsWith(filename, ".json"))
        {
            remoteName = filename.substr(8, filename.size() - 8 - 5);
        }

        for (const auto &[channelKey, channelData] : cacheInfo.summaryInfo.channelInfo)
        {
            if (!pmRef.ref.channel.empty() && channelKey != pmRef.ref.channel)
                continue;

            for (const auto &[archKey, archData] : channelData.archInfo)
            {
                if (!pmRef.ref.arch.empty() && pmRef.ref.arch != archKey)
                    continue;

                for (const auto &[kindKey, kindData] : archData.kindInfo)
                {
                    if (!pmRef.ref.kind.empty() && pmRef.ref.kind != kindKey)
                        continue;

                    for (const auto &[idKey, idData] : kindData.idInfo)
                    {
                        if (!pmRef.ref.id.empty() && pmRef.ref.id != idKey)
                            continue;

                        for (const auto &[moduleKey, moduleData] : idData.moduleInfo)
                        {
                            if (pmRef.ref.module.empty())
                            {
                                pmRef.ref.module = moduleKey;
                            }

                            searchPath = channelKey + "/" + archKey + "/" + kindKey + "/" + idKey + "/" + moduleKey + "/";

                            if (matchVersionsTemplate(moduleKey, moduleData, searchPath, remoteName))
                            {
                                isSearched = true;
                                pmRef.ref.kind = kindKey;
                            }
                        }
                    }
                }
            }
        }

        if (isSearched) break;
    }
    if (!isSearched)
    {
        addAppidNotFound(d->m_vecNotFoundAppid, pmRef.ref);
    }

    return isSearched;
}

bool KMParseSummaryJson::matchVersionData(KMPMRef pmRef, string moduleKey, ModuleInfo moduleData, string searchRef, vector<string> &fuzzyMatchRefs, const std::string &remoateName)
{
    bool isSearched = false;
    string tmpSearchRef = searchRef;

    {
        string maxVersionStr;
        bool getMaxVersionStrFlag = false;
        getMaxVersionStrFlag = findMatchMaxVesion(pmRef, moduleKey, moduleData, maxVersionStr);

        if (!getMaxVersionStrFlag)
        {
            addAppidNotFound(d->m_vecNotFoundAppid, pmRef.ref);
            return isSearched;
        }

        for (const auto &[versionKey, versionData] : moduleData.versionInfo)
        {
            searchRef = tmpSearchRef;
            kmlogger.debug("versionKey: %s", versionKey.c_str());
            if (maxVersionStr != versionKey)
            {
                continue;
            }
            string versionStr = searchRef.append(versionKey);

            isSearched = true;

            bool isMatch = false;
            for (const auto &version : fuzzyMatchRefs)
            {
                if (version.find(versionStr) != std::string::npos)
                {
                    isMatch = true;
                    break;
                }
            }

            if (isMatch)
            {
                continue;
            }

            if(!remoateName.empty()) {
                d->m_repoToRefs[remoateName].push_back(versionStr);
            }

            fuzzyMatchRefs.push_back(versionStr);

            // base 获取队列
            if (!versionData.base.empty())
            {
                KMPMRef pmBaseRef = KMUtil::parseSummaryKMPMRef(versionData.base);
                pmBaseRef.ref.kind = BASE_TYPE_BASE;
                pmBaseRef.ref.module = pmRef.ref.module;

                if (!currentFuzzyRefIsInstalled(pmBaseRef))
                {
                    if (!fuzzyMatchCompletedRef(pmBaseRef, fuzzyMatchRefs))
                    {
                        return false;
                    }
                }
            }

            // runtime 获取队列
            if (!versionData.runtime.empty())
            {
                KMPMRef pmRuntimeRef = KMUtil::parseSummaryKMPMRef(versionData.runtime);
                pmRuntimeRef.ref.kind = BASE_TYPE_RUNTIME;
                pmRuntimeRef.ref.module = pmRef.ref.module;

                if (!currentFuzzyRefIsInstalled(pmRuntimeRef))
                {
                    if (!fuzzyMatchCompletedRef(pmRuntimeRef, fuzzyMatchRefs))
                    {
                        return false;
                    }
                }
            }

            // depend 获取队列
            if (!versionData.depend.empty())
            {
                for (std::string dependStr : versionData.depend)
                {
                    KMPMRef pmDependRef;
                    std::vector<std::string> tokens;
                    std::stringstream ss(dependStr);
                    std::string token;
                    while (std::getline(ss, token, '/'))
                    {
                        tokens.push_back(token); // 将切分后的部分添加到 tokens 向量中
                    }
                    if (dependStr.find('/') != std::string::npos)
                    {
                        pmDependRef.ref.arch = pmRef.ref.arch;
                        pmDependRef.ref.id = tokens[0];
                        pmDependRef.ref.version = tokens[1];
                        pmDependRef.ref.module = pmRef.ref.module;
                        pmDependRef.ref.channel = pmRef.ref.channel;
                        if (!currentFuzzyRefIsInstalled(pmDependRef))
                        {
                            if (!fuzzyMatchCompletedRef(pmDependRef, fuzzyMatchRefs))
                            {
                                return false;
                            }
                        }
                    }
                    else
                    {
                        pmDependRef.ref.arch = pmRef.ref.arch;
                        pmDependRef.ref.id = tokens[0];
                        pmDependRef.ref.version = "-";
                        pmDependRef.ref.module = pmRef.ref.module;
                        pmDependRef.ref.channel = pmRef.ref.channel;

                        if (!currentFuzzyRefIsInstalled(pmDependRef))
                        {
                            if (!fuzzyMatchCompletedRef(pmDependRef, fuzzyMatchRefs))
                            {
                                return false;
                            }
                        }
                    }
                }
            }

            // 打印每个module
            kmlogger.debug("installedSize: %ld base: %s runtime: %s", versionData.installedSize, versionData.base.empty() ? "" : versionData.base.c_str(),
                           versionData.runtime.empty() ? "" : versionData.runtime.c_str());

            // extension 获取队列
            if (!versionData.extensions.empty())
            {
                kmlogger.debug(" Extension: ");
                for (const auto &ext : versionData.extensions)
                {
                    if (!ext.empty())
                    {
                        kmlogger.debug("ext: %s", ext.c_str());
                        KMPMRef pmExtensionRef = KMUtil::parseSummaryKMPMRef(ext);
                        pmExtensionRef.ref.kind = BASE_TYPE_EXTENSION;

                        if (!fuzzyMatchCompletedRef(pmExtensionRef, fuzzyMatchRefs))
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }

    if (isSearched == false)
    {
        addAppidNotFound(d->m_vecNotFoundAppid, pmRef.ref);
    }

    return isSearched;
}

bool KMParseSummaryJson::currentFuzzyRefIsInstalled(KMPMRef &pmRef)
{
    KMInstalledAppQuery query;
    bool isInstalled = false;

    string kindStr = pmRef.ref.kind.empty() ? "" : pmRef.ref.kind;
    string versionStr = "";
    string channelStr = pmRef.ref.channel.empty() ? "" : pmRef.ref.channel;
    string moduleStr = pmRef.ref.module.empty() ? "binary" : pmRef.ref.module;

    std::vector<KMRef> queryVector = query.kind(kindStr).id(pmRef.ref.id).channel(channelStr).version(versionStr).module(moduleStr).query(KMInstalledAppQuery::All);

    KMVersion installedVersion;
    KMVersion currentVersion;

    if (!queryVector.empty())
    {
        if (pmRef.ref.version[0] == '-')
        {
            isInstalled = true;
            return isInstalled;
        }
        currentVersion = KMVersion::parse(pmRef.ref.version, false);

        // 指定版本，则检查是否已安装
        for (const auto &ref : queryVector)
        {
            installedVersion = KMVersion::parse(ref.version, false);
            if (ref.kind == "base" || ref.kind == "runtime" || ref.kind == "depend")
            {
                if (installedVersion.getMajor() == currentVersion.getMajor())
                {
                    if (installedVersion >= currentVersion)
                    {
                        isInstalled = true;
                        break;
                    }
                }
            }
        }
    }
    return isInstalled;
}

int KMParseSummaryJson::getDbus(const char *interfaceName, const char *methodName, const char *package)
{
    GError *error = NULL;

    // 1. 获取或创建 DBus 连接
    // 复用 DBus 连接，避免重复创建
    if (!d->m_dbusConnection)
    {
        d->m_dbusConnection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
        if (!d->m_dbusConnection)
        {
            kmlogger.error("D-Bus connection failed: %s", error->message);
            g_clear_error(&error);
            return -1;
        }
    }

    // 2. 直接调用目标方法
    GVariant *params = g_variant_new("(s)", package);
    GVariant *result = g_dbus_connection_call_sync(
        d->m_dbusConnection,
        "com.kylin.kysdk.applicationsec",
        "/com/kylin/kysdk/applicationsec",
        interfaceName,
        methodName,
        params,
        G_VARIANT_TYPE("(i)"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error);

    // 3. 处理调用结果
    int ret = 1; // 默认禁止安装
    
    if (error)
    {
        kmlogger.error("D-Bus call '%s.%s' failed: %s", interfaceName, methodName, error->message);
        g_clear_error(&error);
        ret = -1; // 调用失败返回-1

        if (d->m_dbusConnection)
        {
            g_object_unref(d->m_dbusConnection);
            d->m_dbusConnection = nullptr;
        }
    }
    else if (!result)
    {
        kmlogger.error("D-Bus call returned NULL");
        ret = -1;
    }
    else
    {
        gint32 response_value;
        g_variant_get(result, "(i)", &response_value);
        kmlogger.debug("D-Bus call returned: %d", response_value);
        ret = response_value;
        g_variant_unref(result);
    }

    return ret;
}

std::string KMParseSummaryJson::getNotInstallable()
{
    return d->m_notInstallable;
}

vector<std::string> KMParseSummaryJson::getNotFoundAppIds()
{
    return d->m_vecNotFoundAppid;
}

bool KMParseSummaryJson::clearNotInstallable()
{
    d->m_vecNotFoundAppid.clear();
    d->m_notInstallable = "";
    return true;
}

bool KMParseSummaryJson::isCacheValid(const std::string& filePath, time_t currentMtime) {
    for (const auto& [key, cache] : d->m_summaryCache) {
        if (key == filePath) {
            return cache.isValid && cache.lastModified == currentMtime;
        }
    }
    return false;
}

void KMParseSummaryJson::updateFileCache(const std::string& filePath, time_t mtime) {
    for (auto& [key, cache] : d->m_summaryCache) {
        if (key == filePath) {
            cache.filePath = filePath;
            cache.lastModified = mtime;
            cache.isValid = true;
            cache.summaryInfo = d->m_summaryInfo;
            d->m_cacheValid = true;
            return;
        }
    }
    // 不存在则按插入顺序添加到末尾
    CacheInfo cache;
    cache.filePath = filePath;
    cache.lastModified = mtime;
    cache.isValid = true;
    cache.summaryInfo = d->m_summaryInfo;
    d->m_summaryCache.emplace_back(filePath, cache);
    d->m_cacheValid = true;
}

void KMParseSummaryJson::restoreFromCache(const std::string& filePath) {
    for (const auto& [key, cache] : d->m_summaryCache) {
        if (key == filePath) {
            d->m_summaryInfo = cache.summaryInfo;
            d->m_cacheValid = true;
            return;
        }
    }
}

void KMParseSummaryJson::clearFileCache(const std::string& filePath) {
    for (auto it = d->m_summaryCache.begin(); it != d->m_summaryCache.end(); ++it) {
        if (it->first == filePath) {
            d->m_summaryCache.erase(it);
            return;
        }
    }
}

std::map<std::string, std::vector<std::string>> KMParseSummaryJson::getRepoToRefs() const {
    return d->m_repoToRefs;
}

void KMParseSummaryJson::clearRepoToRefs() {
    d->m_repoToRefs.clear();
}

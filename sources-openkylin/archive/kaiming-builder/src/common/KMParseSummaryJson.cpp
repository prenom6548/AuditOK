/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include <fnmatch.h>
#include "KMParseSummaryJson.h"

#include "KMLogger.h"

class KMParseSummaryJson::Private {
public:
    Private();
    ~Private();

    json m_jsonData;
    SummaryInfo m_summaryInfo;
    string completeRef;
};

KMParseSummaryJson::Private::Private()
{


}

KMParseSummaryJson::Private::~Private()
{
    
}

KMParseSummaryJson::KMParseSummaryJson()
    : d(make_shared<Private>())
{

}

KMParseSummaryJson::KMParseSummaryJson(const std::string &filePath)
    : d(make_shared<Private>())
{
    std::ifstream inputFile(filePath);

    if (inputFile.is_open())
    {
        // std::string jsonContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
        // std::string cleanedJson = removeComments(jsonContent); // 移除注释
        // std::cout << cleanedJson << std::endl;

        // 使用 nlohmann::json 解析
        d->m_jsonData =  json::parse(inputFile);

        // d->m_jsonData = json::parse(filePath); // 解析带注释的 json 文件会报错，需要直接赋值，否则会报错
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

std::string KMParseSummaryJson::removeComments(const std::string &jsonString)
{
    std::regex commentPattern(R"(\/\/[^\n]*|\/\*[\s\S]*?\*\/)");
    return std::regex_replace(jsonString, commentPattern, "");
}

bool KMParseSummaryJson::loadFile(const std::string &filePath)
{
    std::ifstream inputFile(filePath);

    if (inputFile.is_open())
    {
        d->m_jsonData = json::parse(inputFile); // 需要直接赋值，否则会报错
        inputFile.close();
        return true;
    }
    else
    {
        return false;
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

}

void KMParseSummaryJson::from_json(const json &j, ModuleInfo &module)
{
    for (auto &item : j.items())
    {
        if (KMBuilderUtils::startsWith(item.key(), "name"))
        {
            module.nameInfo[item.key()] = item.value();
        }
        else if (KMBuilderUtils::startsWith(item.key(), "description"))
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
    for (auto &channel : j.items())
    {
        ChannelInfo channelData;
        from_json(channel.value(), channelData);
        summary.channelInfo[channel.key()] = channelData;

    }
}

void KMParseSummaryJson::parseSummaryJson()
{
    from_json(d->m_jsonData, d->m_summaryInfo);
}

std::vector<KMRef> KMParseSummaryJson::search(const std::string &specifyArch, const std::string &specifyKind, const std::string &specifyModule, const std::string &searchId , const std::string &specifyVersion, const std::string &base, const std::string &runtime)
{
    std::vector<KMRef> refs;

    KMRef ref;
    for (auto &channel: d->m_jsonData.items())
    {
        ref.m_channel = channel.key();
        for (auto &arch: channel.value().items())
        {
            ref.m_arch = arch.key();
            if (!specifyArch.empty() && ref.m_arch != specifyArch)
            {
                continue;
            }
            for (auto &kind: arch.value().items())
            {
                ref.m_kind = kind.key();
                if (!specifyKind.empty() && ref.m_kind != specifyKind )
                {
                    continue;
                }
                for (auto &id: kind.value().items())
                {
                    ref.m_id = id.key();
                    if (!KMBuilderUtils::matchID(KMBuilderUtils::toLowerUtf8(ref.m_id), KMBuilderUtils::toLowerUtf8(searchId)))
                    {
                        continue;
                    }
                    for (auto &model: id.value().items())
                    {
                        ref.m_module = model.key();
                        if (!specifyModule.empty() && ref.m_module != specifyModule)
                        {
                            continue;
                        }
                        if (model.value().contains("name"))
                        {
                            ref.m_name = model.value()["name"];
                        }
                        if (model.value().contains("description"))
                        {
                            ref.m_description = model.value()["description"];
                        }
                        for (auto &version: model.value().items())
                        {
                            if (!KMBuilderUtils::startsWith(version.key(), "name") && !KMBuilderUtils::startsWith(version.key() ,"description"))
                            {
                                if (!specifyVersion.empty() && !KMBuilderUtils::matchVersion(version.key(), specifyVersion))
                                {
                                    continue;
                                }
                                ref.m_version = version.key();
                                if (version.value().contains("base"))
                                {
                                    ref.m_base = version.value()["base"];
                                }
                                if (version.value().contains("runtime"))
                                {
                                    ref.m_runtime = version.value()["runtime"];
                                }
                                if (KMBuilderUtils::checkEnvironment(ref.m_base, base) && KMBuilderUtils::checkEnvironment(ref.m_runtime, runtime))
                                {
                                    ref.m_repo = "kaiming-repo";
                                    refs.push_back(ref);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return refs;
    
}

bool KMParseSummaryJson::regexVersion(const std::string &version, bool strict)
{
    const std::string versionPattern = "^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)"  // 必须包含四个数字部分
                                       "(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))"    // -先行版本号必须存在 ?:非捕获组，匹配但在结果中不保存
                                       "?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$";

    const std::string versionPatternThree = "^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)"    // 必须包含三个数字部分
                                            "(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))" // -先行版本号可选
                                            "?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$";   // 可选是否+

    const std::string looseVersionPattern = "^v?(0|[1-9]\\d*)(?:\\.(0|[1-9]\\d*))?(?:\\.(0|[1-9]\\d*))?(?:\\.(0|[1-9]\\d*))"    // 可选前缀v，可以包含1到4个数字部分
                                            "?(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))"  // -先行版本号可选
                                            "?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$";
                                            
    const std::string looseVersionPatternThree = "^v?(0|[1-9]\\d*)(?:\\.(0|[1-9]\\d*))?(?:\\.(0|[1-9]\\d*))"    // 可选前缀v，可以包含1到3个数字部分
                                                 "?(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))"
                                                 "?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$";

    std::regex regex(strict ? versionPattern : looseVersionPattern);
    std::regex regexThree(strict ? versionPatternThree : looseVersionPatternThree);
    std::cmatch match;
    if (!std::regex_match(version.c_str(), match, regex))
    {
        if (!std::regex_match(version.c_str(), match, regexThree))
        {
            return false;
        }
    }
    return true;
}

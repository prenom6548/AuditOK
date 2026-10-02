/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMInstalledAppQuery.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <iterator>

#include "KMSemVer.h"
#include "../KMBuilderUtils.h"

namespace fs = std::filesystem;

KMInstalledAppQuery::KMInstalledAppQuery()
{
    m_condition = {
        .id = "",
        .channel = "",
        .version = "",
        .arch = "",
        .module = "",
        .kind = "",
        .baseInstallDir = ""
    };
}

KMInstalledAppQuery::~KMInstalledAppQuery()
{}

std::string KMInstalledAppQuery::getUserInstallBaseDir()
{
    const char* homeDir = std::getenv("HOME");
    return std::string(homeDir) + "/.local/share/kaiming/layers";
}

std::string KMInstalledAppQuery::getSystemInstallBaseDir()
{
    return "/opt/kaiming/layers";
}

/**
 * @brief : 解析base/runtime等的pref
 * @param : [in] pref, 格式如：top.openkylin.base/stable/2.0.0
 * @return: std::optional<Ref>
 */
std::optional<Ref> KMInstalledAppQuery::parsePref(const std::string& pref)
{
    std::optional<Ref> ref;

    std::regex reg{ "[:/]" };
    std::vector<std::string> splits{ std::sregex_token_iterator(pref.begin(), pref.end(), reg, -1), std::sregex_token_iterator() };

    if (splits.size() != 4)
    {
        return ref;
    }

    ref = {
        .id = splits.at(1),
        .channel = splits.at(0),
        .version = splits.at(2),
        .arch = splits.at(3),
        .module = "",
        .kind = "",
        .baseInstallDir = ""
    };

    return ref;
}

/**
 * @brief 将后面的std::vector中的元素追加到前面std::vector中
 * @param src追加元素后的std::vector
 * @param dest待追加的std::vector
 */
void KMInstalledAppQuery::append(std::vector<Ref> &src, std::vector<Ref> &&dest)
{
    for (auto &item : dest)
    {
        src.push_back(item);
    }
}

std::vector<Ref> KMInstalledAppQuery::query(Position pos)
{
    Ref condition = m_condition;
    if (pos == Position::User)
    {
        std::string userDir = KMInstalledAppQuery::getUserInstallBaseDir();
        condition.baseInstallDir = userDir;
        return collect(userDir, condition);
    }
    else
    {
        std::string systemDir = KMInstalledAppQuery::getSystemInstallBaseDir();
        condition.baseInstallDir = systemDir;
        return collect(systemDir, condition);
    }
}

/**
 * childrenDir:
 * @brief : 列出指定目录的子目录名列表
 * @param : [in] baseDir, 查找的目标目录
 * @return: std::vector<std::string> 子目录名列表
 */
std::vector<std::string> KMInstalledAppQuery::childrenDir(const std::string &baseDir)
{
    std::vector<std::string> subdirs;

    fs::path basePath(baseDir);
    if (!fs::exists(basePath))
    {
        return subdirs;
    }

    for (auto const &dirEntry : fs::directory_iterator{ basePath })
    {
        fs::path p = dirEntry.path();
        if (fs::is_directory(p))
        {
            if (p.filename().string() == "." || p.filename().string() == "..")
            {
                continue;
            }
            subdirs.push_back(p.filename());
        }
    }

    return subdirs;
}

/**
 * orderVersions:
 * @brief : 根据版本号规则进行降序排序
 * @param : [in] versions, 版本号列表
 * @return: std::vector<std::string> 降序排序后的版本号列表
 */
std::vector<std::string> KMInstalledAppQuery::orderVersions(const std::vector<std::string>& versions)
{
    if (versions.size() == 1)
    {
        return versions;
    }

    //版本号排序
    KMSemVer tmpParse;
    std::vector<KMSemVer> versionsParses;
    for(auto &version:versions)
    {
        tmpParse = KMSemVer::parse(version);
        versionsParses.push_back(tmpParse);
    }
    //降序排序
    std::sort(versionsParses.begin(),versionsParses.end());  
    std::reverse(versionsParses.begin(),versionsParses.end());

    std::vector<std::string> orderedVersions;
    for(auto &version: versionsParses)
    {
        orderedVersions.push_back(version.version());
    }
    return orderedVersions;
}

/**
 * matchVersions:
 * @brief : 查找版本号列表versions中与版本version模糊匹配的版本列表，版本号列表versions中精确包含版本version时，不需要调用此函数
 * @param : [in] versions, 版本号列表
 * @param : [in] version, 精确版本号或前缀版本号
 * @return: std::vector<std::string> 降序排序的匹配的版本列表；
 * @note  : 进行匹配前，先查看下是否有精确匹配，有则不需要调用此函数
 */
std::vector<std::string> KMInstalledAppQuery::matchVersions(const std::vector<std::string>& versions, const std::string& version)
{
    std::vector<std::string> matchs;
    if (KMBuilderUtils::contains(versions, version))
    {
        // 精确匹配的只返回精确版本号
        matchs.push_back(version);
        return matchs;
    }

    std::vector<std::string> orderedVersions = orderVersions(versions);

    if (version.empty())
    {
        return orderedVersions;
    }

    KMSemVer versionCondition = KMSemVer::parse(version);
    KMSemVer tmp;
    for (auto& ver : orderedVersions)
    {
        tmp = KMSemVer::parse(ver);
        if (tmp.compatible(versionCondition))
        {
            matchs.push_back(ver);
        }
    }

    return matchs;
}

/**
 * collect:
 * @brief : 根据设置的条件查询已安装的开明包(包括base\runtime\app)列表
 * @param : [in] baseInstallDir, 安装目录：系统级默认安装路径或用户级默认安装路径
 * @param : [in] condition, 筛选条件
 * @return: std::vector<std::string> 降序排序后的版本号列表
 */
std::vector<Ref> KMInstalledAppQuery::collect(const std::string& baseInstallDir, const Ref &condition)
{
    std::vector<Ref> result;
    KMInstalledAppQuery query;
    Ref ref = condition;

    // 开明包系统级安装目录，形如：          /opt/kaiming/layers/${channel}/${arch}/${kind}/${id}/{module}/${version}
    // 开明包用户级安装目录，形如：~/.local/share/kaiming/layers/${channel}/${arch}/${kind}/${id}/{module}/${version}
    std::string baseDir = baseInstallDir;
    {   // channel
        if (!fs::exists(baseDir))
        {
            return result;
        }

        if (condition.channel.empty())
        {
            std::vector<std::string> channels = childrenDir(baseDir);

            for (auto& channel : channels)
            {
                ref.channel = channel;
                KMInstalledAppQuery::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }
    
    baseDir = KMBuilderUtils::buildFilename(baseDir, condition.channel);
    {   // arch
        if (!fs::exists(baseDir))
        {
            return result;
        }

        if (condition.arch.empty())
        {
            std::vector<std::string> archs = childrenDir(baseDir);

            for (auto& arch : archs)
            {
                ref.arch = arch;
                KMInstalledAppQuery::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = KMBuilderUtils::buildFilename(baseDir, condition.arch);
    {   // kind
        if (!fs::exists(baseDir))
        {
            return result;
        }

        if (condition.kind.empty())
        {
            std::vector<std::string> kinds = childrenDir(baseDir);
            for (auto& kind : kinds)
            {
                ref.kind = kind;
                KMInstalledAppQuery::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }
    
    baseDir = KMBuilderUtils::buildFilename(baseDir, condition.kind);
    {   // id
        if (!fs::exists(baseDir))
        {
            return result;
        }

        if (condition.id.empty())
        {
            std::vector<std::string> ids = childrenDir(baseDir);

            for (auto& id : ids)
            {
                ref.id = id;
                KMInstalledAppQuery::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = KMBuilderUtils::buildFilename(baseDir, condition.id);
    {   // module
        if (!fs::exists(baseDir))
        {
            return result;
        }

        if (condition.module.empty())
        {
            std::vector<std::string> modules = childrenDir(baseDir);

            for (auto& module : modules)
            {
                ref.module = module;
                KMInstalledAppQuery::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = KMBuilderUtils::buildFilename(baseDir, condition.module);
    {   // version
        if (!fs::exists(baseDir))
        {
            return result;
        }

        std::vector<std::string> versions = childrenDir(baseDir);
        if (versions.empty())
        {
            return result;
        }

        if (!condition.version.empty() && KMBuilderUtils::contains(versions, condition.version))
        {
            // do nothing
        }
        else
        {
            std::vector<std::string> matchs;
            if (condition.version.empty())
            {
                matchs = orderVersions(versions);
            }
            else
            {
                matchs = matchVersions(versions, condition.version);
            }

            for (auto& version : matchs)
            {
                ref.version = version;
                KMInstalledAppQuery::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = KMBuilderUtils::buildFilename(baseDir, condition.version);
    {   // deploy
        if (!fs::exists(baseDir))
        {
            return result;
        }
        
        std::string deploy = KMBuilderUtils::buildFilename(baseDir, "deploy");
        if(fs::exists(deploy))
        {
            result.push_back(ref);
        }
    }

    return result;
}


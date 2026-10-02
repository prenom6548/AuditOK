/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMInstalledAppQuery.h"
#include "KMInfoJson.h"
#include "KMVersion.h"
#include "KMStringUtils.h"
#include "KMDirUtils.h"
#include "KMStorageDir.h"
#include "KMConfig.h"

#include <fstream>

namespace fs = std::filesystem;

KMInstalledAppQuery::KMInstalledAppQuery()
{
    m_condition = {
        .id = "",
        .channel = "",
        .version = "",
        .arch = "",
        .module = "",
        .branch = "",
        .kind = "",
        .baseInstallDir = ""
    };
}

KMInstalledAppQuery::~KMInstalledAppQuery()
{}

std::vector<KMRef> KMInstalledAppQuery::query(Position pos)
{
    KMRef condition = m_condition;
    if (pos == Position::All)
    {
        std::string userDir = KMStorageDir::getUserBaseDirLocation();
        condition.baseInstallDir = userDir;
        std::vector<KMRef> userRefs = collect(userDir, condition);

        std::string systemDir = KMStorageDir::getSystemDefaultBaseDirLocation();
        condition.baseInstallDir = systemDir;
        std::vector<KMRef> systemRefs = collect(systemDir, condition);

        std::vector<KMRef> merged;
        int i = 0, j = 0;
        while (i < userRefs.size() && j < systemRefs.size())
        {
            const KMRef& userRef = userRefs[i];
            const KMRef& systemRef = systemRefs[j];
            if (KMVersion::compare(userRef.version, systemRef.version) >= 0)
            {
                merged.push_back(userRefs[i]);
                i++;
            }
            else
            {
                merged.push_back(systemRefs[j]);
                j++;
            }
        }
        // 将剩余的元素添加到merged数组中
        while (i < userRefs.size())
        {
            merged.push_back(userRefs[i]);
            i++;
        }
        while (j < systemRefs.size())
        {
            merged.push_back(systemRefs[j]);
            j++;
        }

        return merged;
    }
    else if (pos == Position::User)
    {
        std::string userDir = KMStorageDir::getUserBaseDirLocation();
        condition.baseInstallDir = userDir;
        return collect(userDir, condition);
    }
    else
    {
        std::string systemDir = KMStorageDir::getSystemDefaultBaseDirLocation();
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
    KMVersion tmpParse;
    std::vector<KMVersion> versionsParses;
    for(auto &version:versions)
    {
        tmpParse = KMVersion::parse(version);
        versionsParses.push_back(tmpParse);
    }
    //降序排序
    std::sort(versionsParses.begin(),versionsParses.end());  
    std::reverse(versionsParses.begin(),versionsParses.end());

    std::vector<std::string> orderedVersions;
    for(auto &version: versionsParses)
    {
        orderedVersions.push_back(version.str());
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
    if (KMStringUtils::contains(versions, version))
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

    KMInfoJson::InfoBase versionStandard , versionChecked;  
    KMVersion versionStandardParse , versionCheckedParse;

    //解析version
    versionStandard.m_baseVersion = version;
    versionStandard.versionParse();
    //version是从yaml文件中读取出来的，非标准格式
    versionStandardParse = KMVersion::parse(version, false);  

    for (auto& ver : orderedVersions)
    {
        //解析ver
        versionChecked.m_baseVersion = ver;
        versionChecked.versionParse();
        versionCheckedParse = KMVersion::parse(ver);
            
        if (versionChecked.checkVersion(versionStandard) && versionCheckedParse >= versionStandardParse)
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
std::vector<KMRef> KMInstalledAppQuery::collect(const std::string& baseInstallDir, const KMRef &condition)
{
    std::vector<KMRef> result;
    KMInstalledAppQuery query;
    KMRef ref = condition;

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
                KMDirUtils::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }
    
    baseDir = KMStringUtils::buildFilename(baseDir, condition.channel);
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
                KMDirUtils::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = KMStringUtils::buildFilename(baseDir, condition.arch);
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
                KMDirUtils::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }
    
    baseDir = KMStringUtils::buildFilename(baseDir, condition.kind);
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
                KMDirUtils::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = KMStringUtils::buildFilename(baseDir, condition.id);
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
                KMDirUtils::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = KMStringUtils::buildFilename(baseDir, condition.module);
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

        if (!condition.version.empty() && KMStringUtils::contains(versions, condition.version))
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
                KMDirUtils::append(result, query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = KMStringUtils::buildFilename(baseDir, condition.version);
    {   // deploy
        if (!fs::exists(baseDir))
        {
            return result;
        }
        
        std::string deploy = KMStringUtils::buildFilename(baseDir, "deploy");
        if(fs::exists(deploy))
        {
            result.push_back(ref);
        }
    }

    return result;
}

std::unordered_map<std::string, std::string> KMInstalledAppQuery::getPackageMappingsFromExec(const std::string &execName)
{
    std::unordered_map<std::string, std::string> execMap;
    const fs::path kaimingPath = KAIMING_SYSTEM_PATH;
    const fs::path infoDir = kaimingPath / "info";

    if (!fs::exists(infoDir))
    {
        return execMap;
    }

    for (const auto &entry : fs::directory_iterator(infoDir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".bin")
        {
            continue;
        }

        std::ifstream ifs(entry.path());
        if (!ifs)
        {
            kmlogger.warn("Failed to open info file: %s", entry.path().string().c_str());
            continue;
        }

        std::string line;
        while (std::getline(ifs, line))
        {
            const size_t spacePos = line.find(' ');
            if (spacePos == std::string::npos)
            {
                continue;
            }

            const std::string_view execPath(line.data(), spacePos);
            const std::string_view pkg(line.data() + spacePos + 1);

            const size_t lastSlash = execPath.rfind('/');
            const std::string_view filename = (lastSlash == std::string_view::npos) ? execPath : execPath.substr(lastSlash + 1);
            if (filename == execName)
            {
                execMap.emplace(execPath, pkg);
            }
        }
    }

    return execMap;
}

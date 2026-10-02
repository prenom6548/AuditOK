/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMInstalledAppQuery.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <algorithm>

#include "common/utils.h"
#include "common/kmversion.h"

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

QString KMInstalledAppQuery::getUserInstallBaseDir()
{
    return QDir::homePath() + "/.local/share/kaiming/layers";
}

QString KMInstalledAppQuery::getSystemInstallBaseDir()
{
    return "/opt/kaiming/layers";
}

/**
 * @brief : 解析base/runtime等的pref
 * @param : [in] pref, 格式如：top.openkylin.base/stable/2.0.0
 * @return: std::optional<Ref>
 */
std::optional<Ref> KMInstalledAppQuery::parsePref(const QString& pref)
{
    std::optional<Ref> ref;

    QStringList splits = pref.split(":");
    QString channel = splits.at(0);
    QString left = splits.at(1);

    QStringList parts = left.split("/");
    QString id = parts.at(0);
    QString version = parts.at(1);

    ref = {
        .id = id,
        .channel = channel,
        .version = version,
        .arch = "",
        .module = "",
        .kind = "",
        .baseInstallDir = ""
    };

    return ref;
}

QVector<Ref> KMInstalledAppQuery::query(Position pos)
{
    Ref condition = m_condition;
    if (pos == Position::User)
    {
        QString userDir = KMInstalledAppQuery::getUserInstallBaseDir();
        condition.baseInstallDir = userDir;
        return collect(userDir, condition);
    }
    else
    {
        QString systemDir = KMInstalledAppQuery::getSystemInstallBaseDir();
        condition.baseInstallDir = systemDir;
        return collect(systemDir, condition);
    }
}

/**
 * childrenDir:
 * @brief : 列出指定目录的子目录名列表
 * @param : [in] baseDir, 查找的目标目录
 * @return: QStringList 子目录名列表
 */
QStringList KMInstalledAppQuery::childrenDir(const QString &baseDir)
{
    QDir dir(baseDir);
    return dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}

/**
 * orderVersions:
 * @brief : 根据版本号规则进行降序排序
 * @param : [in] versions, 版本号列表
 * @return: QStringList 降序排序后的版本号列表
 */
QStringList KMInstalledAppQuery::orderVersions(const QStringList& versions)
{
    if (versions.size() == 1)
    {
        return versions;
    }

    //版本号排序
    KMVersion tmpParse;
    QVector<KMVersion> versionsParses;
    for(auto &version:versions)
    {
        tmpParse = KMVersion::parse(version);
        versionsParses.push_back(tmpParse);
    }
    //降序排序
    std::sort(versionsParses.begin(),versionsParses.end());  
    std::reverse(versionsParses.begin(),versionsParses.end());

    QStringList orderedVersions;
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
 * @return: QStringList 降序排序的匹配的版本列表；
 * @note  : 进行匹配前，先查看下是否有精确匹配，有则不需要调用此函数
 */
QStringList KMInstalledAppQuery::matchVersions(const QStringList& versions, const QString& version)
{
    QStringList matchs;
    if (versions.contains(version))
    {
        // 精确匹配的只返回精确版本号
        matchs.push_back(version);
        return matchs;
    }

    QStringList orderedVersions = orderVersions(versions);

    if (version.isEmpty())
    {
        return orderedVersions;
    }

    KMVersion versionCondition = KMVersion::parse(version);
    KMVersion tmp;
    for (auto& ver : versions)
    {
        tmp = KMVersion::parse(ver);
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
 * @return: QStringList 降序排序后的版本号列表
 */
QVector<Ref> KMInstalledAppQuery::collect(const QString& baseInstallDir, const Ref &condition)
{
    QVector<Ref> result;
    KMInstalledAppQuery query;
    Ref ref = condition;

    // 开明包系统级安装目录，形如：          /opt/kaiming/layers/${channel}/${arch}/${kind}/${id}/{module}/${version}
    // 开明包用户级安装目录，形如：~/.local/share/kaiming/layers/${channel}/${arch}/${kind}/${id}/{module}/${version}
    QString baseDir = baseInstallDir;
    {   // channel
        QFileInfo basePath(baseDir);
        if (!basePath.exists())
        {
            return result;
        }

        if (condition.channel.isEmpty())
        {
            QStringList channels = childrenDir(baseDir);

            for (auto& channel : channels)
            {
                ref.channel = channel;
                result.append(query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = Utils::buildFilename(baseDir, condition.channel);
    {   // arch
        QFileInfo basePath(baseDir);
        if (!basePath.exists())
        {
            return result;
        }

        if (condition.arch.isEmpty())
        {
            QStringList archs = childrenDir(baseDir);

            for (auto& arch : archs)
            {
                ref.arch = arch;
                result.append(query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = Utils::buildFilename(baseDir, condition.arch);
    {   // kind
        QFileInfo basePath(baseDir);
        if (!basePath.exists())
        {
            return result;
        }

        if (condition.kind.isEmpty())
        {
            QStringList kinds = childrenDir(baseDir);
            for (auto& kind : kinds)
            {
                ref.kind = kind;
                result.append(query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = Utils::buildFilename(baseDir, condition.kind);
    {   // id
        QFileInfo basePath(baseDir);
        if (!basePath.exists())
        {
            return result;
        }

        if (condition.id.isEmpty())
        {
            QStringList ids = childrenDir(baseDir);

            for (auto& id : ids)
            {
                ref.id = id;
                result.append(query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = Utils::buildFilename(baseDir, condition.id);
    {   // module
        QFileInfo basePath(baseDir);
        if (!basePath.exists())
        {
            return result;
        }

        if (condition.module.isEmpty())
        {
            QStringList modules = childrenDir(baseDir);

            for (auto& module : modules)
            {
                ref.module = module;
                result.append(query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = Utils::buildFilename(baseDir, condition.module);
    {   // version
        QFileInfo basePath(baseDir);
        if (!basePath.exists())
        {
            return result;
        }

        QStringList versions = childrenDir(baseDir);
        if (versions.empty())
        {
            return result;
        }

        if (!condition.version.isEmpty() && versions.contains(condition.version))
        {
            // do nothing
        }
        else
        {
            QStringList matchs;
            if (condition.version.isEmpty())
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
                result.append(query.collect(baseInstallDir, ref));
            }

            return result;
        }
    }

    baseDir = Utils::buildFilename(baseDir, condition.version);
    {   // deploy
        QFileInfo basePath(baseDir);
        if (!basePath.exists())
        {
            return result;
        }

        QString deploy = Utils::buildFilename(baseDir, "deploy");
        QFileInfo deployPath(baseDir);
        if(deployPath.exists(deploy))
        {
            result.push_back(ref);
        }
    }

    return result;
}

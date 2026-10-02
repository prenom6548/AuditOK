/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "AptUtils.h"

#include <unistd.h>

#include "KMProcessUtils.h"
#include "../KMBuilderUtils.h"

/**
 * @brief : dpkg -l | grep <package>
 * @param : [in], packageName, 包名
 * @return: 已安装的相应包的版本号
 */
std::string Dpkg::list(const std::string& packageName)
{
    std::string version;

    std::string result;
    KMProcessUtils::spawn("bash", { "-c", "dpkg -l | grep " + packageName }, result, true);
    if (!result.empty())
    {
        std::vector<std::string> lines = KMBuilderUtils::splitString(result, "\n");
        for (auto& line: lines)
        {
            if (KMBuilderUtils::startsWith(line, "ii"))
            {
                KMBuilderUtils::replace(line, " ", " ");
                std::string tmp = KMBuilderUtils::removeDuplicateChars(line, ' ');
                std::vector<std::string> fields = KMBuilderUtils::splitString(tmp, " ");
                if (fields.size() > 3)
                {
                    std::vector<std::string> tmps = KMBuilderUtils::splitString(fields.at(1), ":");
                    if (tmps.at(0) != packageName)
                    {
                        continue;
                    }

                    version = fields.at(2);
                }
                break;
            }
        }
    }

    return version;
}

/**
 * @brief : dpkg --compare-versions <v1> <op> <v2>
 * @param : [in], v1, 版本号1
 * @param : [in], op, 比较运算符
 * @param : [in], v2, 版本号2
 * @return: 已安装的相应包的版本号
 */
bool Dpkg::compareVersions(const std::string& v1, const std::string& op, const std::string& v2)
{
    return KMProcessUtils::spawn("dpkg", { "--compare-versions", v1, op, v2 }, false);
}

/**
 * @brief : apt list --installed <package>
 * @param : [in], packageName, 包名
 * @return: 已安装的相应包的版本号
 */
std::string Apt::list(const std::string& packageName)
{
    std::string version;

    std::string result;
    std::vector<std::string> args;
    args.push_back("list");
    args.push_back("--installed");
    args.push_back(packageName);
    KMProcessUtils::spawn("apt", args, result, true);
    if (!result.empty())
    {
        std::vector<std::string> lines = KMBuilderUtils::splitString(result, "\n");
        for (auto& line: lines)
        {
            if (KMBuilderUtils::startsWith(line, packageName + "/"))
            {
                std::vector<std::string> fields = KMBuilderUtils::splitString(line, " ");
                if (fields.size() > 2)
                {
                    version = fields.at(1);
                }
                break;
            }
        }
    }

    return version;
}

/**
 * @brief : apt-get install -y <packages>
 */
bool Apt::install(const std::vector<std::string>& packages)
{
    uid_t uid = ::getuid();

    if (uid > 0)
    {
        std::vector<std::string> args;
        args.push_back("/opt/kaiming-tools/bin/kaiming-builder");
        args.push_back("exec");
        args.push_back("apt-get");
        args.push_back("--");
        args.push_back("install");
        args.push_back("-y");
        for (auto& package : packages)
        {
            args.push_back(package);
        }

        return KMProcessUtils::spawn("pkexec", args, false);
    }
    else
    {
        std::vector<std::string> args;
        args.push_back("install");
        args.push_back("-y");
        for (auto& package : packages)
        {
            args.push_back(package);
        }

        return KMProcessUtils::spawn("apt-get", args, false);
    }
}

/**
 * @brief : apt-cache madison <package>
 * @param : [in], packageName, 包名
 * @return: 已安装的相应包的版本号
 */
std::vector<std::string> AptCache::madison(const std::string& packageName)
{
    std::vector<std::string> versions;

    std::string result;
    KMProcessUtils::spawn("apt-cache", { "madison", packageName }, result, true);
    if (!result.empty())
    {
        std::vector<std::string> lines = KMBuilderUtils::splitString(result, "\n");
        for (auto& line: lines)
        {
            std::vector<std::string> fields = KMBuilderUtils::splitString(line, "|");
            if (fields.size() > 2)
            {
                std::string version = fields.at(1);
                versions.push_back(KMBuilderUtils::trimStr(version));
            }
        }
    }

    return versions;
}

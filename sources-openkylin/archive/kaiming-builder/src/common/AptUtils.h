/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __APTUTILS_H__
#define __APTUTILS_H__

#include <string>
#include <vector>

class Dpkg
{
public:
    /**
     * @brief : dpkg -l | grep <package>
     * @param : [in], packageName, 包名
     * @return: 已安装的相应包的版本号
     */
    static std::string list(const std::string& packageName);

    /**
     * @brief : dpkg --compare-versions <v1> <op> <v2>
     * @param : [in], v1, 版本号1
     * @param : [in], op, 比较运算符
     * @param : [in], v2, 版本号2
     * @return: 已安装的相应包的版本号
     */
    static bool compareVersions(const std::string& v1, const std::string& op, const std::string& v2);
};

class Apt
{
public:
    /**
     * @brief : apt list --installed <package>
     * @param : [in], packageName, 包名
     * @return: 已安装的相应包的版本号
     */
    static std::string list(const std::string& packageName);

    /**
     * @brief : apt-get install -y <packages>
     */
    static bool install(const std::vector<std::string>& packages);
};

class AptCache
{
public:
    /**
     * @brief : apt-cache madison <package>
     * @param : [in], packageName, 包名
     * @return: 已安装的相应包的版本号
     */
    static std::vector<std::string> madison(const std::string& packageName);
};

#endif // !__APTUTILS_H__

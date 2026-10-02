/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMVERSION_H
#define KMVERSION_H

#include <iostream>
#include <ostream>
#include <string>
#include <regex>
#include <utility>
#include <vector>

#include "KMException.h"
#include "../KMBuilderUtils.h"

enum KMMainVersion
{
    major,
    minor,
    patch,
    secure,
    prerelease
};

/**
 * @brief 先行号版本解析
 * 
 */
class KMPrereleasePart
{
public:
    KMPrereleasePart(const std::string &part);

    bool getNumeric() const;
    std::string getValue() const;
    uint64_t getNumericValue() const;
    int compare(const KMPrereleasePart &other) const;

private:
    bool m_numeric = false;
    std::string m_value;
    uint64_t m_numericValue;
};

/**
 * @brief 先行号版本
 * 
 */
class KMPrereleaseDesc
{
public:
    KMPrereleaseDesc(const std::vector<KMPrereleasePart> &parts);
    std::string str();

    bool isEmpty() const;
    std::string identity() const;
    KMPrereleaseDesc increment() const;

    int compare(const KMPrereleaseDesc &other) const;
    bool operator<(const KMPrereleaseDesc &other) const;
    bool operator>(const KMPrereleaseDesc &other) const;
    bool operator==(const KMPrereleaseDesc &other) const;
    bool operator!=(const KMPrereleaseDesc &other) const;

    static KMPrereleaseDesc parse(const std::string &prereleaseParts);
    static KMPrereleaseDesc empty();
    static KMPrereleaseDesc initial();

private:
    std::vector<KMPrereleasePart> m_parts;
    std::string m_prereleaseString;
};


/**
 * @brief 解析 Semver 2.0 标准版本4段式规范
 * 
 */
class KMVersion
{
public:
    KMVersion(uint64_t major = 0, uint64_t minor = 0, uint64_t patch = 0, uint64_t secure = 0, const std::string &prerelease = "", const std::string buildMeta = "");

    uint64_t getMajor();
    uint64_t getMinor();
    uint64_t getPatch();
    uint64_t getSecure();
    std::string getRerelease();
    std::string getBuildMeta();
    bool isPrerelease();

    /**
     * @brief 判断是否是稳定版本
     * 
     * @return true 是稳定版本，如　"1.0.0.0"
     * @return false 不是稳定版本，如　"1.0.0.0-alpha"
     */
    bool isStable();

    /**
     * @brief 将字符串转变成符合规范的字符串
     * 
     * @return std::string 符合规范的字符串
     */
    std::string str();

    /**
     * @brief 返回无后缀的稳定版本
     * 
     * @return KMVersion 无后缀的稳定版本
     */
    KMVersion stableVersion();

    /**
     * @brief 返回主版本号递增的版本
     * 
     * @param prerelease 先行版本号
     * @return KMVersion 主版本号递增的版本
     */
    KMVersion nextMajor(const std::string &prerelease = "");

    /**
     * @brief 返回次版本号递增的版本
     * 
     * @param prerelease 先行版本号
     * @return KMVersion 次版本号递增的版本
     */
    KMVersion nextMinor(const std::string &prerelease = "");

    /**
     * @brief 返回修订版本号递增的版本
     * 
     * @param prerelease 先行版本号
     * @return KMVersion 修订版本号递增的版本
     */
    KMVersion nextPatch(const std::string &prerelease = "");

    /**
     * @brief 返回扩展安全保留版本号递增的版本
     * 
     * @param prerelease 先行版本号
     * @return KMVersion 扩展安全保留版本号递增的版本
     */
    KMVersion nextSecure(const std::string &prerelease = "");

    /**
     * @brief 返回先行版本号递增的版本
     * 
     * @param prerelease 先行版本号
     * @return KMVersion 先行版本号递增的版本
     */
    KMVersion nextPrerelease(const std::string &prerelease = "");

    /**
     * @brief 版本号递增
     * 
     * @param by 
     * @param prerelease 先行版本号
     * @return KMVersion 版本号递增后的版本号
     */
    KMVersion increment(KMMainVersion mainVersion, const std::string &prerelease);

    int compare(const KMVersion &other);
    bool operator<(const KMVersion &other);
    bool operator<=(const KMVersion &other);
    bool operator>(const KMVersion &other);
    bool operator>=(const KMVersion &other);
    bool operator==(const KMVersion &other);
    bool operator!=(const KMVersion &other);

    static int compare(const std::string &v1, const std::string &v2);

    /**
     * @brief 解析版本号字符串
     * 
     * @param versionString 待解析的版本号字符串
     * @param strict 是否是严格模式
     * @return KMVersion 解析后的版本号字符串
     */
    static KMVersion parse(const std::string &versionString, bool strict = true);

    static bool isThreeVersion;

private:
    uint64_t m_major;
    uint64_t m_minor;
    uint64_t m_patch;
    uint64_t m_secure;
    KMPrereleaseDesc m_prerelease;
    std::string m_buildMeta;
};

#endif // KMVERSION_H

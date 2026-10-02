/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMVersion.h"

static uint64_t parseNumericPart(const std::string &versionPart)
{
    return static_cast<uint64_t>(std::stoull(versionPart));
}

KMPrereleasePart::KMPrereleasePart(const std::string &part)
{
    if (part.empty())
    {
        throw KMException("Pre-release identity contains an empty part.");
    }

    if (KMStringUtils::isNumeric(part))
    {
        m_numericValue = parseNumericPart(part);
        m_numeric = true;
    }

    if (!KMStringUtils::isValidPrerelease(part))
    {
        throw KMException("Pre-release part '" + part + "' contains an invalid character.");
    }
    m_value = part;
}

bool KMPrereleasePart::getNumeric() const
{
    return m_numeric;
}

std::string KMPrereleasePart::getValue() const
{
    return m_value;
}

uint64_t KMPrereleasePart::getNumericValue() const
{
    return m_numericValue;
}

int KMPrereleasePart::compare(const KMPrereleasePart &other) const 
{   
    const std::string& str1 = m_value;
    const std::string& str2 = other.m_value;
    
    size_t i = 0, j = 0;
    size_t len1 = str1.size(), len2 = str2.size();
    
    while (i < len1 && j < len2) 
    {
        // 如果当前位置都是数字，提取完整数字进行数值比较
        if (std::isdigit(str1[i]) && std::isdigit(str2[j])) 
        {
            // 提取数字部分
            size_t start_i = i, start_j = j;
            
            while (i < len1 && std::isdigit(str1[i])) i++;
            while (j < len2 && std::isdigit(str2[j])) j++;
            
            std::string num1_str = str1.substr(start_i, i - start_i);
            std::string num2_str = str2.substr(start_j, j - start_j);
            
            // 转换为数值比较（忽略前导零）
            long long num1 = std::stoll(num1_str);
            long long num2 = std::stoll(num2_str);
            
            if (num1 < num2) return -1;
            if (num1 > num2) return 1;
            
            // 数字相等，继续比较后面的字符
            continue;
        }
        
        // 普通字符比较（按ASCII，但注意字母大小写）
        // dpkg实际上不区分大小写，这里我们按区分大小写实现
        if (str1[i] < str2[j]) return -1;
        if (str1[i] > str2[j]) return 1;
        
        i++;
        j++;
    }
    
    // 一个字符串是另一个的前缀
    if (i < len1) return 1;   // str1更长
    if (j < len2) return -1;  // str2更长
    
    return 0;  // 完全相等
}

KMPrereleaseDesc::KMPrereleaseDesc(const std::vector<KMPrereleasePart> &parts)
    : m_parts(parts)
{
    if (parts.empty())
    {
        m_prereleaseString = "";
    }

    for (const auto &part : parts)
    {
        if (!m_prereleaseString.empty())
        {
            m_prereleaseString += ".";
        }

        m_prereleaseString += part.getValue();
    }
}

std::string KMPrereleaseDesc::str()
{
    return m_prereleaseString;
}

bool KMPrereleaseDesc::isEmpty() const 
{
    return m_parts.empty();
}

std::string KMPrereleaseDesc::identity() const
{
    if (isEmpty())
    {
        return "";
    }

    return m_parts.front().getValue();
}

KMPrereleaseDesc KMPrereleaseDesc::increment() const
{
    std::vector<KMPrereleasePart> newparts = (m_parts);
    size_t lastNumericIndex = 0;
    bool lastNumericIndexFound = false;
    for (size_t i = 0; i < newparts.size(); ++i)
    {
        if (newparts[i].getNumeric())
        {
            lastNumericIndex = i;
            lastNumericIndexFound = true;
        }
    }

    if (lastNumericIndexFound)
    {
        KMPrereleasePart last = newparts[lastNumericIndex];
        newparts[lastNumericIndex] = KMPrereleasePart(std::to_string(last.getNumericValue() + 1));
    }
    else
    {
        newparts.emplace_back("0");
    }
    return KMPrereleaseDesc(newparts);
}

int KMPrereleaseDesc::compare(const KMPrereleaseDesc &other) const
{
    auto thisSize = m_parts.size();
    auto otherSize = other.m_parts.size();

    auto count = std::min(thisSize, otherSize);
    for (size_t i = 0; i < count; ++i)
    {
        int cmp = m_parts[i].compare(other.m_parts[i]);
        if (cmp != 0)
        {
            return cmp;
        }
    }
    return (thisSize < otherSize) ? -1 : (thisSize > otherSize);
}

bool KMPrereleaseDesc::operator<(const KMPrereleaseDesc &other) const
{
    return compare(other) == -1;
}

bool KMPrereleaseDesc::operator>(const KMPrereleaseDesc &other)  const
{
    return (other < *this);
}

bool KMPrereleaseDesc::operator==(const KMPrereleaseDesc &other) const
{
    return m_prereleaseString == other.m_prereleaseString;
}

bool KMPrereleaseDesc::operator!=(const KMPrereleaseDesc &other) const
{
    return m_prereleaseString != other.m_prereleaseString;
}

KMPrereleaseDesc KMPrereleaseDesc::parse(const std::string &prereleaseParts)
{
    if (prereleaseParts.empty())
    {
        return empty();
    }

    std::vector<KMPrereleasePart> prereleasePartsVector;
    std::vector<std::string> parts = KMStringUtils::splitString(prereleaseParts, ".");
    for (auto &part : parts)
    {
        prereleasePartsVector.emplace_back(part);
    }
    return KMPrereleaseDesc(prereleasePartsVector);
}

KMPrereleaseDesc KMPrereleaseDesc::empty()
{
    return KMPrereleaseDesc({});
}

KMPrereleaseDesc KMPrereleaseDesc::initial()
{
    return KMPrereleaseDesc::parse("0");
}

KMVersion::KMVersion(uint64_t major, uint64_t minor, uint64_t patch, uint64_t secure, const std::string& majorStr, const std::string& minorStr, const std::string &patchStr, const std::string &secureStr, const std::string &prerelease, const std::string buildMeta)
    : m_major{ major }
    , m_minor{ minor }
    , m_patch{ patch }
    , m_secure{ secure }
    , m_majorStr{majorStr}
    , m_minorStr{ minorStr }
    , m_patchStr{ patchStr }
    , m_secureStr{ secureStr }
    , m_prerelease{ KMPrereleaseDesc::parse(prerelease) }
    , m_buildMeta{ std::move(buildMeta) }
{
}

KMVersion::KMVersion(uint64_t major, uint64_t minor, uint64_t patch, uint64_t secure, const std::string &prerelease, const std::string buildMeta)
    : KMVersion(major, minor, patch, secure, std::to_string(major), std::to_string(minor), std::to_string(patch), std::to_string(secure), prerelease, buildMeta)
{

}

uint64_t KMVersion::getMajor()
{
    return m_major;
}

uint64_t KMVersion::getMinor()
{
    return m_minor;
}

uint64_t KMVersion::getPatch()
{
    return m_patch;
}

uint64_t KMVersion::getSecure()
{
    return m_secure;
}

std::string KMVersion::getRerelease()
{
    return m_prerelease.str();
}

std::string KMVersion::getBuildMeta()
{
    return m_buildMeta;
}

bool KMVersion::isPrerelease()
{
    return !m_prerelease.isEmpty();
}

bool KMVersion::isStable()
{
    return m_major > 0 && m_prerelease.isEmpty();
}

std::string KMVersion::str()
{
    if (m_secureStr.empty())
    {
        std::string result = m_majorStr;

        if (!m_minorStr.empty())
        {
            result += "." + m_minorStr;
        }

        if (!m_patchStr.empty())
        {
            result += "." + m_patchStr;
        }

        if (!m_prerelease.isEmpty())
        {
            result += "-" + m_prerelease.str();
        }

        if ( m_secure!= 0 )
        {   
            if (!m_buildMeta.empty())
            {
                result += "+" + m_buildMeta + ".security." + std::to_string(m_secure);
            }
            else
            {
                result +=  "+security." + std::to_string(m_secure);
            }
        }
        else
        {
            if (!m_buildMeta.empty())
            {
                result += "+" + m_buildMeta;
            }
        }
        return result;
    }
    else
    {
        std::string result = m_majorStr + "." + m_minorStr+ "." + m_patchStr + "." + m_secureStr;
        if (!m_prerelease.isEmpty())
        {
            result += "-" + m_prerelease.str();
        }

        if (!m_buildMeta.empty())
        {
            result += "+" + m_buildMeta;
        }

        return result;
    }
}

KMVersion KMVersion::stableVersion()
{
    if (m_secureStr.empty())
    {
        return KMVersion(m_major, m_minor, m_patch);
    }
    else
    {
        return KMVersion(m_major, m_minor, m_patch, m_secure);
    }
}

KMVersion KMVersion::nextMajor(const std::string &prerelease)
{
    return KMVersion(m_major + 1, 0, 0, 0, prerelease);
}

KMVersion KMVersion::nextMinor(const std::string &prerelease)
{
    return KMVersion(m_major, m_minor + 1, 0, 0, prerelease);
}

KMVersion KMVersion::nextPatch(const std::string &prerelease)
{
    return KMVersion(m_major, m_minor, (!isPrerelease() || !prerelease.empty() ? m_patch + 1 : m_patch), 0, prerelease);
}

KMVersion KMVersion::nextSecure(const std::string &prerelease)
{
    return KMVersion(m_major, m_minor, m_patch, (!isPrerelease() || !prerelease.empty() ? m_secure + 1 : m_secure), prerelease);
}

KMVersion KMVersion::nextPrerelease(const std::string &prerelease)
{
    std::string pre = "0";
    if (!prerelease.empty())
    {
        pre = isPrerelease() && m_prerelease.identity() == prerelease ? m_prerelease.increment().str() : prerelease;
    }
    else if (prerelease.empty() && isPrerelease())
    {
        pre = m_prerelease.increment().str();
    }
    return KMVersion(m_major, m_minor, m_patch, isPrerelease() ? m_secure : m_secure + 1, pre);
}

KMVersion KMVersion::increment(KMMainVersion mainVersion, const std::string &prerelease)
{
    switch (mainVersion)
    {
    case KMMainVersion::major:
        return nextMajor(prerelease);
    case KMMainVersion::minor:
        return nextMinor(prerelease);
    case KMMainVersion::patch:
        return nextPatch(prerelease);
    case KMMainVersion::secure:
        return nextSecure(prerelease);
    case KMMainVersion::prerelease:
        return nextPrerelease(prerelease);
    default:
        throw KMException("Invalid 'main version' parameter in 'increment()' function.");
    }
}

int KMVersion::compare(const KMVersion &other)
{
    if (m_major > other.m_major)
    {
        return 1;
    }

    if (m_major < other.m_major)
    {
        return -1;
    }

    if (m_minor > other.m_minor)
    {
        return 1;
    }

    if (m_minor < other.m_minor)
    {
        return -1;
    }

    if (m_patch > other.m_patch)
    {
        return 1;
    }

    if (m_patch < other.m_patch)
    {
        return -1;
    }

    if (m_secure > other.m_secure)
    {
        return 1;
    }

    if (m_secure < other.m_secure)
    {
        return -1;
    }

    if (!m_prerelease.isEmpty() && other.m_prerelease.isEmpty())
    {
        return -1;
    }

    if (m_prerelease.isEmpty() && !other.m_prerelease.isEmpty())
    {
        return 1;
    }

    if (!m_prerelease.isEmpty() && !other.m_prerelease.isEmpty())
    {
        return m_prerelease.compare(other.m_prerelease);
    }

    return 0;
}

bool KMVersion::operator<(const KMVersion &other)
{
    return compare(other) == -1;
}

bool KMVersion::operator<=(const KMVersion &other)
{
    return compare(other) <= 0;
}

bool KMVersion::operator>(const KMVersion &other)
{
    return compare(other) == 1;
}

bool KMVersion::operator>=(const KMVersion &other)
{
    return compare(other) >= 0;
}

bool KMVersion::operator==(const KMVersion &other)
{
    return compare(other) == 0;
}

bool KMVersion::operator!=(const KMVersion &other)
{
    return compare(other) != 0;
}

int KMVersion::compare(const std::string &v1, const std::string &v2)
{
    KMVersion version1 = KMVersion::parse(v1);
    KMVersion version2 = KMVersion::parse(v2);

    return version1.compare(version2);
}

KMVersion KMVersion::parse(const std::string &versionString, bool strict)
{
    std::string corePart, preRelease, buildMeta;
    bool isThreeVersion = false;
    uint64_t major = 0, minor = 0, patch = 0, secure = 0;
    std::string majorStr, minorStr, patchStr, secureStr;

    // 数字解析
    auto parsePart = [](const std::string &s) -> uint64_t {
        if (s.empty())
            return 0;
        for (char c : s)
        {
            if (!isdigit(c))
            {
                throw KMException("Invalid numeric part: " + s);
            }
        }
        return std::stoull(s);
    };

    // 去掉可选的 v 前缀
    std::string processedString = versionString;
    if (!versionString.empty() && (versionString[0] == 'v' || versionString[0] == 'V'))
    {
        processedString = versionString.substr(1);
    }

    // 先找到 -
    size_t dashPos = processedString.find('-');
    size_t plusPos = processedString.find('+');

    if (plusPos != std::string::npos && (dashPos == std::string::npos || plusPos < dashPos))
    {
        // + 出现在 - 之前，或没有 -
        corePart = processedString.substr(0, plusPos);
        buildMeta = processedString.substr(plusPos + 1);
    }
    else if (dashPos != std::string::npos)
    {
        // - 出现在 + 之前或没有 +
        corePart = processedString.substr(0, dashPos);
        if (plusPos != std::string::npos && plusPos > dashPos)
        {
            preRelease = processedString.substr(dashPos + 1, plusPos - dashPos - 1);
            buildMeta = processedString.substr(plusPos + 1);
        }
        else
        {
            preRelease = processedString.substr(dashPos + 1);
        }
    }
    else if (plusPos != std::string::npos)
    {
        // 只有 +
        corePart = processedString.substr(0, plusPos);
        buildMeta = processedString.substr(plusPos + 1);
    }
    else
    {
        // 纯数字
        corePart = processedString;
    }

    // 解析 corePart
    std::vector<std::string> nums;
    std::string token;
    for (char c : corePart)
    {
        if (c == '.')
        {
            nums.push_back(token);
            token.clear();
        }
        else
        {
            token += c;
        }
    }
    nums.push_back(token);

    auto getOrDefault = [](const std::vector<std::string> &v, size_t i) -> std::string {
        return i < v.size() ? v[i] : "";
    };

    majorStr = getOrDefault(nums, 0);
    minorStr = getOrDefault(nums, 1);
    patchStr = getOrDefault(nums, 2);
    secureStr = getOrDefault(nums, 3);

    try
    {
        if (strict)
        {
            if (nums.size() == 3)
            {
                isThreeVersion = true;
                secureStr = "";
                secure = 0;
            }
            else if (nums.size() == 4)
            {
                isThreeVersion = false;
            }
            else
            {
                throw KMException("Invalid KMVersion: must have 3 or 4 numeric components in strict mode: " + versionString);
            }
        }
        else
        {
            isThreeVersion = (nums.size() <= 3);
            if (isThreeVersion)
            {
                secureStr = "";
                secure = 0;
            }
        }

        major = parsePart(majorStr);
        minor = parsePart(minorStr);
        patch = parsePart(patchStr);
        if (!isThreeVersion)
            secure = parsePart(secureStr);
    }
    catch (std::exception &e)
    {
        throw KMException("Version parse error: " + std::string(e.what()));
    }

    // 从 buildMeta 中尝试提取 .security.X
    if (isThreeVersion && !buildMeta.empty())
    {
        size_t secPos = buildMeta.find("security.");
        if (secPos != std::string::npos)
        {
            size_t dotBefore = (secPos >= 1 && buildMeta[secPos - 1] == '.') ? secPos - 1 : std::string::npos;
            size_t start = (dotBefore != std::string::npos) ? secPos : std::string::npos;
            if (start != std::string::npos)
            {
                std::string tail = buildMeta.substr(start + 9);
                size_t dot = tail.find('.');
                std::string secVal = (dot != std::string::npos) ? tail.substr(0, dot) : tail;
                try
                {
                    secure = parsePart(secVal);
                    secureStr = secVal;
                    buildMeta = buildMeta.substr(0, start - 1);
                }
                catch (...)
                {
                    // 忽略非法
                }
            }
        }
    }

    return KMVersion(major, minor, patch, secure, majorStr, minorStr, patchStr, secureStr, preRelease, buildMeta);
}
/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMVersion.h"

bool KMVersion::isThreeVersion = false;

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

    if (KMBuilderUtils::isNumeric(part))
    {
        if (part.size() > 1 && part[0] == '0')
        {
            throw KMException("Pre-release part '" + part + "' is getNumeric but contains a leading zero.");
        }
        m_numericValue = parseNumericPart(part);
        m_numeric = true;
    }

    if (!KMBuilderUtils::isValidPrerelease(part))
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
    if (m_numeric && !other.m_numeric)
    {
        return -1;
    }

    if (!m_numeric && other.m_numeric)
    {
        return 1;
    }

    if (m_numeric)
    {
        return (m_numericValue < other.m_numericValue) ? -1 : (m_numericValue > other.m_numericValue);
    }
    return (m_value < other.m_value) ? -1 : (m_value > other.m_value);
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
    std::vector<std::string> parts = KMBuilderUtils::splitString(prereleaseParts, ".");
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

KMVersion::KMVersion(uint64_t major, uint64_t minor, uint64_t patch, uint64_t secure, const std::string &prerelease, const std::string buildMeta)
    : m_major{ major }
    , m_minor{ minor }
    , m_patch{ patch }
    , m_secure{ secure }
    , m_prerelease{ KMPrereleaseDesc::parse(prerelease) }
    , m_buildMeta{ std::move(buildMeta) }
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
    if (isThreeVersion)
    {
        std::string result = std::to_string(m_major) + "." + std::to_string(m_minor) + "." + std::to_string(m_patch);

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
        return result;
    }
    else
    {
        std::string result = std::to_string(m_major) + "." + std::to_string(m_minor) + "." + std::to_string(m_patch) + "." + std::to_string(m_secure);

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
    if (isThreeVersion)
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
    uint64_t major;
    uint64_t minor;
    uint64_t patch;
    uint64_t secure;
    std::string prerelease;
    std::string buildMeta;
    isThreeVersion = false;
    if (!std::regex_match(versionString.c_str(), match, regex))
    {
        if (!std::regex_match(versionString.c_str(), match, regexThree))
        {
            throw KMException("Invalid KMVersion, regex match error. : " + versionString);
        }
        else
        {
            isThreeVersion = true;
            secure = 0;
        }
    }

    auto majorMatch = match[1];
    auto minorMatch = match[2];
    auto patchMatch = match[3];

    auto secureMatch = match[4];
    auto prereleaseMatch = match[5];
    auto buildMetaMatch = match[6];


    if (isThreeVersion == true)
    {
        majorMatch = match[1];         // 主版本号
        minorMatch = match[2];         // 次版本号
        patchMatch = match[3];         // 修订版本号
        prereleaseMatch = match[4];    // 先行版本号
        buildMetaMatch = match[5];     // 编译版本号
    }

    if (prereleaseMatch.matched)
    {
        prerelease = prereleaseMatch.str();
    }

    if (buildMetaMatch.matched)
    {
        buildMeta = buildMetaMatch.str();

        if (isThreeVersion == true)
        {
            // 截断
            std::size_t pos = buildMetaMatch.str().find(".security.");
            std::size_t tailSecurity = buildMetaMatch.str().find("security.");
            std::size_t preSecurity = buildMetaMatch.str().find(".security");
            if (pos != std::string::npos) 
            {
                buildMeta = buildMetaMatch.str().substr(0, pos);
                secure =  std::stoul(buildMetaMatch.str().substr(pos + 10));
            }
            else if (tailSecurity != std::string::npos && preSecurity == std::string::npos) 
            {
                buildMeta = "";
                secure =  std::stoul(buildMetaMatch.str().substr(pos + 10));
            }
        }
    }

    try
    {   
        if (isThreeVersion == true)
        {
            if (strict && majorMatch.matched && minorMatch.matched && patchMatch.matched)
            {
                major = parseNumericPart(majorMatch);
                minor = parseNumericPart(minorMatch);
                patch = parseNumericPart(patchMatch);
            }
            else if (!strict && majorMatch.matched)
            {
                major = parseNumericPart(majorMatch);
                minor = minorMatch.matched ? parseNumericPart(minorMatch) : 0;
                patch = patchMatch.matched ? parseNumericPart(patchMatch) : 0;
            }
            else
            {
                throw KMException("Invalid KMVersion: " + versionString);
            }
        }
        else
        {
            if (strict && majorMatch.matched && minorMatch.matched && patchMatch.matched && secureMatch.matched)
            {
                major = parseNumericPart(majorMatch);
                minor = parseNumericPart(minorMatch);
                patch = parseNumericPart(patchMatch);
                secure = parseNumericPart(secureMatch);
            }
            else if (!strict && majorMatch.matched)
            {
                major = parseNumericPart(majorMatch);
                minor = minorMatch.matched ? parseNumericPart(minorMatch) : 0;
                patch = patchMatch.matched ? parseNumericPart(patchMatch) : 0;
                secure = secureMatch.matched ? parseNumericPart(secureMatch) : 0;
            }
            else
            {
                throw KMException("Invalid KMVersion: " + versionString);
            }
        }
        
        return KMVersion(major, minor, patch, secure, prerelease, buildMeta);
    }
    catch (std::exception &exception)
    {
        throw KMException("Version parse error: " + std::string(exception.what()));
    }
}

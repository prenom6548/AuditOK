#include "KMSemVer.h"
#include "../KMBuilderUtils.h"

KMSemVer::KMSemVer(const std::string& version)
{
    m_version = version;
}

KMSemVer KMSemVer::parse(const std::string& version)
{
    KMSemVer v(version);

    bool findCore = false;
    bool findPre = false;
    for(int i = 0; i < version.size(); ++i) {
        char ch = version.at(i);
        if (ch == '+')
        {
            v.m_core = version.substr(0, i);
            std::string left = version.substr(i + 1);
            if (KMBuilderUtils::startsWith(left, "security."))
            {
                v.m_securityUpdate = left;
                break;
            }
            else
            {
                v.m_buildMeta = left;
            }
        }
        else if (ch == '-')
        {
            v.m_core = version.substr(0, i);
            std::string left = version.substr(i + 1);
            for (int j = 0; j < left.size(); j++)
            {
                char c = left.at(j);
                if (c == '+')
                {
                    v.m_preRelease = left.substr(0, j);
                    std::string other = left.substr(j + 1);
                    if (KMBuilderUtils::startsWith(other, "security."))
                    {
                        v.m_securityUpdate = other;
                        break;
                    }
                    else
                    {
                        v.m_buildMeta = other;
                    }
                }
            }

            if (!findPre)
            {
                v.m_preRelease = left;
            }
        }
    }

    if (!findCore)
    {
        v.m_core = version;
    }

    std::vector<std::string> splits = KMBuilderUtils::splitString(v.m_core, ".");
    if (splits.size() > 0)
    {
        v.m_major = splits.at(0);
    }
    else if (splits.size() > 1)
    {
        v.m_minor = splits.at(1);
    }
    else if (splits.size() > 2)
    {
        v.m_patch = splits.at(2);
    }
    else if (splits.size() > 3)
    {
        v.m_extend = splits.at(3);
    }

    return v;
}

static int compareString(const std::string& v1, const std::string& v2)
{
    int shortLen = v1.length() > v2.length() ? v2.length() : v1.length();
    for (int i = 0; i < shortLen; i++)
    {
        if (v1.at(i) == v2.at(i))
        {
            continue;
        }
        else if (v1.at(i) > v2.at(i))
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }

    if (v1.length() > v2.length())
    {
        return 1;
    }
    else if (v1.length() < v2.length())
    {
        return -1;
    }

    return 0;
}

int KMSemVer::compare(const KMSemVer &o)
{
    int ret = compareString(m_major, o.m_major);
    if (ret != 0)
    {
        return ret;
    }

    ret = compareString(m_minor, o.m_minor);
    if (ret != 0)
    {
        return ret;
    }

    ret = compareString(m_patch, o.m_patch);
    if (ret != 0)
    {
        return ret;
    }

    ret = compareString(m_extend, o.m_extend);
    if (ret != 0)
    {
        return ret;
    }

    if (!m_preRelease.empty() && o.m_preRelease.empty())
    {
        return -1;
    }
    else if (m_preRelease.empty() && !o.m_preRelease.empty())
    {
        return 1;
    }

    ret = compareString(m_preRelease, o.m_preRelease);
    if (ret != 0)
    {
        return ret;
    }

    ret = compareString(m_securityUpdate, o.m_securityUpdate);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

bool KMSemVer::operator<(const KMSemVer &other)
{
    return compare(other) == -1;
}

bool KMSemVer::operator<=(const KMSemVer &other)
{
    return compare(other) <= 0;
}

bool KMSemVer::operator>(const KMSemVer &other)
{
    return compare(other) == 1;
}

bool KMSemVer::operator>=(const KMSemVer &other)
{
    return compare(other) >= 0;
}

bool KMSemVer::operator==(const KMSemVer &other)
{
    return compare(other) == 0;
}

bool KMSemVer::operator!=(const KMSemVer &other)
{
    return compare(other) != 0;
}

int KMSemVer::compare(const std::string &v1, const std::string &v2)
{
    KMSemVer version1 = KMSemVer::parse(v1);
    KMSemVer version2 = KMSemVer::parse(v2);

    return version1.compare(version2);
}

bool KMSemVer::compatible(const KMSemVer &o)
{
    return compareString(m_major, o.m_major) == 0 && compare(o) >= 0;
}


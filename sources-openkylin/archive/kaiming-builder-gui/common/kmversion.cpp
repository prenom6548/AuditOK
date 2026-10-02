#include "kmversion.h"

KMVersion::KMVersion(const QString& version)
{
    m_version = version;
}

KMVersion KMVersion::parse(const QString& version)
{
    KMVersion v(version);

    bool findCore = false;
    bool findPre = false;
    for(int i = 0; i < version.size(); ++i) {
        QChar ch = version.at(i);
        if (ch == QChar('+'))
        {
            v.m_core = version.mid(0, i);
            QString left = version.mid(i + 1);
            if (left.startsWith("security."))
            {
                v.m_securityUpdate = left;
                break;
            }
            else
            {
                v.m_buildMeta = left;
            }
        }
        else if (ch == QChar('-'))
        {
            v.m_core = version.mid(0, i);
            QString left = version.mid(i + 1);
            for (int j = 0; j < left.size(); j++)
            {
                QChar c = left.at(j);
                if (c == QChar('+'))
                {
                    v.m_preRelease = left.mid(0, j);
                    QString other = left.mid(j + 1);
                    if (other.startsWith("security."))
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

    QStringList splits = v.m_core.split(".");
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

static int compareString(const QString& v1, const QString& v2)
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

int KMVersion::compare(const KMVersion &o)
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

    if (!m_preRelease.isEmpty() && o.m_preRelease.isEmpty())
    {
        return -1;
    }
    else if (m_preRelease.isEmpty() && !o.m_preRelease.isEmpty())
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

int KMVersion::compare(const QString &v1, const QString &v2)
{
    KMVersion version1 = KMVersion::parse(v1);
    KMVersion version2 = KMVersion::parse(v2);

    return version1.compare(version2);
}

bool KMVersion::compatible(const KMVersion &o)
{
    return compareString(m_major, o.m_major) == 0 && compare(o) >= 0;
}


#ifndef KMSEMVER_H
#define KMSEMVER_H

#include <string>
#include <vector>

class KMSemVer
{
public:
    KMSemVer() = default;
    ~KMSemVer() = default;

    int compare(const KMSemVer &other);
    bool operator<(const KMSemVer &other);
    bool operator<=(const KMSemVer &other);
    bool operator>(const KMSemVer &other);
    bool operator>=(const KMSemVer &other);
    bool operator==(const KMSemVer &other);
    bool operator!=(const KMSemVer &other);

    const std::string& version() { return m_version; }

    bool compatible(const KMSemVer &other);

    static KMSemVer parse(const std::string& version);
    static int compare(const std::string &v1, const std::string &v2);

private:
    KMSemVer(const std::string& version);

private:
    std::string m_major;
    std::string m_minor;
    std::string m_patch;
    std::string m_extend;
    std::string m_preRelease; // 先行部分
    std::string m_buildMeta;  // 构建元数据
    std::string m_securityUpdate; // 安全更新部分

    std::string m_version;
    std::string m_core; // 核心部分
};

#endif // KMSEMVER_H

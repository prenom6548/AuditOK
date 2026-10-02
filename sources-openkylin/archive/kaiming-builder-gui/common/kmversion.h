#ifndef KMVERSION_H
#define KMVERSION_H

#include <QString>
#include <QStringList>

class KMVersion
{
public:
    KMVersion() = default;
    ~KMVersion() = default;

    int compare(const KMVersion &other);
    bool operator<(const KMVersion &other);
    bool operator<=(const KMVersion &other);
    bool operator>(const KMVersion &other);
    bool operator>=(const KMVersion &other);
    bool operator==(const KMVersion &other);
    bool operator!=(const KMVersion &other);

    const QString& version() { return m_version; }

    bool compatible(const KMVersion &other);

    static KMVersion parse(const QString& version);
    static int compare(const QString &v1, const QString &v2);

private:
    KMVersion(const QString& version);

private:
    QString m_major;
    QString m_minor;
    QString m_patch;
    QString m_extend;
    QString m_preRelease; // 先行部分
    QString m_buildMeta;  // 构建元数据
    QString m_securityUpdate; // 安全更新部分

    QString m_version;
    QString m_core; // 核心部分
};

#endif // KMVERSION_H

#ifndef KMREF_H
#define KMREF_H

#include <QString>
#include <QRegularExpression>

class KMRef
{
public:
    bool local = false;
    QString m_id;
    QString m_name;
    QString m_kind;
    QString m_version;
    QString m_channel;
    QString m_module;

    QString m_base;
    QString m_runtime;

public:
    void loadFromInfoJson(QString &path);

    /**
     * @brief 获取格式channel:id/version的base/runtime
     * @return 形式channel:id/version
     */
    QString getStandard();
    /**
     * @brief 获取格式channel:id/version的base/runtime
     * @return 形式channel:id/version
     */
    QString getStardardDepend();

public:
    int m_major = -1;

public:
    /**
     * @brief 获取主版本号与此版本号
     */
    void parseVersion();

    /**
     * @brief 校验版本号
     * @param another
     * @return true:校验合格, false:校验不合格
     */
    bool checkVersion(const KMRef &another);

    /**
     * @brief 解析channel:id/version的base或runtime
     * @param baseStr
     */
    void parseFromInfo(const QString &baseStr);

    bool operator < (const KMRef &ref)
    {
        if (m_channel != ref.m_channel)
        {
            return m_channel < ref.m_channel;
        }
        else if (m_id != ref.m_id)
        {
            return m_id < ref.m_id;
        }
        else
        {
            return m_version < ref.m_version;
        }
    };

    bool operator == (const KMRef &ref) const
    {
        return (m_channel == ref.m_channel) && (m_id == ref.m_id) && (m_version == ref.m_version);
    };




};

#endif // KMREF_H

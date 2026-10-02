/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMINSTALLEDAPPQUERY_H__
#define __KMINSTALLEDAPPQUERY_H__

#include <QString>
#include <QVector>
#include <QMap>
#include <QStringList>
#include <optional>

typedef struct
{
    QString id;
    QString channel;
    QString version;
    QString arch;
    QString module;
    QString kind;
    QString baseInstallDir;
} Ref;

/**
 * @brief : 根据设置的条件查询已安装的开明包(包括base\runtime\app)列表
 * @note  : 使用场景：
 *          1、查询已安装的base或runtime
 *          2、可以查询是否有安装某软件包
 *          3、版本号支持仅输入前面几段，至少1段
 * @template:
 *        KMInstalledAppQuery query;
 *        QVector<Ref> refs = query.kind("app").id("top.kylin.demo").query();
 */
class KMInstalledAppQuery
{
public:
    enum Position
    {
        System,
        User
    };

public:
    KMInstalledAppQuery();
    ~KMInstalledAppQuery();

    KMInstalledAppQuery &ref(const Ref &ref)
    {
        m_condition = ref;
        return *this;
    }

    KMInstalledAppQuery &channel(const QString &channel)
    {
        m_condition.channel = channel;
        return *this;
    }

    KMInstalledAppQuery &arch(const QString &arch)
    {
        m_condition.arch = arch;
        return *this;
    }

    KMInstalledAppQuery &kind(const QString &kind)
    {
        m_condition.kind = kind;
        return *this;
    }

    KMInstalledAppQuery &id(const QString &id)
    {
        m_condition.id = id;
        return *this;
    }

    KMInstalledAppQuery &version(const QString &version)
    {
        m_condition.version = version;
        return *this;
    }

    KMInstalledAppQuery &module(const QString &module)
    {
        m_condition.module = module;
        return *this;
    }

    QVector<Ref> query(Position pos = System);

    static std::optional<Ref> parsePref(const QString& pref);

    /**
     * @brief 用户级默认安装基础路径
     * @return
     */
    static QString getUserInstallBaseDir();

    /**
     * @brief 系统级默认安装基础路径
     * @return
     */
    static QString getSystemInstallBaseDir();

private:

    /**
     * childrenDir:
     * @brief : 列出指定目录的子目录名列表
     * @param : [in] baseDir, 查找的目标目录
     * @return: QStringList 子目录名列表
     */
    static QStringList childrenDir(const QString &baseDir);

    /**
     * orderVersions:
     * @brief : 根据版本号规则进行降序排序
     * @param : [in] versions, 版本号列表
     * @return: QStringList 降序排序后的版本号列表
     */
    static QStringList orderVersions(const QStringList &versions);

    /**
     * matchVersions:
     * @brief : 查找版本号列表versions中与版本version模糊匹配的版本列表，版本号列表versions中精确包含版本version时，不需要调用此函数
     * @param : [in] versions, 版本号列表
     * @param : [in] version, 精确版本号或前缀版本号
     * @return: QStringList 降序排序的匹配的版本列表；
     * @note  : 进行匹配前，先查看下是否有精确匹配，有则不需要调用此函数
     */
    static QStringList matchVersions(const QStringList &versions, const QString &version);

    /**
     * collect:
     * @brief : 根据设置的条件查询已安装的开明包(包括base\runtime\app)列表
     * @param : [in] baseDir, 安装目录：系统级默认安装路径或用户级默认安装路径
     * @param : [in] condition, 筛选条件
     * @return: QStringList 降序排序后的版本号列表
     */
    static QVector<Ref> collect(const QString &baseDir, const Ref &condition);

private:
    Ref m_condition;
    QString m_baseDir;
};

#endif // !__KMINSTALLEDAPPQUERY_H__

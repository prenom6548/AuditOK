/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMINSTALLEDAPPQUERY_H__
#define __KMINSTALLEDAPPQUERY_H__

#include "KMLogger.h"
#include "KMCommonTypePrivate.h"

/**
 * @brief : 根据设置的条件查询已安装的开明包(包括base\runtime\app)列表
 * @note  : 使用场景：
 *          1、可用在kaiming list子命令中，辅助丰富kaiming list功能，可根据条件筛选
 *          2、可以查询是否有安装某软件包
 *          3、版本号支持仅输入前面几段，至少1段
 * @template:
 *        KMInstalledAppQuery query;
 *        std::vector<KMRef> refs = query.kind("app").id("top.kylin.demo").query();
 */
class KMInstalledAppQuery
{
public:
    enum Position
    {
        System,
        User,
        All
    };

public:
    KMInstalledAppQuery();
    ~KMInstalledAppQuery();

    KMInstalledAppQuery &ref(const KMRef &ref)
    {
        m_condition = ref;
        return *this;
    }

    KMInstalledAppQuery &channel(const std::string &channel)
    {
        m_condition.channel = channel;
        return *this;
    }

    KMInstalledAppQuery &arch(const std::string &arch)
    {
        m_condition.arch = arch;
        return *this;
    }

    KMInstalledAppQuery &kind(const std::string &kind)
    {
        m_condition.kind = kind;
        return *this;
    }

    KMInstalledAppQuery &id(const std::string &id)
    {
        m_condition.id = id;
        return *this;
    }

    KMInstalledAppQuery &version(const std::string &version)
    {
        m_condition.version = version;
        return *this;
    }

    KMInstalledAppQuery &module(const std::string &module)
    {
        m_condition.module = module;
        return *this;
    }

    std::vector<KMRef> query(Position pos = System);

    /**
     * @description: 获取可执行文件和对应的包名 for crun
     * @param {string} &execName 可执行文件名
     * @return {std::unordered_map}: 返回可执行文件和对应的包名的映射
     */
    std::unordered_map<std::string, std::string> getPackageMappingsFromExec(const std::string &execName);

private:
    friend class KMTestInstalledAppQuery;
    /**
     * childrenDir:
     * @brief : 列出指定目录的子目录名列表
     * @param : [in] baseDir, 查找的目标目录
     * @return: std::vector<std::string> 子目录名列表
     */
    static std::vector<std::string> childrenDir(const std::string &baseDir);

    /**
     * orderVersions:
     * @brief : 根据版本号规则进行降序排序
     * @param : [in] versions, 版本号列表
     * @return: std::vector<std::string> 降序排序后的版本号列表
     */
    static std::vector<std::string> orderVersions(const std::vector<std::string> &versions);

    /**
     * matchVersions:
     * @brief : 查找版本号列表versions中与版本version模糊匹配的版本列表，版本号列表versions中精确包含版本version时，不需要调用此函数
     * @param : [in] versions, 版本号列表
     * @param : [in] version, 精确版本号或前缀版本号
     * @return: std::vector<std::string> 降序排序的匹配的版本列表；
     * @note  : 进行匹配前，先查看下是否有精确匹配，有则不需要调用此函数
     */
    static std::vector<std::string> matchVersions(const std::vector<std::string> &versions, const std::string &version);

    /**
     * collect:
     * @brief : 根据设置的条件查询已安装的开明包(包括base\runtime\app)列表
     * @param : [in] baseDir, 安装目录：系统级默认安装路径或用户级默认安装路径
     * @param : [in] condition, 筛选条件
     * @return: std::vector<std::string> 降序排序后的版本号列表
     */
    static std::vector<KMRef> collect(const std::string &baseDir, const KMRef &condition);

private:
    KMRef m_condition;
    std::string m_baseDir;
};

#endif // !__KMINSTALLEDAPPQUERY_H__
